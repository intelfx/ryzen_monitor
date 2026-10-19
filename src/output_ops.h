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

#ifndef OUTPUT_OPS_H
#define OUTPUT_OPS_H

#include <stdbool.h>

#include "metrics.h"

/**
 * The layout every output follows: groups of rows, each row showing one
 * to three metrics. The human-readable output prints a row per entry; the
 * machine-readable outputs use the groups to structure their output and
 * the row kinds to decide how to encode the metrics.
 **/

enum group {
    GROUP_SYSINFO,
    GROUP_CORES,
    GROUP_CORE_STATS_CALC,
    GROUP_CORE_STATS_SMU,
    GROUP_LIMITS,
    GROUP_MEMORY,
    GROUP_GRAPHICS,
    GROUP_POWER,
    GROUP_POWER_REPORTS,
    GROUP_SUMMARY,
    GROUP_COUNT,
};

enum row_kind {
    ROW_VALUE,          // m[0]
    ROW_LIMIT,          // m[0], a KIND_LIMIT metric: value, limit and usage
    ROW_MULTI,          // m[0..n-1] side by side
    ROW_SUM,            // m[0], an INDEX_L3 metric: all values and their sum
};

struct row {
    enum row_kind kind;
    const char *label;
    int n;
    enum metric_id m[3];
    bool blank_before;
    bool detail;        // hidden in compact mode
};

struct group_def {
    const char *key;
    const struct row *rows;     // NULL for groups with their own layout (sysinfo, cores)
    int nrows;
};

extern const struct group_def group_defs[GROUP_COUNT];

/**
 * Views are the units the user selects: each shows one or more groups.
 **/

enum view {
    VIEW_INFO,
    VIEW_CORES,
    VIEW_STATS,
    VIEW_ELECTRICAL,
    VIEW_MEMORY,
    VIEW_GFX,
    VIEW_POWER,
    VIEW_SUMMARY,
    VIEW_COUNT,
};

struct view_def {
    const char *key;
    char hotkey;                // toggles the view in the interactive monitor
    unsigned int groups;        // bit per enum group
};

extern const struct view_def view_defs[VIEW_COUNT];

#define VIEWS_ALL ((1u << VIEW_COUNT) - 1)
#define VIEWS_DEFAULT (VIEWS_ALL & ~(1u << VIEW_SUMMARY))

// What the user asked to see.
struct display {
    unsigned int views;         // bit per enum view
    bool compact;               // leave out details (rows marked as such, system information)
};

// Parse a comma-separated list of view names into a view mask; returns false on unknown names.
bool parse_views(const char *list, unsigned int *views);

// Whether a group is selected and has anything to show in this sample.
bool group_shown(const struct sample *s, const struct display *d, enum group group);
// Whether a row has anything to show in this sample.
bool row_shown(const struct sample *s, const struct display *d, const struct row *row);

struct output_ops {
    void (*init)(bool repeating, bool interactive);
    void (*cleanup)(void);
    void (*emit)(const struct sample *s, const struct display *d);
};

extern const struct output_ops box_drawing_ops;
extern const struct output_ops json_ops;
extern const struct output_ops ndjson_ops;
extern const struct output_ops text_ops;

#endif
