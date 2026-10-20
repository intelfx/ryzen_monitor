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

/**
 * Plain "Label: value" lines, one per row, for scripts and status bars.
 **/

#include <stdio.h>

#include "output_ops.h"

static void text_init(bool repeating, bool interactive) {
    (void)repeating;
    (void)interactive;
}

static void text_cleanup(void) {}

static void print_value(float value, enum unit unit, enum metric_kind kind) {
    if (kind == KIND_BOOL) {
        fprintf(stdout, "%s", value ? "on" : "off");
        return;
    }
    switch (unit) {
        case UNIT_COUNT:     fprintf(stdout, "%.0f", value); break;
        case UNIT_RATIO:     fprintf(stdout, "%.2f %%", value * 100.f); break;
        case UNIT_MEGAHERTZ: fprintf(stdout, "%.0f MHz", value); break;
        case UNIT_VOLTS:     fprintf(stdout, "%.3f V", value); break;
        case UNIT_AMPS:      fprintf(stdout, "%.3f A", value); break;
        case UNIT_WATTS:     fprintf(stdout, "%.3f W", value); break;
        case UNIT_CELSIUS:   fprintf(stdout, "%.2f C", value); break;
    }
}

static void print_row(const struct sample *s, const struct row *row) {
    const struct metric_def *def = &metric_defs[row->m[0]];
    float sum = 0;

    fprintf(stdout, "%s: ", row->label);
    switch (row->kind) {
        case ROW_VALUE:
            print_value(metric_value(s, row->m[0], 0), def->unit, def->kind);
            break;
        case ROW_LIMIT:
            print_value(metric_value(s, row->m[0], 0), def->unit, def->kind);
            fprintf(stdout, " of ");
            print_value(metric_limit(s, row->m[0], 0), def->unit, def->kind);
            fprintf(stdout, " (%.2f %%)", metric_value(s, row->m[0], 0) / metric_limit(s, row->m[0], 0) * 100.f);
            break;
        case ROW_MULTI:
            for (int i = 0; i < row->n; i++) {
                if (i)
                    fprintf(stdout, " | ");
                print_value(metric_value(s, row->m[i], 0), metric_defs[row->m[i]].unit, metric_defs[row->m[i]].kind);
            }
            break;
        case ROW_SUM:
            for (int i = 0; i < metric_count(s, row->m[0]); i++) {
                if (i)
                    fprintf(stdout, " + ");
                print_value(metric_value(s, row->m[0], i), def->unit, def->kind);
                sum += metric_value(s, row->m[0], i);
            }
            if (metric_count(s, row->m[0]) > 1) {
                fprintf(stdout, " = ");
                print_value(sum, def->unit, def->kind);
            }
            break;
    }
    fputc('\n', stdout);
}

static void print_sysinfo(const struct sample *s) {
    const system_info *sysinfo = s->sysinfo;

    fprintf(stdout, "CPU Model: %s\n", sysinfo->cpu_name);
    fprintf(stdout, "Processor Code Name: %s\n", sysinfo->codename);
    fprintf(stdout, "Cores: %u\n", sysinfo->cores);
    fprintf(stdout, "Core CCDs: %u\n", sysinfo->ccds);
    if (s->zen_version < 3) {
        fprintf(stdout, "Core CCXs: %u\n", sysinfo->ccxs);
        fprintf(stdout, "Cores Per CCX: %u\n", sysinfo->cores_per_ccx);
    } else
        fprintf(stdout, "Cores Per CCD: %u\n", sysinfo->cores_per_ccx);
    fprintf(stdout, "SMU FW Version: v%s\n", sysinfo->smu_fw_ver);
    fprintf(stdout, "MP1 IF Version: v%u\n", sysinfo->if_ver);
}

static void print_cores(const struct sample *s) {
    for (int i = 0; i < s->ncores; i++) {
        const struct core_state *core = &s->core[i];

        if (core->number < 0)
            continue;

        fprintf(stdout, "Core %d: ", core->number);
        if (core->disabled)
            fprintf(stdout, "disabled");
        else if (core->sleeping)
            fprintf(stdout, "sleeping");
        else
            print_value(metric_value(s, M_CORE_FREQUENCY, i), UNIT_MEGAHERTZ, KIND_GAUGE);
        fprintf(stdout, " | %.3f W | %.3f V | %.2f C | C0 %.1f %% | C1 %.1f %% | C6 %.1f %%\n",
                metric_value(s, M_CORE_POWER, i),
                metric_value(s, M_CORE_VOLTAGE, i),
                metric_value(s, M_CORE_TEMPERATURE, i),
                metric_value(s, M_CORE_C0, i) * 100.f,
                metric_value(s, M_CORE_CC1, i) * 100.f,
                metric_value(s, M_CORE_CC6, i) * 100.f);
    }
}

static void text_emit(const struct sample *s, const struct display *d) {
    for (int g = 0; g < GROUP_COUNT; g++) {
        const struct group_def *group = &group_defs[g];

        if (!group_shown(s, d, g))
            continue;

        if (g == GROUP_SYSINFO)
            print_sysinfo(s);
        else if (g == GROUP_CORES)
            print_cores(s);
        else if (g == GROUP_CO) {
            fprintf(stdout, "Curve Optimizer Counts:");
            for (int i = 0; i < s->ncores; i++)
                if (s->core[i].number >= 0 && metric_present(s, M_CORE_CO, i))
                    fprintf(stdout, " C%02d %+.0f", s->core[i].number, metric_value(s, M_CORE_CO, i));
            fputc('\n', stdout);
        }
        else
            for (int r = 0; r < group->nrows; r++)
                if (row_shown(s, d, &group->rows[r]))
                    print_row(s, &group->rows[r]);
    }

    fflush(stdout);
}

const struct output_ops text_ops = {
    .init = text_init,
    .cleanup = text_cleanup,
    .emit = text_emit,
};
