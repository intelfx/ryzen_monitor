/**
 * Ryzen SMU Userspace Sensor Monitor
 * Copyright (C) 2021-2022
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

#include <stddef.h>
#include <string.h>

#include "pm_tables.h"

/**
 * Every known PM Table version and the function that maps it.
 **/

static const struct {
    unsigned int version;
    void (*map)(pm_table *pmt, void *base_addr);
} pm_table_versions[] = {
    { 0x380904, pm_table_0x380904 }, //Ryzen 5600X
    { 0x380905, pm_table_0x380905 }, //Ryzen 5600X
    { 0x380804, pm_table_0x380804 }, //Ryzen 5900X / 5950X
    { 0x380805, pm_table_0x380805 }, //Ryzen 5900X / 5950X
    { 0x400005, pm_table_0x400005 }, //Ryzen 5700G
    { 0x240903, pm_table_0x240903 }, //Ryzen 3700X / 3800X
    { 0x240803, pm_table_0x240803 }, //Ryzen 3950X
    { 0x370003, pm_table_0x370003 }, //Lucienne, Renoir
    { 0x370005, pm_table_0x370005 }, //Lucienne, Renoir
};

int select_pm_table_version(unsigned int version, pm_table *pmt, unsigned char *pm_buf) {
    size_t i;

    //Initialize pmt to 0. This also sets all pointers to 0, which signifies non-existiting fields.
    //Access via pmta(...) will check if pointer is 0 before trying to access the value.
    memset(pmt, 0, sizeof(pm_table));

    //Select matching PM Table
    for (i = 0; i < sizeof(pm_table_versions) / sizeof(pm_table_versions[0]); i++)
        if (pm_table_versions[i].version == version)
            break;
    if (i == sizeof(pm_table_versions) / sizeof(pm_table_versions[0]))
        return 0;
    pm_table_versions[i].map(pmt, pm_buf);

    //Avoid access bejond bounds of the defined arrays.
    if (pmt->max_l3 > PMT_MAX_NUM_L3) pmt->max_l3 = PMT_MAX_NUM_L3;
    if (pmt->max_cores > PMT_MAX_NUM_CORES) pmt->max_cores = PMT_MAX_NUM_CORES;

    //APML_POWER is probably identical to PACKAGE_POWER
    if (pmt->PACKAGE_POWER == NULL) pmt->PACKAGE_POWER = pmt->APML_POWER;

    if (pmt->VDD18_POWER == NULL) pmt->VDD18_POWER = pmt->IO_VDD18_POWER;

    return 1;
}
