/**
 * Ryzen SMU Userspace Sensor Monitor
 * Copyright (C) 2020-2021
 *    Florian Huehn <hattedsquirrel@gmail.com> (https://hattedsquirrel.net)
 *    Based on work of:
 *    Leonardo Gates <leogatesx9r@protonmail.com>
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
#include <stdio.h>
#include <cpuid.h>
#include <ctype.h>
#include <libsmu.h>
#include "readinfo.h"

extern smu_obj_t obj;

#define READ_SMN_V1(offs) { if (smu_read_smn_addr(&obj, offs + offset, &value1) != SMU_Return_OK) goto _READ_ERROR; }
#define READ_SMN_V2(offs) { if (smu_read_smn_addr(&obj, offs + offset, &value2) != SMU_Return_OK) goto _READ_ERROR; }

void append_u32_to_str(char* buffer, unsigned int val) {
    buffer[0] = val & 0xff;
    buffer[1] = (val >>  8) & 0xff;
    buffer[2] = (val >> 16) & 0xff;
    buffer[3] = (val >> 24) & 0xff;
}

const char* get_processor_name() {
    unsigned int eax, ebx, ecx, edx;
    int i;
    static char buffer[50] = { 0 }, *p;

    i=0;
    __get_cpuid(0x80000002, &eax, &ebx, &ecx, &edx);
    append_u32_to_str(buffer+i, eax); i+=4;
    append_u32_to_str(buffer+i, ebx); i+=4;
    append_u32_to_str(buffer+i, ecx); i+=4;
    append_u32_to_str(buffer+i, edx); i+=4;

    __get_cpuid(0x80000003, &eax, &ebx, &ecx, &edx);
    append_u32_to_str(buffer+i, eax); i+=4;
    append_u32_to_str(buffer+i, ebx); i+=4;
    append_u32_to_str(buffer+i, ecx); i+=4;
    append_u32_to_str(buffer+i, edx); i+=4;

    __get_cpuid(0x80000004, &eax, &ebx, &ecx, &edx);
    append_u32_to_str(buffer+i, eax); i+=4;
    append_u32_to_str(buffer+i, ebx); i+=4;
    append_u32_to_str(buffer+i, ecx); i+=4;
    append_u32_to_str(buffer+i, edx); i+=4;

    // Trim whitespaces
    p = buffer;
    i = strlen(p)-1;
    while(isspace(p[i]) && i>=0) p[i--]=0;
    while(*p && isspace(*p)) p++;

    return p;
}

unsigned int count_set_bits(unsigned int v) {
    unsigned int result = 0;

    while(v!=0) {
        if(v&1) result++;
        v >>= 1;
    }

    return result;
}

/**
 * Where the CCD and core disable fuses of each processor are. The addresses
 * and layouts are those of ZenStates-Core (github.com/irusanov/ZenStates-Core),
 * which verified most of them against real processors.
 **/
struct topology_fuses {
    unsigned int ccd_fuse;              // CCD present/down fuses, two consecutive registers; 0 for a single die
    int ccd_map_from_down;              // up to 8 CCDs: the enabled CCDs are the ones not marked down
    unsigned int core_fuse;             // core disable fuse of the first CCD (or the die); 0 if not known
    int core_fuse_shift;                // bit position of the core disable mask in it
    unsigned int core_fuse_ccd_stride;  // address step between the core fuses of consecutive CCDs
    unsigned int ccx_per_ccd;
    unsigned int core_slots_per_ccx;
};

#define CCD_FUSE_ZEN2 0x5D218
#define CCD_FUSE_ZEN4 (0x5D218 + 0x1A4)
#define CCD_CORE_FUSE_STRIDE (1u << 25)

static struct topology_fuses get_topology_fuses(int codename, unsigned int fam) {
    switch (codename) {
        // Zen / Zen+ desktop and HEDT: one die, 2 CCX of 4 cores
        case CODENAME_SUMMITRIDGE:
        case CODENAME_PINNACLERIDGE:
        case CODENAME_THREADRIPPER:
        case CODENAME_COLFAX:
        case CODENAME_NAPLES:
            return (struct topology_fuses){ .core_fuse = 0x5D25C, .ccx_per_ccd = 2, .core_slots_per_ccx = 4 };

        // Zen 2 chiplets: 2 CCX of 4 per CCD
        case CODENAME_MATISSE:
        case CODENAME_CASTLEPEAK:
            return (struct topology_fuses){
                .ccd_fuse = CCD_FUSE_ZEN2, .ccd_map_from_down = 1,
                .core_fuse = 0x30081A38, .core_fuse_ccd_stride = CCD_CORE_FUSE_STRIDE,
                .ccx_per_ccd = 2, .core_slots_per_ccx = 4,
            };

        // Zen 3 chiplets: 1 CCX of 8 per CCD
        case CODENAME_VERMEER:
        case CODENAME_CHAGALL:
        case CODENAME_MILAN:
            return (struct topology_fuses){
                .ccd_fuse = CCD_FUSE_ZEN2, .ccd_map_from_down = 1,
                .core_fuse = 0x30081D98, .core_fuse_ccd_stride = CCD_CORE_FUSE_STRIDE,
                .ccx_per_ccd = 1, .core_slots_per_ccx = 8,
            };

        // Zen 4 / Zen 5 desktop: up to 2 CCDs
        case CODENAME_RAPHAEL:
        case CODENAME_DRAGONRANGE:
            return (struct topology_fuses){
                .ccd_fuse = CCD_FUSE_ZEN4,
                .core_fuse = 0x30081CD0, .core_fuse_ccd_stride = CCD_CORE_FUSE_STRIDE,
                .ccx_per_ccd = 1, .core_slots_per_ccx = 8,
            };
        case CODENAME_GRANITERIDGE:
            return (struct topology_fuses){
                .ccd_fuse = CCD_FUSE_ZEN4,
                .core_fuse = 0x304A03DC, .core_fuse_ccd_stride = CCD_CORE_FUSE_STRIDE,
                .ccx_per_ccd = 1, .core_slots_per_ccx = 8,
            };

        // Monolithic APUs: one die, the core mask sits at a bit offset in an SMU fuse register
        case CODENAME_CEZANNE:
            return (struct topology_fuses){ .core_fuse = 0x5D448, .core_fuse_shift = 11, .ccx_per_ccd = 1, .core_slots_per_ccx = 8 };
        case CODENAME_PHOENIX:
        case CODENAME_HAWKPOINT:
            return (struct topology_fuses){ .core_fuse = 0x5D528, .core_fuse_shift = 7, .ccx_per_ccd = 1, .core_slots_per_ccx = 8 };

        // Monolithic APUs whose core fuse is not known
        case CODENAME_RAVENRIDGE:
        case CODENAME_RAVENRIDGE2:
        case CODENAME_PICASSO:
        case CODENAME_DALI:
        case CODENAME_VANGOGH:
            return (struct topology_fuses){ .ccx_per_ccd = 1, .core_slots_per_ccx = 4 };
        case CODENAME_RENOIR:
        case CODENAME_LUCIENNE:
            return (struct topology_fuses){ .ccx_per_ccd = 2, .core_slots_per_ccx = 4 };
        case CODENAME_REMBRANDT:
            return (struct topology_fuses){ .ccx_per_ccd = 1, .core_slots_per_ccx = 8 };
        case CODENAME_STRIXPOINT:
            return (struct topology_fuses){ .ccx_per_ccd = 2, .core_slots_per_ccx = 4 };

        // Anything else: the CCD count is not known, assume one CCX of 8 per CCD on Zen 3 and later
        default:
            return (struct topology_fuses){ .ccx_per_ccd = fam >= 0x19 ? 1 : 2, .core_slots_per_ccx = fam >= 0x19 ? 8 : 4 };
    }
}

void get_processor_topology(system_info *sysinfo, int codename) {
    struct topology_fuses fuses;
    unsigned int eax, ebx, ecx, edx, logical_cores, threads_per_core, slots, ccd_enable_map, core_map, enabled_cores;
    unsigned int ccds_present = 0, ccds_down = 0, value;
    int physical, logical;

    __get_cpuid(0x00000001, &eax, &ebx, &ecx, &edx);
    sysinfo->family = ((eax & 0xf00) >> 8) + ((eax & 0xff00000) >> 20);
    sysinfo->model = ((eax & 0xf0000) >> 12) + ((eax & 0xf0) >> 4);
    logical_cores = (ebx >> 16) & 0xFF;

    __get_cpuid(0x8000001E, &eax, &ebx, &ecx, &edx);
    threads_per_core = ((ebx >> 8) & 0xF) + 1;
    sysinfo->cores = logical_cores / threads_per_core;

    fuses = get_topology_fuses(codename, sysinfo->family);
    slots = fuses.ccx_per_ccd * fuses.core_slots_per_ccx;

    // Enabled CCDs from the CCD fuses: present in [23:22] of the first register, down in [31:30] of the first
    // and [5:0] of the second.
    ccd_enable_map = 1;
    if (fuses.ccd_fuse &&
            smu_read_smn_addr(&obj, fuses.ccd_fuse, &ccds_present) == SMU_Return_OK &&
            smu_read_smn_addr(&obj, fuses.ccd_fuse + 4, &ccds_down) == SMU_Return_OK &&
            ccds_present != 0xFFFFFFFF) {
        unsigned int down = ((ccds_present >> 30) & 0x3) | ((ccds_down & 0x3F) << 2);
        unsigned int map = fuses.ccd_map_from_down ? ~down & 0xFF : ((ccds_present >> 22) & 0x3) & ~down;

        sysinfo->ccd_fuses[0] = ccds_present;
        sysinfo->ccd_fuses[1] = ccds_down;
        if (map)
            ccd_enable_map = map;
    }
    sysinfo->ccds = count_set_bits(ccd_enable_map);
    sysinfo->ccxs = sysinfo->ccds * fuses.ccx_per_ccd;

    // Disabled cores from the core fuse of each enabled CCD. Each CCD takes a fixed number of core slots in
    // the PM table. The map is only used if it adds up to the number of cores CPUID reports.
    core_map = 0;
    enabled_cores = 0;
    sysinfo->core_fuse = fuses.core_fuse;
    for (physical = 0, logical = 0; fuses.core_fuse && physical < 8; physical++) {
        if (!(ccd_enable_map & (1u << physical)))
            continue;
        // The map has room for 32 core slots.
        if ((logical + 1) * slots > 32) {
            enabled_cores = 0;
            break;
        }
        if (smu_read_smn_addr(&obj, fuses.core_fuse + fuses.core_fuse_ccd_stride * physical, &value) != SMU_Return_OK ||
                value == 0xFFFFFFFF) {
            enabled_cores = 0;
            break;
        }
        sysinfo->core_fuse_values[logical] = value;
        value = (value >> fuses.core_fuse_shift) & ((1u << slots) - 1);
        core_map |= value << (logical * slots);
        enabled_cores += slots - count_set_bits(value);
        logical++;
    }

    if (enabled_cores && enabled_cores == sysinfo->cores) {
        sysinfo->core_disable_map = core_map;
        sysinfo->core_map_source = "fuses";
    } else {
        sysinfo->core_disable_map = sysinfo->core_disable_map_pmt;
        sysinfo->core_map_source = "PM table";
    }
    sysinfo->enabled_cores_count = sysinfo->ccds * slots - count_set_bits(sysinfo->core_disable_map);
    sysinfo->cores_per_ccx = (slots - count_set_bits(sysinfo->core_disable_map & ((1u << slots) - 1))) / fuses.ccx_per_ccd;
    // Parts with all cores in one CCX, like the Ryzen 3 3300X
    if (sysinfo->cores == sysinfo->cores_per_ccx)
        sysinfo->ccxs = 1;

    sysinfo->available=1;
}

void print_memory_timings() {
    const char* bool_str[2] = { "Disabled", "Enabled" };
    unsigned int value1, value2, offset;

    READ_SMN_V1(0x50200);
    offset = value1 == 0x300 ? 0x100000 : 0;

    READ_SMN_V1(0x50050); READ_SMN_V2(0x50058);
    fprintf(stdout, "BankGroupSwap: %s\n",
        bool_str[!(value1 == value2 && value1 == 0x87654321)]);

    READ_SMN_V1(0x500D0); READ_SMN_V2(0x500D4);
    fprintf(stdout, "BankGroupSwapAlt: %s\n",
        bool_str[(value1 >> 4 & 0x7F) != 0 || (value2 >> 4 & 0x7F) != 0]);

    READ_SMN_V1(0x50200); READ_SMN_V2(0x50204);
    fprintf(stdout, "Memory Clock: %.0f MHz\nGDM: %s\nCR: %s\nTcl: %d\nTras: %d\nTrcdrd: %d\nTrcdwr: %d\n",
        (value1 & 0x7f) / 3.f * 100.f,
        bool_str[((value1 >> 11) & 1) == 1],
        ((value1 & 0x400) >> 10) != 0 ? "2T" : "1T",
        value2 & 0x3f,
        value2 >> 8 & 0x7f,
        value2 >> 16 & 0x3f,
        value2 >> 24 & 0x3f);

    READ_SMN_V1(0x50208); READ_SMN_V2(0x5020C);
    fprintf(stdout, "Trc: %d\nTrp: %d\nTrrds: %d\nTrrdl: %d\nTrtp: %d\n",
        value1 & 0xff,
        value1 >> 16 & 0x3f,
        value2 & 0x1f,
        value2 >> 8 & 0x1f,
        value2 >> 24 & 0x1f);

    READ_SMN_V1(0x50210); READ_SMN_V2(0x50214);
    fprintf(stdout, "Tfaw: %d\nTcwl: %d\nTwtrs: %d\nTwtrl: %d\n",
        value1 & 0xff,
        value2 & 0x3f,
        value2 >> 8 & 0x1f,
        value2 >> 16 & 0x3f);

    READ_SMN_V1(0x50218); READ_SMN_V2(0x50220);
    fprintf(stdout, "Twr: %d\nTrdrddd: %d\nTrdrdsd: %d\nTrdrdsc: %d\nTrdrdscl: %d\n",
        value1 & 0xff,
        value2 & 0xf,
        value2 >> 8 & 0xf,
        value2 >> 16 & 0xf,
        value2 >> 24 & 0x3f);

    READ_SMN_V1(0x50224); READ_SMN_V2(0x50228);
    fprintf(stdout, "Twrwrdd: %d\nTwrwrsd: %d\nTwrwrsc: %d\nTwrwrscl: %d\nTwrrd: %d\nTrdwr: %d\n",
        value1 & 0xf,
        value1 >> 8 & 0xf,
        value1 >> 16 & 0xf,
        value1 >> 24 & 0x3f,
        value2 & 0xf,
        value2 >> 8 & 0x1f);

    READ_SMN_V1(0x50254);
    fprintf(stdout, "Tcke: %d\n", value1 >> 24 & 0x1f);

    READ_SMN_V1(0x50260); READ_SMN_V2(0x50264);
    if (value1 != value2 && value1 == 0x21060138)
        value1 = value2;

    fprintf(stdout, "Trfc: %d\nTrfc2: %d\nTrfc4: %d\n",
        value1 & 0x3ff,
        value1 >> 11 & 0x3ff,
        value1 >> 22 & 0x3ff);

    exit(0);

_READ_ERROR:
    fprintf(stderr, "Unable to read SMN address space.\n");
    exit(1);
}
