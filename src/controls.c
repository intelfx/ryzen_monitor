/**
 * Ryzen SMU Userspace Sensor Monitor
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <libsmu.h>

#include "controls.h"
#include "smu_control.h"

extern smu_obj_t obj;

enum control_kind {
    CTL_GET_LIMIT,
    CTL_SET_LIMIT,
    CTL_GET_SCALAR,
    CTL_SET_SCALAR,
    CTL_GET_OCMODE,
    CTL_ENABLE_OC,
    CTL_DISABLE_OC,
    CTL_ENABLE_ECO,
    CTL_ENABLE_MAXPERF,
    CTL_GET_CO,
    CTL_GET_CO_ALL,
    CTL_SET_CO,
    CTL_SET_CO_ALL,
};

struct control_action {
    enum control_kind kind;
    const struct smu_limit *limit;
    int value;
    const char *list;       // CTL_GET_CO, CTL_SET_CO: the option argument
};

#define MAX_ACTIONS 64
static struct control_action actions[MAX_ACTIONS];
static int nactions;

#define OPT_BASE 0x200

static const struct {
    const char *name;
    enum control_kind kind;
    int has_arg;
    const char *help;
} fixed_options[] = {
    { "get-scalar", CTL_GET_SCALAR, no_argument, "Get the PBO scalar" },
    { "set-scalar", CTL_SET_SCALAR, required_argument, "Set the PBO scalar" },
    { "get-ocmode", CTL_GET_OCMODE, no_argument, "Get whether OC mode is enabled" },
    { "set-enable-oc", CTL_ENABLE_OC, no_argument, "Enable OC mode" },
    { "set-disable-oc", CTL_DISABLE_OC, no_argument, "Disable OC mode" },
    { "set-enable-eco", CTL_ENABLE_ECO, no_argument, "Enable power saving mode (APU)" },
    { "set-enable-maxperf", CTL_ENABLE_MAXPERF, no_argument, "Enable maximum performance mode (APU)" },
    { "get-cocount", CTL_GET_CO, required_argument, "Get the Curve Optimizer count of cores, comma-separated (from 0)" },
    { "get-cocountall", CTL_GET_CO_ALL, no_argument, "Get the Curve Optimizer counts of all cores" },
    { "set-cocount", CTL_SET_CO, required_argument, "Set the Curve Optimizer count of cores, e.g. 0+10,1-5,2-20" },
    { "set-cocountall", CTL_SET_CO_ALL, required_argument, "Set the Curve Optimizer count of all cores" },
};

#define NFIXED ((int)(sizeof(fixed_options) / sizeof(fixed_options[0])))

static char limit_option_names[2 * 16][32];

int controls_option_count(void) {
    return NFIXED + 2 * smu_limits_count;
}

void controls_options(struct option *opts, int *n) {
    for (int i = 0; i < smu_limits_count; i++) {
        snprintf(limit_option_names[2 * i], sizeof(limit_option_names[0]), "get-%s", smu_limits[i].name);
        snprintf(limit_option_names[2 * i + 1], sizeof(limit_option_names[0]), "set-%s", smu_limits[i].name);
        opts[(*n)++] = (struct option){ limit_option_names[2 * i], no_argument, NULL, OPT_BASE + 2 * i };
        opts[(*n)++] = (struct option){ limit_option_names[2 * i + 1], required_argument, NULL, OPT_BASE + 2 * i + 1 };
    }
    for (int i = 0; i < NFIXED; i++)
        opts[(*n)++] = (struct option){ fixed_options[i].name, fixed_options[i].has_arg, NULL, OPT_BASE + 2 * smu_limits_count + i };
}

bool controls_handle_option(int code, const char *arg) {
    struct control_action a = { 0 };
    int index = code - OPT_BASE;

    if (index < 0 || index >= controls_option_count())
        return false;

    if (index < 2 * smu_limits_count) {
        a.kind = index % 2 ? CTL_SET_LIMIT : CTL_GET_LIMIT;
        a.limit = &smu_limits[index / 2];
    } else {
        a.kind = fixed_options[index - 2 * smu_limits_count].kind;
    }
    if (a.kind == CTL_GET_CO || a.kind == CTL_SET_CO) {
        a.list = arg;
    } else if (arg) {
        char *end;

        // Only Curve Optimizer counts can be negative.
        a.value = strtol(arg, &end, 10);
        if (*end || (a.kind != CTL_SET_CO_ALL && a.value <= 0)) {
            fprintf(stderr, "Invalid value \"%s\".\n", arg);
            exit(-1);
        }
    }

    if (nactions == MAX_ACTIONS) {
        fprintf(stderr, "Too many --get-* and --set-* options.\n");
        exit(-1);
    }
    actions[nactions++] = a;
    return true;
}

bool controls_requested(void) {
    return nactions > 0;
}

void controls_print_help(void) {
    fprintf(stdout, "\nSMU settings (print the result and exit):\n");
    for (int i = 0; i < smu_limits_count; i++)
        fprintf(stdout, "\t    --get-%s, --set-%s=<value>\n\t                            - %s\n",
                smu_limits[i].name, smu_limits[i].name, smu_limits[i].help);
    for (int i = 0; i < NFIXED; i++)
        fprintf(stdout, "\t    --%s%s\n\t                            - %s\n", fixed_options[i].name,
                fixed_options[i].has_arg ? "=<value>" : "", fixed_options[i].help);
}

static float *limit_field(const pm_table *pmt, const struct smu_limit *limit) {
    return *(float **)((const char *)pmt + limit->pm_field);
}

// Read the PM table again, after giving the SMU time to update it.
static void refresh(unsigned char *pm_buf) {
    struct timespec t = { 0, 100000000L };

    nanosleep(&t, NULL);
    smu_read_pm_table(&obj, pm_buf, obj.pm_table_size);
}

static const char *result_str(enum smu_control_result ret) {
    return ret == SMU_CONTROL_UNSUPPORTED ? "NA" : "ERR";
}

// OC mode is enabled when the SMU reports a PBO scalar of 0.
static int oc_mode(const struct smu_settings *s) {
    float scalar;

    if (!s || smu_get_pbo_scalar(s, &scalar) != SMU_CONTROL_OK)
        return -1;
    return scalar == 0;
}

static void run_action(const struct control_action *a, const struct smu_settings *s,
        const pm_table *pmt, unsigned char *pm_buf) {
    enum smu_control_result ret = SMU_CONTROL_UNSUPPORTED;
    float *field, scalar = 0;
    int before = 0, after = 0;

    switch (a->kind) {
        case CTL_GET_LIMIT:
            field = limit_field(pmt, a->limit);
            if (field)
                fprintf(stdout, "get-%s: %.0f\n", a->limit->name, *field);
            else
                fprintf(stdout, "get-%s: NA\n", a->limit->name);
            break;

        case CTL_SET_LIMIT:
            field = limit_field(pmt, a->limit);
            fprintf(stdout, "set-%s: ", a->limit->name);
            if (field)
                before = *field;
            if (s)
                ret = smu_set_limit(s, a->limit, a->value);
            if (ret == SMU_CONTROL_OK)
                fprintf(stdout, "%d", a->value);
            else
                fprintf(stdout, "%s", result_str(ret));
            if (field) {
                refresh(pm_buf);
                after = *field;
                fprintf(stdout, " before: %d after: %d", before, after);
            }
            fprintf(stdout, "\n");
            break;

        case CTL_GET_SCALAR:
            if (s)
                ret = smu_get_pbo_scalar(s, &scalar);
            if (ret == SMU_CONTROL_OK)
                fprintf(stdout, "get-scalar: %.0f\n", scalar);
            else
                fprintf(stdout, "get-scalar: %s\n", result_str(ret));
            break;

        case CTL_SET_SCALAR:
            fprintf(stdout, "set-scalar: ");
            before = s && smu_get_pbo_scalar(s, &scalar) == SMU_CONTROL_OK ? (int)scalar : -1;
            if (s)
                ret = smu_set_pbo_scalar(s, a->value);
            if (ret == SMU_CONTROL_OK)
                fprintf(stdout, "%d", a->value);
            else
                fprintf(stdout, "%s", result_str(ret));
            after = s && smu_get_pbo_scalar(s, &scalar) == SMU_CONTROL_OK ? (int)scalar : -1;
            if (before >= 0)
                fprintf(stdout, " before: %d", before);
            if (after >= 0)
                fprintf(stdout, " after: %d", after);
            fprintf(stdout, "\n");
            break;

        case CTL_GET_OCMODE:
            before = oc_mode(s);
            if (before >= 0)
                fprintf(stdout, "get-ocmode: %d\n", before);
            else
                fprintf(stdout, "get-ocmode: NA\n");
            break;

        case CTL_ENABLE_OC:
        case CTL_DISABLE_OC:
            fprintf(stdout, "set-%s-oc: ", a->kind == CTL_ENABLE_OC ? "enable" : "disable");
            before = oc_mode(s);
            if (s)
                ret = smu_set_oc_mode(s, a->kind == CTL_ENABLE_OC);
            if (ret == SMU_CONTROL_OK)
                fprintf(stdout, "%d", a->kind == CTL_ENABLE_OC);
            else
                fprintf(stdout, "%s", result_str(ret));
            after = oc_mode(s);
            if (before >= 0)
                fprintf(stdout, " before: %d", before);
            if (after >= 0)
                fprintf(stdout, " after: %d", after);
            fprintf(stdout, "\n");
            break;

        case CTL_ENABLE_ECO:
        case CTL_ENABLE_MAXPERF:
            if (s)
                ret = smu_set_power_mode(s, a->kind == CTL_ENABLE_MAXPERF);
            fprintf(stdout, "set-enable-%s: %s\n", a->kind == CTL_ENABLE_ECO ? "eco" : "maxperf",
                    ret == SMU_CONTROL_OK ? "1" : result_str(ret));
            break;

        default:
            break;
    }
}

// --get-cocount=0,1,2 and --set-cocount=0+10,1-5: a core number, then for setting a signed count.
static void run_co_list(const struct control_action *a, const struct smu_settings *s, const system_info *sysinfo) {
    const char *name = a->kind == CTL_SET_CO ? "set-cocount" : "get-cocount";
    const char *p = a->list;

    while (*p) {
        enum smu_control_result ret = SMU_CONTROL_UNSUPPORTED;
        char *end;
        long core = strtol(p, &end, 10);
        long count = 0;

        if (end == p || core < 0) {
            fprintf(stdout, "%s: ERR (%s)\n", name, p);
            return;
        }
        p = end;
        if (a->kind == CTL_SET_CO) {
            if (*p != '+' && *p != '-') {
                fprintf(stdout, "%s: ERR (%s)\n", name, a->list);
                return;
            }
            count = strtol(p, &end, 10);
            p = end;
            if (s)
                ret = smu_set_co(s, sysinfo, core, count);
        } else if (s) {
            int c;

            ret = smu_get_co(s, sysinfo, core, &c);
            count = c;
        }

        if (ret == SMU_CONTROL_OK)
            fprintf(stdout, "%s: %ld%+ld\n", name, core, count);
        else
            fprintf(stdout, "%s: %s\n", name, result_str(ret));

        if (*p == ',')
            p++;
        else if (*p) {
            fprintf(stdout, "%s: ERR (%s)\n", name, p);
            return;
        }
    }
}

void controls_run(const pm_table *pmt, unsigned char *pm_buf, const system_info *sysinfo) {
    const struct smu_settings *s = smu_settings_for(obj.codename);

    for (int i = 0; i < nactions; i++) {
        const struct control_action *a = &actions[i];
        enum smu_control_result ret = SMU_CONTROL_UNSUPPORTED;

        switch (a->kind) {
            case CTL_GET_CO:
            case CTL_SET_CO:
                run_co_list(a, s, sysinfo);
                break;

            case CTL_GET_CO_ALL:
                fprintf(stdout, "get-cocountall: ");
                for (unsigned int core = 0; core < sysinfo->enabled_cores_count; core++) {
                    int count = 0;

                    if (s)
                        ret = smu_get_co(s, sysinfo, core, &count);
                    if (ret != SMU_CONTROL_OK) {
                        fprintf(stdout, "%s", result_str(ret));
                        break;
                    }
                    fprintf(stdout, "%s%u%+d", core ? "," : "", core, count);
                }
                fprintf(stdout, "\n");
                break;

            case CTL_SET_CO_ALL:
                if (s)
                    ret = smu_set_co_all(s, a->value);
                if (ret == SMU_CONTROL_OK)
                    fprintf(stdout, "set-cocountall: %+d\n", a->value);
                else
                    fprintf(stdout, "set-cocountall: %s\n", result_str(ret));
                break;

            default:
                run_action(a, s, pmt, pm_buf);
                break;
        }
    }
}
