/**
 * Ryzen SMU Userspace Sensor Monitor
 * Copyright (C) 2020-2021
 *    Florian Huehn <hattedsquirrel@gmail.com> (https://hattedsquirrel.net)
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
#include <string.h>

#include "metrics.h"

#define G(k, h, u)      { .key = k, .help = h, .unit = u, .kind = KIND_GAUGE, .index = INDEX_NONE }
#define B(k, h)         { .key = k, .help = h, .unit = UNIT_COUNT, .kind = KIND_BOOL, .index = INDEX_NONE }
#define L(k, h, u)      { .key = k, .help = h, .unit = u, .kind = KIND_LIMIT, .index = INDEX_NONE }
#define C(k, h, u)      { .key = k, .help = h, .unit = u, .kind = KIND_GAUGE, .index = INDEX_CORE }
#define C3(k, h, u)     { .key = k, .help = h, .unit = u, .kind = KIND_GAUGE, .index = INDEX_L3 }

const struct metric_def metric_defs[M_COUNT] = {
    [M_CORE_FREQUENCY]            = C("core_frequency", "Effective core frequency", UNIT_MEGAHERTZ),
    [M_CORE_POWER]                = C("core_power", "Core power", UNIT_WATTS),
    [M_CORE_VOLTAGE]              = C("core_voltage", "Core voltage, estimated from the package voltage and core sleep time", UNIT_VOLTS),
    [M_CORE_TEMPERATURE]          = C("core_temperature", "Core temperature", UNIT_CELSIUS),
    [M_CORE_C0]                   = C("core_c0", "Core C0 residency", UNIT_RATIO),
    [M_CORE_CC1]                  = C("core_cc1", "Core CC1 residency", UNIT_RATIO),
    [M_CORE_CC6]                  = C("core_cc6", "Core CC6 residency", UNIT_RATIO),
    [M_CORE_CO]                   = C("core_curve_optimizer", "Curve Optimizer count", UNIT_COUNT),

    [M_PEAK_CORE_FREQUENCY]       = G("peak_core_frequency", "Highest effective core frequency", UNIT_MEGAHERTZ),
    [M_PEAK_CORE_TEMPERATURE]     = G("peak_core_temperature", "Highest core temperature", UNIT_CELSIUS),
    [M_PEAK_CORE_VOLTAGE]         = G("peak_core_voltage", "Highest estimated core voltage", UNIT_VOLTS),
    [M_AVERAGE_CORE_VOLTAGE]      = G("average_core_voltage", "Average estimated core voltage", UNIT_VOLTS),
    [M_AVERAGE_CORE_CC6]          = G("average_core_cc6", "Average core CC6 residency", UNIT_RATIO),
    [M_TOTAL_CORE_POWER]          = G("total_core_power", "Sum of core power", UNIT_WATTS),
    [M_AVERAGE_CORE_TEMPERATURE]  = G("average_core_temperature", "Average core temperature", UNIT_CELSIUS),
    [M_AVERAGE_CORE_FREQUENCY]    = G("average_core_frequency", "Average effective core frequency", UNIT_MEGAHERTZ),
    [M_AVERAGE_CORE_C0]           = G("average_core_c0", "Average core C0 residency (CPU utilization)", UNIT_RATIO),
    [M_PACKAGE_CC6]               = G("package_cc6", "Package CC6 residency", UNIT_RATIO),

    [M_PEAK_TEMPERATURE]          = G("peak_temperature", "Peak temperature", UNIT_CELSIUS),
    [M_SOC_TEMPERATURE]           = G("soc_temperature", "SoC temperature", UNIT_CELSIUS),
    [M_GFX_TEMPERATURE]           = G("gfx_temperature", "Graphics temperature", UNIT_CELSIUS),
    [M_VID]                       = L("vid", "Voltage requested from the core VRM", UNIT_VOLTS),
    [M_PPT]                       = L("ppt", "Package power tracking", UNIT_WATTS),
    [M_PPT_APU]                   = L("ppt_apu", "Package power tracking, APU", UNIT_WATTS),
    [M_TDC]                       = L("tdc", "Thermal design current", UNIT_AMPS),
    [M_TDC_ACTUAL]                = L("tdc_actual", "Thermal design current, actual", UNIT_AMPS),
    [M_TDC_SOC]                   = L("tdc_soc", "Thermal design current, SoC", UNIT_AMPS),
    [M_EDC]                       = L("edc", "Electrical design current, estimated from core usage", UNIT_AMPS),
    [M_EDC_SOC]                   = L("edc_soc", "Electrical design current, SoC", UNIT_AMPS),
    [M_THM]                       = L("thm", "Thermal limit", UNIT_CELSIUS),
    [M_THM_SOC]                   = L("thm_soc", "Thermal limit, SoC", UNIT_CELSIUS),
    [M_THM_GFX]                   = L("thm_gfx", "Thermal limit, graphics", UNIT_CELSIUS),
    [M_FIT]                       = L("fit", "Reliability (FIT) limit", UNIT_COUNT),

    [M_MEMORY_COUPLED]            = B("memory_coupled", "Whether UCLK runs at MCLK"),
    [M_FCLK_AVERAGE]              = G("fclk_average", "Average fabric clock", UNIT_MEGAHERTZ),
    [M_FCLK]                      = G("fclk", "Fabric clock", UNIT_MEGAHERTZ),
    [M_UCLK]                      = G("uclk", "Memory controller clock", UNIT_MEGAHERTZ),
    [M_MCLK]                      = G("mclk", "Memory clock", UNIT_MEGAHERTZ),
    [M_CLDO_VDDM]                 = G("cldo_vddm", "cLDO VDDM voltage", UNIT_VOLTS),
    [M_CLDO_VDDP]                 = G("cldo_vddp", "cLDO VDDP voltage", UNIT_VOLTS),
    [M_CLDO_VDDG]                 = G("cldo_vddg", "cLDO VDDG voltage", UNIT_VOLTS),
    [M_CLDO_VDDG_IOD]             = G("cldo_vddg_iod", "cLDO VDDG voltage, IOD", UNIT_VOLTS),
    [M_CLDO_VDDG_CCD]             = G("cldo_vddg_ccd", "cLDO VDDG voltage, CCD", UNIT_VOLTS),

    [M_GFX_VOLTAGE]               = G("gfx_voltage", "Graphics voltage", UNIT_VOLTS),
    [M_GFX_FREQUENCY]             = G("gfx_frequency", "Graphics clock", UNIT_MEGAHERTZ),
    [M_GFX_FREQUENCY_EFFECTIVE]   = G("gfx_frequency_effective", "Effective graphics clock", UNIT_MEGAHERTZ),
    [M_GFX_BUSY]                  = G("gfx_busy", "Graphics busy", UNIT_RATIO),
    [M_GFX_EDC_LIMIT]             = G("gfx_edc_limit", "Graphics EDC limit", UNIT_AMPS),
    [M_GFX_EDC_RESIDENCY]         = G("gfx_edc_residency", "Graphics EDC limit residency", UNIT_RATIO),
    [M_DISPLAY_COUNT]             = G("displays", "Active displays", UNIT_COUNT),
    [M_FPS]                       = G("fps", "Frames per second", UNIT_COUNT),
    [M_DGPU_POWER]                = G("dgpu_power", "Discrete GPU power", UNIT_WATTS),
    [M_DGPU_FREQUENCY_TARGET]     = G("dgpu_frequency_target", "Discrete GPU target frequency", UNIT_MEGAHERTZ),
    [M_DGPU_BUSY]                 = G("dgpu_busy", "Discrete GPU busy", UNIT_RATIO),

    [M_VDDCR_SOC_POWER]           = G("vddcr_soc_power", "VDDCR_SOC power", UNIT_WATTS),
    [M_IO_VDDCR_SOC_POWER]        = G("io_vddcr_soc_power", "IO VDDCR_SOC power", UNIT_WATTS),
    [M_GMI2_VDDG_POWER]           = G("gmi2_vddg_power", "GMI2 VDDG power", UNIT_WATTS),
    [M_ROC_POWER]                 = G("roc_power", "Rest-of-chip power", UNIT_WATTS),
    [M_L3_LOGIC_POWER]            = C3("l3_logic_power", "L3 cache logic power", UNIT_WATTS),
    [M_L3_VDDM_POWER]             = C3("l3_vddm_power", "L3 cache VDDM power", UNIT_WATTS),
    [M_VDDIO_MEM_POWER]           = G("vddio_mem_power", "VDDIO_MEM power", UNIT_WATTS),
    [M_IOD_VDDIO_MEM_POWER]       = G("iod_vddio_mem_power", "IOD VDDIO_MEM power", UNIT_WATTS),
    [M_DDR_VDDP_POWER]            = G("ddr_vddp_power", "DDR VDDP power", UNIT_WATTS),
    [M_DDR_PHY_POWER]             = G("ddr_phy_power", "DDR PHY power", UNIT_WATTS),
    [M_VDD18_POWER]               = G("vdd18_power", "VDD18 power", UNIT_WATTS),
    [M_IO_DISPLAY_POWER]          = G("io_display_power", "Display IO power", UNIT_WATTS),
    [M_IO_USB_POWER]              = G("io_usb_power", "USB IO power", UNIT_WATTS),
    [M_CALCULATED_THERMAL_OUTPUT] = G("calculated_thermal_output", "Thermal output of the package, sum of all rails", UNIT_WATTS),

    [M_SOC_TELEMETRY_VOLTAGE]     = G("soc_telemetry_voltage", "SoC voltage reported by the VRM", UNIT_VOLTS),
    [M_SOC_TELEMETRY_CURRENT]     = G("soc_telemetry_current", "SoC current reported by the VRM", UNIT_AMPS),
    [M_SOC_TELEMETRY_POWER]       = G("soc_telemetry_power", "SoC power reported by the VRM", UNIT_WATTS),
    [M_CORE_TELEMETRY_VOLTAGE]    = G("core_telemetry_voltage", "Core voltage reported by the VRM", UNIT_VOLTS),
    [M_CORE_TELEMETRY_CURRENT]    = G("core_telemetry_current", "Core current reported by the VRM", UNIT_AMPS),
    [M_CORE_TELEMETRY_POWER]      = G("core_telemetry_power", "Core power reported by the VRM", UNIT_WATTS),
    [M_SMU_CORE_POWER]            = G("smu_core_power", "Core power reported by the SMU", UNIT_WATTS),
    [M_SMU_SOCKET_POWER]          = G("smu_socket_power", "Socket power reported by the SMU", UNIT_WATTS),
    [M_SMU_PACKAGE_POWER]         = G("smu_package_power", "Package power reported by the SMU", UNIT_WATTS),
};

static void set(struct sample *s, enum metric_id id, int i, float value) {
    s->m[id].present |= 1u << i;
    s->m[id].value[i] = value;
}

static void set_limit(struct sample *s, enum metric_id id, float value, float limit) {
    set(s, id, 0, value);
    s->m[id].limit[0] = limit;
}

// Helper to access the PM Table elements. If an element doesn't exist in the
// current PM Table version, its pointer is set to 0. This helper returns
// NAN for not available fields.
#define pmta(elem) ((pmt->elem)?(*pmt->elem):NAN)
// Same, but with 0 as return. For summations that should not fail if one value is not present.
#define pmta0(elem) ((pmt->elem)?(*pmt->elem):0)

// A metric that is a PM table field as is.
#define FIELD(id, elem) do { if (pmt->elem) set(s, id, 0, *pmt->elem); } while (0)
// A value and its limit, present when the value is.
#define LIMIT(id, value, limit) do { if (pmt->value) set_limit(s, id, *pmt->value, pmta(limit)); } while (0)

void compute_sample(const pm_table *pmt, const system_info *sysinfo,
        const struct sample_options *opts, struct sample *s) {
    float package_sleep_time, average_voltage;
    float peak_core_frequency, peak_core_temp, peak_core_voltage;
    float total_core_voltage, total_core_power, total_usage, total_core_CC6;
    float total_core_temp, total_core_frequency;
    float l3_logic_power, l3_vddm_power, edc_value;
    float clock_scale = opts->refclk_mhz > 0 ? opts->refclk_mhz / 100.0 : 1.f;
    int i, core_number;

    memset(s, 0, sizeof(*s));
    for (i = 0; i < M_COUNT; i++)
        for (int j = 0; j < METRIC_MAX_INDEX; j++)
            s->m[i].value[j] = s->m[i].limit[j] = NAN;

    clock_gettime(CLOCK_MONOTONIC_RAW, &s->monotonic_raw);
    clock_gettime(CLOCK_REALTIME, &s->realtime);

    s->sysinfo = sysinfo;
    s->pm_table_version = pmt->version;
    s->zen_version = pmt->zen_version;
    s->experimental = pmt->experimental;
    s->has_graphics = pmt->has_graphics;
    s->ncores = pmt->max_cores;
    s->nl3 = pmt->max_l3;

    // Cores

    peak_core_frequency = peak_core_temp = peak_core_voltage = 0;
    total_core_voltage = total_core_power = total_usage = total_core_CC6 = 0;
    total_core_temp = total_core_frequency = 0;
    core_number = 0;

    if (pmt->PC6) {
        package_sleep_time = pmta(PC6) / 100.f;
        average_voltage = (pmta(CPU_TELEMETRY_VOLTAGE) - (0.2 * package_sleep_time)) / (1.0 - package_sleep_time);
    } else {
        average_voltage = pmta(CPU_TELEMETRY_VOLTAGE);
    }

    for (i = 0; i < pmt->max_cores; i++) {
        struct core_state *core = &s->core[i];
        float core_frequency, core_voltage, core_sleep_time;

        core->disabled = (sysinfo->core_disable_map >> i) & 0x01;
        // AMD denotes a sleeping core as having spent less than 6% of the time in C0.
        // Source: Ryzen Master
        core->sleeping = pmta(CORE_C0[i]) < 6.f;

        //Don't confuse people by numbering cores that are disabled and hence not shown on 6 | 12 core CPUs
        //(which actually have 8 | 16 cores)
        core->number = (opts->show_disabled_cores || !core->disabled) ? core_number++ : -1;

        core_frequency = pmta(CORE_FREQEFF[i]) * 1000.f * clock_scale;

        if (pmt->CPU_TELEMETRY_VOLTAGE) {
            // Rumours say this is how AMD calculates core voltage
            core_sleep_time = pmta(CORE_CC6[i]) / 100.f;
            core_voltage = ((1.0 - core_sleep_time) * average_voltage) + (0.2 * core_sleep_time);
        } else {
            // Without the package voltage, use the core's own.
            core_voltage = pmta(CORE_VOLTAGE[i]);
        }

        // Per-core values are kept even when absent: the core table prints a full row.
        s->m[M_CORE_FREQUENCY].value[i] = core_frequency;
        s->m[M_CORE_POWER].value[i] = pmta(CORE_POWER[i]);
        s->m[M_CORE_VOLTAGE].value[i] = core_voltage;
        s->m[M_CORE_TEMPERATURE].value[i] = pmta(CORE_TEMP[i]);
        s->m[M_CORE_C0].value[i] = pmta(CORE_C0[i]) / 100.f;
        s->m[M_CORE_CC1].value[i] = pmta(CORE_CC1[i]) / 100.f;
        s->m[M_CORE_CC6].value[i] = pmta(CORE_CC6[i]) / 100.f;
        if (pmt->CORE_FREQEFF[i]) s->m[M_CORE_FREQUENCY].present |= 1u << i;
        if (pmt->CORE_POWER[i]) s->m[M_CORE_POWER].present |= 1u << i;
        if (pmt->CPU_TELEMETRY_VOLTAGE ? pmt->CORE_CC6[i] != NULL : pmt->CORE_VOLTAGE[i] != NULL)
            s->m[M_CORE_VOLTAGE].present |= 1u << i;
        if (pmt->CORE_TEMP[i]) s->m[M_CORE_TEMPERATURE].present |= 1u << i;
        if (pmt->CORE_C0[i]) s->m[M_CORE_C0].present |= 1u << i;
        if (pmt->CORE_CC1[i]) s->m[M_CORE_CC1].present |= 1u << i;
        if (pmt->CORE_CC6[i]) s->m[M_CORE_CC6].present |= 1u << i;
        if (opts->co_present & (1u << i))
            set(s, M_CORE_CO, i, opts->co_counts[i]);

        //Statistics
        if (!core->disabled) {
            if (peak_core_frequency < core_frequency) peak_core_frequency = core_frequency;
            if (peak_core_temp < pmta(CORE_TEMP[i])) peak_core_temp = pmta(CORE_TEMP[i]);
            if (peak_core_voltage < core_voltage) peak_core_voltage = core_voltage;
            total_core_voltage += core_voltage;
            total_core_power += pmta(CORE_POWER[i]);
            total_usage += pmta(CORE_C0[i]);
            total_core_CC6 += pmta(CORE_CC6[i]);
            total_core_temp += pmta(CORE_TEMP[i]);
            total_core_frequency += core_frequency;
        }
    }

    // Statistics only exist where the per-core values they are made of do.
    if (s->m[M_CORE_FREQUENCY].present) {
        set(s, M_PEAK_CORE_FREQUENCY, 0, peak_core_frequency);
        set(s, M_AVERAGE_CORE_FREQUENCY, 0, total_core_frequency / sysinfo->enabled_cores_count);
    }
    if (s->m[M_CORE_TEMPERATURE].present) {
        set(s, M_PEAK_CORE_TEMPERATURE, 0, peak_core_temp);
        set(s, M_AVERAGE_CORE_TEMPERATURE, 0, total_core_temp / sysinfo->enabled_cores_count);
    }
    if (s->m[M_CORE_VOLTAGE].present) {
        set(s, M_PEAK_CORE_VOLTAGE, 0, peak_core_voltage);
        set(s, M_AVERAGE_CORE_VOLTAGE, 0, total_core_voltage / sysinfo->enabled_cores_count);
    }
    if (s->m[M_CORE_CC6].present)
        set(s, M_AVERAGE_CORE_CC6, 0, (total_core_CC6 / sysinfo->enabled_cores_count) / 100.f);
    if (s->m[M_CORE_POWER].present)
        set(s, M_TOTAL_CORE_POWER, 0, total_core_power);
    if (s->m[M_CORE_C0].present)
        set(s, M_AVERAGE_CORE_C0, 0, (total_usage / sysinfo->enabled_cores_count) / 100.f);
    if (pmt->PC6) set(s, M_PACKAGE_CC6, 0, pmta(PC6) / 100.f);

    // Electrical and thermal constraints

    FIELD(M_PEAK_TEMPERATURE, PEAK_TEMP);
    FIELD(M_SOC_TEMPERATURE, SOC_TEMP);
    FIELD(M_GFX_TEMPERATURE, GFX_TEMP);

    LIMIT(M_VID, VID_VALUE, VID_LIMIT);
    LIMIT(M_PPT, PPT_VALUE, PPT_LIMIT);
    LIMIT(M_PPT_APU, PPT_VALUE_APU, PPT_LIMIT_APU);
    LIMIT(M_TDC, TDC_VALUE, TDC_LIMIT);
    LIMIT(M_TDC_ACTUAL, TDC_ACTUAL, TDC_LIMIT);
    LIMIT(M_TDC_SOC, TDC_VALUE_SOC, TDC_LIMIT_SOC);

    if (pmt->EDC_VALUE) {
        edc_value = pmta(EDC_VALUE) * (total_usage / sysinfo->cores / 100);
        if (edc_value < pmta(TDC_VALUE)) edc_value = pmta(TDC_VALUE);
        set_limit(s, M_EDC, edc_value, pmta(EDC_LIMIT));
    }

    LIMIT(M_EDC_SOC, EDC_VALUE_SOC, EDC_LIMIT_SOC);
    if (pmt->THM_VALUE) {
        set_limit(s, M_THM, *pmt->THM_VALUE, pmta(THM_LIMIT));
    } else {
        // Some tables only have a value per core; the hottest one is held to the limit.
        float thm_value = -INFINITY;

        for (i = 0; i < pmt->max_cores; i++)
            if (pmt->THM_VALUE_CORES[i] && *pmt->THM_VALUE_CORES[i] > thm_value)
                thm_value = *pmt->THM_VALUE_CORES[i];
        if (thm_value > -INFINITY)
            set_limit(s, M_THM, thm_value, pmta(THM_LIMIT));
    }
    LIMIT(M_THM_SOC, THM_VALUE_SOC, THM_LIMIT_SOC);
    LIMIT(M_THM_GFX, THM_VALUE_GFX, THM_LIMIT_GFX);
    // STT_VALUE_APU / STT_LIMIT_APU always zero
    // STT_VALUE_DGPU / STT_LIMIT_DGPU always zero
    LIMIT(M_FIT, FIT_VALUE, FIT_LIMIT);

    // Memory interface

    if (pmt->UCLK_FREQ && pmt->MEMCLK_FREQ)
        set(s, M_MEMORY_COUPLED, 0, pmta(UCLK_FREQ) == pmta(MEMCLK_FREQ));
    if (pmt->FCLK_FREQ_EFF) set(s, M_FCLK_AVERAGE, 0, *pmt->FCLK_FREQ_EFF * clock_scale);
    if (pmt->FCLK_FREQ) set(s, M_FCLK, 0, *pmt->FCLK_FREQ * clock_scale);
    if (pmt->UCLK_FREQ) set(s, M_UCLK, 0, *pmt->UCLK_FREQ * clock_scale);
    if (pmt->MEMCLK_FREQ) set(s, M_MCLK, 0, *pmt->MEMCLK_FREQ * clock_scale);
    // SOC_SET_VOLTAGE might be the default voltage, not the actually set one
    FIELD(M_CLDO_VDDM, V_VDDM);
    FIELD(M_CLDO_VDDP, V_VDDP);
    FIELD(M_CLDO_VDDG, V_VDDG);
    FIELD(M_CLDO_VDDG_IOD, V_VDDG_IOD);
    FIELD(M_CLDO_VDDG_CCD, V_VDDG_CCD);

    // Graphics

    FIELD(M_GFX_VOLTAGE, GFX_VOLTAGE);
    FIELD(M_GFX_FREQUENCY, GFX_FREQ);
    FIELD(M_GFX_FREQUENCY_EFFECTIVE, GFX_FREQEFF);
    FIELD(M_GFX_BUSY, GFX_BUSY);
    FIELD(M_GFX_EDC_LIMIT, GFX_EDC_LIM);
    FIELD(M_GFX_EDC_RESIDENCY, GFX_EDC_RESIDENCY);
    FIELD(M_DISPLAY_COUNT, DISPLAY_COUNT);
    FIELD(M_FPS, FPS);
    FIELD(M_DGPU_POWER, DGPU_POWER);
    FIELD(M_DGPU_FREQUENCY_TARGET, DGPU_FREQ_TARGET);
    FIELD(M_DGPU_BUSY, DGPU_GFX_BUSY);

    // Power

    // VDDCR_CPU_POWER doesn't correlate with what the cores report, nor with
    // what is actually consumed, but is the value HWiNFO shows.
    FIELD(M_VDDCR_SOC_POWER, VDDCR_SOC_POWER);
    FIELD(M_IO_VDDCR_SOC_POWER, IO_VDDCR_SOC_POWER);
    FIELD(M_GMI2_VDDG_POWER, GMI2_VDDG_POWER);
    FIELD(M_ROC_POWER, ROC_POWER);

    //L3 caches (2 per CCD on Zen2, 1 per CCD on Zen3)
    l3_logic_power = l3_vddm_power = 0;
    for (i = 0; i < pmt->max_l3; i++) {
        l3_logic_power += pmta0(L3_LOGIC_POWER[i]);
        l3_vddm_power += pmta0(L3_VDDM_POWER[i]);
        if (pmt->L3_LOGIC_POWER[i]) set(s, M_L3_LOGIC_POWER, i, *pmt->L3_LOGIC_POWER[i]);
        if (pmt->L3_VDDM_POWER[i]) set(s, M_L3_VDDM_POWER, i, *pmt->L3_VDDM_POWER[i]);
    }

    //These powers are supplied by other power lines to the CPU and are drawn from the 24 pin ATX connector on most boards
    FIELD(M_VDDIO_MEM_POWER, VDDIO_MEM_POWER);
    FIELD(M_IOD_VDDIO_MEM_POWER, IOD_VDDIO_MEM_POWER);
    FIELD(M_DDR_VDDP_POWER, DDR_VDDP_POWER);
    FIELD(M_DDR_PHY_POWER, DDR_PHY_POWER);
    FIELD(M_VDD18_POWER, VDD18_POWER);
    FIELD(M_IO_DISPLAY_POWER, IO_DISPLAY_POWER);
    FIELD(M_IO_USB_POWER, IO_USB_POWER);

    //The sum is the thermal output of the whole package. Yes, this is higher than PPT and SOCKET_POWER.
    //Confirmed by measuring the actual current draw on the mainboard.
    if (!pmt->powersum_unclear)
        set(s, M_CALCULATED_THERMAL_OUTPUT, 0, total_core_power + pmta0(VDDCR_SOC_POWER) + pmta0(GMI2_VDDG_POWER)
                + l3_logic_power + l3_vddm_power
                + pmta0(VDDIO_MEM_POWER) + pmta0(IOD_VDDIO_MEM_POWER) + pmta0(DDR_VDDP_POWER) + pmta0(VDD18_POWER));

    // Additional reports

    FIELD(M_SOC_TELEMETRY_VOLTAGE, SOC_TELEMETRY_VOLTAGE);
    FIELD(M_SOC_TELEMETRY_CURRENT, SOC_TELEMETRY_CURRENT);
    FIELD(M_SOC_TELEMETRY_POWER, SOC_TELEMETRY_POWER);
    FIELD(M_CORE_TELEMETRY_VOLTAGE, CPU_TELEMETRY_VOLTAGE);
    FIELD(M_CORE_TELEMETRY_CURRENT, CPU_TELEMETRY_CURRENT);
    if (pmt->CPU_TELEMETRY_POWER)
        set(s, M_CORE_TELEMETRY_POWER, 0, *pmt->CPU_TELEMETRY_POWER);
    else if (pmt->CPU_TELEMETRY_VOLTAGE && pmt->CPU_TELEMETRY_CURRENT)
        set(s, M_CORE_TELEMETRY_POWER, 0, *pmt->CPU_TELEMETRY_VOLTAGE * *pmt->CPU_TELEMETRY_CURRENT);
    FIELD(M_SMU_CORE_POWER, VDDCR_CPU_POWER);
    FIELD(M_SMU_SOCKET_POWER, SOCKET_POWER);
    FIELD(M_SMU_PACKAGE_POWER, PACKAGE_POWER);
}
