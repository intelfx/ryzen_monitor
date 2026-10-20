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
 * Netdata external plugin protocol (plugins.d). Charts are defined from the
 * first sample: one per group and unit, and one per per-core or per-L3
 * metric with a dimension per core or cache. Netdata collects integers, so
 * values are sent multiplied by MULTIPLIER and divided back by the
 * dimension's divisor.
 **/

#include <math.h>
#include <stdio.h>

#include "output_ops.h"

#define TYPE "ryzen_monitor"
#define MULTIPLIER 1000
#define MAX_CHARTS 64
#define MAX_DIMENSIONS 32

struct dimension {
    enum metric_id id;
    int index;              // core slot or L3 cache
    bool limit;             // the limit of a KIND_LIMIT metric
};

struct chart {
    char id[64];
    int ndims;
    struct dimension dims[MAX_DIMENSIONS];
};

static struct chart charts[MAX_CHARTS];
static int ncharts;
static bool defined;
static int update_every = 1;

static void netdata_init(bool repeating, bool interactive) {
    (void)repeating;
    (void)interactive;
}

static void netdata_cleanup(void) {}

static const char *unit_name(enum unit unit) {
    switch (unit) {
        case UNIT_RATIO:     return "percentage";
        case UNIT_MEGAHERTZ: return "MHz";
        case UNIT_VOLTS:     return "V";
        case UNIT_AMPS:      return "A";
        case UNIT_WATTS:     return "W";
        case UNIT_CELSIUS:   return "Celsius";
        default:             return "count";
    }
}

static const char *unit_id(enum unit unit) {
    switch (unit) {
        case UNIT_RATIO:     return "ratio";
        case UNIT_MEGAHERTZ: return "frequency";
        case UNIT_VOLTS:     return "voltage";
        case UNIT_AMPS:      return "current";
        case UNIT_WATTS:     return "power";
        case UNIT_CELSIUS:   return "temperature";
        default:             return "count";
    }
}

static void dimension_id(char *buf, size_t len, const struct dimension *dim) {
    const struct metric_def *def = &metric_defs[dim->id];

    if (def->index == INDEX_NONE)
        snprintf(buf, len, "%s%s", def->key, dim->limit ? "_limit" : "");
    else
        snprintf(buf, len, "%s%d", def->index == INDEX_CORE ? "core" : "l3_", dim->index);
}

static struct chart *new_chart(void) {
    return ncharts < MAX_CHARTS ? &charts[ncharts++] : NULL;
}

static void add_dimension(struct chart *chart, enum metric_id id, int index, bool limit) {
    for (int i = 0; i < chart->ndims; i++)
        if (chart->dims[i].id == id && chart->dims[i].index == index && chart->dims[i].limit == limit)
            return;
    if (chart->ndims < MAX_DIMENSIONS)
        chart->dims[chart->ndims++] = (struct dimension){ id, index, limit };
}

static void print_chart(const struct chart *chart, const char *title, const char *units, const char *family, int priority) {
    fprintf(stdout, "CHART " TYPE ".%s '' '%s' '%s' '%s' '" TYPE ".%s' line %d %d\n",
            chart->id, title, units, family, chart->id, priority, update_every);
    for (int i = 0; i < chart->ndims; i++) {
        const struct metric_def *def = &metric_defs[chart->dims[i].id];
        char id[64];
        int multiplier = def->unit == UNIT_RATIO ? 100 : 1;

        dimension_id(id, sizeof(id), &chart->dims[i]);
        fprintf(stdout, "DIMENSION %s '' absolute %d %d\n", id, multiplier, MULTIPLIER);
    }
}

// Per-core and per-L3 metrics: a chart each, with a dimension per core or cache.
static void define_indexed(const struct sample *s, enum metric_id id, const char *family, int priority) {
    const struct metric_def *def = &metric_defs[id];
    struct chart *chart = new_chart();

    if (!chart)
        return;
    snprintf(chart->id, sizeof(chart->id), "%s", def->key);
    for (int i = 0; i < metric_count(s, id); i++) {
        if (!metric_present(s, id, i))
            continue;
        if (def->index == INDEX_CORE && (s->core[i].number < 0 || s->core[i].disabled))
            continue;
        add_dimension(chart, id, i, false);
    }
    if (!chart->ndims) {
        ncharts--;
        return;
    }
    print_chart(chart, def->help, unit_name(def->unit), family, priority);
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

static void define_charts(const struct sample *s, const struct display *d) {
    int priority = 90000;

    if (group_shown(s, d, GROUP_CORES))
        for (size_t i = 0; i < sizeof(core_metrics) / sizeof(core_metrics[0]); i++)
            define_indexed(s, core_metrics[i], "cores", priority++);
    if (group_shown(s, d, GROUP_CO))
        define_indexed(s, M_CORE_CO, "co", priority++);

    for (int g = 0; g < GROUP_COUNT; g++) {
        const struct group_def *group = &group_defs[g];

        if (!group_shown(s, d, g) || !group->rows)
            continue;

        // One chart per unit; Netdata charts cannot mix units.
        for (enum unit unit = UNIT_COUNT; unit <= UNIT_CELSIUS; unit++) {
            struct chart *chart = new_chart();

            if (!chart)
                break;
            snprintf(chart->id, sizeof(chart->id), "%s_%s", group->key, unit_id(unit));
            for (int r = 0; r < group->nrows; r++) {
                const struct row *row = &group->rows[r];

                for (int i = 0; i < row->n; i++) {
                    enum metric_id id = row->m[i];
                    const struct metric_def *def = &metric_defs[id];

                    if (def->index != INDEX_NONE || def->unit != unit || !metric_present(s, id, 0))
                        continue;
                    add_dimension(chart, id, 0, false);
                    if (def->kind == KIND_LIMIT)
                        add_dimension(chart, id, 0, true);
                }
            }
            if (!chart->ndims) {
                ncharts--;
                continue;
            }
            print_chart(chart, group->title, unit_name(unit), group->key, priority++);
        }

        for (int r = 0; r < group->nrows; r++)
            if (group->rows[r].kind == ROW_SUM && metric_present(s, group->rows[r].m[0], 0))
                define_indexed(s, group->rows[r].m[0], group->key, priority++);
    }
}

static void netdata_emit(const struct sample *s, const struct display *d) {
    if (!defined) {
        define_charts(s, d);
        defined = true;
    }

    for (int c = 0; c < ncharts; c++) {
        const struct chart *chart = &charts[c];

        fprintf(stdout, "BEGIN " TYPE ".%s\n", chart->id);
        for (int i = 0; i < chart->ndims; i++) {
            const struct dimension *dim = &chart->dims[i];
            float value = dim->limit ? metric_limit(s, dim->id, dim->index) : metric_value(s, dim->id, dim->index);
            char id[64];

            // Netdata has no NaN; a dimension that is not set shows a gap.
            if (!metric_present(s, dim->id, dim->index) || !isfinite(value))
                continue;
            dimension_id(id, sizeof(id), dim);
            fprintf(stdout, "SET %s = %lld\n", id, llround((double)value * MULTIPLIER));
        }
        fprintf(stdout, "END\n");
    }

    fflush(stdout);
}

void netdata_set_update_every(int seconds) {
    update_every = seconds > 0 ? seconds : 1;
}

const struct output_ops netdata_ops = {
    .init = netdata_init,
    .cleanup = netdata_cleanup,
    .emit = netdata_emit,
};
