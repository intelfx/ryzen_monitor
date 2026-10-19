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

#define PROGRAM_VERSION "1.0.6"

smu_obj_t obj;
static struct timespec update_time_s = {1, 0};
static int show_disabled_cores = 0;
static struct display display = { .views = VIEWS_ALL };

//Helper to access the PM Table elements. If an element doesn't exist in the
//current PM Table version, it's pointer is set to 0. This helper returns
//NAN for not available fields.
#define pmta(elem) ((pmt->elem)?(*pmt->elem):NAN)
//Same, but with 0 as return. For summations that should not fail if one value is not present.
#define pmta0(elem) ((pmt->elem)?(*pmt->elem):0)

static void emit_sample(const pm_table *pmt, const system_info *sysinfo, const struct output_ops *ops) {
    struct sample_options opts = { .show_disabled_cores = show_disabled_cores };
    struct sample sample;

    compute_sample(pmt, sysinfo, &opts, &sample);
    ops->emit(&sample, &display);
}

int select_pm_table_version(unsigned int version, pm_table *pmt, unsigned char *pm_buf) {
    //Initialize pmt to 0. This also sets all pointers to 0, which signifies non-existiting fields.
    //Access via pmta(...) will check if pointer is 0 before trying to access the value.
    memset(pmt, 0, sizeof(pm_table));

     //Select matching PM Table
    switch(version) {
        case 0x380904: pm_table_0x380904(pmt, pm_buf); break; //Ryzen 5600X
        case 0x380905: pm_table_0x380905(pmt, pm_buf); break; //Ryzen 5600X
        case 0x380804: pm_table_0x380804(pmt, pm_buf); break; //Ryzen 5900X / 5950X
        case 0x380805: pm_table_0x380805(pmt, pm_buf); break; //Ryzen 5900X / 5950X
        case 0x400005: pm_table_0x400005(pmt, pm_buf); break; //Ryzen 5700G
        case 0x240903: pm_table_0x240903(pmt, pm_buf); break; //Ryzen 3700X / 3800X
        case 0x240803: pm_table_0x240803(pmt, pm_buf); break; //Ryzen 3950X
        default:
            return 0;
    }

    //Avoid access bejond bounds of the defined arrays.
    if (pmt->max_l3 > PMT_MAX_NUM_L3) pmt->max_l3 = PMT_MAX_NUM_L3;
    if (pmt->max_cores > PMT_MAX_NUM_CORES) pmt->max_cores = PMT_MAX_NUM_CORES;

    //APML_POWER is probably identical to PACKAGE_POWER
    if (pmt->PACKAGE_POWER == NULL) pmt->PACKAGE_POWER = pmt->APML_POWER;

    if (pmt->VDD18_POWER == NULL) pmt->VDD18_POWER = pmt->IO_VDD18_POWER;

    return 1;
}

void disabled_cores_0x400005(pm_table *pmt, system_info *sysinfo) {
    int i, mask;
    float power, voltage, fit, iddmax, freq, freqeff, c0, cc1, irm;
    for (i = 0; i < 8; i++) {
        power = pmta(CORE_POWER[i]);
        voltage = pmta(CORE_VOLTAGE[i]);
        fit = pmta(CORE_FIT[i]);
        iddmax = pmta(CORE_IDDMAX[i]);
        freq = pmta(CORE_FREQ[i]);
        freqeff = pmta(CORE_FREQEFF[i]);
        c0 = pmta(CORE_C0[i]);
        cc1 = pmta(CORE_CC1[i]);
        irm = pmta(CORE_IRM[i]);
        
        if (power == 0 && voltage == 0 && fit == 0 && iddmax == 0 && freq == 0 && freqeff == 0 && c0 == 0 && cc1 == 0 && irm == 0 ) {
            mask = 1 << i;
        } else {
            mask = 0 << i;
        }
        sysinfo->core_disable_map_pmt = sysinfo->core_disable_map_pmt | mask;
    }
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

void start_pm_monitor(unsigned int force, int repeating, const struct output_ops* ops) {
    unsigned char *pm_buf;
    pm_table pmt;
    system_info sysinfo;
    bool interactive, have_sample;

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
    if(!select_pm_table_version(force?force:obj.pm_table_version, &pmt, pm_buf)) {
        fprintf(stderr, "This PM Table version (0x%x) is currently not supported.\n", force?force:obj.pm_table_version);
        fprintf(stderr, "Processor name: %s\n", get_processor_name());
        fprintf(stderr, "SMU FW version: %s\n", smu_get_fw_version(&obj));
        exit(0);
    }
    //Prevent illegal memory access
    if (obj.pm_table_size < pmt.min_size) {
        fprintf(stderr, "Selected PM Table is larger than the PM Table returned by the SMU.\n");
        exit(0);
    }
    //Maximum core count. Just to be safe. Will be overwritten by get_processor_topology(...).
    sysinfo.enabled_cores_count = pmt.max_cores;

    sysinfo.cpu_name    = get_processor_name();
    sysinfo.codename    = smu_codename_to_str(&obj);
    sysinfo.smu_fw_ver  = smu_get_fw_version(&obj);

    //PMT hack for Cezanne's core_disabled_map 
    if (obj.pm_table_version == 0x400005) {
        if (smu_read_pm_table(&obj, pm_buf, obj.pm_table_size) == SMU_Return_OK) {
            disabled_cores_0x400005(&pmt, &sysinfo);
        }
    }
    
    get_processor_topology(&sysinfo, pmt.zen_version);

    switch (obj.smu_if_version) {
        case IF_VERSION_9:  sysinfo.if_ver =  9; break;
        case IF_VERSION_10: sysinfo.if_ver = 10; break;
        case IF_VERSION_11: sysinfo.if_ver = 11; break;
        case IF_VERSION_12: sysinfo.if_ver = 12; break;
        case IF_VERSION_13: sysinfo.if_ver = 13; break;
        default:            sysinfo.if_ver =  0; break;
    }

    interactive = repeating && isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
    if (interactive)
        terminal_setup();

    ops->init(repeating, isatty(fileno(stdout)));

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
        fprintf(stderr, "This PM Table version (0x%x) is currently not supported.\n", version);
        exit(0);
    }
    else fprintf(stderr, "Using PM Table version 0x%x.\n", version);

    //Prevent illegal memory access
    if (bytes_read < pmt.min_size) {
        fprintf(stderr, "Read %d bytes from \"%s\", but the selected PM Table is %d bytes long.\n", bytes_read, dumpfile, pmt.min_size);
        exit(0);
    }
    
    sysinfo.available=0; //Did not read sysinfo
    sysinfo.enabled_cores_count = pmt.max_cores;
    sysinfo.core_disable_map=0;
    sysinfo.cores=sysinfo.enabled_cores_count;

    ops->init(0, isatty(fileno(stdout)));
    emit_sample(&pmt, &sysinfo, ops);
    ops->cleanup();
}

void print_version() {
    fprintf(stdout, "Ryzen Monitor " PROGRAM_VERSION "\n");
    exit(0);
}

void show_help(char* program) {
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
            "\t-o, --format=<format>     - Output format (boxdrawing, json, ndjson)\n"
            "\t-1, --once                - Print once and exit\n"
            "\t    --view=<view>[,...]   - Show only these views (info, cores, stats, electrical, memory, gfx, power, all)\n"
            "\t-c, --compact             - Leave out details.\n"
            "\n"
        "Keys in the interactive monitor:\n"
            "\tq - quit, c - compact mode, i/k/s/e/m/g/p - info/cores/stats/electrical/memory/gfx/power view\n",
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
    char *dumpfile=0;
    const struct output_ops* ops = &box_drawing_ops;

    //Set up signal handlers
    if ((signal(SIGTERM, signal_interrupt) == SIG_ERR) ||
        (signal(SIGINT, signal_interrupt) == SIG_ERR)) {
        fprintf(stderr, "Can't set up signal hooks.\n");
        exit(-1);
    }

    //Parse arguments
    enum { OPT_VIEW = 0x100 };
    static const struct option long_options[] = {
        { "help",       no_argument,       NULL, 'h' },
        { "version",    no_argument,       NULL, 'v' },
        { "timings",    no_argument,       NULL, 'm' },
        { "disabled",   optional_argument, NULL, 'd' },
        { "update",     required_argument, NULL, 'u' },
        { "forcetable", required_argument, NULL, 'f' },
        { "dumpfile",   required_argument, NULL, 't' },
        { "format",     required_argument, NULL, 'o' },
        { "once",       no_argument,       NULL, '1' },
        { "view",       required_argument, NULL, OPT_VIEW },
        { "compact",    no_argument,       NULL, 'c' },
        { 0 }
    };

    while ((c = getopt_long(argc, argv, "vmd::f:t:u:o:1ch", long_options, NULL)) != -1) {
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
                else {
                    show_help(argv[0]);
                    exit(0);
                }
                break;
            case '1':
                repeating = 0;
                break;
            case 'c':
                display.compact = true;
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
                break;
        }
    }

    if (!repeating)
        update_time_s.tv_sec = update_time_s.tv_nsec = 0;

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

        if(printtimings) print_memory_timings();
        else start_pm_monitor(force, repeating, ops);
    }

    return 0;
}
