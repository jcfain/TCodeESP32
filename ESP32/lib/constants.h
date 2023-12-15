#pragma once

#include "struct/channel.h"

const Channel ChannelMapV2[9] = {
    {"L0","Stroke",0,500,999,false,false},
    {"L1","Surge",0,500,999,false,true},
    {"L2","Sway",0,500,999,false,true},
    {"L3","Suck",0,500,999,false,false},
    {"R0","Twist",0,500,999,false,false},
    {"R1","Roll",0,500,999,false,false},
    {"R2","Pitch",0,500,999,false,false},
    {"V0","Vibe 0",0,500,999,true,false},
    {"V1","Vibe 1/Lube",0,500,999,true,false}
};

const Channel ChannelMapV3[14] = {
    {"L0","Stroke",0,5000,9999,false,false},
    {"L1","Surge",0,5000,9999,false,true},
    {"L2","Sway",0,5000,9999,false,true},
    {"R0","Twist",0,5000,9999,false,false},
    {"R1","Roll",0,5000,9999,false,false},
    {"R2","Pitch",0,5000,9999,false,false},
    {"V0","Vibe 1",0,5000,9999,true,false},
    {"V1","Vibe 2",0,5000,9999,true,false},
    {"V2","Vibe 3",0,5000,9999,true,false},
    {"V3","Vibe 4",0,5000,9999,true,false},
    {"A0","Suck manual",0,5000,9999,false,false},
    {"A1","Suck level",0,5000,9999,false,false},
    {"A2","Lube",0,5000,9999,true,false},
    {"A3","Auxiliary",0,5000,9999,false,false}
};

const Channel ChannelMapBLDC[10] = {
    {"L0","Stroke",0,5000,9999,false,false},
    {"R0","Twist",0,5000,9999,false,false},
    {"V0","Vibe 1",0,5000,9999,true,false},
    {"V1","Vibe 2",0,5000,9999,true,false},
    {"V2","Vibe 3",0,5000,9999,true,false},
    {"V3","Vibe 4",0,5000,9999,true,false},
    {"A0","Suck manual",0,5000,9999,false,false},
    {"A1","Suck level",0,5000,9999,false,false},
    {"A2","Lube",0,5000,9999,true,false},
    {"A3","Auxiliary",0,5000,9999,false,false}
};