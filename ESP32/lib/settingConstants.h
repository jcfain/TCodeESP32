#pragma once
#include <stdint.h>

#include "enum.h"

//#define GET_DEFAULT(X)

#define COMMON_SETTINGS_PATH "/userSettings.json"
#define PIN_SETTINGS_PATH "/pins.json"
#define ESP_TIMER_SETTINGS_PATH "/espTimers.json"
#define NETWORK_SETTINGS_PATH "/networkSettings.json"
#define BUTTON_SETTINGS_PATH "/buttons.json"
#define MOTION_PROFILE_SETTINGS_PATH "/motionProfiles.json"

// Setting defaults

#if MOTOR_TYPE == 0
    #define DEVICE_TYPE_DEFAULT (uint8_t)DeviceType::OSR
#else 
    #define DEVICE_TYPE_DEFAULT (uint8_t)DeviceType::SSR1
#endif
#define MOTOR_TYPE_DEFAULT MOTOR_TYPE
#define IP_ADDRESS_LEN 15 // TODO: Should be 13?
#define SSID_DEFAULT "YOUR SSID HERE"
#define SSID_LEN 32
#define WIFI_PASS_DEFAULT "YOUR PASSWORD HERE"
#define WIFI_PASS_LEN 63
#define AP_MODE_SSID "TCodeESP32Setup"
#define AP_MODE_PASS "12345678" // Not used?
#define DEFAULT_IP "192.168.69.1"
#define DEFAULT_GATEWAY "192.168.69.254"
#define DEFAULT_SUBNET "255.255.255.0"
#define BOARD_TYPE_DEFAULT (uint8_t)BoardType::DEVKIT
#define LOG_LEVEL_DEFAULT (uint8_t)LogLevel::INFO
//#define FULL_BUILD_DEFAULT false
#define TCODE_VERSION_DEFAULT (uint8_t)TCodeVersion::v0_3
#define UDP_SERVER_PORT_DEFAULT 8000
#define WEBSERVER_PORT_DEFAULT 80
#define HOST_NAME_DEFAULT "tcode"
#define HOST_NAME_LEN 63
#define FRIENDLY_NAME_DEFAULT "ESP32 TCode"
#define FRIENDLY_NAME_LEN 100
#define BLUETOOTH_ENABLED_DEFAULT false
#define BLE_DEVICE_TYPE_DEFAULT (uint8_t)BLEDeviceType::TCODE
#define BLE_LOVE_DEVICE_TYPE_DEFAULT (uint8_t)BLELoveDeviceType::EDGE
// #define PITCH_FREQUENCY_IS_DIFFERENT_DEFAULT false
#define MS_PER_RAD_DEFAULT 637
#define CONTINUOUS_TWIST_DEFAULT false
#define FEEDBACK_TWIST_DEFAULT false
#define ANALOG_TWIST_DEFAULT false

#define BLDC_ENCODER_DEFAULT 0
#define BLDC_USEHALLSENSOR_DEFAULT false
#define BLDC_PULLEY_CIRCUMFERENCE_DEFAULT 60
#define BLDC_MOTORA_VOLTAGE_DEFAULT 20.0f
#define BLDC_MOTORA_CURRENT_DEFAULT 1.0f
#define BLDC_MOTORA_PARAMETERSKNOWN_DEFAULT false
#define BLDC_MOTORA_ZEROELECANGLE_DEFAULT 0.0f
#define BLDC_RAILLENGTH_DEFAULT 125
#define BLDC_STROKELENGTH_DEFAULT 120
#define STATICIP_DEFAULT false
#define LOCALIP_DEFAULT "192.168.0.150"
#define GATEWAY_DEFAULT "192.168.0.1"
#define SUBNET_DEFAULT "255.255.255.0"
#define DNS1_DEFAULT "8.8.8.8"
#define DNS2_DEFAULT "8.8.4.4"
#define RIGHT_SERVO_ZERO_DEFAULT 1500
#define LEFT_SERVO_ZERO_DEFAULT 1500
#define RIGHT_UPPER_SERVO_ZERO_DEFAULT 1500
#define LEFT_UPPER_SERVO_ZERO_DEFAULT 1500
#define PITCH_LEFT_SERVO_ZERO_DEFAULT 1500
#define PITCH_RIGHT_SERVO_ZERO_DEFAULT 1500
#define TWIST_SERVO_ZERO_DEFAULT 1500
#define VALVE_SERVO_ZERO_DEFAULT 1500
#define SQUEEZE_ZERO_DEFAULT 1500
#define AUTO_VALVE_DEFAULT true
#define INVERSE_VALVE_DEFAULT false
#define VALVE_SERVO_90DEGREES_DEFAULT false
#define VIB_TIMEOUT_ENABLED_DEFAULT true
#define VIB_TIMEOUT_DEFAULT 2000
#define INVERSE_STROKE_DEFAULT false
#define INVERSE_PITCH_DEFAULT false
#define LUBE_AMOUNT_DEFAULT 255
#define LUBE_ENABLED_DEFAULT false
#define DISPLAY_ENABLED_DEFAULT true
#define SLEEVE_TEMP_DISPLAYED_DEFAULT false
#define VERSION_DISPLAYED_DEFAULT true
#define INTERNAL_TEMP_DISPLAYED_DEFAULT false
#define TEMP_SLEEVE_ENABLED_DEFAULT false
#define DISPLAY_SCREEN_WIDTH_DEFAULT 128
#define DISPLAY_SCREEN_HEIGHT_DEFAULT 64
#define TARGET_TEMP_DEFAULT 40.0f
#define HEAT_PWM_DEFAULT 255
#define HOLD_PWM_DEFAULT 110
#define DISPLAY_I2C_ADDRESS_DEFAULT "0x3c"
#define DISPLAY_I2C_ADDRESS_LEN 5
#define HEATER_THRESHOLD_DEFAULT 5.0f
#define HEATER_RESOLUTION_DEFAULT 8
// #define HEATER_FREQUENCY_DEFAULT 50
#define FAN_CONTROL_ENABLED_DEFAULT false
// #define CASE_FAN_FREQUENCY_DEFAULT 25
#define CASE_FAN_RESOLUTION_DEFAULT 10
#define INTERNAL_TEMP_FOR_FAN_DEFAULT 30.0
#define INTERNAL_MAX_TEMP_DEFAULT 50.0
#define TEMP_INTERNAL_ENABLED_DEFAULT false
#define BATTERY_LEVEL_ENABLED_DEFAULT false
#define BATTERY_LEVEL_NUMERIC_DEFAULT false
#define BATTERY_VOLTAGE_MAX_DEFAULT 12.6
#define BATTERY_CAPACITY_MAX_DEFAULT 3500
#define VOICE_ENABLED_DEFAULT false
#define VOICE_MUTED_DEFAULT false
#define VOICE_WAKE_TIME_DEFAULT 10
#define VOICE_VOLUME_DEFAULT 5
// Arrays dont work like this. See Settingsfactory::loadDefaultVector for defaults workaround
#define LOG_INCLUDETAGS_DEFAULT {}
#define LOG_EXCLUDETAGS_DEFAULT {}

#define BOOT_BUTTON_ENABLED_DEFAULT false
#define BOOT_BUTTON_COMMAND_DEFAULT "#motion-profile-cycle"
#define BOOT_BUTTON_COMMAND_LEN MAX_COMMAND
#define BUTTON_SETS_ENABLED_DEFAULT false
#define BUTTON_ANALOG_DEBOUNCE_DEFAULT 200

#define MOTION_PROFILE_SELECTED_INDEX_DEFAULT 0

#define DEVICE_TYPE "deviceType"
#define MOTOR_TYPE_SETTING "motorType"
#define SSID_SETTING "ssid"
#define WIFI_PASS_SETTING "wifiPass"
#define BOARD_TYPE_SETTING "boardType"
#define LOG_LEVEL_SETTING "logLevel"
//#define FULL_BUILD "fullBuild"
#define TCODE_VERSION_SETTING "TCodeVersion"
#define UDP_SERVER_PORT "udpServerPort"
#define WEBSERVER_PORT "webServerPort"
#define HOST_NAME "hostname"
#define FRIENDLY_NAME "friendlyName"
#define BLUETOOTH_ENABLED "bluetoothEnabled"
#define BLE_DEVICE_TYPE "bleDeviceType"
#define BLE_LOVE_DEVICE_TYPE "bleLoveDeviceType"
#define MS_PER_RAD "msPerRad"
#define CONTINUOUS_TWIST "continuousTwist"
#define FEEDBACK_TWIST "feedbackTwist"
#define ANALOG_TWIST "analogTwist"
#define BLDC_ENCODER "BLDC_Encoder"
#define BLDC_USEHALLSENSOR "BLDC_UseHallSensor"
#define BLDC_PULLEY_CIRCUMFERENCE "BLDC_Pulley_Circumference"
#define BLDC_MOTORA_VOLTAGE "BLDC_MotorA_Voltage"
#define BLDC_MOTORA_CURRENT "BLDC_MotorA_Current"
#define BLDC_MOTORA_PARAMETERSKNOWN "BLDC_MotorA_ParametersKnown"
#define BLDC_MOTORA_ZEROELECANGLE "BLDC_MotorA_ZeroElecAngle"
#define BLDC_RAILLENGTH "BLDC_RailLength"
#define BLDC_STROKELENGTH "BLDC_StrokeLength"
#define STATICIP "staticIP"
#define LOCALIP "localIP"
#define GATEWAY "gateway"
#define SUBNET "subnet"
#define DNS1 "dns1"
#define DNS2 "dns2"
//#define SR6MODE "sr6Mode"
#define RIGHT_SERVO_ZERO "RightServo_ZERO"
#define LEFT_SERVO_ZERO "LeftServo_ZERO"
#define RIGHT_UPPER_SERVO_ZERO "RightUpperServo_ZERO"
#define LEFT_UPPER_SERVO_ZERO "LeftUpperServo_ZERO"
#define PITCH_LEFT_SERVO_ZERO "PitchLeftServo_ZERO"
#define PITCH_RIGHT_SERVO_ZERO "PitchRightServo_ZERO"
#define TWIST_SERVO_ZERO "TwistServo_ZERO"
#define VALVE_SERVO_ZERO "ValveServo_ZERO"
#define SQUEEZE_ZERO "Squeeze_ZERO"
#define AUTO_VALVE "autoValve"
#define INVERSE_VALVE "inverseValve"
#define VALVE_SERVO_90DEGREES "valveServo90Degrees"
#define VIB_TIMEOUT_ENABLED "vibTimeoutEnabled"
#define VIB_TIMEOUT "vibTimeout"
#define INVERSE_STROKE "inverseStroke"
#define INVERSE_PITCH "inversePitch"
#define LUBE_AMOUNT "lubeAmount"
#define LUBE_ENABLED "lubeEnabled"
#define DISPLAY_ENABLED "displayEnabled"
#define SLEEVE_TEMP_DISPLAYED "sleeveTempDisplayed"
#define VERSION_DISPLAYED "versionDisplayed"
#define INTERNAL_TEMP_DISPLAYED "internalTempDisplayed"
#define TEMP_SLEEVE_ENABLED "tempSleeveEnabled"
#define DISPLAY_SCREEN_WIDTH "Display_Screen_Width"
#define DISPLAY_SCREEN_HEIGHT "Display_Screen_Height"
#define TARGET_TEMP "TargetTemp"
#define HEAT_PWM "HeatPWM"
#define HOLD_PWM "HoldPWM"
#define DISPLAY_I2C_ADDRESS "Display_I2C_Address"
#define HEATER_THRESHOLD "heaterThreshold"
#define HEATER_RESOLUTION "heaterResolution"
// #define HEATER_FREQUENCY "heaterFrequency"
#define FAN_CONTROL_ENABLED "fanControlEnabled"
// #define CASE_FAN_FREQUENCY "caseFanFrequency"
#define CASE_FAN_RESOLUTION "caseFanResolution"
#define INTERNAL_TEMP_FOR_FANON "internalTempForFan"
#define INTERNAL_MAX_TEMP "internalMaxTemp"
#define TEMP_INTERNAL_ENABLED "tempInternalEnabled"
#define BATTERY_LEVEL_ENABLED "batteryLevelEnabled"
#define BATTERY_LEVEL_NUMERIC "batteryLevelNumeric"
#define BATTERY_VOLTAGE_MAX "batteryVoltageMax"
#define BATTERY_CAPACITY_MAX "batteryCapacityMax"
#define VOICE_ENABLED "voiceEnabled"
#define VOICE_MUTED "voiceMuted"
#define VOICE_WAKE_TIME "voiceWakeTime"
#define VOICE_VOLUME "voiceVolume"
#define LOG_INCLUDETAGS "log-include-tags"
#define LOG_EXCLUDETAGS "log-exclude-tags"

#define BOOT_BUTTON_ENABLED "bootButtonEnabled"
#define BOOT_BUTTON_COMMAND "bootButtonCommand"
#define BUTTON_SETS_ENABLED "buttonSetsEnabled"
#define BUTTON_ANALOG_DEBOUNCE "buttonAnalogDebounce"

#define MOTION_PROFILE_SELECTED_INDEX "motionSelectedProfileIndex"
#define MOTION_PROFILE_DEFAULT_INDEX "motionDefaultProfileIndex"
#define MOTION_PROFILES "motionProfiles"
#define MOTION_ENABLED "motionEnabled"
#define MOTION_PAUSED "motionPaused"

#define ESP_H_TIMER0_FREQUENCY "ESP_H_TIMER0_FREQUENCY"
#define ESP_H_TIMER1_FREQUENCY "ESP_H_TIMER1_FREQUENCY"
#define ESP_H_TIMER2_FREQUENCY "ESP_H_TIMER2_FREQUENCY"
#define ESP_H_TIMER3_FREQUENCY "ESP_H_TIMER3_FREQUENCY"
#define ESP_L_TIMER0_FREQUENCY "ESP_L_TIMER0_FREQUENCY"
#define ESP_L_TIMER1_FREQUENCY "ESP_L_TIMER1_FREQUENCY"
#define ESP_L_TIMER2_FREQUENCY "ESP_L_TIMER2_FREQUENCY"
#define ESP_L_TIMER3_FREQUENCY "ESP_L_TIMER3_FREQUENCY"

;