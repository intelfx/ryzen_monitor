/**
 * Ryzen SMU Userspace Sensor Monitor
 * Copyright (C) 2020-2021
 *    Florian Huehn <hattedsquirrel@gmail.com> (https://hattedsquirrel.net)
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

#include <stdlib.h>
#include <string.h>

#include "output_ops.h"

#define ARRAY_SIZE(a) ((int)(sizeof(a) / sizeof((a)[0])))

#define VALUE(l, m0)            { ROW_VALUE, l, 1, { m0 }, false }
#define LIMIT(l, m0)            { ROW_LIMIT, l, 1, { m0 }, false }
#define MULTI2(l, m0, m1)       { ROW_MULTI, l, 2, { m0, m1 }, false }
#define MULTI3(l, m0, m1, m2)   { ROW_MULTI, l, 3, { m0, m1, m2 }, false }
#define SUM(l, m0)              { ROW_SUM, l, 1, { m0 }, false }
#define VALUE_BLANK_BEFORE(l, m0) { ROW_VALUE, l, 1, { m0 }, true }

static const struct row core_stats_calc_rows[] = {
    VALUE("Highest Effective Core Frequency", M_PEAK_CORE_FREQUENCY),
    VALUE("Highest Core Temperature", M_PEAK_CORE_TEMPERATURE),
    VALUE("Highest Core Voltage", M_PEAK_CORE_VOLTAGE),
    VALUE("Average Core Voltage", M_AVERAGE_CORE_VOLTAGE),
    VALUE("Average Core CC6", M_AVERAGE_CORE_CC6),
    VALUE("Total Core Power Sum", M_TOTAL_CORE_POWER),
};

static const struct row core_stats_smu_rows[] = {
    //SOCKET_POWER reported in GROUP_POWER
    VALUE("Peak Core Voltage", M_CORE_TELEMETRY_VOLTAGE),
    VALUE("Package CC6", M_PACKAGE_CC6),
};

static const struct row limits_rows[] = {
    VALUE("Peak Temperature", M_PEAK_TEMPERATURE),
    VALUE("SoC Temperature", M_SOC_TEMPERATURE),
    VALUE("GFX Temperature", M_GFX_TEMPERATURE),
    LIMIT("Voltage from Core VRM", M_VID),
    //STAPM_VALUE / STAPM_LIMIT not reported with no explanation
    LIMIT("PPT", M_PPT),
    LIMIT("PPT APU", M_PPT_APU),
    LIMIT("TDC Value", M_TDC),
    LIMIT("TDC Actual", M_TDC_ACTUAL),
    LIMIT("TDC Value, SoC only", M_TDC_SOC),
    LIMIT("EDC", M_EDC),
    LIMIT("EDC, SoC only", M_EDC_SOC),
    LIMIT("THM", M_THM),
    LIMIT("THM SoC", M_THM_SOC),
    LIMIT("THM GFX", M_THM_GFX),
    LIMIT("FIT", M_FIT),
};

static const struct row memory_rows[] = {
    VALUE("Coupled Mode", M_MEMORY_COUPLED),
    VALUE("Fabric Clock (Average)", M_FCLK_AVERAGE),
    VALUE("Fabric Clock", M_FCLK),
    VALUE("Uncore Clock", M_UCLK),
    VALUE("Memory Clock", M_MCLK),
    // VDDIO_MEM_POWER reported in GROUP_POWER
    VALUE("cLDO_VDDM", M_CLDO_VDDM),
    VALUE("cLDO_VDDP", M_CLDO_VDDP),
    VALUE("cLDO_VDDG", M_CLDO_VDDG),
    VALUE("cLDO_VDDG_IOD", M_CLDO_VDDG_IOD),
    VALUE("cLDO_VDDG_CCD", M_CLDO_VDDG_CCD),
};

static const struct row graphics_rows[] = {
    MULTI2("GFX Voltage | ROC Power", M_GFX_VOLTAGE, M_ROC_POWER),
    VALUE("GFX Temperature", M_GFX_TEMPERATURE),
    MULTI2("GFX Clock Real | Effective", M_GFX_FREQUENCY, M_GFX_FREQUENCY_EFFECTIVE),
    VALUE("GFX Busy", M_GFX_BUSY),
    MULTI2("GFX EDC Limit | Residency", M_GFX_EDC_LIMIT, M_GFX_EDC_RESIDENCY),
    MULTI2("Display Count | FPS", M_DISPLAY_COUNT, M_FPS),
    MULTI3("DGPU Power | Freq Target | Busy", M_DGPU_POWER, M_DGPU_FREQUENCY_TARGET, M_DGPU_BUSY),
};

static const struct row power_rows[] = {
    //These powers are drawn via VDDCR_SOC and VDDCR_CPU and thus are pulled from the CPU power connector of the mainboard
    VALUE("Total Core Power Sum", M_TOTAL_CORE_POWER),
    VALUE("VDDCR_SOC Power", M_VDDCR_SOC_POWER),
    VALUE("IO VDDCR_SOC Power", M_IO_VDDCR_SOC_POWER),
    VALUE("GMI2_VDDG Power", M_GMI2_VDDG_POWER),
    VALUE("ROC Power", M_ROC_POWER),
    SUM("L3 Logic Power", M_L3_LOGIC_POWER),
    SUM("L3 VDDM Power", M_L3_VDDM_POWER),
    //These powers are supplied by other power lines to the CPU and are drawn from the 24 pin ATX connector on most boards
    VALUE_BLANK_BEFORE("VDDIO_MEM Power", M_VDDIO_MEM_POWER),
    VALUE("IOD_VDDIO_MEM Power", M_IOD_VDDIO_MEM_POWER),
    VALUE("DDR_VDDP Power", M_DDR_VDDP_POWER),
    VALUE("DDR Phy Power", M_DDR_PHY_POWER),
    VALUE("VDD18 Power", M_VDD18_POWER),
    VALUE("CPU Display IO Power", M_IO_DISPLAY_POWER),
    VALUE("CPU USB IO Power", M_IO_USB_POWER),
    VALUE_BLANK_BEFORE("Calculated Thermal Output", M_CALCULATED_THERMAL_OUTPUT),
};

static const struct row power_reports_rows[] = {
    //ROC_POWER reported in GROUP_GRAPHICS and GROUP_POWER
    MULTI3("SoC Power (SVI2)", M_SOC_TELEMETRY_VOLTAGE, M_SOC_TELEMETRY_CURRENT, M_SOC_TELEMETRY_POWER),
    MULTI3("Core Power (SVI2)", M_CORE_TELEMETRY_VOLTAGE, M_CORE_TELEMETRY_CURRENT, M_CORE_TELEMETRY_POWER),
    VALUE("Core Power (SMU)", M_SMU_CORE_POWER),
    VALUE("Socket Power (SMU)", M_SMU_SOCKET_POWER),
    VALUE("Package Power (SMU)", M_SMU_PACKAGE_POWER),
};

#define ROWS(r) r, ARRAY_SIZE(r)

const struct group_def group_defs[GROUP_COUNT] = {
    [GROUP_SYSINFO]         = { "sysinfo", NULL, 0 },
    [GROUP_CORES]           = { "cores", NULL, 0 },
    [GROUP_CORE_STATS_CALC] = { "core_stats_calc", ROWS(core_stats_calc_rows) },
    [GROUP_CORE_STATS_SMU]  = { "core_stats_reports", ROWS(core_stats_smu_rows) },
    [GROUP_LIMITS]          = { "limits", ROWS(limits_rows) },
    [GROUP_MEMORY]          = { "memory", ROWS(memory_rows) },
    [GROUP_GRAPHICS]        = { "graphics", ROWS(graphics_rows) },
    [GROUP_POWER]           = { "power", ROWS(power_rows) },
    [GROUP_POWER_REPORTS]   = { "power_reports", ROWS(power_reports_rows) },
};

#define GROUP(g) (1u << (g))

const struct view_def view_defs[VIEW_COUNT] = {
    [VIEW_INFO]       = { "info", GROUP(GROUP_SYSINFO) },
    [VIEW_CORES]      = { "cores", GROUP(GROUP_CORES) },
    [VIEW_STATS]      = { "stats", GROUP(GROUP_CORE_STATS_CALC) | GROUP(GROUP_CORE_STATS_SMU) },
    [VIEW_ELECTRICAL] = { "electrical", GROUP(GROUP_LIMITS) },
    [VIEW_MEMORY]     = { "memory", GROUP(GROUP_MEMORY) },
    [VIEW_GFX]        = { "gfx", GROUP(GROUP_GRAPHICS) },
    [VIEW_POWER]      = { "power", GROUP(GROUP_POWER) | GROUP(GROUP_POWER_REPORTS) },
};

bool parse_views(const char *list, unsigned int *views) {
    char *copy = strdup(list), *save = NULL;
    bool ok = true;

    *views = 0;
    for (char *name = strtok_r(copy, ",", &save); name; name = strtok_r(NULL, ",", &save)) {
        int v;

        if (!strcmp(name, "all")) {
            *views |= VIEWS_ALL;
            continue;
        }
        for (v = 0; v < VIEW_COUNT; v++)
            if (!strcmp(name, view_defs[v].key))
                break;
        if (v == VIEW_COUNT) {
            ok = false;
            break;
        }
        *views |= 1u << v;
    }

    free(copy);
    return ok;
}

bool group_shown(const struct sample *s, const struct display *d, enum group group) {
    unsigned int groups = 0;

    for (int v = 0; v < VIEW_COUNT; v++)
        if (d->views & (1u << v))
            groups |= view_defs[v].groups;
    if (!(groups & GROUP(group)))
        return false;

    switch (group) {
        case GROUP_SYSINFO:  return s->sysinfo->available;
        case GROUP_GRAPHICS: return s->has_graphics;
        default:             return true;
    }
}

bool row_shown(const struct sample *s, const struct display *d, const struct row *row) {
    (void)d;
    return metric_present(s, row->m[0], 0);
}
