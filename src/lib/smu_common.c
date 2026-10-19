/* SPDX-License-Identifier: GPL-2.0 */
/* Definitions shared by the driver and the userspace library. */

#include "smu_common.h"

static const char* const codename_names[CODENAME_COUNT] = {
    [CODENAME_UNDEFINED]     = "Undefined",
    [CODENAME_COLFAX]        = "Colfax",
    [CODENAME_RENOIR]        = "Renoir",
    [CODENAME_PICASSO]       = "Picasso",
    [CODENAME_MATISSE]       = "Matisse",
    [CODENAME_THREADRIPPER]  = "Threadripper",
    [CODENAME_CASTLEPEAK]    = "CastlePeak",
    [CODENAME_RAVENRIDGE]    = "RavenRidge",
    [CODENAME_RAVENRIDGE2]   = "RavenRidge2",
    [CODENAME_SUMMITRIDGE]   = "SummitRidge",
    [CODENAME_PINNACLERIDGE] = "PinnacleRidge",
    [CODENAME_REMBRANDT]     = "Rembrandt",
    [CODENAME_VERMEER]       = "Vermeer",
    [CODENAME_VANGOGH]       = "VanGogh",
    [CODENAME_CEZANNE]       = "Cezanne",
    [CODENAME_MILAN]         = "Milan",
    [CODENAME_DALI]          = "Dali",
    [CODENAME_LUCIENNE]      = "Lucienne",
    [CODENAME_NAPLES]        = "Naples",
    [CODENAME_CHAGALL]       = "Chagall",
    [CODENAME_RAPHAEL]       = "Raphael",
    [CODENAME_PHOENIX]       = "Phoenix",
    [CODENAME_STRIXPOINT]    = "StrixPoint",
    [CODENAME_GRANITERIDGE]  = "GraniteRidge",
    [CODENAME_HAWKPOINT]     = "HawkPoint",
    [CODENAME_STORMPEAK]     = "StormPeak",
    [CODENAME_STRIXHALO]     = "StrixHalo",
    [CODENAME_SHIMADAPEAK]   = "ShimadaPeak",
    [CODENAME_KRACKANPOINT]  = "KrackanPoint",
    [CODENAME_DRAGONRANGE]   = "DragonRange",
    [CODENAME_TURIN]         = "Turin",
};

const char* smu_codename_name(smu_processor_codename codename) {
    if ((unsigned int)codename >= CODENAME_COUNT || !codename_names[codename])
        return codename_names[CODENAME_UNDEFINED];

    return codename_names[codename];
}
