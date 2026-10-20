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
 * Every value the monitor reports is a metric, described once in
 * metric_defs[]. A sample holds the values of all metrics at one point in
 * time; output backends only ever read samples, so each derived value is
 * computed in exactly one place (compute_sample()).
 **/

#ifndef METRICS_H
#define METRICS_H

#include <stdbool.h>
#include <time.h>

#include "pm_tables.h"
#include "readinfo.h"

enum unit {
    UNIT_COUNT,
    UNIT_RATIO,         // 0..1
    UNIT_MEGAHERTZ,
    UNIT_VOLTS,
    UNIT_AMPS,
    UNIT_WATTS,
    UNIT_CELSIUS,
};

enum metric_kind {
    KIND_GAUGE,
    KIND_BOOL,
    KIND_LIMIT,         // a value and the limit it is held to
};

enum metric_index {
    INDEX_NONE,
    INDEX_CORE,         // one value per physical core slot of the PM table
    INDEX_L3,           // one value per L3 cache
};

enum metric_id {
    M_CORE_FREQUENCY,
    M_CORE_POWER,
    M_CORE_VOLTAGE,
    M_CORE_TEMPERATURE,
    M_CORE_C0,
    M_CORE_CC1,
    M_CORE_CC6,
    M_CORE_CO,

    M_PEAK_CORE_FREQUENCY,
    M_PEAK_CORE_TEMPERATURE,
    M_PEAK_CORE_VOLTAGE,
    M_AVERAGE_CORE_VOLTAGE,
    M_AVERAGE_CORE_CC6,
    M_TOTAL_CORE_POWER,
    M_AVERAGE_CORE_TEMPERATURE,
    M_AVERAGE_CORE_FREQUENCY,
    M_AVERAGE_CORE_C0,
    M_PACKAGE_CC6,

    M_PEAK_TEMPERATURE,
    M_SOC_TEMPERATURE,
    M_GFX_TEMPERATURE,
    M_VID,
    M_PPT,
    M_PPT_APU,
    M_TDC,
    M_TDC_ACTUAL,
    M_TDC_SOC,
    M_EDC,
    M_EDC_SOC,
    M_THM,
    M_THM_SOC,
    M_THM_GFX,
    M_FIT,

    M_MEMORY_COUPLED,
    M_FCLK_AVERAGE,
    M_FCLK,
    M_UCLK,
    M_MCLK,
    M_CLDO_VDDM,
    M_CLDO_VDDP,
    M_CLDO_VDDG,
    M_CLDO_VDDG_IOD,
    M_CLDO_VDDG_CCD,

    M_GFX_VOLTAGE,
    M_GFX_FREQUENCY,
    M_GFX_FREQUENCY_EFFECTIVE,
    M_GFX_BUSY,
    M_GFX_EDC_LIMIT,
    M_GFX_EDC_RESIDENCY,
    M_DISPLAY_COUNT,
    M_FPS,
    M_DGPU_POWER,
    M_DGPU_FREQUENCY_TARGET,
    M_DGPU_BUSY,

    M_VDDCR_SOC_POWER,
    M_IO_VDDCR_SOC_POWER,
    M_GMI2_VDDG_POWER,
    M_ROC_POWER,
    M_L3_LOGIC_POWER,
    M_L3_VDDM_POWER,
    M_VDDIO_MEM_POWER,
    M_IOD_VDDIO_MEM_POWER,
    M_DDR_VDDP_POWER,
    M_DDR_PHY_POWER,
    M_VDD18_POWER,
    M_IO_DISPLAY_POWER,
    M_IO_USB_POWER,
    M_CALCULATED_THERMAL_OUTPUT,

    M_SOC_TELEMETRY_VOLTAGE,
    M_SOC_TELEMETRY_CURRENT,
    M_SOC_TELEMETRY_POWER,
    M_CORE_TELEMETRY_VOLTAGE,
    M_CORE_TELEMETRY_CURRENT,
    M_CORE_TELEMETRY_POWER,
    M_SMU_CORE_POWER,
    M_SMU_SOCKET_POWER,
    M_SMU_PACKAGE_POWER,

    M_COUNT,
};

struct metric_def {
    const char *key;        // unique snake_case name, used by all machine-readable outputs
    const char *help;
    enum unit unit;
    enum metric_kind kind;
    enum metric_index index;
};

extern const struct metric_def metric_defs[M_COUNT];

#define METRIC_MAX_INDEX PMT_MAX_NUM_CORES

struct metric_value {
    unsigned int present;   // bit i set: value[i] (and limit[i]) are valid
    float value[METRIC_MAX_INDEX];
    float limit[METRIC_MAX_INDEX];
};

struct core_state {
    bool disabled;
    bool sleeping;
    int number;             // number shown to the user, -1 if the core is not shown
};

struct sample {
    struct timespec monotonic_raw;
    struct timespec realtime;

    const system_info *sysinfo;
    unsigned int pm_table_version;
    int zen_version;
    bool experimental;
    bool has_graphics;

    int ncores;             // core slots in the PM table
    int nl3;                // L3 caches in the PM table
    struct core_state core[PMT_MAX_NUM_CORES];

    struct metric_value m[M_COUNT];
};

struct sample_options {
    bool show_disabled_cores;
    // The SMU reports clocks against a nominal 100 MHz reference clock; on a
    // system whose reference clock runs elsewhere they are scaled by refclk/100.
    double refclk_mhz;
    // Curve Optimizer counts per core slot, where co_present has the bit set
    const int *co_counts;
    unsigned int co_present;
};

void compute_sample(const pm_table *pmt, const system_info *sysinfo,
        const struct sample_options *opts, struct sample *s);

static inline bool metric_present(const struct sample *s, enum metric_id id, int i) {
    return s->m[id].present & (1u << i);
}

static inline float metric_value(const struct sample *s, enum metric_id id, int i) {
    return s->m[id].value[i];
}

static inline float metric_limit(const struct sample *s, enum metric_id id, int i) {
    return s->m[id].limit[i];
}

// Number of index slots a metric has in this sample.
static inline int metric_count(const struct sample *s, enum metric_id id) {
    switch (metric_defs[id].index) {
        case INDEX_CORE: return s->ncores;
        case INDEX_L3:   return s->nl3;
        default:         return 1;
    }
}

#endif
