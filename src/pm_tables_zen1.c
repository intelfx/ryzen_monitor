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
 * This file contains the mapping of the Zen and Zen+ PM Table versions
 **/

#include "pm_tables.h"
#include "pm_tables_helpers.h"

void pm_table_0x1E0004(pm_table *pmt, void* base_addr) {

    pmt->version = 0x1E0004;
    pmt->max_cores = 4; //Number of cores supported by this PM table version
    pmt->max_l3 = 1; //Number of L3 caches supported by this PM table version
    pmt->zen_version = 1; //Zen2
    pmt->experimental = 0; //Print experimental note
    pmt->powersum_unclear = 0; //No idea how to calculatec the total power
    pmt->has_graphics = 1; //Print GFX information

    pmt->STAPM_LIMIT                = pm_element(0);
    pmt->STAPM_VALUE                = pm_element(1);
    pmt->PPT_LIMIT_FAST             = pm_element(2);
    pmt->PPT_VALUE_FAST             = pm_element(3);
    pmt->PPT_LIMIT                  = pm_element(4); //Slow
    pmt->PPT_VALUE                  = pm_element(5); //Slow
    //pmt->PPT_LIMIT_APU              = pm_element(6);
    //pmt->PPT_VALUE_APU              = pm_element(7);
    pmt->TDC_LIMIT                  = pm_element(6); //VDD
    pmt->TDC_VALUE                  = pm_element(7); //VDD
    pmt->TDC_LIMIT_SOC              = pm_element(8);
    pmt->TDC_VALUE_SOC              = pm_element(9);
    pmt->EDC_LIMIT                  = pm_element(10); //Core
    //pmt->EDC_VALUE                  = pm_element(13); //Core
    pmt->EDC_LIMIT_SOC              = pm_element(13);
    //pmt->EDC_VALUE_SOC              = pm_element(15);
    pmt->THM_LIMIT                  = pm_element(20); //Core
    assign_pm_elements_4(pmt->THM_VALUE_CORES,  21,  23,  25,  27);
    //pmt->THM_VALUE                  = pm_element(17); //Core
    pmt->THM_LIMIT_GFX              = pm_element(30);
    pmt->THM_VALUE_GFX              = pm_element(31);
    pmt->THM_LIMIT_SOC              = pm_element(32);
    pmt->THM_VALUE_SOC              = pm_element(33);
    //pmt->STT_LIMIT_APU              = pm_element(22);
    //pmt->STT_VALUE_APU              = pm_element(23);
    //pmt->STT_LIMIT_DGPU             = pm_element(24);
    //pmt->STT_VALUE_DGPU             = pm_element(25);
    pmt->FIT_LIMIT                  = pm_element(34);
    pmt->FIT_VALUE                  = pm_element(35);

    pmt->VID_VALUE                  = pm_element(61);
    pmt->VID_LIMIT                  = pm_element(57);
    /*
    pmt->VID_LIMIT                  = pm_element(28);
    pmt->PSI0_LIMIT_VDD             = pm_element(16);
    pmt->PSI0_RESIDENCY_VDD         = pm_element(17);
    pmt->PSI0_LIMIT_SOC             = pm_element(18);
    pmt->PSI0_RESIDENCY_SOC         = pm_element(19);
    
    pmt->VDDCR_CPU_POWER            = pm_element(34);
    pmt->VDDCR_SOC_POWER            = pm_element(35);
    pmt->VDDIO_MEM_POWER            = pm_element(36);
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
    */
    pmt->V_VDDM                     = pm_element(59); //CLDO_VDDM
    pmt->V_VDDP                     = pm_element(60); //CLDO_VDDP
    pmt->CPU_SET_VOLTAGE            = pm_element(61);
    //pmt->CPU_TELEMETRY_VOLTAGE      = pm_element(62);
    //pmt->CPU_TELEMETRY_CURRENT      = pm_element(63);
    //pmt->CPU_TELEMETRY_POWER        = pm_element(64);
    pmt->SOC_SET_VOLTAGE            = pm_element(65);
    //pmt->SOC_TELEMETRY_VOLTAGE      = pm_element(66);
    //pmt->SOC_TELEMETRY_CURRENT      = pm_element(67);
    //pmt->SOC_TELEMETRY_POWER        = pm_element(68);

    //Bunch of zeros
    
    pmt->DF_BUSY                    = pm_element(69);
    pmt->VCN_BUSY                   = pm_element(70);
    pmt->IOHC_BUSY	                = pm_element(71);
    pmt->MMHUB_BUSY                 = pm_element(72);
    pmt->ATHUB_BUSY                 = pm_element(73);
    pmt->OSSSYS_BUSY                = pm_element(74);
    pmt->HDP_BUSY                   = pm_element(75);
    pmt->SDMA_BUSY                  = pm_element(76);
    pmt->SHUB_BUSY                  = pm_element(77);
    pmt->BIF_BUSY                   = pm_element(78);
    pmt->ACP_BUSY                   = pm_element(79);
    pmt->SST0_BUSY                  = pm_element(81);
    pmt->SST1_BUSY                  = pm_element(82);
    pmt->USB0_BUSY                  = pm_element(83);
    pmt->USB1_BUSY                  = pm_element(84);
    pmt->CCM_READS                  = pm_element(85);
    pmt->CCM_WRITES                 = pm_element(86);
    pmt->GCM_64B_READS              = pm_element(87);
    pmt->GCM_64B_WRITES             = pm_element(88);
    //pmt->GCM_32B_READS_WRITES       = pm_element();
    pmt->MMHUB_READS                = pm_element(89);
    pmt->MMHUB_WRITES               = pm_element(90);
    pmt->DCE_READS                  = pm_element(91);
    pmt->IO_READS_WRITES            = pm_element(92);
    pmt->CS_UMC_READS               = pm_element(93);
    pmt->CS_UMC_WRITES              = pm_element(94);
    pmt->MAX_DRAM_BANDWIDTH         = pm_element(95);
    //pmt->VCN_BUSY                   = pm_element(192);
    //pmt->VCN_DECODE                 = pm_element(193);
    //pmt->VCN_ENCODE_GEN             = pm_element(194);
    //pmt->VCN_ENCODE_LOW             = pm_element(195);
    //pmt->VCN_ENCODE_REAL            = pm_element(196);
    //pmt->VCN_PG                     = pm_element(197);
    //pmt->VCN_JPEG                   = pm_element(198);

    assign_pm_elements_4_consec(pmt->CORE_POWER         , 96);
    assign_pm_elements_4_consec(pmt->CORE_VOLTAGE       , 104);
    assign_pm_elements_4_consec(pmt->CORE_TEMP          , 108);
    assign_pm_elements_4_consec(pmt->CORE_FIT           , 112);
    assign_pm_elements_4_consec(pmt->CORE_IDDMAX        , 116);
    assign_pm_elements_4_consec(pmt->CORE_FREQ          , 120);
    assign_pm_elements_4_consec(pmt->CORE_FREQEFF       , 124);
    assign_pm_elements_4_consec(pmt->CORE_C0            , 128);
    assign_pm_elements_4_consec(pmt->CORE_CC1           , 132);
    assign_pm_elements_4_consec(pmt->CORE_CC6           , 136);
    //assign_pm_elements_4_consec(pmt->CORE_CKS_FDD       , 279);
    //assign_pm_elements_4_consec(pmt->CORE_CI_FDD        , 287);
    //assign_pm_elements_4_consec(pmt->CORE_IRM           , 296);
    assign_pm_elements_4_consec(pmt->CORE_PSTATE        , 140);
    //assign_pm_elements_4_consec(pmt->CORE_CPPC_MAX      , 295);
    //assign_pm_elements_4_consec(pmt->CORE_CPPC_MIN      , 303);
    //assign_pm_elements_4_consec(pmt->CORE_CPPC_EPP      , 311);
    //assign_pm_elements_4_consec(pmt->CORE_SC_LIMIT      , 319);
    //assign_pm_elements_4_consec(pmt->CORE_SC_CAC        , 327);
    //assign_pm_elements_4_consec(pmt->CORE_SC_RESIDENCY  , 335);
    //assign_pm_elements_4_consec(pmt->CORE_UOPS_CLK      , 360);
    //assign_pm_elements_4_consec(pmt->CORE_UOPS          , 368);
    //assign_pm_elements_4_consec(pmt->CORE_MEM_LATECY    , 376);

    pmt->P_VDDM                     = pm_element(144);
    //pmt->L3_LOGIC_POWER[0]          = pm_element(343);
    //pmt->L3_VDDM_POWER[0]           = pm_element(345);
    pmt->L3_TEMP[0]                 = pm_element(146);
    pmt->L3_FIT[0]                  = pm_element(147);
    pmt->L3_IDDMAX[0]               = pm_element(148);
    pmt->L3_FREQ[0]                 = pm_element(149);

    //pmt->L3_FREQ_EFF[0]             = pm_element(355);
    //pmt->L3_CKS_FDD[0]              = pm_element(357);
    //pmt->L3_CCA_THRESHOLD[0]        = pm_element(359);
    //pmt->L3_CCA_CAC[0]              = pm_element(361);
    //pmt->L3_CCA_ACTIVATION[0]       = pm_element(363);
    //pmt->L3_EDC_LIMIT[0]            = pm_element(365);
    //pmt->L3_EDC_CAC[0]              = pm_element(367);
    //pmt->L3_EDC_RESIDENCY[0]        = pm_element(369);
    //pmt->L3_FLL_BTC[0]              = pm_element(371);

    pmt->GFX_VOLTAGE                = pm_element(150);
    pmt->GFX_TEMP                   = pm_element(151);
    pmt->GFX_IDDMAX                 = pm_element(153);
    pmt->GFX_FREQ                   = pm_element(154);
    pmt->GFX_FREQEFF                = pm_element(155);
    pmt->GFX_BUSY                   = pm_element(156);
    pmt->GFX_CGPG                   = pm_element(157);
    //pmt->GFX_EDC_LIM                = pm_element(376);
    //pmt->GFX_EDC_RESIDENCY          = pm_element(377);
    //pmt->GFX_DEM_RESIDENCY          = pm_element(377);

    pmt->SOC_POWER                  = pm_element(160); // ??
    pmt->SOC_TEMP                   = pm_element(161); // ??

    pmt->DDR_PHY_POWER              = pm_element(163); // ??

    pmt->FCLK_FREQ                  = pm_element(166);
    pmt->UCLK_FREQ                  = pm_element(167);
    pmt->MEMCLK_FREQ                = pm_element(168);
    pmt->VCLK_FREQ                  = pm_element(169);
    pmt->DCLK_FREQ                  = pm_element(170);
    pmt->SOCCLK                     = pm_element(171);
    pmt->LCLK_FREQ[0]               = pm_element(172);
    pmt->SHUBCLK                    = pm_element(173);
    pmt->MP0CLK                     = pm_element(174);
    pmt->DCF_FREQ                   = pm_element(177);
    pmt->FCLK_FREQ_EFF              = pm_element(178);
    pmt->UCLK_FREQ_EFF              = pm_element(179);
    pmt->MEMCLK_FREQ_EFF            = pm_element(180);
    pmt->VCLK_FREQ_EFF              = pm_element(181);
    pmt->DCLK_FREQ_EFF              = pm_element(182);
    pmt->SOCCLK_FREQ_EFF[0]         = pm_element(183);
    pmt->LCLK_FREQ_EFF[0]           = pm_element(184);
    pmt->SHUBCLK_FREQ_EFF[0]        = pm_element(185);
    pmt->MP0CLK_EFF                 = pm_element(186);
    pmt->DCF_FREQ_EFF               = pm_element(189);

    assign_pm_elements_8_consec(pmt->VCLK_STATE            , 190);
    assign_pm_elements_8_consec(pmt->DCLK_STATE            , 198);
    assign_pm_elements_8_consec(pmt->SOCCLK_STATE          , 230);
    assign_pm_elements_8_consec(pmt->LCLK_STATE            , 238);
    assign_pm_elements_8_consec(pmt->SHUB_STATE            , 246);
    assign_pm_elements_8_consec(pmt->MP0_STATE             , 254);
    assign_pm_elements_4_consec(pmt->DCFCLK_STATE          , 270);
    assign_pm_elements_8_consec(pmt->VCN_STATE_RESIDENCY   , 282);
    assign_pm_elements_8_consec(pmt->SOCCLK_STATE_RESIDENCY, 290);
    assign_pm_elements_8_consec(pmt->LCLK_STATE_RESIDENCY  , 298);
    assign_pm_elements_8_consec(pmt->SHUB_STATE_RESIDENCY  , 306);
    assign_pm_elements_8_consec(pmt->MP0CLK_STATE_RESIDENCY, 314);
    assign_pm_elements_4_consec(pmt->DCFCLK_STATE_RESIDENCY, 330);
    assign_pm_elements_8_consec(pmt->VDDCR_SOC_VOLTAGE     , 334);
    //pmt->POWER_SOURCE               = pm_element(560);
    //pmt->DDR_PHY_POWER              = pm_element(535);
    //pmt->IO_VDD18_POWER             = pm_element(537);
    //pmt->IO_DISPLAY_POWER           = pm_element(538);
    //pmt->IO_USB_POWER               = pm_element(539);
    //pmt->ULV_VOLTAGE                = pm_element(540);
    //pmt->PEAK_VOLTAGE               = pm_element(542);
    //pmt->AVG_CORE_COUNT             = pm_element(543);
    //pmt->MAX_CORE_VOLTAGE           = pm_element(544);

    pmt->DISPLAY_COUNT              = pm_element(342);
    pmt->CPUOFF                     = pm_element(345);
    pmt->CPUOFF_CNT                 = pm_element(346);
    pmt->GFXOFF                     = pm_element(347);
    pmt->GFXOFF_CNT                 = pm_element(348);
    pmt->VDDOFF                     = pm_element(349);
    pmt->VDDOFF_CNT                 = pm_element(350);
    pmt->ULV                        = pm_element(351);
    pmt->ULV_CNT                    = pm_element(352);
    pmt->S0i2                       = pm_element(353);
    pmt->S0i2_CNT                   = pm_element(354);
    pmt->WHISPER                    = pm_element(355);
    pmt->WHISPER_CNT                = pm_element(356);
    pmt->SELFREFRESH0               = pm_element(357);
    pmt->SELFREFRESH1               = pm_element(356);
    pmt->PLL_POWERDOWN_0            = pm_element(357);
    pmt->PLL_POWERDOWN_1            = pm_element(358);
    pmt->PLL_POWERDOWN_2            = pm_element(359);
    //pmt->PLL_POWERDOWN_3            = pm_element(360);
    pmt->PLL_POWERDOWN_4            = pm_element(360);
    pmt->PPT_VALUE_APU              = pm_element(361); // ??
    pmt->ROC_POWER                  = pm_element(363);
    pmt->IOD_VDDIO_MEM_POWER        = pm_element(364);
    //pmt->DC_BTC                     = pm_element(545);
    pmt->DGPU_POWER                 = pm_element(368);
    pmt->DGPU_GFX_BUSY              = pm_element(369);
    //pmt->DGPU_FREQ_TARGET           = pm_element(370);
    pmt->PEAK_TEMP                  = pm_element(370);
    pmt->SKIN_TEMP_MARGIN           = pm_element(371);
    //pmt->PC6                        = pm_element(372);
    pmt->PWM                        = pm_element(373);
    pmt->CSTATE_BOOST               = pm_element(374);
    pmt->PROCHOT                    = pm_element(375);
    pmt->GFX_DUTY                   = pm_element(376);
    pmt->FPS                        = pm_element(377);
    pmt->StapmTimeConstant          = pm_element(378);
    pmt->SlowPPTTimeConstant        = pm_element(379);
    pmt->MP1CLK                     = pm_element(380);
    pmt->MP2CLK                     = pm_element(381);
    pmt->SMNCLK                     = pm_element(382);
    //pmt->ACLK                       = pm_element(556);
    pmt->DISPCLK                    = pm_element(383);
    pmt->DPREFCLK                   = pm_element(384);
    //pmt->DPPCLK                     = pm_element(559);
    pmt->SMU_BUSY                   = pm_element(385);
    pmt->SMU_SKIP_COUNTER           = pm_element(387);

    pmt->min_size = 388*4; //(Highest element we access + 1)*4.
    //Needed to avoid illegal memory access
}

void pm_table_0x100100(pm_table *pmt, void* base_addr) {
    // Zen+ desktop CPUs (Pinnacle Ridge). The SMU reports no version for this
    // table, so the version is made up. The mapping is partly guesswork.

    pmt->version = 0x100100;
    pmt->max_cores = 8; //Number of cores supported by this PM table version
    pmt->max_l3 = 2; //Number of L3 caches supported by this PM table version
    pmt->zen_version = 1; //Zen+
    pmt->experimental = 1;
    pmt->powersum_unclear = 1;

    pmt->PPT_LIMIT       = pm_element( 0);
    pmt->PPT_VALUE       = pm_element( 1);
    pmt->TDC_LIMIT       = pm_element( 2);
    pmt->TDC_VALUE       = pm_element( 3);
    pmt->EDC_LIMIT       = pm_element( 4);
    // 5 might be fast EDC
    pmt->EDC_VALUE       = pm_element( 6);
    pmt->THM_LIMIT       = pm_element( 7);
    pmt->THM_VALUE       = pm_element( 8);
    pmt->FIT_LIMIT       = pm_element( 9);
    pmt->FIT_VALUE       = pm_element(10);
    pmt->TDP             = pm_element(11);
    pmt->CPU_SET_VOLTAGE = pm_element(12);
    pmt->SOC_SET_VOLTAGE = pm_element(13);
    pmt->V_VDDM          = pm_element(14); //probably
    pmt->V_VDDP          = pm_element(15);
    pmt->VDDCR_CPU_POWER = pm_element(16);
    pmt->VDDCR_SOC_POWER = pm_element(17);

    // 18/19 unknown

    pmt->SOCKET_POWER           = pm_element(20);
    pmt->CPU_TELEMETRY_VOLTAGE  = pm_element(21);
    pmt->CPU_TELEMETRY_CURRENT  = pm_element(22);
    pmt->CPU_TELEMETRY_POWER    = pm_element(23);
    pmt->SOC_TELEMETRY_VOLTAGE  = pm_element(24);
    pmt->SOC_TELEMETRY_CURRENT  = pm_element(25);
    pmt->SOC_TELEMETRY_POWER    = pm_element(26);

    // 27 looks like some multipler? fraction part is always .25/.5/.75/.0
    pmt->PEAK_VOLTAGE           = pm_element(28);
    // 29-32 unknown

    pmt->FCLK_FREQ = pmt->UCLK_FREQ = pmt->MEMCLK_FREQ = pm_element(33); // always coupled on Zen/Zen+
    // 33-39 might be frequency table

    assign_pm_elements_8_consec(pmt->CORE_POWER       , 40);
    // followed by 8 zeroes, probably something not implemented
    assign_pm_elements_8_consec(pmt->CORE_VOLTAGE     , 56); // always the same value, might be wrong
    assign_pm_elements_8_consec(pmt->CORE_TEMP        , 64);
    assign_pm_elements_8_consec(pmt->CORE_FIT         , 72);
    assign_pm_elements_8_consec(pmt->CORE_IDDMAX      , 80);
    assign_pm_elements_8_consec(pmt->CORE_FREQ        , 88);
    assign_pm_elements_8_consec(pmt->CORE_FREQEFF     , 96);
    assign_pm_elements_8_consec(pmt->CORE_C0          , 104);
    assign_pm_elements_8_consec(pmt->CORE_CC1         , 112);
    assign_pm_elements_8_consec(pmt->CORE_CC6         , 120);
    assign_pm_elements_8_consec(pmt->CORE_PSTATE      , 128);

    // Copied from Zen2, L3 power consumption seems too low
    assign_pm_elements_2(pmt->L3_LOGIC_POWER   , 136, 137);
    assign_pm_elements_2(pmt->L3_VDDM_POWER    , 138, 139);
    assign_pm_elements_2(pmt->L3_TEMP          , 140, 141);
    assign_pm_elements_2(pmt->L3_FIT           , 142, 143);
    assign_pm_elements_2(pmt->L3_IDDMAX        , 144, 145);
    assign_pm_elements_2(pmt->L3_FREQ          , 146, 147);

    pmt->SOC_TEMP        = pm_element(148); // maybe
    // 149-154 unknown
    // pmt->PACKAGE_POWER   = pm_element(155); // reports same number as #20
    // 156-160 unknown

    // seems too high, tracks SoC+PPT pretty closely.
    // Isn't PPT supposed to include SoC power?
    // pmt->SOCKET_POWER    = pm_element(161);
    // 162 seems like percentage of something remaning, usually stays at 100 and goes down from here

    pmt->min_size = 149*4; //(Highest element we access + 1)*4.
                           //Needed to avoid illegal memory access
}
