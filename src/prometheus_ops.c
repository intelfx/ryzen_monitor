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
 * Prometheus text exposition format, one complete exposition per sample.
 * With --once, the output can be dropped into node_exporter's textfile
 * collector directory. Values are converted to base units as Prometheus
 * naming conventions ask (hertz rather than megahertz, ratios rather than
 * percentages), and the unit is appended to the metric name.
 **/

#include <math.h>
#include <stdio.h>

#include "output_ops.h"

#define PREFIX "ryzen_monitor_"

static void prometheus_init(bool repeating, bool interactive) {
    (void)repeating;
    (void)interactive;
}

static void prometheus_cleanup(void) {}

static const char *unit_suffix(enum unit unit) {
    switch (unit) {
        case UNIT_RATIO:     return "_ratio";
        case UNIT_MEGAHERTZ: return "_hertz";
        case UNIT_VOLTS:     return "_volts";
        case UNIT_AMPS:      return "_amperes";
        case UNIT_WATTS:     return "_watts";
        case UNIT_CELSIUS:   return "_celsius";
        default:             return "";
    }
}

static double base_value(float value, enum unit unit) {
    return unit == UNIT_MEGAHERTZ ? value * 1e6 : value;
}

static void print_label_value(const char *str) {
    fputc('"', stdout);
    for (; *str; str++) {
        if (*str == '"' || *str == '\\')
            fputc('\\', stdout);
        if (*str == '\n')
            fputs("\\n", stdout);
        else
            fputc(*str, stdout);
    }
    fputc('"', stdout);
}

static void print_family(enum metric_id id, bool limit) {
    const struct metric_def *def = &metric_defs[id];
    const char *suffix = unit_suffix(def->unit);
    const char *part = limit ? "_limit" : "";

    fprintf(stdout, "# HELP " PREFIX "%s%s%s %s%s\n", def->key, part, suffix, def->help, limit ? ", limit" : "");
    fprintf(stdout, "# TYPE " PREFIX "%s%s%s gauge\n", def->key, part, suffix);
}

static void print_sample(enum metric_id id, bool limit, const char *label, int index, float value) {
    const struct metric_def *def = &metric_defs[id];

    fprintf(stdout, PREFIX "%s%s%s", def->key, limit ? "_limit" : "", unit_suffix(def->unit));
    if (label)
        fprintf(stdout, "{%s=\"%d\"}", label, index);
    if (isfinite(value))
        fprintf(stdout, " %.9g\n", base_value(value, def->unit));
    else
        fprintf(stdout, " NaN\n");
}

static void print_metric(const struct sample *s, enum metric_id id) {
    const struct metric_def *def = &metric_defs[id];

    for (int part = 0; part < (def->kind == KIND_LIMIT ? 2 : 1); part++) {
        bool limit = part == 1;
        bool header = false;

        for (int i = 0; i < metric_count(s, id); i++) {
            const char *label = NULL;
            int index = i;

            if (!metric_present(s, id, i))
                continue;
            if (def->index == INDEX_CORE) {
                // Disabled cores have nothing to report.
                if (s->core[i].number < 0 || s->core[i].disabled)
                    continue;
                label = "core";
                index = s->core[i].number;
            } else if (def->index == INDEX_L3) {
                label = "l3";
            }

            if (!header) {
                print_family(id, limit);
                header = true;
            }
            print_sample(id, limit, label, index,
                    limit ? metric_limit(s, id, i) : metric_value(s, id, i));
        }
    }
}

static const enum metric_id core_metrics[] = {
    M_CORE_FREQUENCY,
    M_CORE_POWER,
    M_CORE_VOLTAGE,
    M_CORE_TEMPERATURE,
    M_CORE_C0,
    M_CORE_CC1,
    M_CORE_CC6,
};

static void prometheus_emit(const struct sample *s, const struct display *d) {
    bool selected[M_COUNT] = { 0 };

    if (group_shown(s, d, GROUP_SYSINFO)) {
        const system_info *sysinfo = s->sysinfo;

        fprintf(stdout, "# HELP " PREFIX "cpu_info Processor identification\n");
        fprintf(stdout, "# TYPE " PREFIX "cpu_info gauge\n");
        fprintf(stdout, PREFIX "cpu_info{model=");
        print_label_value(sysinfo->cpu_name);
        fprintf(stdout, ",codename=");
        print_label_value(sysinfo->codename);
        fprintf(stdout, ",smu_firmware_version=");
        print_label_value(sysinfo->smu_fw_ver);
        fprintf(stdout, ",pm_table_version=\"0x%x\"} 1\n", s->pm_table_version);
        fprintf(stdout, "# HELP " PREFIX "cores Enabled cores\n");
        fprintf(stdout, "# TYPE " PREFIX "cores gauge\n");
        fprintf(stdout, PREFIX "cores %u\n", sysinfo->cores);
        fprintf(stdout, "# HELP " PREFIX "ccds Core complex dies\n");
        fprintf(stdout, "# TYPE " PREFIX "ccds gauge\n");
        fprintf(stdout, PREFIX "ccds %u\n", sysinfo->ccds);
    }

    // Every metric once, even if several shown groups contain it.
    if (group_shown(s, d, GROUP_CORES))
        for (size_t i = 0; i < sizeof(core_metrics) / sizeof(core_metrics[0]); i++)
            selected[core_metrics[i]] = true;
    for (int g = 0; g < GROUP_COUNT; g++) {
        const struct group_def *group = &group_defs[g];

        if (!group_shown(s, d, g))
            continue;
        for (int r = 0; r < group->nrows; r++)
            for (int i = 0; i < group->rows[r].n; i++)
                selected[group->rows[r].m[i]] = true;
    }

    for (int id = 0; id < M_COUNT; id++)
        if (selected[id])
            print_metric(s, id);

    fflush(stdout);
}

const struct output_ops prometheus_ops = {
    .init = prometheus_init,
    .cleanup = prometheus_cleanup,
    .emit = prometheus_emit,
};
