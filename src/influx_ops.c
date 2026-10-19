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
 * InfluxDB line protocol, for telegraf. The measurement, tags and field
 * names are those of ryzen_monitor_ng's export, misspellings included, so
 * that existing dashboards keep working. Lines carry no timestamp; telegraf
 * assigns one.
 **/

#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include "output_ops.h"

#define MEASUREMENT "ryzen_monitor_ng"

enum field_flags {
    F_INT     = 1 << 0,     // written as an integer
    F_PERCENT = 1 << 1,     // ratio written as a percentage
    F_LIMIT   = 1 << 2,     // the limit of a KIND_LIMIT metric instead of its value
};

struct field {
    const char *name;
    enum metric_id id;
    unsigned int flags;
};

static const struct field cores_fields[] = {
    { "cores_maxfrequencyeff", M_PEAK_CORE_FREQUENCY, F_INT },
    { "cores_maxtemperature", M_PEAK_CORE_TEMPERATURE, 0 },
    { "cores_maxvid", M_PEAK_CORE_VOLTAGE, 0 },
    { "cores_avgvid", M_AVERAGE_CORE_VOLTAGE, 0 },
    { "cores_avgcc6", M_AVERAGE_CORE_CC6, F_PERCENT },
    { "cores_totalpower", M_TOTAL_CORE_POWER, 0 },
    { "package_cc6", M_PACKAGE_CC6, F_PERCENT },
    { "cpu_maxvid_smu", M_CORE_TELEMETRY_VOLTAGE, 0 },
};

static const struct field limits_fields[] = {
    { "package_peaktemperature", M_PEAK_TEMPERATURE, 0 },
    { "soc_temperature", M_SOC_TEMPERATURE, 0 },
    { "gfx_temperature", M_GFX_TEMPERATURE, 0 },
    { "package_vcorevrm_vid", M_VID, 0 },
    { "package_vcorevrm_vidlimit", M_VID, F_LIMIT },
    { "cpu_ppt", M_PPT, 0 },
    { "cpu_pptlimit", M_PPT, F_LIMIT | F_INT },
    { "cpu_pptapu", M_PPT_APU, 0 },
    { "cpu_pptapulimit", M_PPT_APU, F_LIMIT | F_INT },
    { "cpu_tdc", M_TDC, 0 },
    { "cpu_tdclimit", M_TDC, F_LIMIT | F_INT },
    { "cpu_tdcactual", M_TDC_ACTUAL, 0 },
    { "soc_tdc", M_TDC_SOC, 0 },
    { "soc_tdclimit", M_TDC_SOC, F_LIMIT | F_INT },
    { "cpu_edc", M_EDC, 0 },
    { "cpu_edclimit", M_EDC, F_LIMIT | F_INT },
    { "soc_edc", M_EDC_SOC, 0 },
    { "soc_edclimit", M_EDC_SOC, F_LIMIT | F_INT },
    { "cpu_thm", M_THM, 0 },
    { "cpu_thmlimit", M_THM, F_LIMIT | F_INT },
    { "soc_thmsoc", M_THM_SOC, 0 },
    { "soc_thmlimit", M_THM_SOC, F_LIMIT | F_INT },
    { "gfx_thm", M_THM_GFX, 0 },
    { "gfx_thmlimit", M_THM_GFX, F_LIMIT | F_INT },
    { "cpu_fit", M_FIT, F_INT },
    { "cpu_fitlimit", M_FIT, F_LIMIT | F_INT },
};

static const struct field fabric_fields[] = {
    { "cpu_vddm", M_CLDO_VDDM, 0 },
    { "cpu_vddp", M_CLDO_VDDP, 0 },
    { "cpu_vddg", M_CLDO_VDDG, 0 },
    { "cpu_vddg_iod", M_CLDO_VDDG_IOD, 0 },
    { "cpu_cldo_vddg_ccd", M_CLDO_VDDG_CCD, 0 },
    { "cpu_coupled", M_MEMORY_COUPLED, 0 },
    { "cpu_fabricclockeff", M_FCLK_AVERAGE, 0 },
    { "cpu_fabricclock", M_FCLK, 0 },
    { "cpu_uncoreclock", M_UCLK, 0 },
    { "cpu_memoryclock", M_MCLK, 0 },
};

static const struct field gfx_fields[] = {
    { "gfx_voltage", M_GFX_VOLTAGE, 0 },
    { "gfx_rocpower", M_ROC_POWER, 0 },
    { "gfx_temperature", M_GFX_TEMPERATURE, 0 },
    { "gfx_clock", M_GFX_FREQUENCY, 0 },
    { "gfx_clockeff", M_GFX_FREQUENCY_EFFECTIVE, F_INT },
    { "gfx_busy", M_GFX_BUSY, F_PERCENT },
    { "gfx_edclimit", M_GFX_EDC_LIMIT, F_INT },
    { "gfx_residency", M_GFX_EDC_RESIDENCY, F_PERCENT },
    { "gfx_displaycount", M_DISPLAY_COUNT, 0 },
    { "gfx_fps", M_FPS, 0 },
    { "gfx_dgpupower", M_DGPU_POWER, 0 },
    { "gfx_dgpuclocktarget", M_DGPU_FREQUENCY_TARGET, F_INT },
    { "gfx_dgpubusy", M_DGPU_BUSY, F_PERCENT },
};

static const struct field power_fields[] = {
    { "package_vddcrcpupower", M_SMU_CORE_POWER, 0 },
    { "package_vddcrsocpower", M_VDDCR_SOC_POWER, 0 },
    { "package_vddcriosocpower", M_IO_VDDCR_SOC_POWER, 0 },
    { "package_gmi2vddgpower", M_GMI2_VDDG_POWER, 0 },
    { "package_vddiomempowerr", M_VDDIO_MEM_POWER, 0 },
    { "package_iodvddiomempowerr", M_IOD_VDDIO_MEM_POWER, 0 },
    { "package_ddrvddppower", M_DDR_VDDP_POWER, 0 },
    { "package_ddrphypower", M_DDR_PHY_POWER, 0 },
    { "package_displayiopower", M_IO_DISPLAY_POWER, 0 },
    { "package_usbiopower", M_IO_USB_POWER, 0 },
    { "package_calc_thermaloutput", M_CALCULATED_THERMAL_OUTPUT, 0 },
    { "package_vi2socvoltage", M_SOC_TELEMETRY_VOLTAGE, 0 },
    { "package_svi2soccurrent", M_SOC_TELEMETRY_CURRENT, 0 },
    { "package_svi2socpower", M_SOC_TELEMETRY_POWER, 0 },
    { "package_svi2cpuvoltage", M_CORE_TELEMETRY_VOLTAGE, 0 },
    { "package_svi2cpucurrent", M_CORE_TELEMETRY_CURRENT, 0 },
    { "package_svi2cpupower", M_CORE_TELEMETRY_POWER, 0 },
    { "package_smusocketpower", M_SMU_SOCKET_POWER, 0 },
    { "package_smupackagepower", M_SMU_PACKAGE_POWER, 0 },
    { "package_vdd18power", M_VDD18_POWER, 0 },
    { "package_totalcorepower", M_TOTAL_CORE_POWER, 0 },
};

#define FIELDS(f) f, sizeof(f) / sizeof((f)[0])

static char hostname[HOST_NAME_MAX + 1];

static void influx_init(bool repeating, bool interactive) {
    char *d = hostname;

    (void)repeating;
    (void)interactive;

    // Tag values must not contain unescaped spaces, commas or equals signs.
    gethostname(hostname, sizeof(hostname));
    hostname[sizeof(hostname) - 1] = 0;
    for (const char *c = hostname; *c; c++)
        if (!isspace((unsigned char)*c) && *c != ',' && *c != '=')
            *d++ = *c;
    *d = 0;
}

static void influx_cleanup(void) {}

static void begin_line(const char *name) {
    fprintf(stdout, MEASUREMENT ",host=%s,name=%s ", hostname, name);
}

// Line protocol has no representation for NaN or infinity; such fields are left out.
static void print_field(int *n, const char *name, float value, unsigned int flags) {
    if (!isfinite(value))
        return;
    if ((*n)++)
        fputc(',', stdout);
    if (flags & F_PERCENT)
        value *= 100.f;
    if (flags & F_INT)
        fprintf(stdout, "%s=%.0fi", name, value);
    else
        fprintf(stdout, "%s=%.6g", name, value);
}

static void print_line(const struct sample *s, const char *name, const struct field *fields, size_t nfields) {
    int n = 0;

    for (size_t i = 0; i < nfields; i++) {
        const struct field *f = &fields[i];

        if (!metric_present(s, f->id, 0))
            continue;
        if (!n)
            begin_line(name);
        if (metric_defs[f->id].kind == KIND_BOOL) {
            if (n++)
                fputc(',', stdout);
            fprintf(stdout, "%s=%s", f->name, metric_value(s, f->id, 0) ? "true" : "false");
        } else {
            print_field(&n, f->name, (f->flags & F_LIMIT) ? metric_limit(s, f->id, 0) : metric_value(s, f->id, 0), f->flags);
        }
    }
    if (n)
        fputc('\n', stdout);
}

static void print_l3(int *n, const struct sample *s, const char *what, enum metric_id id) {
    char name[64];
    float sum = 0;
    int count = metric_count(s, id);

    for (int i = 0; i < count; i++) {
        if (!metric_present(s, id, i))
            continue;
        sum += metric_value(s, id, i);
        if (count > 1) {
            snprintf(name, sizeof(name), "package_l3%s%dpower", what, i);
            print_field(n, name, metric_value(s, id, i), 0);
        }
    }
    if (metric_present(s, id, 0)) {
        snprintf(name, sizeof(name), "package_l3%spower", what);
        print_field(n, name, sum, 0);
    }
}

static void print_cores(const struct sample *s) {
    char name[16];

    for (int i = 0; i < s->ncores; i++) {
        const struct core_state *core = &s->core[i];
        int n = 0;

        if (core->number < 0)
            continue;

        snprintf(name, sizeof(name), "Core%d", core->number);
        begin_line(name);
        fprintf(stdout, "core_state=\"%s\",core_sleeping=%di,core_active=%di",
                core->disabled ? "Disabled" : core->sleeping ? "Sleeping" : "Active",
                core->disabled || core->sleeping, !core->disabled && !core->sleeping);
        n = 1;
        if (!core->disabled)
            print_field(&n, "core_frequency", metric_value(s, M_CORE_FREQUENCY, i), F_INT);
        print_field(&n, "core_power", metric_value(s, M_CORE_POWER, i), 0);
        print_field(&n, "core_vid", metric_value(s, M_CORE_VOLTAGE, i), 0);
        print_field(&n, "core_temperature", metric_value(s, M_CORE_TEMPERATURE, i), 0);
        print_field(&n, "core_c0", metric_value(s, M_CORE_C0, i), F_PERCENT);
        print_field(&n, "core_c1", metric_value(s, M_CORE_CC1, i), F_PERCENT);
        print_field(&n, "core_c6", metric_value(s, M_CORE_CC6, i), F_PERCENT);
        fputc('\n', stdout);
    }
}

static void influx_emit(const struct sample *s, const struct display *d) {
    if (group_shown(s, d, GROUP_CORES))
        print_cores(s);
    if (group_shown(s, d, GROUP_CORE_STATS_CALC))
        print_line(s, "Cores", FIELDS(cores_fields));
    if (group_shown(s, d, GROUP_LIMITS))
        print_line(s, "Package", FIELDS(limits_fields));
    if (group_shown(s, d, GROUP_MEMORY))
        print_line(s, "FabricIO", FIELDS(fabric_fields));
    if (group_shown(s, d, GROUP_GRAPHICS))
        print_line(s, "GFX", FIELDS(gfx_fields));
    if (group_shown(s, d, GROUP_POWER)) {
        int n = 0;

        print_line(s, "Package", FIELDS(power_fields));
        if (metric_present(s, M_L3_LOGIC_POWER, 0) || metric_present(s, M_L3_VDDM_POWER, 0)) {
            begin_line("Package");
            print_l3(&n, s, "logic", M_L3_LOGIC_POWER);
            print_l3(&n, s, "vddm", M_L3_VDDM_POWER);
            fputc('\n', stdout);
        }
    }

    fflush(stdout);
}

const struct output_ops influx_ops = {
    .init = influx_init,
    .cleanup = influx_cleanup,
    .emit = influx_emit,
};
