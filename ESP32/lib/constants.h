#pragma once

#include "struct/channel.h"

//#define FIRMWARE_VERSION 0.32f Not used currently
#define FIRMWARE_VERSION_NAME "0.38b\n"
#define featureCount 8
#define MAX_BUTTON_SETS 4
#define MAX_BUTTONS 4
#define MAX_COMMAND 256
#define TCODE_MIN 0
#define TCODE_MID 5000
#define TCODE_MAX 9999
#define LOG_PATH "/log.json"
#define DECOY_PASS "Too bad haxor!"
#define TCODE_HANDSHAKE "D1\n"
#define TCODE_SETTINGS "D2\n"
#define WIFI_PASS_DONOTCHANGE_DEFAULT "YOUR PASSWORD HERE"

// const Channel ChannelMapV2[9] = {
//     {"L0","Stroke",0,500,999,false,false,0,500,999},
//     {"L1","Surge",0,500,999,false,true,0,500,999},
//     {"L2","Sway",0,500,999,false,true,0,500,999},
//     {"L3","Suck",0,500,999,false,false,0,500,999},
//     {"R0","Twist",0,500,999,false,false,0,500,999},
//     {"R1","Roll",0,500,999,false,false,0,500,999},
//     {"R2","Pitch",0,500,999,false,false,0,500,999},
//     {"V0","Vibe 0",0,500,999,true,false,0,500,999},
//     {"V1","Vibe 1/Lube",0,500,999,true,false,0,500,999}
// };

const Channel ChannelMapV3[14] = {
    {"L0","Stroke",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {"L1","Surge",false,true,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {"L2","Sway",false,true,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {"R0","Twist",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {"R1","Roll",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {"R2","Pitch",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {"V0","Vibe 1",true,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {"V1","Vibe 2",true,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {"V2","Vibe 3",true,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {"V3","Vibe 4",true,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {"A0","Suck manual",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {"A1","Suck level",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {"A2","Lube",true,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {"A3","Auxiliary",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX}
};

const Channel ChannelMapBLDC[10] = {
    {"L0","Stroke",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {"R0","Twist",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {"V0","Vibe 1",true,false,TCODE_MIN,TCODE_MIN,TCODE_MAX},
    {"V1","Vibe 2",true,false,TCODE_MIN,TCODE_MIN,TCODE_MAX},
    {"V2","Vibe 3",true,false,TCODE_MIN,TCODE_MIN,TCODE_MAX},
    {"V3","Vibe 4",true,false,TCODE_MIN,TCODE_MIN,TCODE_MAX},
    {"A0","Suck manual",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {"A1","Suck level",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {"A2","Lube",true,false,TCODE_MIN,TCODE_MIN,TCODE_MAX},
    {"A3","Auxiliary",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX}
};