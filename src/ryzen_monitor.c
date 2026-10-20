/**
 * Ryzen SMU Userspace Sensor Monitor
 * Copyright (C) 2020-2021
 *    Florian Huehn <hattedsquirrel@gmail.com> (https://hattedsquirrel.net)
 *    Jeffrey Bosboom <jbosboom@jeffreybosboom.com>
 *    Based on work of:
 *    Leonardo Gates <leogatesx9r@protonmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#define _GNU_SOURCE

#include <math.h>
#include <sched.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <poll.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <libsmu.h>
#include "readinfo.h"
#include "pm_tables.h"
#include "metrics.h"
#include "output_ops.h"
#include "controls.h"

#define PROGRAM_VERSION "1.0.6"

smu_obj_t obj;
static struct timespec update_time_s = {1, 0};
static int show_disabled_cores = 0;
static struct display display = { .views = VIEWS_DEFAULT };

//Helper to access the PM Table elements. If an element doesn't exist in the
//current PM Table version, it's pointer is set to 0. This helper returns
//NAN for not available fields.
#define pmta(elem) ((pmt->elem)?(*pmt->elem):NAN)
//Same, but with 0 as return. For summations that should not fail if one value is not present.
#define pmta0(elem) ((pmt->elem)?(*pmt->elem):0)

static bool dump_table = false;
static double refclk_mhz = 0;
static bool init_debug = false;
static const char *export_path = NULL;
static bool export_created = false;

// Create the export FIFO unless it exists already.
static void export_setup(void) {
    struct stat st;

    if (stat(export_path, &st) == 0) {
        if (!S_ISFIFO(st.st_mode)) {
            fprintf(stderr, "%s exists and is not a named pipe.\n", export_path);
            exit(-1);
        }
        return;
    }
    if (mkfifo(export_path, 0644) != 0) {
        perror("Can't create named pipe for export");
        exit(-1);
    }
    export_created = true;
    // A reader that goes away must not kill us.
    signal(SIGPIPE, SIG_IGN);
}

static void export_cleanup(void) {
    if (export_created)
        unlink(export_path);
}

static void emit_sample(const pm_table *pmt, const system_info *sysinfo, const struct output_ops *ops) {
    struct sample_options opts = { .show_disabled_cores = show_disabled_cores, .refclk_mhz = refclk_mhz };
    struct sample sample;
    int fd, saved_stdout = -1;

    compute_sample(pmt, sysinfo, &opts, &sample);

    // Each sample goes to whoever opens the FIFO next; this waits for a reader.
    if (export_path) {
        fd = open(export_path, O_WRONLY);
        if (fd < 0)
            return;
        fflush(stdout);
        saved_stdout = dup(STDOUT_FILENO);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    ops->emit(&sample, &display);

    if (export_path) {
        fflush(stdout);
        clearerr(stdout);
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }
}

// Cores whose per-core values are all zero are disabled. Returns a bit per core slot.
static unsigned int disabled_cores_from_pmt(const pm_table *pmt) {
    unsigned int map = 0;

    for (int i = 0; i < pmt->max_cores; i++) {
        float *fields[] = {
            pmt->CORE_POWER[i], pmt->CORE_VOLTAGE[i], pmt->CORE_FIT[i], pmt->CORE_IDDMAX[i], pmt->CORE_FREQ[i],
            pmt->CORE_FREQEFF[i], pmt->CORE_C0[i], pmt->CORE_CC1[i], pmt->CORE_IRM[i],
        };
        bool any = false, zero = true;

        for (size_t j = 0; j < sizeof(fields) / sizeof(fields[0]); j++) {
            if (!fields[j])
                continue;
            any = true;
            if (*fields[j] != 0)
                zero = false;
        }
        if (any && zero)
            map |= 1u << i;
    }

    return map;
}

static sig_atomic_t stop_requested = 0;
void signal_interrupt(int sig) {
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            if (stop_requested)
                // We got stuck cleaning up and the user sent another signal.
                // Exit now without cleaning up.
                _exit(1);
            stop_requested = 1;
            break;
        default:
            break;
    }
}

static struct termios saved_termios;
static bool terminal_raw = false;

static void terminal_restore(void) {
    if (terminal_raw) {
        tcsetattr(STDIN_FILENO, TCSANOW, &saved_termios);
        terminal_raw = false;
    }
}

// Deliver key presses one at a time and without echo.
static void terminal_setup(void) {
    struct termios t;

    if (tcgetattr(STDIN_FILENO, &saved_termios) != 0)
        return;
    t = saved_termios;
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &t) == 0) {
        terminal_raw = true;
        atexit(terminal_restore);
    }
}

enum { KEY_TIMEOUT = -1, KEY_EOF = -2 };

// Wait for a key press until the deadline (CLOCK_MONOTONIC).
static int wait_for_key(const struct timespec *deadline) {
    for (;;) {
        struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN };
        struct timespec now;
        unsigned char key;
        long timeout_ms;
        int ret;

        clock_gettime(CLOCK_MONOTONIC, &now);
        timeout_ms = (deadline->tv_sec - now.tv_sec) * 1000 + (deadline->tv_nsec - now.tv_nsec) / 1000000;
        if (timeout_ms <= 0)
            return KEY_TIMEOUT;

        ret = poll(&pfd, 1, timeout_ms);
        if (ret < 0 && errno == EINTR && !stop_requested)
            continue;
        if (ret <= 0)
            return KEY_TIMEOUT;
        return read(STDIN_FILENO, &key, 1) == 1 ? key : KEY_EOF;
    }
}

// Apply a key press to what is displayed. Returns false if the user asked to quit.
static bool handle_key(int key) {
    if (key == 'q' || key == 'Q')
        return false;
    if (key == 'c')
        display.compact = !display.compact;
    for (int v = 0; v < VIEW_COUNT; v++)
        if (key == view_defs[v].hotkey)
            display.views ^= 1u << v;
    return true;
}

// Show how the topology and the PM table were determined.
static void print_topology_debug(const system_info *sysinfo, const pm_table *pmt) {
    fprintf(stdout, "Family 0x%X, model 0x%X, codename %s\n", sysinfo->family, sysinfo->model, sysinfo->codename);
    fprintf(stdout, "PM table version 0x%X (reported 0x%X), %u bytes (%u mapped)\n",
            pmt->version, obj.pm_table_version, obj.pm_table_size, pmt->min_size);
    fprintf(stdout, "CCD fuses: 0x%08X 0x%08X\n", sysinfo->ccd_fuses[0], sysinfo->ccd_fuses[1]);
    fprintf(stdout, "Core fuse: 0x%08X, values:", sysinfo->core_fuse);
    for (unsigned int i = 0; i < sysinfo->ccds && i < 8; i++)
        fprintf(stdout, " 0x%08X", sysinfo->core_fuse_values[i]);
    fprintf(stdout, "\n");
    fprintf(stdout, "Disabled cores: 0x%X, from the %s (the PM table says 0x%X)\n",
            sysinfo->core_disable_map, sysinfo->core_map_source, sysinfo->core_disable_map_pmt);
    fprintf(stdout, "Cores: %u, enabled %u; CCDs: %u, CCXs: %u, cores per CCX: %u\n",
            sysinfo->cores, sysinfo->enabled_cores_count, sysinfo->ccds, sysinfo->ccxs, sysinfo->cores_per_ccx);
}

void start_pm_monitor(unsigned int force, int repeating, const struct output_ops* ops) {
    unsigned char *pm_buf;
    pm_table pmt;
    system_info sysinfo;
    bool interactive, have_sample;
    unsigned int version;

    if (!smu_pm_tables_supported(&obj)) {
        fprintf(stderr, "PM Tables are not supported on this platform.\n");
        exit(0);
    }

    pm_buf = calloc(obj.pm_table_size, sizeof(unsigned char));
    if (!pm_buf) {
        fprintf(stderr, "Could not allocate memory for the PM Table.\n");
        exit(0);
    }

    if (force) {
        fprintf(stderr, "Forcing to use PM Table version 0x%x. System reports version 0x%x.\n", force, obj.pm_table_version);
    }

    //Select matching PM Table
    version = force ? force : pm_table_lookup_version(obj.pm_table_version, obj.codename);
    if(!select_pm_table_version(version, &pmt, pm_buf)) {
        unsigned int similar = pm_table_similar_version(version);

        fprintf(stderr, "This PM Table version (0x%x) is currently not supported.\n", version);
        if (similar)
            fprintf(stderr, "Version 0x%x is a revision of the same table; -f 0x%x might work, but the values can be wrong.\n", similar, similar);
        fprintf(stderr, "Processor name: %s\n", get_processor_name());
        fprintf(stderr, "SMU FW version: %s\n", smu_get_fw_version(&obj));
        exit(0);
    }
    //Prevent illegal memory access
    if (obj.pm_table_size < pmt.min_size) {
        fprintf(stderr, "Selected PM Table is larger than the PM Table returned by the SMU.\n");
        exit(0);
    }
    memset(&sysinfo, 0, sizeof(sysinfo));
    //Maximum core count. Just to be safe. Will be overwritten by get_processor_topology(...).
    sysinfo.enabled_cores_count = pmt.max_cores;

    sysinfo.cpu_name    = get_processor_name();
    sysinfo.codename    = smu_codename_to_str(&obj);
    sysinfo.smu_fw_ver  = smu_get_fw_version(&obj);

    // For processors whose core fuses are not known
    if (smu_read_pm_table(&obj, pm_buf, obj.pm_table_size) == SMU_Return_OK)
        sysinfo.core_disable_map_pmt = disabled_cores_from_pmt(&pmt);

    get_processor_topology(&sysinfo, obj.codename);
    if (!refclk_mhz)
        refclk_mhz = detect_refclk();

    if (init_debug) {
        print_topology_debug(&sysinfo, &pmt);
        return;
    }

    switch (obj.smu_if_version) {
        case IF_VERSION_9:  sysinfo.if_ver =  9; break;
        case IF_VERSION_10: sysinfo.if_ver = 10; break;
        case IF_VERSION_11: sysinfo.if_ver = 11; break;
        case IF_VERSION_12: sysinfo.if_ver = 12; break;
        case IF_VERSION_13: sysinfo.if_ver = 13; break;
        default:            sysinfo.if_ver =  0; break;
    }

    interactive = repeating && !export_path && isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
    if (interactive)
        terminal_setup();

    if (export_path)
        export_setup();

    ops->init(repeating, !export_path && isatty(fileno(stdout)));

    do {
        struct timespec deadline;

        have_sample = smu_read_pm_table(&obj, pm_buf, obj.pm_table_size) == SMU_Return_OK;
        if (have_sample)
            emit_sample(&pmt, &sysinfo, ops);

        if (!repeating)
            break;

        if (!terminal_raw) {
            nanosleep(&update_time_s, NULL);
            continue;
        }

        // Until the next update, redraw the last sample on every key press.
        clock_gettime(CLOCK_MONOTONIC, &deadline);
        deadline.tv_sec += update_time_s.tv_sec + (deadline.tv_nsec + update_time_s.tv_nsec) / 1000000000;
        deadline.tv_nsec = (deadline.tv_nsec + update_time_s.tv_nsec) % 1000000000;
        while (!stop_requested) {
            int key = wait_for_key(&deadline);

            if (key == KEY_TIMEOUT)
                break;
            if (key == KEY_EOF) {
                terminal_restore();
                break;
            }
            if (!handle_key(key)) {
                stop_requested = 1;
                break;
            }
            if (have_sample)
                emit_sample(&pmt, &sysinfo, ops);
        }
    } while (!stop_requested);

    ops->cleanup();
    terminal_restore();
    export_cleanup();
}

// Save the current PM table to path, with the table version prepended to the file name.
static void write_dumpfile(const char *path) {
    const char *base = strrchr(path, '/');
    int dirlen = base ? base - path + 1 : 0;
    unsigned char *pm_buf;
    char *name;
    FILE *fd;

    if (!smu_pm_tables_supported(&obj)) {
        fprintf(stderr, "PM Tables are not supported on this platform.\n");
        exit(-1);
    }

    pm_buf = calloc(obj.pm_table_size, sizeof(unsigned char));
    if (!pm_buf || smu_read_pm_table(&obj, pm_buf, obj.pm_table_size) != SMU_Return_OK) {
        fprintf(stderr, "Could not read the PM Table.\n");
        exit(-1);
    }

    if (asprintf(&name, "%.*s%X_%s", dirlen, path, obj.pm_table_version, path + dirlen) < 0)
        exit(-1);
    fd = fopen(name, "wb");
    if (!fd || fwrite(pm_buf, 1, obj.pm_table_size, fd) != obj.pm_table_size || fclose(fd) != 0) {
        fprintf(stderr, "Could not write the dumpfile (\"%s\").\n", name);
        exit(-1);
    }
    fprintf(stdout, "Written %u bytes in dumpfile (\"%s\").\n", obj.pm_table_size, name);

    free(name);
    free(pm_buf);
}

// Print every element of a PM table as "index<TAB>value".
static void print_table_elements(const unsigned char *buf, unsigned int size) {
    float v;

    fprintf(stdout, "\n\n");
    for (unsigned int i = 0; i < size / sizeof(v); i++) {
        memcpy(&v, buf + i * sizeof(v), sizeof(v));
        fprintf(stdout, "%u\t%f\n", i, v);
    }
}

void read_from_dumpfile(char *dumpfile, unsigned int version, const struct output_ops* ops) {
    unsigned char readbuf[10240];
    unsigned int bytes_read;
    pm_table pmt;
    system_info sysinfo;
    FILE *fd;

    if (!version) {
        fprintf(stderr, "You need to specify a PM Table version with -f.\n");
        exit(0);
    }

    //Read file
    fd = fopen(dumpfile, "rb");
    if(!fd) {
        fprintf(stderr, "Could not read the dumpfile (\"%s\").\n", dumpfile);
        exit(0);
    }
    bytes_read=fread(readbuf,sizeof(char),sizeof(readbuf),fd);
    fclose(fd);

    //Select matching PM Table
    if(!select_pm_table_version(version, &pmt, readbuf)) {
        unsigned int similar = pm_table_similar_version(version);

        fprintf(stderr, "This PM Table version (0x%x) is currently not supported.\n", version);
        if (similar)
            fprintf(stderr, "Version 0x%x is a revision of the same table; -f 0x%x might work, but the values can be wrong.\n", similar, similar);
        exit(0);
    }
    else fprintf(stderr, "Using PM Table version 0x%x.\n", version);

    //Prevent illegal memory access
    if (bytes_read < pmt.min_size) {
        fprintf(stderr, "Read %d bytes from \"%s\", but the selected PM Table is %d bytes long.\n", bytes_read, dumpfile, pmt.min_size);
        exit(0);
    }

    memset(&sysinfo, 0, sizeof(sysinfo)); //Did not read sysinfo
    sysinfo.core_disable_map = disabled_cores_from_pmt(&pmt);
    sysinfo.cores = pmt.max_cores - count_set_bits(sysinfo.core_disable_map);
    sysinfo.enabled_cores_count = sysinfo.cores;

    if (export_path)
        export_setup();
    ops->init(0, !export_path && isatty(fileno(stdout)));
    emit_sample(&pmt, &sysinfo, ops);
    ops->cleanup();
    export_cleanup();

    if (dump_table)
        print_table_elements(readbuf, bytes_read);
}

// Run the --get-* and --set-* options.
static void run_controls(unsigned int force) {
    unsigned char *pm_buf = NULL;
    pm_table pmt;
    system_info sysinfo;

    memset(&pmt, 0, sizeof(pmt));
    memset(&sysinfo, 0, sizeof(sysinfo));

    // The current limits are read from the PM table, if there is a known one.
    if (smu_pm_tables_supported(&obj)) {
        unsigned int version = force ? force : pm_table_lookup_version(obj.pm_table_version, obj.codename);

        pm_buf = calloc(obj.pm_table_size, sizeof(unsigned char));
        if (pm_buf && select_pm_table_version(version, &pmt, pm_buf) && obj.pm_table_size >= pmt.min_size)
            smu_read_pm_table(&obj, pm_buf, obj.pm_table_size);
        else
            memset(&pmt, 0, sizeof(pmt));
    }

    controls_run(&pmt, pm_buf, &sysinfo);
    free(pm_buf);
}

void print_version() {
    fprintf(stdout, "Ryzen Monitor " PROGRAM_VERSION "\n");
    exit(0);
}

static void show_help_options(char* program);

void show_help(char* program) {
    show_help_options(program);
    controls_print_help();
}

static void show_help_options(char* program) {
    fprintf(stdout,
        "Ryzen Monitor " PROGRAM_VERSION "\n\n"

        "Usage: %s <option(s)>\n\n"

        "Options:\n"
            "\t-h, --help                - Show this help screen.\n"
            "\t-v, --version             - Show program version.\n"
            "\t-m, --timings             - Print DRAM Timings and exit.\n"
            "\t-d, --disabled            - Show disabled cores.\n"
            "\t-u, --update=<seconds>    - Update the monitoring only after this number of second(s) have passed. Defaults to 1.\n"
            "\t-f, --forcetable=<hex>    - Force to use a specific PM table version.\n"
            "\t-t, --dumpfile=<filename> - Test mode. Read PM Table from raw-dumfile. Use in conjunction with -f\n"
            "\t    --dumptable           - With -t, also print every element of the table.\n"
            "\t-w, --writedump=<file>    - Save the PM Table to <file>, with the table version prepended to the name.\n"
            "\t    --init-debug          - Show how the processor topology was determined and exit.\n"
            "\t-b, --refclk=<MHz>        - Reference clock (BCLK). The SMU reports clocks against a nominal 100 MHz\n"
            "\t                            reference; on a BCLK-overclocked system they are scaled by refclk/100.\n"
            "\t                            Measured from the TSC by default, 100 when reading a dump file.\n"
            "\t-o, --format=<format>     - Output format (boxdrawing, text, json, ndjson, influx, prometheus, netdata)\n"
            "\t-1, --once                - Print once and exit\n"
            "\t    --view=<view>[,...]   - Show only these views (info, cores, stats, electrical, memory, gfx, power, summary, all)\n"
            "\t-c, --compact             - Leave out details.\n"
            "\t-e, --export=<path>       - Write every sample to this named pipe (created if needed), for telegraf.\n"
            "\t                            The format defaults to influx.\n"
            "\t    --test-export         - Same as --format=influx.\n"
            "\n"
        "Keys in the interactive monitor:\n"
            "\tq - quit, c - compact mode, i/k/s/e/m/g/p/y - info/cores/stats/electrical/memory/gfx/power/summary view\n",
        program
    );
}

struct timespec parse_fractional_time(const char* s) {
    // In "3.14", the "3" represents 3 seconds, but the "14" represents
    // 140000000 nanoseconds -- it's a "left-justified" number.  Deal with that
    // by writing the fractional part to the front of a buffer of "0" and then
    // scanning that buffer.
    struct timespec t;
    char fract[] = "000000000";
    sscanf(s, "%ld.%9c", &t.tv_sec, fract);
    t.tv_nsec = atol(fract);
    return t;
}

int main(int argc, char** argv) {
    smu_return_val ret;
    int c=0, force=0, core=0, printtimings=0, repeating=1;
    char *dumpfile=0, *writedump=0;
    const struct output_ops* ops = &box_drawing_ops;
    bool format_set = false;

    //Set up signal handlers. Without SA_RESTART, so that blocking calls return
    //and the main loop notices the request to stop.
    struct sigaction sa = { .sa_handler = signal_interrupt };
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGTERM, &sa, NULL) != 0 ||
        sigaction(SIGINT, &sa, NULL) != 0) {
        fprintf(stderr, "Can't set up signal hooks.\n");
        exit(-1);
    }

    //Parse arguments
    enum { OPT_VIEW = 0x100, OPT_TEST_EXPORT, OPT_DUMPTABLE, OPT_INIT_DEBUG };
    static struct option long_options[64] = {
        { "help",       no_argument,       NULL, 'h' },
        { "version",    no_argument,       NULL, 'v' },
        { "timings",    no_argument,       NULL, 'm' },
        { "disabled",   optional_argument, NULL, 'd' },
        { "update",     required_argument, NULL, 'u' },
        { "forcetable", required_argument, NULL, 'f' },
        { "dumpfile",   required_argument, NULL, 't' },
        { "dumptable",  no_argument,       NULL, OPT_DUMPTABLE },
        { "writedump",  required_argument, NULL, 'w' },
        { "init-debug", no_argument,       NULL, OPT_INIT_DEBUG },
        { "refclk",     required_argument, NULL, 'b' },
        { "format",     required_argument, NULL, 'o' },
        { "once",       no_argument,       NULL, '1' },
        { "view",       required_argument, NULL, OPT_VIEW },
        { "compact",    no_argument,       NULL, 'c' },
        { "export",     required_argument, NULL, 'e' },
        { "test-export", no_argument,      NULL, OPT_TEST_EXPORT },
    };
    int nlong = 0;

    while (long_options[nlong].name)
        nlong++;
    controls_options(long_options, &nlong);
    long_options[nlong] = (struct option){ 0 };

    while ((c = getopt_long(argc, argv, "vmd::f:t:w:u:o:1ce:b:h", long_options, NULL)) != -1) {
        switch (c) {
            case 'v':
                print_version();
                exit(0);
            case 'm':
                printtimings = 1;
                break;
            case 'd':
                if (optarg)
                    show_disabled_cores = atoi(optarg);
                else
                    show_disabled_cores = 1;
                break;
            case 'f':
                if(!optarg || sscanf(optarg,"%x", &force)!=1) {
                    show_help(argv[0]);
                    exit(0);
                }
                break;
            case 't':
                if(!optarg || strlen(optarg)==0) {
                    show_help(argv[0]);
                    exit(0);
                }
                dumpfile=optarg;
                break;
            case 'o':
                if (!optarg) optarg = "";
                if (!strcmp(optarg, "boxdrawing") || !strcmp(optarg, "box-drawing"))
                    ops = &box_drawing_ops;
                else if (!strcmp(optarg, "json"))
                    ops = &json_ops;
                else if (!strcmp(optarg, "ndjson"))
                    ops = &ndjson_ops;
                else if (!strcmp(optarg, "text"))
                    ops = &text_ops;
                else if (!strcmp(optarg, "influx"))
                    ops = &influx_ops;
                else if (!strcmp(optarg, "prometheus"))
                    ops = &prometheus_ops;
                else if (!strcmp(optarg, "netdata"))
                    ops = &netdata_ops;
                else {
                    show_help(argv[0]);
                    exit(0);
                }
                format_set = true;
                break;
            case '1':
                repeating = 0;
                break;
            case 'c':
                display.compact = true;
                break;
            case 'e':
                export_path = optarg;
                break;
            case 'w':
                writedump = optarg;
                break;
            case OPT_DUMPTABLE:
                dump_table = true;
                break;
            case OPT_INIT_DEBUG:
                init_debug = true;
                break;
            case 'b':
                refclk_mhz = strtod(optarg, NULL);
                if (refclk_mhz < 90 || refclk_mhz > 130) {
                    fprintf(stderr, "The reference clock must be between 90 and 130 MHz.\n");
                    exit(-1);
                }
                break;
            case OPT_TEST_EXPORT:
                ops = &influx_ops;
                format_set = true;
                break;
            case OPT_VIEW:
                if (!parse_views(optarg, &display.views)) {
                    fprintf(stderr, "Unknown view in \"%s\".\n", optarg);
                    exit(-1);
                }
                break;
            case 'u':
                update_time_s = parse_fractional_time(optarg);
                break;
            case 'h':
                show_help(argv[0]);
                exit(0);
            case '?':
                exit(0);
            default:
                controls_handle_option(c, optarg);
                break;
        }
    }

    if (!repeating)
        update_time_s.tv_sec = update_time_s.tv_nsec = 0;

    if (export_path && !format_set)
        ops = &influx_ops;

    netdata_set_update_every(update_time_s.tv_sec + (update_time_s.tv_nsec >= 500000000));

    if(dumpfile && !printtimings)
        read_from_dumpfile(dumpfile, force, ops);
    else
    {
        if (getuid() != 0 && geteuid() != 0) {
            fprintf(stderr, "Program must be run as root.\n");
            exit(-2);
        }

        ret = smu_init(&obj);
        if (ret != SMU_Return_OK) {
            fprintf(stderr, "%s\n", smu_return_to_str(ret));
            exit(-2);
        }

        if (controls_requested()) run_controls(force);
        else if (writedump) write_dumpfile(writedump);
        else if (printtimings) print_memory_timings();
        else start_pm_monitor(force, repeating, ops);
    }

    return 0;
}
