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
 * SMU messages that change or query limits and overclocking settings.
 * Message numbers differ between processor generations; smu_settings[] has
 * them per codename.
 **/

#ifndef SMU_CONTROL_H
#define SMU_CONTROL_H

#include <stdbool.h>
#include <stddef.h>

#include "pm_tables.h"
#include "readinfo.h"

enum smu_message {
    MSG_SET_FAST_LIMIT,
    MSG_SET_SLOW_LIMIT,
    MSG_SET_STAPM_LIMIT,
    MSG_SET_APU_SLOW_LIMIT,
    MSG_SET_SLOW_TIME,
    MSG_SET_STAPM_TIME,
    MSG_SET_TDC_VDD_LIMIT,
    MSG_SET_TDC_SOC_LIMIT,
    MSG_SET_EDC_VDD_LIMIT,
    MSG_SET_EDC_SOC_LIMIT,
    MSG_SET_TCTL_MAX,
    MSG_SET_PBO_SCALAR,
    MSG_GET_PBO_SCALAR,
    MSG_SET_PBO_EN,
    MSG_ENABLE_OC_MODE,
    MSG_DISABLE_OC_MODE,
    MSG_SET_DLDO_PSM_MARGIN,
    MSG_SET_ALL_DLDO_PSM_MARGIN,
    MSG_GET_DLDO_PSM_MARGIN,
    MSG_SET_POWER_SAVING,
    MSG_SET_MAX_PERFORMANCE,
    MSG_COUNT,
};

// Classes of SMU firmware, which differ in how messages are sent.
enum settings_type {
    SETTINGS_TYPE_CPU0,     // Zen
    SETTINGS_TYPE_CPU1,     // Zen+
    SETTINGS_TYPE_CPU2,     // Zen 2
    SETTINGS_TYPE_CPU3,     // Zen 3
    SETTINGS_TYPE_CPU4,     // Zen 4 and later
    SETTINGS_TYPE_APU0,     // Raven Ridge, Picasso
    SETTINGS_TYPE_APU1,     // Renoir and later
    SETTINGS_TYPE_APU2,
};

struct smu_settings {
    const char *name;
    enum settings_type type;
    int codenames[4];
    unsigned int rsmu[MSG_COUNT];   // 0: not available
    unsigned int mp1[MSG_COUNT];
};

extern const struct smu_settings smu_settings[];
extern const int smu_settings_count;

// The settings for a codename, or NULL if it has none.
const struct smu_settings *smu_settings_for(int codename);

/**
 * The limits that --get-* and --set-* work on. They are set through an SMU
 * message and read back from the PM table.
 **/
struct smu_limit {
    const char *name;               // option name after "get-"/"set-"
    const char *help;
    enum smu_message cpu_msg;       // on desktop processors, MSG_COUNT if none
    enum smu_message apu_msg;       // on APUs, MSG_COUNT if none
    unsigned int scale;             // the SMU takes value * scale
    size_t pm_field;                // offsetof(pm_table, field) of the current limit
};

extern const struct smu_limit smu_limits[];
extern const int smu_limits_count;

enum smu_control_result {
    SMU_CONTROL_OK,
    SMU_CONTROL_UNSUPPORTED,        // no such message on this processor
    SMU_CONTROL_FAILED,             // the SMU refused or did not answer
};

enum smu_control_result smu_set_limit(const struct smu_settings *s, const struct smu_limit *limit, unsigned int value);
enum smu_control_result smu_set_pbo_scalar(const struct smu_settings *s, unsigned int scalar);
enum smu_control_result smu_get_pbo_scalar(const struct smu_settings *s, float *scalar);
enum smu_control_result smu_set_oc_mode(const struct smu_settings *s, bool enable);
enum smu_control_result smu_set_power_mode(const struct smu_settings *s, bool max_performance);

/**
 * Curve Optimizer counts. A core is addressed by its number as shown (the
 * n-th enabled core); counts are clamped to what the generation supports.
 **/
int smu_co_limit(const struct smu_settings *s);
enum smu_control_result smu_get_co(const struct smu_settings *s, const system_info *sysinfo, int core, int *count);
enum smu_control_result smu_set_co(const struct smu_settings *s, const system_info *sysinfo, int core, int count);
enum smu_control_result smu_set_co_all(const struct smu_settings *s, int count);

#endif
