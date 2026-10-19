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
 * This file contains the mapping of the Zen4 PM Table versions
 **/

#include "pm_tables.h"
#include "pm_tables_helpers.h"

void pm_table_0x540004(pm_table *pmt, void* base_addr) {
    // Zen4 Raphael 2-CCD. Per-core arrays guessed from 1-CCD layout.
    // Scalar offsets from ZenStates-Core PTDef.

    pmt->version = 0x540004;
    pmt->max_cores = 16; //Number of cores supported by this PM table version
    pmt->max_l3 = 2; //Number of L3 caches supported by this PM table version
    pmt->zen_version = 4; //Zen4
    pmt->experimental = 1;
    pmt->powersum_unclear = 1;

    pmt->STAPM_LIMIT                = pm_element( 0);
    pmt->STAPM_VALUE                = pm_element( 1);
    pmt->PPT_LIMIT                  = pm_element( 2);
    pmt->PPT_VALUE                  = pm_element( 3);
    pmt->PPT_LIMIT_FAST             = pm_element( 4);
    pmt->PPT_VALUE_FAST             = pm_element( 5);

    pmt->TDC_LIMIT                  = pm_element( 8);
    pmt->TDC_VALUE                  = pm_element( 9);
    pmt->THM_LIMIT                  = pm_element(10);
    pmt->THM_VALUE                  = pm_element(11);

    pmt->VID_VALUE                  = pm_element(19);
    pmt->VDDCR_CPU_POWER            = pm_element(20);
    pmt->VDDCR_SOC_POWER            = pm_element(21);
    pmt->ROC_POWER                  = pm_element(22);

    pmt->SOCKET_POWER               = pm_element(26);

    pmt->VID_LIMIT                  = pm_element(40);
    pmt->CPU_TELEMETRY_VOLTAGE      = pm_element(47);
    pmt->CPU_TELEMETRY_CURRENT      = pm_element(48);
    pmt->EDC_VALUE                  = pm_element(49);
    pmt->EDC_LIMIT                  = pm_element(61);

    pmt->SOC_TELEMETRY_VOLTAGE      = pm_element(52);
    pmt->SOC_TELEMETRY_CURRENT      = pm_element(53);
    pmt->SOC_TELEMETRY_POWER        = pm_element(54);

    pmt->V_VDDM                     = pm_element(56);

    pmt->FCLK_FREQ                  = pm_element(70);
    pmt->UCLK_FREQ                  = pm_element(74);
    pmt->MEMCLK_FREQ                = pm_element(78);

    pmt->FCLK_FREQ_EFF              = pm_element(105);
    pmt->UCLK_FREQ_EFF             = pm_element(106);
    pmt->MEMCLK_FREQ_EFF           = pm_element(107);

    pmt->SOC_TEMP                   = pm_element(211);

    pmt->V_VDDP                     = pm_element(268);
    pmt->PEAK_TEMP                  = pm_element(270);

    assign_pm_elements_16_consec(pmt->CORE_POWER       , 293); //?
    assign_pm_elements_16_consec(pmt->CORE_VOLTAGE     , 309); //?
    assign_pm_elements_16_consec(pmt->CORE_TEMP        , 325); //?
    assign_pm_elements_16_consec(pmt->CORE_FREQ        , 341); //?
    assign_pm_elements_16_consec(pmt->CORE_FREQEFF     , 357); //?
    assign_pm_elements_16_consec(pmt->CORE_C0          , 373); //?
    assign_pm_elements_16_consec(pmt->CORE_CC1         , 389); //?
    assign_pm_elements_16_consec(pmt->CORE_CC6         , 405); //?
    assign_pm_elements_16_consec(pmt->CORE_FIT         , 437); //?
    assign_pm_elements_16_consec(pmt->CORE_IDDMAX      , 453); //?

    pmt->min_size = 469*4; //(Highest element we access + 1)*4
}

void pm_table_0x540104(pm_table *pmt, void* base_addr) {
    // Zen4 Raphael 1-CCD. Offsets from ZenStates-Core PTDef + dump analysis.
    // Tested with: 7800X3D on X670E, SMU FW v4.84.3.0

    pmt->version = 0x540104;
    pmt->max_cores = 8; //Number of cores supported by this PM table version
    pmt->max_l3 = 1; //Number of L3 caches supported by this PM table version
    pmt->zen_version = 4; //Zen4
    pmt->experimental = 1;
    pmt->powersum_unclear = 1;

    /* Legend: o=ZenStates, p=PTDef, d=dump-confirmed, u=universal, ?=guessed */

    pmt->STAPM_LIMIT                = pm_element( 0); //u
    pmt->STAPM_VALUE                = pm_element( 1); //u
    pmt->PPT_LIMIT                  = pm_element( 2); //u
    pmt->PPT_VALUE                  = pm_element( 3); //o
    pmt->PPT_LIMIT_FAST             = pm_element( 4); //u
    pmt->PPT_VALUE_FAST             = pm_element( 5); //u

    pmt->TDC_LIMIT                  = pm_element( 8); //d
    pmt->TDC_VALUE                  = pm_element( 9); //d
    pmt->THM_LIMIT                  = pm_element(10); //d
    pmt->THM_VALUE                  = pm_element(11); //o

    pmt->VID_VALUE                  = pm_element(19); //d
    pmt->VDDCR_CPU_POWER            = pm_element(20); //o
    pmt->VDDCR_SOC_POWER            = pm_element(21); //o
    pmt->ROC_POWER                  = pm_element(22); //o

    pmt->SOCKET_POWER               = pm_element(26); //o

    pmt->VID_LIMIT                  = pm_element(40); //d
    pmt->CPU_TELEMETRY_VOLTAGE      = pm_element(47); //o
    pmt->CPU_TELEMETRY_CURRENT      = pm_element(48); //d
    pmt->EDC_VALUE                  = pm_element(49); //o
    pmt->EDC_LIMIT                  = pm_element(61); //d

    pmt->SOC_TELEMETRY_VOLTAGE      = pm_element(52); //p
    pmt->SOC_TELEMETRY_CURRENT      = pm_element(53); //d
    pmt->SOC_TELEMETRY_POWER        = pm_element(54); //d

    pmt->V_VDDM                     = pm_element(56); //p

    pmt->FCLK_FREQ                  = pm_element(70); //p
    pmt->UCLK_FREQ                  = pm_element(74); //p
    pmt->MEMCLK_FREQ                = pm_element(78); //p

    pmt->FCLK_FREQ_EFF              = pm_element(105); //d
    pmt->UCLK_FREQ_EFF             = pm_element(106); //d
    pmt->MEMCLK_FREQ_EFF           = pm_element(107); //d

    pmt->SOC_TEMP                   = pm_element(211); //o

    pmt->V_VDDP                     = pm_element(268); //p
    pmt->PEAK_TEMP                  = pm_element(270); //d

    assign_pm_elements_8_consec(pmt->CORE_POWER        , 293); //d
    assign_pm_elements_8_consec(pmt->CORE_VOLTAGE      , 301); //d
    assign_pm_elements_8_consec(pmt->CORE_TEMP         , 309); //d
    assign_pm_elements_8_consec(pmt->CORE_FREQ         , 317); //d
    assign_pm_elements_8_consec(pmt->CORE_FREQEFF      , 325); //d
    assign_pm_elements_8_consec(pmt->CORE_C0           , 333); //d
    assign_pm_elements_8_consec(pmt->CORE_CC1          , 341); //d
    assign_pm_elements_8_consec(pmt->CORE_CC6          , 349); //d
    assign_pm_elements_8_consec(pmt->CORE_FIT          , 365); //d
    assign_pm_elements_8_consec(pmt->CORE_IDDMAX       , 373); //d

    pmt->min_size = 381*4; //(Highest element we access + 1)*4
}
