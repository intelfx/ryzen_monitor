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
 * How messages are sent follows ZenStates-Core
 * (github.com/irusanov/ZenStates-Core).
 **/

#include <stddef.h>
#include <string.h>

#include <libsmu.h>

#include "smu_control.h"

extern smu_obj_t obj;

#define LIMIT(n, h, cpu, apu, sc, field) { n, h, cpu, apu, sc, offsetof(pm_table, field) }

const struct smu_limit smu_limits[] = {
    LIMIT("ppt", "PPT limit (W); the slow limit on APUs", MSG_SET_FAST_LIMIT, MSG_SET_SLOW_LIMIT, 1000, PPT_LIMIT),
    LIMIT("pptfast", "PPT fast limit (W), APUs", MSG_COUNT, MSG_SET_FAST_LIMIT, 1000, PPT_LIMIT_FAST),
    LIMIT("pptapu", "PPT APU limit (W), APUs", MSG_COUNT, MSG_SET_APU_SLOW_LIMIT, 1000, PPT_LIMIT_APU),
    LIMIT("stapm", "STAPM power limit (W), APUs", MSG_COUNT, MSG_SET_STAPM_LIMIT, 1000, STAPM_LIMIT),
    LIMIT("ppt-time", "Slow PPT time constant (s), APUs", MSG_COUNT, MSG_SET_SLOW_TIME, 1, SlowPPTTimeConstant),
    LIMIT("stapm-time", "STAPM time constant (s), APUs", MSG_COUNT, MSG_SET_STAPM_TIME, 1, StapmTimeConstant),
    LIMIT("tdc", "TDC limit (A)", MSG_SET_TDC_VDD_LIMIT, MSG_SET_TDC_VDD_LIMIT, 1000, TDC_LIMIT),
    LIMIT("tdcsoc", "TDC SoC limit (A), APUs", MSG_COUNT, MSG_SET_TDC_SOC_LIMIT, 1000, TDC_LIMIT_SOC),
    LIMIT("edc", "EDC limit (A)", MSG_SET_EDC_VDD_LIMIT, MSG_SET_EDC_VDD_LIMIT, 1000, EDC_LIMIT),
    LIMIT("edcsoc", "EDC SoC limit (A), APUs", MSG_COUNT, MSG_SET_EDC_SOC_LIMIT, 1000, EDC_LIMIT_SOC),
    LIMIT("thm", "Temperature limit (C)", MSG_SET_TCTL_MAX, MSG_SET_TCTL_MAX, 1, THM_LIMIT),
};

const int smu_limits_count = sizeof(smu_limits) / sizeof(smu_limits[0]);

const struct smu_settings *smu_settings_for(int codename) {
    for (int i = 0; i < smu_settings_count; i++)
        for (int j = 0; j < 4; j++)
            if (smu_settings[i].codenames[j] == codename && codename != CODENAME_UNDEFINED)
                return &smu_settings[i];
    return NULL;
}

static bool is_apu(const struct smu_settings *s) {
    return s->type >= SETTINGS_TYPE_APU0;
}

static enum smu_control_result send(unsigned int op, enum smu_mailbox mailbox, smu_arg_t *args) {
    if (!op)
        return SMU_CONTROL_UNSUPPORTED;
    return smu_send_command(&obj, op, args, mailbox) == SMU_Return_OK ? SMU_CONTROL_OK : SMU_CONTROL_FAILED;
}

static void make_args(smu_arg_t *args, unsigned int arg0) {
    memset(args, 0, sizeof(*args));
    args->args[0] = arg0;
}

// Send a message with one argument to the RSMU, and to MP1 if that fails.
static enum smu_control_result send_rsmu_then_mp1(const struct smu_settings *s, enum smu_message msg,
        unsigned int arg, smu_arg_t *args) {
    enum smu_control_result ret;

    if (!s->rsmu[msg] && !s->mp1[msg])
        return SMU_CONTROL_UNSUPPORTED;

    make_args(args, arg);
    ret = send(s->rsmu[msg], SMU_TYPE_RSMU, args);
    if (ret != SMU_CONTROL_OK && s->mp1[msg]) {
        make_args(args, arg);
        ret = send(s->mp1[msg], SMU_TYPE_MP1, args);
    }
    return ret;
}

// Limits: like send_rsmu_then_mp1(), but older mobile firmware only takes the
// MP1 variant, so only that is sent there if it exists.
static enum smu_control_result send_limit(const struct smu_settings *s, enum smu_message msg,
        unsigned int arg, smu_arg_t *args) {
    if (s->type == SETTINGS_TYPE_APU0 || s->type == SETTINGS_TYPE_APU1) {
        make_args(args, arg);
        if (s->mp1[msg])
            return send(s->mp1[msg], SMU_TYPE_MP1, args);
        return send(s->rsmu[msg], SMU_TYPE_RSMU, args);
    }
    return send_rsmu_then_mp1(s, msg, arg, args);
}

enum smu_control_result smu_set_limit(const struct smu_settings *s, const struct smu_limit *limit, unsigned int value) {
    enum smu_message msg = is_apu(s) ? limit->apu_msg : limit->cpu_msg;
    smu_arg_t args;

    if (msg == MSG_COUNT)
        return SMU_CONTROL_UNSUPPORTED;
    return send_limit(s, msg, value * limit->scale, &args);
}

enum smu_control_result smu_set_pbo_scalar(const struct smu_settings *s, unsigned int scalar) {
    smu_arg_t args;

    // Zen 2 and Zen 3 have to be allowed to use PBO first.
    if (s->mp1[MSG_SET_PBO_EN]) {
        make_args(&args, 0);
        send(s->mp1[MSG_SET_PBO_EN], SMU_TYPE_MP1, &args);
    }
    return send_rsmu_then_mp1(s, MSG_SET_PBO_SCALAR, scalar * 100, &args);
}

enum smu_control_result smu_get_pbo_scalar(const struct smu_settings *s, float *scalar) {
    smu_arg_t args;
    enum smu_control_result ret;

    make_args(&args, 0);
    ret = send(s->rsmu[MSG_GET_PBO_SCALAR], SMU_TYPE_RSMU, &args);
    if (ret == SMU_CONTROL_OK)
        *scalar = args.args_f[0];
    return ret;
}

enum smu_control_result smu_set_oc_mode(const struct smu_settings *s, bool enable) {
    enum smu_message msg = enable ? MSG_ENABLE_OC_MODE : MSG_DISABLE_OC_MODE;
    enum smu_control_result ret;
    smu_arg_t args;

    if (!s->rsmu[msg] && !s->mp1[msg])
        return SMU_CONTROL_UNSUPPORTED;

    if (s->type == SETTINGS_TYPE_CPU0 || s->type == SETTINGS_TYPE_APU0) {
        // Both messages are needed: one disables PROCHOT, the other enables
        // or disables the voltage and frequency override.
        make_args(&args, enable);
        send(s->rsmu[msg], SMU_TYPE_RSMU, &args);
        make_args(&args, enable);
        ret = send(s->mp1[msg], SMU_TYPE_MP1, &args);
    } else {
        ret = send_rsmu_then_mp1(s, msg, 0, &args);
    }

    // Some firmware does not reset the scalar when OC mode is disabled.
    if (!enable && ret == SMU_CONTROL_OK)
        smu_set_pbo_scalar(s, 1);

    return ret;
}

enum smu_control_result smu_set_power_mode(const struct smu_settings *s, bool max_performance) {
    smu_arg_t args;

    make_args(&args, 0);
    return send(s->mp1[max_performance ? MSG_SET_MAX_PERFORMANCE : MSG_SET_POWER_SAVING], SMU_TYPE_MP1, &args);
}
