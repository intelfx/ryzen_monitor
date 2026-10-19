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
 * This file contains the mapping of the Zen2 PM Table versions
 **/

#include "pm_tables.h"
#include "pm_tables_helpers.h"

void pm_table_0x240903(pm_table *pmt, void* base_addr) {
    // Order of elements extracted from
    // https://gitlab.com/leogx9r/ryzen_smu/-/blob/master/userspace/monitor_cpu.c
    // Credit to Leonardo Gates <leogatesx9r@protonmail.com> under GPL V3
    //
    // Tested to work with Ryzen 3600. Should also work just fine for:
    // Ryzen 3600X / 3700X / 3800X / 3800XT

    pmt->version = 0x240903;
    pmt->max_cores = 8; //Number of cores supported by this PM table version
    pmt->max_l3 = 2; //Each CCX has its own L3$. Each CCD has two CCX. Hence, 2.
    pmt->zen_version = 2; //Zen2

    pmt->PPT_LIMIT                  = pm_element( 0);
    pmt->PPT_VALUE                  = pm_element( 1);
    pmt->TDC_LIMIT                  = pm_element( 2);
    pmt->TDC_VALUE                  = pm_element( 3);
    pmt->THM_LIMIT                  = pm_element( 4);
    pmt->THM_VALUE                  = pm_element( 5);
    pmt->FIT_LIMIT                  = pm_element( 6);
    pmt->FIT_VALUE                  = pm_element( 7);
    pmt->EDC_LIMIT                  = pm_element( 8);
    pmt->EDC_VALUE                  = pm_element( 9);
    pmt->VID_LIMIT                  = pm_element(10);
    pmt->VID_VALUE                  = pm_element(11);

    pmt->PPT_WC                     = pm_element(12);
    pmt->PPT_ACTUAL                 = pm_element(13);
    pmt->TDC_WC                     = pm_element(14);
    pmt->TDC_ACTUAL                 = pm_element(15);
    pmt->THM_WC                     = pm_element(16);
    pmt->THM_ACTUAL                 = pm_element(17);
    pmt->FIT_WC                     = pm_element(18);
    pmt->FIT_ACTUAL                 = pm_element(19);
    pmt->EDC_WC                     = pm_element(20);
    pmt->EDC_ACTUAL                 = pm_element(21);
    pmt->VID_WC                     = pm_element(22);
    pmt->VID_ACTUAL                 = pm_element(23);

    pmt->VDDCR_CPU_POWER            = pm_element(24);
    pmt->VDDCR_SOC_POWER            = pm_element(25);
    pmt->VDDIO_MEM_POWER            = pm_element(26);
    pmt->VDD18_POWER                = pm_element(27);
    pmt->ROC_POWER                  = pm_element(28);
    pmt->SOCKET_POWER               = pm_element(29);
    //pmt->PACKAGE_POWER   = pm_element(29); //PACKAGE_POWER does not exist in this table. It
                                           // *should* be somewhat similar to SOCKET_POWER.
                                           // However, since we show both fields in the output
                                           // there is no use in having it show the exact same
                                           // value two times. So don't set it.

    pmt->PPT_FREQUENCY              = pm_element(30);
    pmt->TDC_FREQUENCY              = pm_element(31);
    pmt->THM_FREQUENCY              = pm_element(32);
    pmt->PROCHOT_FREQUENCY          = pm_element(33);
    pmt->VOLTAGE_FREQUENCY          = pm_element(34);
    pmt->CCA_FREQUENCY              = pm_element(35);

    pmt->FIT_VOLTAGE                = pm_element(36);
    pmt->FIT_PRE_VOLTAGE            = pm_element(37);
    pmt->LATCHUP_VOLTAGE            = pm_element(38);
    pmt->CPU_SET_VOLTAGE            = pm_element(39);
    pmt->CPU_TELEMETRY_VOLTAGE      = pm_element(40);
    pmt->CPU_TELEMETRY_CURRENT      = pm_element(41);
    pmt->CPU_TELEMETRY_POWER        = pm_element(42);
    //CPU_TELEMETRY_POWER_ALT         = pm_element(43);
    pmt->SOC_SET_VOLTAGE            = pm_element(44);
    pmt->SOC_TELEMETRY_VOLTAGE      = pm_element(45);
    pmt->SOC_TELEMETRY_CURRENT      = pm_element(46);
    pmt->SOC_TELEMETRY_POWER        = pm_element(47);

    pmt->FCLK_FREQ                  = pm_element(48);
    pmt->FCLK_FREQ_EFF              = pm_element(49);
    pmt->UCLK_FREQ                  = pm_element(50);
    pmt->MEMCLK_FREQ                = pm_element(51);
    pmt->FCLK_DRAM_SETPOINT         = pm_element(52);
    pmt->FCLK_DRAM_BUSY             = pm_element(53);
    pmt->FCLK_GMI_SETPOINT          = pm_element(54);
    pmt->FCLK_GMI_BUSY              = pm_element(55);
    pmt->FCLK_IOHC_SETPOINT         = pm_element(56);
    pmt->FCLK_IOHC_BUSY             = pm_element(57);
    pmt->FCLK_XGMI_SETPOINT         = pm_element(58);
    pmt->FCLK_XGMI_BUSY             = pm_element(59);

    pmt->CCM_READS                  = pm_element(60);
    pmt->CCM_WRITES                 = pm_element(61);
    pmt->IOMS                       = pm_element(62);
    pmt->XGMI                       = pm_element(63);
    pmt->CS_UMC_READS               = pm_element(64);
    pmt->CS_UMC_WRITES              = pm_element(65);
    assign_pm_elements_4(pmt->FCLK_RESIDENCY,     66,  67,  68,  69);
    assign_pm_elements_4(pmt->FCLK_FREQ_TABLE,    70,  71,  72,  73);
    assign_pm_elements_4(pmt->UCLK_FREQ_TABLE,    74,  75,  76,  77);
    assign_pm_elements_4(pmt->MEMCLK_FREQ_TABLE,  78,  79,  80,  81);
    assign_pm_elements_4(pmt->FCLK_VOLTAGE,       82,  83,  84,  85);

    assign_pm_elements_4(pmt->LCLK_SETPOINT,  86,  92,  98, 104);
    assign_pm_elements_4(pmt->LCLK_BUSY,      87,  93,  99, 105);
    assign_pm_elements_4(pmt->LCLK_FREQ,      88,  94, 100, 106);
    assign_pm_elements_4(pmt->LCLK_FREQ_EFF,  89,  95, 101, 107);
    assign_pm_elements_4(pmt->LCLK_MAX_DPM,   90,  96, 102, 108);
    assign_pm_elements_4(pmt->LCLK_MIN_DPM,   91,  97, 103, 109);

    pmt->XGMI_SETPOINT              = pm_element(110);
    pmt->XGMI_BUSY                  = pm_element(111);
    pmt->XGMI_LANE_WIDTH            = pm_element(112);
    pmt->XGMI_DATA_RATE             = pm_element(113);

    pmt->SOC_POWER                  = pm_element(114);
    pmt->SOC_TEMP                   = pm_element(115);
    pmt->DDR_VDDP_POWER             = pm_element(116);
    pmt->DDR_VDDIO_MEM_POWER        = pm_element(117);
    pmt->GMI2_VDDG_POWER            = pm_element(118);
    pmt->IO_VDDCR_SOC_POWER         = pm_element(119);
    pmt->IOD_VDDIO_MEM_POWER        = pm_element(120);
    pmt->IO_VDD18_POWER             = pm_element(121);
    pmt->TDP                        = pm_element(122);
    pmt->DETERMINISM                = pm_element(123);
    pmt->V_VDDG_CCD                 = pm_element(124); //Same as V_VDDM in newer AGESA versions
    pmt->V_VDDP                     = pm_element(125);
    pmt->V_VDDG_IOD                 = pm_element(126); //Same as V_VDDG in newer AGESA versions

    pmt->PEAK_TEMP                  = pm_element(127);
    pmt->PEAK_VOLTAGE               = pm_element(128);
    pmt->AVG_CORE_COUNT             = pm_element(129);
    pmt->CCLK_LIMIT                 = pm_element(130);
    pmt->MAX_SOC_VOLTAGE            = pm_element(131);
    pmt->DC_BTC                     = pm_element(132);
    //pmt->PACKAGE_POWER              = pm_element(xxx); //Does not exist. Use SOCKET_POWER
    pmt->CSTATE_BOOST               = pm_element(133);
    pmt->PROCHOT                    = pm_element(134);
    pmt->PC6                        = pm_element(135);
    pmt->PWM                        = pm_element(136);

    pmt->SOCCLK                     = pm_element(137);
    pmt->SHUBCLK                    = pm_element(138);
    pmt->MP0CLK                     = pm_element(139);
    pmt->MP1CLK                     = pm_element(140);
    pmt->MP5CLK                     = pm_element(141);
    pmt->SMNCLK                     = pm_element(142);
    pmt->TWIXCLK                    = pm_element(143);
    pmt->WAFLCLK                    = pm_element(144);

    pmt->DPM_BUSY                   = pm_element(145);
    pmt->MP1_BUSY                   = pm_element(146);

    assign_pm_elements_8_consec(pmt->CORE_POWER       , 147);
    assign_pm_elements_8_consec(pmt->CORE_VOLTAGE     , 155);
    assign_pm_elements_8_consec(pmt->CORE_TEMP        , 163);
    assign_pm_elements_8_consec(pmt->CORE_FIT         , 171);
    assign_pm_elements_8_consec(pmt->CORE_IDDMAX      , 179);
    assign_pm_elements_8_consec(pmt->CORE_FREQ        , 187);
    assign_pm_elements_8_consec(pmt->CORE_FREQEFF     , 195);
    assign_pm_elements_8_consec(pmt->CORE_C0          , 203);
    assign_pm_elements_8_consec(pmt->CORE_CC1         , 211);
    assign_pm_elements_8_consec(pmt->CORE_CC6         , 219);
    assign_pm_elements_8_consec(pmt->CORE_CKS_FDD     , 227);
    assign_pm_elements_8_consec(pmt->CORE_CI_FDD      , 235);
    assign_pm_elements_8_consec(pmt->CORE_IRM         , 243);
    assign_pm_elements_8_consec(pmt->CORE_PSTATE      , 251);
    assign_pm_elements_8_consec(pmt->CORE_FREQ_LIM_MAX, 259);
    assign_pm_elements_8_consec(pmt->CORE_FREQ_LIM_MIN, 267);
    assign_pm_elements_8_consec(pmt->CORE_SC_LIMIT    , 275);
    assign_pm_elements_8_consec(pmt->CORE_SC_CAC      , 283);
    assign_pm_elements_8_consec(pmt->CORE_SC_RESIDENCY, 291);

    assign_pm_elements_2(pmt->L3_LOGIC_POWER   , 299, 300);
    assign_pm_elements_2(pmt->L3_VDDM_POWER    , 301, 302);
    assign_pm_elements_2(pmt->L3_TEMP          , 303, 304);
    assign_pm_elements_2(pmt->L3_FIT           , 305, 306);
    assign_pm_elements_2(pmt->L3_IDDMAX        , 307, 308);
    assign_pm_elements_2(pmt->L3_FREQ          , 309, 310);
    assign_pm_elements_2(pmt->L3_CKS_FDD       , 311, 312);
    assign_pm_elements_2(pmt->L3_CCA_THRESHOLD , 313, 314);
    assign_pm_elements_2(pmt->L3_CCA_CAC       , 315, 316);
    assign_pm_elements_2(pmt->L3_CCA_ACTIVATION, 317, 318);
    assign_pm_elements_2(pmt->L3_EDC_LIMIT     , 319, 320);
    assign_pm_elements_2(pmt->L3_EDC_CAC       , 321, 322);
    assign_pm_elements_2(pmt->L3_EDC_RESIDENCY , 323, 324);
    pmt->MP5_BUSY[0] = pm_element(325);

    pmt->min_size = 326*4; //(Highest element we access + 1)*4.
                           //Needed to avoid illegal memory access
}

void pm_table_0x240803(pm_table *pmt, void* base_addr) {
    // Ryzen 3950X
    // Could not test myself, as I dont have the hardware. But most values seem reasonable.
    // (Re)constructed using the pm table dumps from 
    // https://github.com/hattedsquirrel/ryzen_monitor/issues/2#

    pmt->version = 0x240803;
    pmt->max_cores = 16; //Number of cores supported by this PM table version
    pmt->max_l3 = 4; //Number of L3 caches supported by this PM table version
    pmt->zen_version = 2; //Zen2

    pmt->PPT_LIMIT                  = pm_element( 0);
    pmt->PPT_VALUE                  = pm_element( 1);
    pmt->TDC_LIMIT                  = pm_element( 2);
    pmt->TDC_VALUE                  = pm_element( 3);
    pmt->THM_LIMIT                  = pm_element( 4);
    pmt->THM_VALUE                  = pm_element( 5);
    pmt->FIT_LIMIT                  = pm_element( 6);
    pmt->FIT_VALUE                  = pm_element( 7);
    pmt->EDC_LIMIT                  = pm_element( 8);
    pmt->EDC_VALUE                  = pm_element( 9);
    pmt->VID_LIMIT                  = pm_element(10);
    pmt->VID_VALUE                  = pm_element(11);

    pmt->PPT_WC                     = pm_element(12);
    pmt->PPT_ACTUAL                 = pm_element(13);
    pmt->TDC_WC                     = pm_element(14);
    pmt->TDC_ACTUAL                 = pm_element(15);
    pmt->THM_WC                     = pm_element(16);
    pmt->THM_ACTUAL                 = pm_element(17);
    pmt->FIT_WC                     = pm_element(18);
    pmt->FIT_ACTUAL                 = pm_element(19);
    pmt->EDC_WC                     = pm_element(20);
    pmt->EDC_ACTUAL                 = pm_element(21);
    pmt->VID_WC                     = pm_element(22);
    pmt->VID_ACTUAL                 = pm_element(23);

    pmt->VDDCR_CPU_POWER            = pm_element(24);
    pmt->VDDCR_SOC_POWER            = pm_element(25);
    pmt->VDDIO_MEM_POWER            = pm_element(26);
    pmt->VDD18_POWER                = pm_element(27);
    pmt->ROC_POWER                  = pm_element(28);
    pmt->SOCKET_POWER               = pm_element(29);
    //pmt->PACKAGE_POWER   = pm_element(29); //PACKAGE_POWER does not exist in this table. It
                                           // *should* be somewhat similar to SOCKET_POWER.
                                           // However, since we show both fields in the output
                                           // there is no use in having it show the exact same
                                           // value two times. So don't set it.

    pmt->PPT_FREQUENCY              = pm_element(30);
    pmt->TDC_FREQUENCY              = pm_element(31);
    pmt->THM_FREQUENCY              = pm_element(32);
    pmt->PROCHOT_FREQUENCY          = pm_element(33);
    pmt->VOLTAGE_FREQUENCY          = pm_element(34);
    pmt->CCA_FREQUENCY              = pm_element(35);

    pmt->FIT_VOLTAGE                = pm_element(37);
    pmt->FIT_PRE_VOLTAGE            = pm_element(38);
    pmt->LATCHUP_VOLTAGE            = pm_element(39);
    pmt->CPU_SET_VOLTAGE            = pm_element(40);
    pmt->CPU_TELEMETRY_VOLTAGE      = pm_element(41);
    pmt->CPU_TELEMETRY_CURRENT      = pm_element(42);
    pmt->CPU_TELEMETRY_POWER        = pm_element(43);
    pmt->SOC_SET_VOLTAGE            = pm_element(44);
    pmt->SOC_TELEMETRY_VOLTAGE      = pm_element(45);
    pmt->SOC_TELEMETRY_CURRENT      = pm_element(46);
    pmt->SOC_TELEMETRY_POWER        = pm_element(47);

    pmt->FCLK_FREQ                  = pm_element(48);
    pmt->FCLK_FREQ_EFF              = pm_element(49);
    pmt->UCLK_FREQ                  = pm_element(50);
    pmt->MEMCLK_FREQ                = pm_element(51);
    pmt->FCLK_DRAM_SETPOINT         = pm_element(52);
    pmt->FCLK_DRAM_BUSY             = pm_element(53);
    pmt->FCLK_GMI_SETPOINT          = pm_element(54);
    pmt->FCLK_GMI_BUSY              = pm_element(55);
    pmt->FCLK_IOHC_SETPOINT         = pm_element(56);
    pmt->FCLK_IOHC_BUSY             = pm_element(57);
    pmt->FCLK_XGMI_SETPOINT         = pm_element(58);
    pmt->FCLK_XGMI_BUSY             = pm_element(59);

    pmt->CCM_READS                  = pm_element(60);
    pmt->CCM_WRITES                 = pm_element(61);
    pmt->IOMS                       = pm_element(62);
    pmt->XGMI                       = pm_element(63);
    pmt->CS_UMC_READS               = pm_element(64);
    pmt->CS_UMC_WRITES              = pm_element(65);
    assign_pm_elements_4(pmt->FCLK_RESIDENCY,     66,  67,  68,  69);
    assign_pm_elements_4(pmt->FCLK_FREQ_TABLE,    70,  71,  72,  73);
    assign_pm_elements_4(pmt->UCLK_FREQ_TABLE,    74,  75,  76,  77);
    assign_pm_elements_4(pmt->MEMCLK_FREQ_TABLE,  78,  79,  80,  81);
    assign_pm_elements_4(pmt->FCLK_VOLTAGE,       82,  83,  84,  85);

    assign_pm_elements_4(pmt->LCLK_SETPOINT,  86,  92,  98, 104);
    assign_pm_elements_4(pmt->LCLK_BUSY,      87,  93,  99, 105);
    assign_pm_elements_4(pmt->LCLK_FREQ,      88,  94, 100, 106);
    assign_pm_elements_4(pmt->LCLK_FREQ_EFF,  89,  95, 101, 107);
    assign_pm_elements_4(pmt->LCLK_MAX_DPM,   90,  96, 102, 108);
    assign_pm_elements_4(pmt->LCLK_MIN_DPM,   91,  97, 103, 109);

    pmt->XGMI_SETPOINT              = pm_element(110);
    pmt->XGMI_BUSY                  = pm_element(111);
    pmt->XGMI_LANE_WIDTH            = pm_element(112);
    pmt->XGMI_DATA_RATE             = pm_element(113);

    pmt->SOC_POWER                  = pm_element(114);
    pmt->SOC_TEMP                   = pm_element(115);
    pmt->DDR_VDDP_POWER             = pm_element(116);
    pmt->DDR_VDDIO_MEM_POWER        = pm_element(117);
    pmt->GMI2_VDDG_POWER            = pm_element(118);
    pmt->IO_VDDCR_SOC_POWER         = pm_element(119);
    pmt->IOD_VDDIO_MEM_POWER        = pm_element(120);
    pmt->IO_VDD18_POWER             = pm_element(121);
    pmt->TDP                        = pm_element(122);
    pmt->DETERMINISM                = pm_element(123);
    pmt->V_VDDM                     = pm_element(124);
    pmt->V_VDDP                     = pm_element(125);
    pmt->V_VDDG                     = pm_element(126);

    pmt->PEAK_TEMP                  = pm_element(127);
    pmt->PEAK_VOLTAGE               = pm_element(128);
    pmt->AVG_CORE_COUNT             = pm_element(129);
    pmt->CCLK_LIMIT                 = pm_element(130);
    pmt->MAX_SOC_VOLTAGE            = pm_element(131);
    pmt->DC_BTC                     = pm_element(132);
    //pmt->PACKAGE_POWER              = pm_element(xxx); //Does not exist. Use SOCKET_POWER
    pmt->CSTATE_BOOST               = pm_element(133);
    pmt->PROCHOT                    = pm_element(134);
    pmt->PC6                        = pm_element(135);
    pmt->PWM                        = pm_element(136);

    pmt->SOCCLK                     = pm_element(137); //elem 137 -> 457.14
    pmt->SHUBCLK                    = pm_element(138); //elem 138 -> 457.14
    pmt->MP0CLK                     = pm_element(139); //elem 139 -> 457.14
    pmt->MP1CLK                     = pm_element(140); //elem 140 -> 457.14
    pmt->MP5CLK                     = pm_element(141); //elem 141 -> 400.00
    pmt->SMNCLK                     = pm_element(142); //elem 142 -> 500.00
    pmt->TWIXCLK                    = pm_element(143); //elem 143 -> 400.00
    pmt->WAFLCLK                    = pm_element(144);

    pmt->DPM_BUSY                   = pm_element(145);
    pmt->MP1_BUSY                   = pm_element(146);

    assign_pm_elements_16_consec(pmt->CORE_POWER       , 147);
    assign_pm_elements_16_consec(pmt->CORE_VOLTAGE     , 163);
    assign_pm_elements_16_consec(pmt->CORE_TEMP        , 179);
    assign_pm_elements_16_consec(pmt->CORE_FIT         , 195);
    assign_pm_elements_16_consec(pmt->CORE_IDDMAX      , 211);
    assign_pm_elements_16_consec(pmt->CORE_FREQ        , 227);
    assign_pm_elements_16_consec(pmt->CORE_FREQEFF     , 243);
    assign_pm_elements_16_consec(pmt->CORE_C0          , 259);
    assign_pm_elements_16_consec(pmt->CORE_CC1         , 275);
    assign_pm_elements_16_consec(pmt->CORE_CC6         , 291);
    assign_pm_elements_16_consec(pmt->CORE_CKS_FDD     , 307);
    assign_pm_elements_16_consec(pmt->CORE_CI_FDD      , 323);
    assign_pm_elements_16_consec(pmt->CORE_IRM         , 339);
    assign_pm_elements_16_consec(pmt->CORE_PSTATE      , 355);
    assign_pm_elements_16_consec(pmt->CORE_FREQ_LIM_MAX, 371);
    assign_pm_elements_16_consec(pmt->CORE_FREQ_LIM_MIN, 387);
    assign_pm_elements_16_consec(pmt->CORE_SC_LIMIT    , 403);
    assign_pm_elements_16_consec(pmt->CORE_SC_CAC      , 419);
    assign_pm_elements_16_consec(pmt->CORE_SC_RESIDENCY, 435);

    assign_pm_elements_4_consec(pmt->L3_LOGIC_POWER   , 451);
    assign_pm_elements_4_consec(pmt->L3_VDDM_POWER    , 455);
    assign_pm_elements_4_consec(pmt->L3_TEMP          , 459);
    assign_pm_elements_4_consec(pmt->L3_FIT           , 463);
    assign_pm_elements_4_consec(pmt->L3_IDDMAX        , 467);
    assign_pm_elements_4_consec(pmt->L3_FREQ          , 471);
    assign_pm_elements_4_consec(pmt->L3_CKS_FDD       , 475);
    assign_pm_elements_4_consec(pmt->L3_CCA_THRESHOLD , 479);
    assign_pm_elements_4_consec(pmt->L3_CCA_CAC       , 483);
    assign_pm_elements_4_consec(pmt->L3_CCA_ACTIVATION, 487);
    assign_pm_elements_4_consec(pmt->L3_EDC_LIMIT     , 491);
    assign_pm_elements_4_consec(pmt->L3_EDC_CAC       , 495);
    assign_pm_elements_4_consec(pmt->L3_EDC_RESIDENCY , 499);
    assign_pm_elements_2(pmt->MP5_BUSY, 503, 504);

    pmt->min_size = 505*4; //(Highest element we access + 1)*4.
                           //Needed to avoid illegal memory access
}

void pm_table_0x370003(pm_table *pmt, void* base_addr) {

    pmt->version = 0x370003;
    pmt->max_cores = 8; //Number of cores supported by this PM table version
    pmt->max_l3 = 1; //Number of L3 caches supported by this PM table version
    pmt->zen_version = 2; //Zen2
    pmt->experimental = 0; //Print experimental note
    pmt->powersum_unclear = 1; //No idea how to calculatec the total power
    pmt->has_graphics = 1; //Print GFX information


    pmt->STAPM_LIMIT                = pm_element(0);
    pmt->STAPM_VALUE                = pm_element(1);
    pmt->PPT_LIMIT_FAST             = pm_element(2);
    pmt->PPT_VALUE_FAST             = pm_element(3);
    pmt->PPT_LIMIT                  = pm_element(4); //Slow
    pmt->PPT_VALUE                  = pm_element(5); //Slow
    pmt->PPT_LIMIT_APU              = pm_element(6);
    pmt->PPT_VALUE_APU              = pm_element(7);
    pmt->TDC_LIMIT                  = pm_element(8); //VDD
    pmt->TDC_VALUE                  = pm_element(9); //VDD
    pmt->TDC_LIMIT_SOC              = pm_element(10);
    pmt->TDC_VALUE_SOC              = pm_element(11);
    pmt->EDC_LIMIT                  = pm_element(12); //Core
    pmt->EDC_VALUE                  = pm_element(13); //Core
    pmt->EDC_LIMIT_SOC              = pm_element(14);
    pmt->EDC_VALUE_SOC              = pm_element(15);
    pmt->THM_LIMIT                  = pm_element(16); //Core
    pmt->THM_VALUE                  = pm_element(17); //Core
    pmt->THM_LIMIT_GFX              = pm_element(18);
    pmt->THM_VALUE_GFX              = pm_element(19);
    pmt->THM_LIMIT_SOC              = pm_element(20);
    pmt->THM_VALUE_SOC              = pm_element(21);
    pmt->STT_LIMIT_APU              = pm_element(22);
    pmt->STT_VALUE_APU              = pm_element(23);
    pmt->STT_LIMIT_DGPU             = pm_element(24);
    pmt->STT_VALUE_DGPU             = pm_element(25);
    pmt->FIT_LIMIT                  = pm_element(26);
    pmt->FIT_VALUE                  = pm_element(27);
    pmt->VID_LIMIT                  = pm_element(28);
    pmt->VID_VALUE                  = pm_element(29);
    pmt->PSI0_LIMIT_VDD             = pm_element(30);
    pmt->PSI0_RESIDENCY_VDD         = pm_element(31);
    pmt->PSI0_LIMIT_SOC             = pm_element(32);
    pmt->PSI0_RESIDENCY_SOC         = pm_element(33);

    
    pmt->VDDCR_CPU_POWER            = pm_element(34);
    pmt->VDDCR_SOC_POWER            = pm_element(35);
    pmt->VDDIO_MEM_POWER            = pm_element(36);
    pmt->ROC_POWER                  = pm_element(37);
    pmt->SOCKET_POWER               = pm_element(38);
    pmt->TDP                        = pm_element(38); //Don't know where to get TDP from. Set equal to socket power for now
    pmt->PACKAGE_POWER              = pm_element(38); //Doesn't seem to have package power. Use socket power instead.
    pmt->GLOB_FREQUENCY             = pm_element(39);
    pmt->STAPM_FREQUENCY            = pm_element(40);
    pmt->PPT_FREQUENCY_FAST         = pm_element(41);
    pmt->PPT_FREQUENCY              = pm_element(42); //PPT_FREQUENCY_SLOW
    pmt->PPT_FREQUENCY_APU          = pm_element(43);
    pmt->TDC_FREQUENCY              = pm_element(44);
    pmt->THM_FREQUENCY              = pm_element(45);
    pmt->HTFMAX_FREQUENCY           = pm_element(46);
    pmt->PROCHOT_FREQUENCY          = pm_element(47);
    pmt->VOLTAGE_FREQUENCY          = pm_element(48);
    pmt->CCA_FREQUENCY              = pm_element(49);
    pmt->GFX_GLOB_FREQUENCY         = pm_element(50);
    pmt->GFX_STAPM_FREQUENCY        = pm_element(51);
    pmt->GFX_PPT_FREQUENCY_FAST     = pm_element(52);
    pmt->GFX_PPT_FREQUENCY          = pm_element(53);
    pmt->GFX_PPT_FREQUENCY_APU      = pm_element(54);
    pmt->GFX_TDC_FREQUENCY          = pm_element(55);
    pmt->GFX_THM_FREQUENCY          = pm_element(56);
    pmt->GFX_HTFMAX_FREQUENCY       = pm_element(57);
    pmt->GFX_PROCHOT_FREQUENCY      = pm_element(58);
    pmt->GFX_VOLTAGE_FREQUENCY      = pm_element(59);
    pmt->GFX_CCA_FREQUENCY          = pm_element(60);
    pmt->GFX_DEM_FREQUENCY          = pm_element(60); // NO DEM FREQ
    
    pmt->FIT_VOLTAGE                = pm_element(61);
    pmt->LATCHUP_VOLTAGE            = pm_element(62);
    pmt->CORE_SETPOINT              = pm_element(63);
    pmt->CORE_BUSY                  = pm_element(64);
    pmt->GFX_SETPOINT               = pm_element(65);
    pmt->GFX_BUSY                   = pm_element(66);
    pmt->FCLK_CCLK_SETPOINT         = pm_element(67);
    pmt->FCLK_CCLK_FREQ             = pm_element(68);
    pmt->FCLK_GFX_SETPOINT          = pm_element(69);
    pmt->FCLK_GFX_BUSY              = pm_element(70);
    pmt->FCLK_IOHC_SETPOINT         = pm_element(71);
    pmt->FCLK_IOHC_BUSY             = pm_element(72);
    pmt->FCLK_DRAM_SETPOINT         = pm_element(73);
    pmt->FCLK_DRAM_BUSY             = pm_element(74);
    pmt->LCLK_SETPOINT[0]           = pm_element(75);
    pmt->LCLK_BUSY[0]               = pm_element(76);
    
    assign_pm_elements_4(pmt->FCLK_RESIDENCY,     77,  78,  79,  80);
    assign_pm_elements_4(pmt->FCLK_FREQ_TABLE,    81,  82,  83,  84);
    assign_pm_elements_4(pmt->UCLK_FREQ_TABLE,    85,  86,  87,  88);
    assign_pm_elements_4(pmt->MEMCLK_FREQ_TABLE,  89,  90,  91,  92);
    assign_pm_elements_4(pmt->FCLK_VOLTAGE,       93,  94,  95,  96);

    pmt->CPU_SET_VOLTAGE            = pm_element(97);
    pmt->CPU_TELEMETRY_VOLTAGE      = pm_element(98);
    pmt->CPU_TELEMETRY_CURRENT      = pm_element(99);
    pmt->CPU_TELEMETRY_POWER        = pm_element(100);
    pmt->SOC_SET_VOLTAGE            = pm_element(101);
    pmt->SOC_TELEMETRY_VOLTAGE      = pm_element(102);
    pmt->SOC_TELEMETRY_CURRENT      = pm_element(103);
    pmt->SOC_TELEMETRY_POWER        = pm_element(104);

    //Bunch of zeros
    
    pmt->DF_BUSY                    = pm_element(165);
    pmt->VCN_BUSY                   = pm_element(166);
    pmt->IOHC_BUSY	                = pm_element(167);
    pmt->MMHUB_BUSY                 = pm_element(168);
    pmt->ATHUB_BUSY                 = pm_element(169);
    pmt->OSSSYS_BUSY                = pm_element(170);
    pmt->HDP_BUSY                   = pm_element(171);
    pmt->SDMA_BUSY                  = pm_element(172);
    pmt->SHUB_BUSY                  = pm_element(173);
    pmt->BIF_BUSY                   = pm_element(174);
    pmt->ACP_BUSY                   = pm_element(175);
    pmt->SST0_BUSY                  = pm_element(176);
    pmt->SST1_BUSY                  = pm_element(177);
    pmt->USB0_BUSY                  = pm_element(178);
    pmt->USB1_BUSY                  = pm_element(179);
    pmt->CCM_READS                  = pm_element(180);
    pmt->CCM_WRITES                 = pm_element(181);
    pmt->GCM_64B_READS              = pm_element(182);
    pmt->GCM_64B_WRITES             = pm_element(183);
    pmt->GCM_32B_READS_WRITES       = pm_element(184);
    pmt->MMHUB_READS                = pm_element(185);
    pmt->MMHUB_WRITES               = pm_element(186);
    pmt->DCE_READS                  = pm_element(187);
    pmt->IO_READS_WRITES            = pm_element(188);
    pmt->CS_UMC_READS               = pm_element(189);
    pmt->CS_UMC_WRITES              = pm_element(190);
    pmt->MAX_DRAM_BANDWIDTH         = pm_element(191);
    //pmt->VCN_BUSY                   = pm_element(192);
    //pmt->VCN_DECODE                 = pm_element(193);
    //pmt->VCN_ENCODE_GEN             = pm_element(194);
    //pmt->VCN_ENCODE_LOW             = pm_element(195);
    //pmt->VCN_ENCODE_REAL            = pm_element(196);
    //pmt->VCN_PG                     = pm_element(197);
    //pmt->VCN_JPEG                   = pm_element(198);

    assign_pm_elements_8_consec(pmt->CORE_POWER         , 192);
    assign_pm_elements_8_consec(pmt->CORE_VOLTAGE       , 200);
    assign_pm_elements_8_consec(pmt->CORE_TEMP          , 208);
    assign_pm_elements_8_consec(pmt->CORE_FIT           , 216);
    assign_pm_elements_8_consec(pmt->CORE_IDDMAX        , 224);
    assign_pm_elements_8_consec(pmt->CORE_FREQ          , 232);
    assign_pm_elements_8_consec(pmt->CORE_FREQEFF       , 240);
    assign_pm_elements_8_consec(pmt->CORE_C0            , 248);
    assign_pm_elements_8_consec(pmt->CORE_CC1           , 256);
    assign_pm_elements_8_consec(pmt->CORE_CC6           , 264);
    assign_pm_elements_8_consec(pmt->CORE_CKS_FDD       , 272);
    //assign_pm_elements_8_consec(pmt->CORE_CI_FDD        , 288);
    //assign_pm_elements_8_consec(pmt->CORE_IRM           , 296);
    assign_pm_elements_8_consec(pmt->CORE_PSTATE        , 280);
    assign_pm_elements_8_consec(pmt->CORE_CPPC_MAX      , 288);
    assign_pm_elements_8_consec(pmt->CORE_CPPC_MIN      , 296);
    assign_pm_elements_8_consec(pmt->CORE_CPPC_EPP      , 304);
    assign_pm_elements_8_consec(pmt->CORE_SC_LIMIT      , 312);
    assign_pm_elements_8_consec(pmt->CORE_SC_CAC        , 320);
    assign_pm_elements_8_consec(pmt->CORE_SC_RESIDENCY  , 328);
    //assign_pm_elements_8_consec(pmt->CORE_UOPS_CLK      , 360);
    //assign_pm_elements_8_consec(pmt->CORE_UOPS          , 368);
    //assign_pm_elements_8_consec(pmt->CORE_MEM_LATECY    , 376);
    
    pmt->L3_LOGIC_POWER[0]          = pm_element(336);
    pmt->L3_VDDM_POWER[0]           = pm_element(338);
    pmt->L3_TEMP[0]                 = pm_element(340);
    pmt->L3_FIT[0]                  = pm_element(342);
    pmt->L3_IDDMAX[0]               = pm_element(344);
    pmt->L3_FREQ[0]                 = pm_element(346);
    pmt->L3_FREQ_EFF[0]             = pm_element(348);
    pmt->L3_CKS_FDD[0]              = pm_element(350);
    pmt->L3_CCA_THRESHOLD[0]        = pm_element(352);
    pmt->L3_CCA_CAC[0]              = pm_element(354);
    pmt->L3_CCA_ACTIVATION[0]       = pm_element(356);
    pmt->L3_EDC_LIMIT[0]            = pm_element(358);
    pmt->L3_EDC_CAC[0]              = pm_element(360);
    pmt->L3_EDC_RESIDENCY[0]        = pm_element(362);
    //pmt->L3_FLL_BTC[0]              = pm_element(364);

    pmt->GFX_VOLTAGE                = pm_element(362);
    pmt->GFX_TEMP                   = pm_element(363);
    pmt->GFX_IDDMAX                 = pm_element(364);
    pmt->GFX_FREQ                   = pm_element(365);
    pmt->GFX_FREQEFF                = pm_element(366);
    pmt->GFX_BUSY                   = pm_element(367);
    pmt->GFX_CGPG                   = pm_element(368);
    pmt->GFX_EDC_LIM                = pm_element(369);
    pmt->GFX_EDC_RESIDENCY          = pm_element(370);
    //pmt->GFX_DEM_RESIDENCY          = pm_element(371);

    pmt->FCLK_FREQ                  = pm_element(371);
    pmt->UCLK_FREQ                  = pm_element(372);
    pmt->MEMCLK_FREQ                = pm_element(373);
    pmt->VCLK_FREQ                  = pm_element(374);
    pmt->DCLK_FREQ                  = pm_element(375);
    pmt->SOCCLK                     = pm_element(376);
    pmt->LCLK_FREQ[0]               = pm_element(377);
    pmt->SHUBCLK                    = pm_element(378);
    pmt->MP0CLK                     = pm_element(379);
    pmt->DCF_FREQ                   = pm_element(380);
    pmt->FCLK_FREQ_EFF              = pm_element(381);
    pmt->UCLK_FREQ_EFF              = pm_element(382);
    pmt->MEMCLK_FREQ_EFF            = pm_element(383);
    pmt->VCLK_FREQ_EFF              = pm_element(384);
    pmt->DCLK_FREQ_EFF              = pm_element(385);
    pmt->SOCCLK_FREQ_EFF[0]         = pm_element(386);
    pmt->LCLK_FREQ_EFF[0]           = pm_element(387);
    pmt->SHUBCLK_FREQ_EFF[0]        = pm_element(388);
    pmt->MP0CLK_EFF                 = pm_element(389);
    pmt->DCF_FREQ_EFF               = pm_element(390);

    assign_pm_elements_8_consec(pmt->VCLK_STATE            , 391);
    assign_pm_elements_8_consec(pmt->DCLK_STATE            , 399);
    assign_pm_elements_8_consec(pmt->SOCCLK_STATE          , 407);
    assign_pm_elements_8_consec(pmt->LCLK_STATE            , 415);
    assign_pm_elements_8_consec(pmt->SHUB_STATE            , 423);
    assign_pm_elements_8_consec(pmt->MP0_STATE             , 431);
    assign_pm_elements_8_consec(pmt->DCFCLK_STATE          , 439);
    assign_pm_elements_8_consec(pmt->VCN_STATE_RESIDENCY   , 447);
    assign_pm_elements_8_consec(pmt->SOCCLK_STATE_RESIDENCY, 455);
    assign_pm_elements_8_consec(pmt->LCLK_STATE_RESIDENCY  , 463);
    assign_pm_elements_8_consec(pmt->SHUB_STATE_RESIDENCY  , 471);
    assign_pm_elements_8_consec(pmt->MP0CLK_STATE_RESIDENCY, 479);
    assign_pm_elements_8_consec(pmt->DCFCLK_STATE_RESIDENCY, 487);
    assign_pm_elements_8_consec(pmt->VDDCR_SOC_VOLTAGE     , 495);

    pmt->CPUOFF                     = pm_element(503);
    pmt->CPUOFF_CNT                 = pm_element(504);
    pmt->GFXOFF                     = pm_element(505);
    pmt->GFXOFF_CNT                 = pm_element(506);
    pmt->VDDOFF                     = pm_element(507);
    pmt->VDDOFF_CNT                 = pm_element(508);
    pmt->ULV                        = pm_element(509);
    pmt->ULV_CNT                    = pm_element(510);
    pmt->S0i2                       = pm_element(511);
    pmt->S0i2_CNT                   = pm_element(512);
    pmt->WHISPER                    = pm_element(513);
    pmt->WHISPER_CNT                = pm_element(514);
    pmt->SELFREFRESH0               = pm_element(515);
    pmt->SELFREFRESH1               = pm_element(516);
    pmt->PLL_POWERDOWN_0            = pm_element(517);
    pmt->PLL_POWERDOWN_1            = pm_element(518);
    pmt->PLL_POWERDOWN_2            = pm_element(519);
    pmt->PLL_POWERDOWN_3            = pm_element(520);
    pmt->PLL_POWERDOWN_4            = pm_element(521);
    //pmt->POWER_SOURCE               = pm_element(560);
    pmt->DGPU_POWER                 = pm_element(523);
    pmt->DGPU_GFX_BUSY              = pm_element(524);
    pmt->DGPU_FREQ_TARGET           = pm_element(525);
    pmt->V_VDDM                     = pm_element(526); //CLDO_VDDM
    pmt->V_VDDP                     = pm_element(527); //CLDO_VDDP
    pmt->DDR_PHY_POWER              = pm_element(528);
    pmt->IOD_VDDIO_MEM_POWER        = pm_element(529);
    pmt->IO_VDD18_POWER             = pm_element(530);
    pmt->IO_DISPLAY_POWER           = pm_element(531);
    pmt->IO_USB_POWER               = pm_element(532);
    pmt->ULV_VOLTAGE                = pm_element(533);
    pmt->PEAK_TEMP                  = pm_element(534);
    pmt->PEAK_VOLTAGE               = pm_element(535);
    pmt->AVG_CORE_COUNT             = pm_element(536);
    pmt->MAX_CORE_VOLTAGE           = pm_element(537);
    pmt->DC_BTC                     = pm_element(538);
    pmt->CSTATE_BOOST               = pm_element(539);
    pmt->PROCHOT                    = pm_element(540);
    pmt->PWM                        = pm_element(541);
    pmt->FPS                        = pm_element(542);
    pmt->DISPLAY_COUNT              = pm_element(543);
    pmt->StapmTimeConstant          = pm_element(544);
    pmt->SlowPPTTimeConstant        = pm_element(545);
    /*
    pmt->MP1CLK                     = pm_element(546);
    pmt->MP2CLK                     = pm_element(547);
    pmt->SMNCLK                     = pm_element(548);
    pmt->ACLK                       = pm_element(549);
    pmt->DISPCLK                    = pm_element(550);
    pmt->DPREFCLK                   = pm_element(551);
    pmt->DPPCLK                     = pm_element(552);
    pmt->SMU_BUSY                   = pm_element(553);
    pmt->SMU_SKIP_COUNTER           = pm_element(554);
    */

    pmt->min_size = 545*4; //(Highest element we access + 1)*4.
    //Needed to avoid illegal memory access
}

void pm_table_0x370005(pm_table *pmt, void* base_addr) {

    pmt->version = 0x370005;
    pmt->max_cores = 8; //Number of cores supported by this PM table version
    pmt->max_l3 = 1; //Number of L3 caches supported by this PM table version
    pmt->zen_version = 2; //Zen2
    pmt->experimental = 0; //Print experimental note
    pmt->powersum_unclear = 1; //No idea how to calculatec the total power
    pmt->has_graphics = 1; //Print GFX information

    pmt->STAPM_LIMIT                = pm_element(0);
    pmt->STAPM_VALUE                = pm_element(1);
    pmt->PPT_LIMIT_FAST             = pm_element(2);
    pmt->PPT_VALUE_FAST             = pm_element(3);
    pmt->PPT_LIMIT                  = pm_element(4); //Slow
    pmt->PPT_VALUE                  = pm_element(5); //Slow
    pmt->PPT_LIMIT_APU              = pm_element(6);
    pmt->PPT_VALUE_APU              = pm_element(7);
    pmt->TDC_LIMIT                  = pm_element(8); //VDD
    pmt->TDC_VALUE                  = pm_element(9); //VDD
    pmt->TDC_LIMIT_SOC              = pm_element(10);
    pmt->TDC_VALUE_SOC              = pm_element(11);
    pmt->EDC_LIMIT                  = pm_element(12); //Core
    pmt->EDC_VALUE                  = pm_element(13); //Core
    //pmt->EDC_LIMIT_SOC              = pm_element(14);
    //pmt->EDC_VALUE_SOC              = pm_element(15);
    pmt->THM_LIMIT                  = pm_element(16); //Core
    pmt->THM_VALUE                  = pm_element(17); //Core
    pmt->THM_LIMIT_GFX              = pm_element(18);
    pmt->THM_VALUE_GFX              = pm_element(19);
    pmt->THM_LIMIT_SOC              = pm_element(20);
    pmt->THM_VALUE_SOC              = pm_element(21);
    pmt->STT_LIMIT_APU              = pm_element(22);
    pmt->STT_VALUE_APU              = pm_element(23);
    pmt->STT_LIMIT_DGPU             = pm_element(24);
    pmt->STT_VALUE_DGPU             = pm_element(25);
    pmt->FIT_LIMIT                  = pm_element(26);
    pmt->FIT_VALUE                  = pm_element(27);
    pmt->VID_LIMIT                  = pm_element(28);
    pmt->VID_VALUE                  = pm_element(29);
    pmt->PSI0_LIMIT_VDD             = pm_element(30);
    pmt->PSI0_RESIDENCY_VDD         = pm_element(31);
    pmt->PSI0_LIMIT_SOC             = pm_element(32);
    pmt->PSI0_RESIDENCY_SOC         = pm_element(33);
    
    pmt->VDDCR_CPU_POWER            = pm_element(34);
    pmt->VDDCR_SOC_POWER            = pm_element(35);
    pmt->VDDIO_MEM_POWER            = pm_element(36);
    pmt->ROC_POWER                  = pm_element(37);
    pmt->SOCKET_POWER               = pm_element(38);
    pmt->TDP                        = pm_element(38); //Don't know where to get TDP from. Set equal to socket power for now
    pmt->PACKAGE_POWER              = pm_element(38); //Doesn't seem to have package power. Use socket power instead.
    pmt->GLOB_FREQUENCY             = pm_element(39);
    pmt->STAPM_FREQUENCY            = pm_element(40);
    pmt->PPT_FREQUENCY_FAST         = pm_element(41);
    pmt->PPT_FREQUENCY              = pm_element(42); //PPT_FREQUENCY_SLOW
    pmt->PPT_FREQUENCY_APU          = pm_element(43);
    pmt->TDC_FREQUENCY              = pm_element(44);
    pmt->THM_FREQUENCY              = pm_element(45);
    pmt->HTFMAX_FREQUENCY           = pm_element(46);
    pmt->PROCHOT_FREQUENCY          = pm_element(47);
    pmt->VOLTAGE_FREQUENCY          = pm_element(48);
    pmt->CCA_FREQUENCY              = pm_element(49);
    pmt->GFX_GLOB_FREQUENCY         = pm_element(50);
    pmt->GFX_STAPM_FREQUENCY        = pm_element(51);
    pmt->GFX_PPT_FREQUENCY_FAST     = pm_element(52);
    pmt->GFX_PPT_FREQUENCY          = pm_element(53);
    pmt->GFX_PPT_FREQUENCY_APU      = pm_element(54);
    pmt->GFX_TDC_FREQUENCY          = pm_element(55);
    pmt->GFX_THM_FREQUENCY          = pm_element(56);
    pmt->GFX_HTFMAX_FREQUENCY       = pm_element(57);
    pmt->GFX_PROCHOT_FREQUENCY      = pm_element(58);
    pmt->GFX_VOLTAGE_FREQUENCY      = pm_element(59);
    pmt->GFX_CCA_FREQUENCY          = pm_element(60);
    //pmt->GFX_DEM_FREQUENCY          = pm_element(61);
    
    pmt->FIT_VOLTAGE                = pm_element(61);
    pmt->LATCHUP_VOLTAGE            = pm_element(62);
    pmt->CORE_SETPOINT              = pm_element(63);
    pmt->CORE_BUSY                  = pm_element(64);
    pmt->GFX_SETPOINT               = pm_element(65);
    pmt->GFX_BUSY                   = pm_element(66);
    pmt->FCLK_CCLK_SETPOINT         = pm_element(67);
    pmt->FCLK_CCLK_FREQ             = pm_element(68);
    pmt->FCLK_GFX_SETPOINT          = pm_element(69);
    pmt->FCLK_GFX_BUSY              = pm_element(70);
    pmt->FCLK_IOHC_SETPOINT         = pm_element(71);
    pmt->FCLK_IOHC_BUSY             = pm_element(72);
    pmt->FCLK_DRAM_SETPOINT         = pm_element(73);
    pmt->FCLK_DRAM_BUSY             = pm_element(74);
    pmt->LCLK_SETPOINT[0]           = pm_element(75);
    pmt->LCLK_BUSY[0]               = pm_element(76);
    
    assign_pm_elements_4(pmt->FCLK_RESIDENCY,     77,  78,  79,  80);
    assign_pm_elements_4(pmt->FCLK_FREQ_TABLE,    81,  82,  83,  84);
    assign_pm_elements_4(pmt->UCLK_FREQ_TABLE,    85,  86,  87,  88);
    assign_pm_elements_4(pmt->MEMCLK_FREQ_TABLE,  89,  90,  91,  92);
    assign_pm_elements_4(pmt->FCLK_VOLTAGE,       93,  94,  95,  96);

    pmt->CPU_SET_VOLTAGE            = pm_element(97);
    pmt->CPU_TELEMETRY_VOLTAGE      = pm_element(98);
    pmt->CPU_TELEMETRY_CURRENT      = pm_element(99);
    pmt->CPU_TELEMETRY_POWER        = pm_element(100);
    pmt->SOC_SET_VOLTAGE            = pm_element(101);
    pmt->SOC_TELEMETRY_VOLTAGE      = pm_element(102);
    pmt->SOC_TELEMETRY_CURRENT      = pm_element(103);
    pmt->SOC_TELEMETRY_POWER        = pm_element(104);

    //Bunch of zeros
    
    pmt->DF_BUSY                    = pm_element(165);
    pmt->VCN_BUSY                   = pm_element(166);
    pmt->IOHC_BUSY	                = pm_element(167);
    pmt->MMHUB_BUSY                 = pm_element(168);
    pmt->ATHUB_BUSY                 = pm_element(169);
    pmt->OSSSYS_BUSY                = pm_element(170);
    pmt->HDP_BUSY                   = pm_element(171);
    pmt->SDMA_BUSY                  = pm_element(172);
    pmt->SHUB_BUSY                  = pm_element(173);
    pmt->BIF_BUSY                   = pm_element(174);
    pmt->ACP_BUSY                   = pm_element(175);
    pmt->SST0_BUSY                  = pm_element(176);
    pmt->SST1_BUSY                  = pm_element(177);
    pmt->USB0_BUSY                  = pm_element(178);
    pmt->USB1_BUSY                  = pm_element(179);
    pmt->CCM_READS                  = pm_element(180);
    pmt->CCM_WRITES                 = pm_element(181);
    pmt->GCM_64B_READS              = pm_element(182);
    pmt->GCM_64B_WRITES             = pm_element(183);
    pmt->GCM_32B_READS_WRITES       = pm_element(184);
    pmt->MMHUB_READS                = pm_element(185);
    pmt->MMHUB_WRITES               = pm_element(186);
    pmt->DCE_READS                  = pm_element(187);
    pmt->IO_READS_WRITES            = pm_element(188);
    pmt->CS_UMC_READS               = pm_element(189);
    pmt->CS_UMC_WRITES              = pm_element(190);
    pmt->MAX_DRAM_BANDWIDTH         = pm_element(191);
    //pmt->VCN_BUSY                   = pm_element(192);
    //pmt->VCN_DECODE                 = pm_element(193);
    //pmt->VCN_ENCODE_GEN             = pm_element(194);
    //pmt->VCN_ENCODE_LOW             = pm_element(195);
    //pmt->VCN_ENCODE_REAL            = pm_element(196);
    //pmt->VCN_PG                     = pm_element(197);
    //pmt->VCN_JPEG                   = pm_element(198);

    assign_pm_elements_8_consec(pmt->CORE_POWER         , 199);
    assign_pm_elements_8_consec(pmt->CORE_VOLTAGE       , 207);
    assign_pm_elements_8_consec(pmt->CORE_TEMP          , 215);
    assign_pm_elements_8_consec(pmt->CORE_FIT           , 223);
    assign_pm_elements_8_consec(pmt->CORE_IDDMAX        , 231);
    assign_pm_elements_8_consec(pmt->CORE_FREQ          , 239);
    assign_pm_elements_8_consec(pmt->CORE_FREQEFF       , 247);
    assign_pm_elements_8_consec(pmt->CORE_C0            , 255);
    assign_pm_elements_8_consec(pmt->CORE_CC1           , 263);
    assign_pm_elements_8_consec(pmt->CORE_CC6           , 271);
    assign_pm_elements_8_consec(pmt->CORE_CKS_FDD       , 279);
    //assign_pm_elements_8_consec(pmt->CORE_CI_FDD        , 287);
    //assign_pm_elements_8_consec(pmt->CORE_IRM           , 296);
    assign_pm_elements_8_consec(pmt->CORE_PSTATE        , 287);
    assign_pm_elements_8_consec(pmt->CORE_CPPC_MAX      , 295);
    assign_pm_elements_8_consec(pmt->CORE_CPPC_MIN      , 303);
    assign_pm_elements_8_consec(pmt->CORE_CPPC_EPP      , 311);
    assign_pm_elements_8_consec(pmt->CORE_SC_LIMIT      , 319);
    assign_pm_elements_8_consec(pmt->CORE_SC_CAC        , 327);
    assign_pm_elements_8_consec(pmt->CORE_SC_RESIDENCY  , 335);
    //assign_pm_elements_8_consec(pmt->CORE_UOPS_CLK      , 360);
    //assign_pm_elements_8_consec(pmt->CORE_UOPS          , 368);
    //assign_pm_elements_8_consec(pmt->CORE_MEM_LATECY    , 376);

    /*
    assign_pm_elements_8_consec(pmt->CORE_POWER         , 192);
    assign_pm_elements_8_consec(pmt->CORE_VOLTAGE       , 200);
    assign_pm_elements_8_consec(pmt->CORE_TEMP          , 208);
    assign_pm_elements_8_consec(pmt->CORE_FIT           , 216);
    assign_pm_elements_8_consec(pmt->CORE_IDDMAX        , 224);
    assign_pm_elements_8_consec(pmt->CORE_FREQ          , 232);
    assign_pm_elements_8_consec(pmt->CORE_FREQEFF       , 240);
    assign_pm_elements_8_consec(pmt->CORE_C0            , 248);
    assign_pm_elements_8_consec(pmt->CORE_CC1           , 256);
    assign_pm_elements_8_consec(pmt->CORE_CC6           , 264);
    assign_pm_elements_8_consec(pmt->CORE_CKS_FDD       , 272);
    //assign_pm_elements_8_consec(pmt->CORE_CI_FDD        , 288);
    //assign_pm_elements_8_consec(pmt->CORE_IRM           , 296);
    assign_pm_elements_8_consec(pmt->CORE_PSTATE        , 280);
    assign_pm_elements_8_consec(pmt->CORE_CPPC_MAX      , 288);
    assign_pm_elements_8_consec(pmt->CORE_CPPC_MIN      , 296);
    assign_pm_elements_8_consec(pmt->CORE_CPPC_EPP      , 304);
    assign_pm_elements_8_consec(pmt->CORE_SC_LIMIT      , 312);
    assign_pm_elements_8_consec(pmt->CORE_SC_CAC        , 320);
    assign_pm_elements_8_consec(pmt->CORE_SC_RESIDENCY  , 328);
    //assign_pm_elements_8_consec(pmt->CORE_UOPS_CLK      , 360);
    //assign_pm_elements_8_consec(pmt->CORE_UOPS          , 368);
    //assign_pm_elements_8_consec(pmt->CORE_MEM_LATECY    , 376);
    */


    pmt->L3_LOGIC_POWER[0]          = pm_element(343);
    pmt->L3_VDDM_POWER[0]           = pm_element(345);
    pmt->L3_TEMP[0]                 = pm_element(347);
    pmt->L3_FIT[0]                  = pm_element(349);
    pmt->L3_IDDMAX[0]               = pm_element(351);
    pmt->L3_FREQ[0]                 = pm_element(353);
    pmt->L3_FREQ_EFF[0]             = pm_element(355);
    pmt->L3_CKS_FDD[0]              = pm_element(357);
    pmt->L3_CCA_THRESHOLD[0]        = pm_element(359);
    pmt->L3_CCA_CAC[0]              = pm_element(361);
    pmt->L3_CCA_ACTIVATION[0]       = pm_element(363);
    pmt->L3_EDC_LIMIT[0]            = pm_element(365);
    pmt->L3_EDC_CAC[0]              = pm_element(367);
    pmt->L3_EDC_RESIDENCY[0]        = pm_element(369);
    //pmt->L3_FLL_BTC[0]              = pm_element(371);

    pmt->GFX_VOLTAGE                = pm_element(369);
    pmt->GFX_TEMP                   = pm_element(370);
    pmt->GFX_IDDMAX                 = pm_element(371);
    pmt->GFX_FREQ                   = pm_element(372);
    pmt->GFX_FREQEFF                = pm_element(373);
    pmt->GFX_BUSY                   = pm_element(374);
    pmt->GFX_CGPG                   = pm_element(375);
    pmt->GFX_EDC_LIM                = pm_element(376);
    pmt->GFX_EDC_RESIDENCY          = pm_element(377);
    //pmt->GFX_DEM_RESIDENCY          = pm_element(377);

    pmt->FCLK_FREQ                  = pm_element(378);
    pmt->UCLK_FREQ                  = pm_element(379);
    pmt->MEMCLK_FREQ                = pm_element(380);
    pmt->VCLK_FREQ                  = pm_element(381);
    pmt->DCLK_FREQ                  = pm_element(382);
    pmt->SOCCLK                     = pm_element(383);
    pmt->LCLK_FREQ[0]               = pm_element(384);
    pmt->SHUBCLK                    = pm_element(385);
    pmt->MP0CLK                     = pm_element(386);
    pmt->DCF_FREQ                   = pm_element(387);
    pmt->FCLK_FREQ_EFF              = pm_element(388);
    pmt->UCLK_FREQ_EFF              = pm_element(389);
    pmt->MEMCLK_FREQ_EFF            = pm_element(390);
    pmt->VCLK_FREQ_EFF              = pm_element(391);
    pmt->DCLK_FREQ_EFF              = pm_element(392);
    pmt->SOCCLK_FREQ_EFF[0]         = pm_element(393);
    pmt->LCLK_FREQ_EFF[0]           = pm_element(394);
    pmt->SHUBCLK_FREQ_EFF[0]        = pm_element(395);
    pmt->MP0CLK_EFF                 = pm_element(396);
    pmt->DCF_FREQ_EFF               = pm_element(397);

    assign_pm_elements_8_consec(pmt->VCLK_STATE            , 398);
    assign_pm_elements_8_consec(pmt->DCLK_STATE            , 406);
    assign_pm_elements_8_consec(pmt->SOCCLK_STATE          , 414);
    assign_pm_elements_8_consec(pmt->LCLK_STATE            , 422);
    assign_pm_elements_8_consec(pmt->SHUB_STATE            , 430);
    assign_pm_elements_8_consec(pmt->MP0_STATE             , 438);
    assign_pm_elements_8_consec(pmt->DCFCLK_STATE          , 446);
    assign_pm_elements_8_consec(pmt->VCN_STATE_RESIDENCY   , 454);
    assign_pm_elements_8_consec(pmt->SOCCLK_STATE_RESIDENCY, 462);
    assign_pm_elements_8_consec(pmt->LCLK_STATE_RESIDENCY  , 470);
    assign_pm_elements_8_consec(pmt->SHUB_STATE_RESIDENCY  , 478);
    assign_pm_elements_8_consec(pmt->MP0CLK_STATE_RESIDENCY, 486);
    assign_pm_elements_8_consec(pmt->DCFCLK_STATE_RESIDENCY, 494);
    assign_pm_elements_8_consec(pmt->VDDCR_SOC_VOLTAGE     , 502);

    pmt->CPUOFF                     = pm_element(510);
    pmt->CPUOFF_CNT                 = pm_element(511);
    pmt->GFXOFF                     = pm_element(512);
    pmt->GFXOFF_CNT                 = pm_element(513);
    pmt->VDDOFF                     = pm_element(514);
    pmt->VDDOFF_CNT                 = pm_element(515);
    pmt->ULV                        = pm_element(516);
    pmt->ULV_CNT                    = pm_element(517);
    pmt->S0i2                       = pm_element(518);
    pmt->S0i2_CNT                   = pm_element(519);
    pmt->WHISPER                    = pm_element(520);
    pmt->WHISPER_CNT                = pm_element(521);
    pmt->SELFREFRESH0               = pm_element(522);
    pmt->SELFREFRESH1               = pm_element(523);
    pmt->PLL_POWERDOWN_0            = pm_element(524);
    pmt->PLL_POWERDOWN_1            = pm_element(525);
    pmt->PLL_POWERDOWN_2            = pm_element(526);
    pmt->PLL_POWERDOWN_3            = pm_element(527);
    //pmt->PLL_POWERDOWN_4            = pm_element(528);
    //pmt->POWER_SOURCE               = pm_element(560);
    pmt->DGPU_POWER                 = pm_element(530);
    pmt->DGPU_GFX_BUSY              = pm_element(531);
    pmt->DGPU_FREQ_TARGET           = pm_element(532);
    pmt->V_VDDM                     = pm_element(533); //CLDO_VDDM
    pmt->V_VDDP                     = pm_element(534); //CLDO_VDDP
    pmt->DDR_PHY_POWER              = pm_element(535);
    pmt->IOD_VDDIO_MEM_POWER        = pm_element(536);
    pmt->IO_VDD18_POWER             = pm_element(537);
    pmt->IO_DISPLAY_POWER           = pm_element(538);
    pmt->IO_USB_POWER               = pm_element(539);
    pmt->ULV_VOLTAGE                = pm_element(540);
    pmt->PEAK_TEMP                  = pm_element(541);
    pmt->PEAK_VOLTAGE               = pm_element(542);
    pmt->AVG_CORE_COUNT             = pm_element(543);
    pmt->MAX_CORE_VOLTAGE           = pm_element(544);
    pmt->DC_BTC                     = pm_element(545);
    pmt->CSTATE_BOOST               = pm_element(546);
    pmt->PROCHOT                    = pm_element(547);
    pmt->PWM                        = pm_element(548);
    pmt->FPS                        = pm_element(549);
    pmt->DISPLAY_COUNT              = pm_element(550);
    pmt->StapmTimeConstant          = pm_element(551);
    pmt->SlowPPTTimeConstant        = pm_element(552);
    pmt->MP1CLK                     = pm_element(553);
    pmt->MP2CLK                     = pm_element(554);
    pmt->SMNCLK                     = pm_element(555);
    pmt->ACLK                       = pm_element(556);
    pmt->DISPCLK                    = pm_element(557);
    pmt->DPREFCLK                   = pm_element(558);
    pmt->DPPCLK                     = pm_element(559);
    pmt->SMU_BUSY                   = pm_element(560);
    pmt->SMU_SKIP_COUNTER           = pm_element(561);

    pmt->min_size = 562*4; //(Highest element we access + 1)*4.
    //Needed to avoid illegal memory access
}
