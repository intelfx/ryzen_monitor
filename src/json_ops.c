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

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "output_ops.h"

// floats are written with enough precision to round-trip, to allow
// differentiating between values that are similar but different in each
// sample (probably measured) and values that are exactly the same in each
// sample (static or calculated).

// JSON does not allow trailing commas, so we write commas before each
// non-initial element in an object.
static void maybe_comma(int* counter) {
    if ((*counter)++)
        fprintf(stdout, ",");
}

static void json_init(bool repeating, bool interactive) {
    (void)repeating;
    (void)interactive;
}

static void json_cleanup(void) {}

static void print_number(float v) {
    if (isfinite(v))
        fprintf(stdout, "%.9g", v);
    else
        fprintf(stdout, "null");
}

static void print_string(const char *str) {
    fputc('"', stdout);
    for (; *str; str++) {
        if (*str == '"' || *str == '\\')
            fputc('\\', stdout);
        if ((unsigned char)*str < 0x20)
            fprintf(stdout, "\\u%04x", *str);
        else
            fputc(*str, stdout);
    }
    fputc('"', stdout);
}

static void print_key(int *counter, const char *key) {
    maybe_comma(counter);
    print_string(key);
    fputc(':', stdout);
}

static void print_sysinfo(const struct sample *s) {
    const system_info *sysinfo = s->sysinfo;
    int n = 0;

    print_key(&n, "model"); print_string(sysinfo->cpu_name);
    print_key(&n, "codename"); print_string(sysinfo->codename);
    print_key(&n, "cores"); fprintf(stdout, "%u", sysinfo->cores);
    print_key(&n, "ccds"); fprintf(stdout, "%u", sysinfo->ccds);
    if (s->zen_version < 3) {
        print_key(&n, "ccxs"); fprintf(stdout, "%u", sysinfo->ccxs);
        print_key(&n, "cores_per_ccx"); fprintf(stdout, "%u", sysinfo->cores_per_ccx);
    } else {
        print_key(&n, "cores_per_ccd"); fprintf(stdout, "%u", sysinfo->cores_per_ccx);
    }
    print_key(&n, "smu_firmware_version"); print_string(sysinfo->smu_fw_ver);
    print_key(&n, "mp1_if_version"); fprintf(stdout, "%u", sysinfo->if_ver);
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

static void print_cores(const struct sample *s) {
    int n = 0;

    for (int i = 0; i < s->ncores; i++) {
        const struct core_state *core = &s->core[i];
        int m = 0;

        if (core->number < 0)
            continue;

        maybe_comma(&n);
        fputc('{', stdout);
        print_key(&m, "number"); fprintf(stdout, "%d", core->number);
        print_key(&m, "disabled"); fprintf(stdout, "%s", core->disabled ? "true" : "false");
        print_key(&m, "sleeping"); fprintf(stdout, "%s", core->sleeping ? "true" : "false");
        for (size_t j = 0; j < sizeof(core_metrics) / sizeof(core_metrics[0]); j++) {
            enum metric_id id = core_metrics[j];
            // Inside a core object the "core_" prefix is redundant.
            print_key(&m, metric_defs[id].key + strlen("core_"));
            print_number(metric_value(s, id, i));
        }
        fputc('}', stdout);
    }
}

static void print_metric(int *n, const struct sample *s, enum metric_id id) {
    const struct metric_def *def = &metric_defs[id];

    print_key(n, def->key);
    if (def->kind == KIND_BOOL)
        fprintf(stdout, "%s", metric_value(s, id, 0) ? "true" : "false");
    else
        print_number(metric_value(s, id, 0));
}

static void print_row(int *n, const struct sample *s, const struct row *row) {
    enum metric_id id = row->m[0];
    float sum = 0;

    switch (row->kind) {
        case ROW_VALUE:
            print_metric(n, s, id);
            break;
        case ROW_LIMIT:
            print_key(n, metric_defs[id].key);
            fprintf(stdout, "{\"value\":");
            print_number(metric_value(s, id, 0));
            fprintf(stdout, ",\"limit\":");
            print_number(metric_limit(s, id, 0));
            fprintf(stdout, ",\"usage\":");
            print_number(metric_value(s, id, 0) / metric_limit(s, id, 0));
            fputc('}', stdout);
            break;
        case ROW_MULTI:
            for (int i = 0; i < row->n; i++)
                if (metric_present(s, row->m[i], 0))
                    print_metric(n, s, row->m[i]);
            break;
        case ROW_SUM:
            print_key(n, metric_defs[id].key);
            fprintf(stdout, "{\"values\":[");
            for (int i = 0; i < metric_count(s, id); i++) {
                if (i)
                    fputc(',', stdout);
                print_number(metric_value(s, id, i));
                sum += metric_value(s, id, i);
            }
            fprintf(stdout, "],\"sum\":");
            print_number(sum);
            fputc('}', stdout);
            break;
    }
}

static void json_print(const struct sample *s) {
    int top = 0;

    fputc('{', stdout);
    print_key(&top, "timestamp");
    fprintf(stdout, "%lld", (long long)s->monotonic_raw.tv_sec * 1000000000 + s->monotonic_raw.tv_nsec);
    if (s->experimental) {
        print_key(&top, "experimental");
        fprintf(stdout, "true");
    }

    for (int g = 0; g < GROUP_COUNT; g++) {
        const struct group_def *group = &group_defs[g];
        int n = 0;

        if (!group_shown(s, g))
            continue;

        print_key(&top, group->key);
        if (g == GROUP_CORES) {
            fputc('[', stdout);
            print_cores(s);
            fputc(']', stdout);
            continue;
        }

        fputc('{', stdout);
        if (g == GROUP_SYSINFO)
            print_sysinfo(s);
        else
            for (int r = 0; r < group->nrows; r++)
                if (row_shown(s, &group->rows[r]))
                    print_row(&n, s, &group->rows[r]);
        fputc('}', stdout);
    }

    fputc('}', stdout);
}

static void json_emit(const struct sample *s) {
    json_print(s);
    fflush(stdout);
}

static void ndjson_emit(const struct sample *s) {
    json_print(s);
    fputc('\n', stdout);
    fflush(stdout);
}

const struct output_ops json_ops = {
    .init = json_init,
    .cleanup = json_cleanup,
    .emit = json_emit,
};

const struct output_ops ndjson_ops = {
    .init = json_init,
    .cleanup = json_cleanup,
    .emit = ndjson_emit,
};
