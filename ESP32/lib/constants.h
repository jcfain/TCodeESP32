#pragma once

#define FIRMWARE_VERSION 0.55f
#define FIRMWARE_VERSION_NAME "0.55b\n"
#define FIRMWARE_NAME "TCode ESP32 Firmware"
#define TCODE_DEVICE_INFO FIRMWARE_NAME " v" FIRMWARE_VERSION_NAME
#define MAX_WS_COMMAND 25
#define MAX_WS_MESSAGE 512
#define MAX_LOG_STORE 256
#define MAX_BUTTON_SETS 4
#define MAX_BUTTONS 4
#define MAX_COMMAND 256
#define TCODE_MIN 0
#define TCODE_MID 5000
#define TCODE_MAX 9999
#define LOG_PATH "/log.json"
#define DECOY_PASS "Too bad haxor!"
#define TCODE_COMMAND_FIRMWARE "D0\n"
#define TCODE_COMMAND_VERSION "D1\n"
#define TCODE_COMMAND_SETTINGS "D2\n"
#define WIFI_PASS_DONOTCHANGE_DEFAULT "YOUR PASSWORD HERE"
#if defined(CONFIG_IDF_TARGET_ESP32S3)
#include "S3/config.h"
#elif defined(CONFIG_IDF_TARGET_ESP32)
#include "ESP32/config.h"
#elif defined(CONFIG_IDF_TARGET_ESP32C5)
#include "C5/config.h"
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
#include "C6/config.h"
#elif defined(CONFIG_IDF_TARGET_ESP32E22)
#include "E22/config.h"
#endif

#if !defined(MOTOR_TYPE_SERVO) && !defined(MOTOR_TYPE_BLDC)
    #error "Invalid motor type"
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


// const Channel ChannelMapV2[9] = {
//     {TCODE_CHANNEL_STROKE,"Stroke",0,500,999,false,false,0,500,999},
//     {TCODE_CHANNEL_SURGE,"Surge",0,500,999,false,true,0,500,999},
//     {TCODE_CHANNEL_SWAY,"Sway",0,500,999,false,true,0,500,999},
//     {"L3","Suck",0,500,999,false,false,0,500,999},
//     {TCODE_CHANNEL_TWIST,"Twist",0,500,999,false,false,0,500,999},
//     {TCODE_CHANNEL_ROLL,"Roll",0,500,999,false,false,0,500,999},
//     {TCODE_CHANNEL_PITCH,"Pitch",0,500,999,false,false,0,500,999},
//     {TCODE_CHANNEL_VIBE1,"Vibe 0",0,500,999,true,false,0,500,999},
//     {TCODE_CHANNEL_VIBE2,"Vibe 1/Lube",0,500,999,true,false,0,500,999}
// };

#define TCODE_CHANNEL_STROKE "L0"
#define TCODE_CHANNEL_SURGE "L1"
#define TCODE_CHANNEL_SWAY "L2"
#define TCODE_CHANNEL_TWIST "R0"
#define TCODE_CHANNEL_ROLL "R1"
#define TCODE_CHANNEL_PITCH "R2"
#define TCODE_CHANNEL_VIBE1 "V0"
#define TCODE_CHANNEL_VIBE2 "V1"
#define TCODE_CHANNEL_VIBE3 "V2"
#define TCODE_CHANNEL_VIBE4 "V3"
#define TCODE_CHANNEL_SUCK "A0"
#define TCODE_CHANNEL_SUCK_LEVEL "A1"
#define TCODE_CHANNEL_LUBE "A2"
#define TCODE_CHANNEL_AUX "A3"

#include "channel.h"
const Channel ChannelMapV3[14] = {
    {TCODE_CHANNEL_STROKE,"Stroke",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {TCODE_CHANNEL_SURGE,"Surge",false,true,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {TCODE_CHANNEL_SWAY,"Sway",false,true,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {TCODE_CHANNEL_TWIST,"Twist",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {TCODE_CHANNEL_ROLL,"Roll",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {TCODE_CHANNEL_PITCH,"Pitch",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {TCODE_CHANNEL_VIBE1,"Vibe 1",true,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {TCODE_CHANNEL_VIBE2,"Vibe 2",true,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {TCODE_CHANNEL_AUX,"Vibe 3",true,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {TCODE_CHANNEL_VIBE4,"Vibe 4",true,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {TCODE_CHANNEL_SUCK,"Suck manual",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {TCODE_CHANNEL_SUCK_LEVEL,"Suck level",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {TCODE_CHANNEL_LUBE,"Lube",true,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {TCODE_CHANNEL_AUX,"Auxiliary",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX}
};

const Channel ChannelMapBLDC[10] = {
    {TCODE_CHANNEL_STROKE,"Stroke",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {TCODE_CHANNEL_TWIST,"Twist",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {TCODE_CHANNEL_VIBE1,"Vibe 1",true,false,TCODE_MIN,TCODE_MIN,TCODE_MAX},
    {TCODE_CHANNEL_VIBE2,"Vibe 2",true,false,TCODE_MIN,TCODE_MIN,TCODE_MAX},
    {TCODE_CHANNEL_VIBE3,"Vibe 3",true,false,TCODE_MIN,TCODE_MIN,TCODE_MAX},
    {TCODE_CHANNEL_VIBE4,"Vibe 4",true,false,TCODE_MIN,TCODE_MIN,TCODE_MAX},
    {TCODE_CHANNEL_SUCK,"Suck manual",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {TCODE_CHANNEL_SUCK_LEVEL,"Suck level",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX},
    {TCODE_CHANNEL_LUBE,"Lube",true,false,TCODE_MIN,TCODE_MIN,TCODE_MAX},
    {TCODE_CHANNEL_AUX,"Auxiliary",false,false,TCODE_MIN,TCODE_MID,TCODE_MAX}
};