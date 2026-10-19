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

/**
 * This file contains the mapping of the Zen5 PM Table versions
 **/

#include "pm_tables.h"
#include "pm_tables_helpers.h"

void pm_table_0x620205(pm_table *pmt, void* base_addr) {
    // Granite Ridge, 16 cores (Ryzen 9 9950X). Verified at idle and under
    // synthetic load, cross-checked against RAPL, k10temp and cpufreq.
    // The per-core values of sleeping cores may be stale.

    pmt->version = 0x620205;
    pmt->max_cores = 16; //Number of cores supported by this PM table version
    pmt->max_l3 = 0; //L3 cache fields are not known
    pmt->zen_version = 5; //Zen5
    pmt->powersum_unclear = 1;

    pmt->PPT_LIMIT                  = pm_element( 2);
    pmt->PPT_VALUE                  = pm_element( 3);
    pmt->TDC_LIMIT                  = pm_element( 8);
    pmt->TDC_VALUE                  = pm_element( 9);
    pmt->THM_LIMIT                  = pm_element(10);
    pmt->THM_VALUE                  = pm_element(11); //Tctl
    pmt->SOCKET_POWER               = pm_element(20);
    pmt->EDC_LIMIT                  = pm_element(63); //There is no EDC value in the table

    pmt->FCLK_FREQ                  = pm_element(71);
    pmt->UCLK_FREQ                  = pm_element(75);
    pmt->MEMCLK_FREQ                = pm_element(79);

    // 301..316: current per core
    assign_pm_elements_16_consec(pmt->CORE_VOLTAGE     , 317);
    assign_pm_elements_16_consec(pmt->CORE_TEMP        , 333);
    assign_pm_elements_16_consec(pmt->CORE_FREQEFF     , 349);
    assign_pm_elements_16_consec(pmt->CORE_POWER       , 365);

    pmt->min_size = 381*4; //(Highest element we access + 1)*4.
                           //Needed to avoid illegal memory access
}
