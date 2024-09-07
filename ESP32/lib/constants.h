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
#ifdef CONFIG_IDF_TARGET_ESP32S3
#define MODULE_CURRENT ModuleType::S3
#elif CONFIG_IDF_TARGET_ESP32
#define MODULE_CURRENT ModuleType::WROOM32
#endif

// Servo operating frequencies
// #define VibePWM_Freq 8000   // Vibe motor control PWM frequency

// Other functions
#define VALVE_DEFAULT 5000        // Auto-valve default suction level (low-high, 0-9999) 

// ----------------------------
//  Auto Settings
// ----------------------------
// Do not change

#ifdef CONFIG_IDF_TARGET_ESP32
#define SERVO_PWM_RES 16
#elif CONFIG_IDF_TARGET_ESP32S3
#define SERVO_PWM_RES 14
#endif

#define ESP_TIMER_FREQUENCY_DEFAULT 50
#define ESP_VIB_TIMER_FREQUENCY_DEFAULT 8000
// #define ESP_TIMER_MAX_CHANNEL 2


// // Servo PWM channels

//     #define LowerLeftServo_PWM 0     // Lower Left Servo
//     #define UpperLeftServo_PWM 1     // Upper Left Servo

//     #define LowerRightServo_PWM 2    // Lower Right Servo
//     #define UpperRightServo_PWM 3    // Upper Right Servo

//     #define LeftPitchServo_PWM 4     // Left Pitch Servo
//     #define RightPitchServo_PWM 5    // Right Pitch Servo

//     #define TwistServo_PWM 6         // Twist Servo
//     #define SqueezeServo_PWM 7

//     #define Vibe0_PWM 8              // Vibration motor 1
//     #define Vibe1_PWM 9            // Vibration motor 2

//     #define Vibe2_PWM 10
//     #define Vibe3_PWM 11

//     #define Heater_PWM 12             // Heating pad
//     #define CaseFan_PWM 13

//     #define ValveServo_PWM 14         // Valve Servo
#ifdef CONFIG_IDF_TARGET_ESP32
    #define MAX_TIMERS 8
#elif CONFIG_IDF_TARGET_ESP32S3
    #define MAX_TIMERS 4
#endif

#define MAX_CHANNELS (MAX_TIMERS << 1)

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