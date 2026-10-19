/**
 * Ryzen SMU Userspace Sensor Monitor
 * Copyright (C) 2020-2021
 *    Jeffrey Bosboom <jbosboom@jeffreybosboom.com>
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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "output_ops.h"

static void print_line(const char* label, const char* value_format, ...) {
    static char buffer[1024];
    va_list list;

    va_start(list, value_format);
    vsnprintf(buffer, sizeof(buffer), value_format, list);
    va_end(list);

    fprintf(stdout, "│ %45s │ %46s │\n", label, buffer);
}

static bool use_control_codes = 0;
static void box_drawing_init(bool repeating, bool interactive) {
    use_control_codes = repeating && interactive;
    if (use_control_codes)
        fprintf(stdout, "\e[?25l"); // Hide Cursor
}

static void box_drawing_cleanup(void) {
    if (use_control_codes)
        fprintf(stdout, "\e[?25h"); // Show Cursor
}

static const char* const group_begin_strings[] = {
    [GROUP_SYSINFO] = "╭───────────────────────────────────────────────┬────────────────────────────────────────────────╮\n",
    [GROUP_CORES] = "╭─────────┬────────────┬──────────┬─────────┬──────────┬─────────────┬─────────────┬─────────────╮\n",
    [GROUP_CORE_STATS_CALC] = "╭── Core Statistics (Calculated) ───────────────┬────────────────────────────────────────────────╮\n",
    [GROUP_CORE_STATS_SMU] = "├── Reported by SMU ────────────────────────────┼────────────────────────────────────────────────┤\n",
    [GROUP_LIMITS] = "╭── Electrical & Thermal Constraints ───────────┬────────────────────────────────────────────────╮\n",
    [GROUP_MEMORY] = "╭── Memory Interface ───────────────────────────┬────────────────────────────────────────────────╮\n",
    [GROUP_GRAPHICS] = "╭── Graphics Subsystem──────────────────────────┬────────────────────────────────────────────────╮\n",
    [GROUP_POWER] = "╭── Power Consumption ──────────────────────────┬────────────────────────────────────────────────╮\n",
    [GROUP_POWER_REPORTS] = "├── Additional Reports ─────────────────────────┼────────────────────────────────────────────────┤\n",
    [GROUP_SUMMARY] = "╭── Summary ────────────────────────────────────┬────────────────────────────────────────────────╮\n",
};

static const char usual_end_string[] = "╰───────────────────────────────────────────────┴────────────────────────────────────────────────╯\n";
static const char* const group_end_strings[] = {
    [GROUP_SYSINFO] = usual_end_string,
    [GROUP_CORES] = "╰─────────┴────────────┴──────────┴─────────┴──────────┴─────────────┴─────────────┴─────────────╯\n",
    [GROUP_CORE_STATS_CALC] = "",
    [GROUP_CORE_STATS_SMU] = usual_end_string,
    [GROUP_LIMITS] = usual_end_string,
    [GROUP_MEMORY] = usual_end_string,
    [GROUP_GRAPHICS] = usual_end_string,
    [GROUP_POWER] = "",
    [GROUP_POWER_REPORTS] = usual_end_string,
    [GROUP_SUMMARY] = usual_end_string,
};

static void print_sysinfo(const struct sample *s) {
    const system_info *sysinfo = s->sysinfo;

    print_line("CPU Model", "%s", sysinfo->cpu_name);
    print_line("Processor Code Name", "%s", sysinfo->codename);
    print_line("Cores", "%d", sysinfo->cores);
    print_line("Core CCDs", "%d", sysinfo->ccds);
    if (s->zen_version < 3) {
        print_line("Core CCXs", "%d", sysinfo->ccxs);
        print_line("Cores Per CCX", "%d", sysinfo->cores_per_ccx);
    } else
        print_line("Cores Per CCD", "%d", sysinfo->cores_per_ccx); //Zen3 does not have CCXs anymore
    print_line("SMU FW Version", "v%s", sysinfo->smu_fw_ver);
    print_line("MP1 IF Version", "v%d", sysinfo->if_ver);
}

static void print_cores(const struct sample *s) {
    for (int i = 0; i < s->ncores; i++) {
        const struct core_state *core = &s->core[i];
        char number_str[8];
        char frequency_str_buf[10];
        const char* frequency_str = frequency_str_buf;

        if (core->number < 0)
            continue;

        snprintf(number_str, sizeof(number_str), "Core %d", core->number);
        if (core->disabled)
            frequency_str = "Disabled";
        else if (core->sleeping)
            frequency_str = "Sleeping";
        else
            snprintf(frequency_str_buf, sizeof(frequency_str_buf), "%4.f MHz", metric_value(s, M_CORE_FREQUENCY, i));

        fprintf(stdout, "│ %7s │ %10s | %6.3f W | %5.3f V | %6.2f C | C0: %5.1f %% | C1: %5.1f %% | C6: %5.1f %% │\n",
                number_str, frequency_str,
                metric_value(s, M_CORE_POWER, i),
                metric_value(s, M_CORE_VOLTAGE, i),
                metric_value(s, M_CORE_TEMPERATURE, i),
                metric_value(s, M_CORE_C0, i) * 100.f,
                metric_value(s, M_CORE_CC1, i) * 100.f,
                metric_value(s, M_CORE_CC6, i) * 100.f);
    }
}

static void print_value(const char *label, enum metric_kind kind, float value, enum unit unit) {
    const char* format = "";
    if (kind == KIND_BOOL) {
        print_line(label, "%s", value ? "ON" : "OFF");
        return;
    }
    switch (unit) {
        case UNIT_COUNT: format = "%.f"; break;
        case UNIT_RATIO: format = "%6.2f %%"; value *= 100.f; break;
        case UNIT_MEGAHERTZ: format = "%8.0f MHz"; break;
        case UNIT_VOLTS: format = "%7.4f V"; break;
        case UNIT_AMPS: assert(0 && "value amps"); break;
        case UNIT_WATTS: format = "%7.3f W"; break;
        case UNIT_CELSIUS: format = "%8.2f C"; break;
    }
    print_line(label, format, value);
}

static void print_multi(const char *label, int n, const float *value, const enum unit *unit) {
    const char* format = "";
    float v[3] = { value[0], value[1], n > 2 ? value[2] : 0 };
    if (n == 2 && unit[0] == UNIT_VOLTS && unit[1] == UNIT_WATTS) format = "%7.4f V | %8.3f W";
    else if (n == 2 && unit[0] == UNIT_MEGAHERTZ && unit[1] == UNIT_MEGAHERTZ) format = "%5.f MHz | %6.f MHz";
    else if (n == 2 && unit[0] == UNIT_AMPS && unit[1] == UNIT_RATIO) format = "%7.3f A | %8.2f %%";
    else if (n == 2 && unit[0] == UNIT_COUNT && unit[1] == UNIT_COUNT) format = "%2.f | %8.2f  ";
    else if (n == 3 && unit[0] == UNIT_VOLTS && unit[1] == UNIT_AMPS && unit[2] == UNIT_WATTS) format = "%8.3f V | %7.3f A | %8.3f W";
    else if (n == 3 && unit[0] == UNIT_WATTS && unit[1] == UNIT_MEGAHERTZ && unit[2] == UNIT_RATIO) format = "%7.3f W | %5.f MHz | %8.2f %%";
    else {assert(0 && "unhandled unit combination");}
    for (int i = 0; i < n; i++)
        if (unit[i] == UNIT_RATIO) v[i] *= 100.f;
    print_line(label, format, v[0], v[1], v[2]);
}

static void print_limit(const char *label, float value, float limit, enum unit unit) {
    const char* format = "";
    switch (unit) {
        case UNIT_COUNT: format = "%7.f   | %7.f   | %8.2f %%"; break;
        case UNIT_RATIO: assert(0 && "limit ratio"); break;
        case UNIT_MEGAHERTZ: assert(0 && "limit megahertz"); break;
        case UNIT_VOLTS: format = "%7.3f V | %7.3f V | %8.2f %%"; break;
        case UNIT_AMPS: format = "%7.3f A | %7.f A | %8.2f %%"; break;
        case UNIT_WATTS: format = "%7.3f W | %7.f W | %8.2f %%"; break;
        case UNIT_CELSIUS: format = "%7.2f C | %7.f C | %8.2f %%"; break;
    }
    print_line(label, format, value, limit, (value / limit) * 100.f);
}

static void print_sum(const char *label, int n, const float *v) {
    if (n == 1)
        print_value(label, KIND_GAUGE, v[0], UNIT_WATTS);
    else if (n == 2)
        print_line(label, "%7.3f W + %7.3f W = %7.3f W", v[0], v[1], v[0]+v[1]);
    else if (n == 4) {
        print_line(label, "%7.3f W + %7.3f W = %7s", v[0], v[1], "");
        print_line("", "%7.3f W + %7.3f W = %7.3f W", v[2], v[3], v[0]+v[1]+v[2]+v[3]);
    } else {
        assert(0 && "sum unhandled length");
        print_line(label, "unhandled length %d", n);
    }
}

static void print_row(const struct sample *s, const struct row *row) {
    const struct metric_def *def = &metric_defs[row->m[0]];
    float value[3];
    enum unit unit[3];

    if (row->blank_before)
        print_line("", "");

    switch (row->kind) {
        case ROW_VALUE:
            print_value(row->label, def->kind, metric_value(s, row->m[0], 0), def->unit);
            break;
        case ROW_LIMIT:
            print_limit(row->label, metric_value(s, row->m[0], 0), metric_limit(s, row->m[0], 0), def->unit);
            break;
        case ROW_MULTI:
            for (int i = 0; i < row->n; i++) {
                value[i] = metric_value(s, row->m[i], 0);
                unit[i] = metric_defs[row->m[i]].unit;
            }
            print_multi(row->label, row->n, value, unit);
            break;
        case ROW_SUM:
            print_sum(row->label, metric_count(s, row->m[0]), s->m[row->m[0]].value);
            break;
    }
}

static void box_drawing_emit(const struct sample *s, const struct display *d) {
    if (use_control_codes)
        fprintf(stdout, "\e[1;1H\e[2J"); //Move cursor to (1,1); Clear entire screen

    if (s->experimental)
        fprintf(stdout, "Warning: Support for this PM table version is expermiental. Can't trust anything.\n");

    for (int g = 0; g < GROUP_COUNT; g++) {
        const struct group_def *group = &group_defs[g];

        if (!group_shown(s, d, g))
            continue;

        fprintf(stdout, "%s", group_begin_strings[g]);
        if (g == GROUP_SYSINFO)
            print_sysinfo(s);
        else if (g == GROUP_CORES)
            print_cores(s);
        else
            for (int r = 0; r < group->nrows; r++)
                if (row_shown(s, d, &group->rows[r]))
                    print_row(s, &group->rows[r]);
        fprintf(stdout, "%s", group_end_strings[g]);
    }

    fflush(stdout);
}

const struct output_ops box_drawing_ops = {
    .init = box_drawing_init,
    .cleanup = box_drawing_cleanup,
    .emit = box_drawing_emit,
};
