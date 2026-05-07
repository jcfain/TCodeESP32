#pragma once
#include <stdint.h>

#include "enum.h"

// #define GET_DEFAULT(X)

#define COMMON_SETTINGS_PATH "/userSettings.json"
#define PIN_SETTINGS_PATH "/pins.json"
#define ESP_TIMER_SETTINGS_PATH "/espTimers.json"
#define NETWORK_SETTINGS_PATH "/networkSettings.json"
#define BUTTON_SETTINGS_PATH "/buttons.json"
#define MOTION_PROFILE_SETTINGS_PATH "/motionProfiles.json"
#define CHANNELS_SETTINGS_PATH "/channels.json"
#define DEBUG_INFO_PATH "/debugInfo.json"

// Setting defaults
#ifndef DEFAULT_DEVICE
#if MOTOR_TYPE == 0
#define DEVICE_TYPE_NAME_DEFAULT "OSR"
#define DEVICE_TYPE_DEFAULT (uint8_t)DeviceType::OSR
#else
#define DEVICE_TYPE_NAME_DEFAULT "NONE"
#define DEVICE_TYPE_DEFAULT (uint8_t)DeviceType::SSR1
#endif
#else
#define DEVICE_TYPE_NAME_DEFAULT "OSR"
#define DEVICE_TYPE_DEFAULT (int)DEFAULT_DEVICE
#endif
#define MOTOR_TYPE_DEFAULT MOTOR_TYPE
#define IP_ADDRESS_LEN IP4ADDR_STRLEN_MAX // 16
#define SSID_DEFAULT "YOUR SSID HERE"
#define SSID_LEN 32
#define WIFI_PASS_DEFAULT "YOUR PASSWORD HERE"
#define WIFI_PASS_LEN 63
#define WIFI_BAND_SETTING_DEFAULT (uint8_t)WifiBand::AUTO
#define AP_MODE_SSID_DEFAULT "TCodeESP32Setup"
#define AP_MODE_PASS_DEFAULT "tcode_6969"
#define AP_MODE_HIDDEN_DEFAULT false
#define AP_MODE_CHANNEL_DEFAULT 1
#define AP_MODE_IP_DEFAULT "192.168.69.1"
#define AP_MODE_GATEWAY_DEFAULT "192.168.69.254"
#define AP_MODE_SUBNET_DEFAULT "255.255.255.0"
#ifndef DEFAULT_BOARD
#if CONFIG_IDF_TARGET_ESP32
#define BOARD_TYPE_DEFAULT (uint8_t)BoardType::N8R8
#elif CONFIG_IDF_TARGET_ESP32S3
#ifdef S3_ZERO
#define BOARD_TYPE_DEFAULT (uint8_t)BoardType::ZERO
#else
#define BOARD_TYPE_DEFAULT (uint8_t)BoardType::N8R8
#endif
#endif
#else
#define BOARD_TYPE_DEFAULT (int)DEFAULT_BOARD
#endif
#define LOG_LEVEL_DEFAULT (uint8_t)LogLevel::INFO
// #define FULL_BUILD_DEFAULT false
#define TCODE_VERSION_DEFAULT (uint8_t)TCodeVersion::v0_3
#define UDP_SERVER_PORT_DEFAULT 8000
#define WEBSERVER_PORT_DEFAULT 80
#define HOST_NAME_DEFAULT "tcode"
#define HOST_NAME_LEN 63
#define FRIENDLY_NAME_DEFAULT "ESP32 TCode"
#define FRIENDLY_NAME_LEN 100
#define BLUETOOTH_ENABLED_DEFAULT false
#define BLE_ENABLED_DEFAULT false
#define BLE_DEVICE_TYPE_DEFAULT (uint8_t)BLEDeviceType::TCODE
#define BLE_LOVE_DEVICE_TYPE_DEFAULT (uint8_t)BLELoveDeviceType::EDGE
// #define PITCH_FREQUENCY_IS_DIFFERENT_DEFAULT false
#define MAX_SERVO_RANGE_DEFAULT 180
#define CONTINUOUS_TWIST_DEFAULT false
#define FEEDBACK_TWIST_DEFAULT false
#define ANALOG_TWIST_DEFAULT false

#define BLDC_ENCODER_DEFAULT (uint8_t)BLDCEncoderType::NONE
#define BLDC_USEHALLSENSOR_DEFAULT false
#define BLDC_TWIST_MULTIPLIER_DEFAULT 1.0f
#define BLDC_RAILLENGTH_DEFAULT 125
#define BLDC_STROKELENGTH_DEFAULT 120
#define BLDC_PID_PROPORTIONAL_CONST_DEFAULT 0.002f
#define BLDC_LOWPASS_FILTER_DEFAULT 0.8f
#define BLDC_TWIST_LIMIT_DEFAULT 0.5f

#define BLDC_MOTORA_PULLEY_CIRCUMFERENCE_DEFAULT 60
#define BLDC_PULLEY_CIRCUMFERENCE_DEFAULT BLDC_MOTORA_PULLEY_CIRCUMFERENCE_DEFAULT
#define BLDC_MOTORA_VOLTAGE_DEFAULT 20.0f
#define BLDC_MOTORA_SUPPLY_DEFAULT 20.0f
#define BLDC_MOTORA_CURRENT_DEFAULT 1.0f
#define BLDC_MOTORA_PARAMETERSKNOWN_DEFAULT false
#define BLDC_MOTORA_ZEROELECANGLE_DEFAULT -12345.0f // FOC NOT_SET

#define BLDC_MOTORB_PULLEY_CIRCUMFERENCE_DEFAULT 60
#define BLDC_MOTORB_VOLTAGE_DEFAULT 20.0f
#define BLDC_MOTORB_SUPPLY_DEFAULT 20.0f
#define BLDC_MOTORB_CURRENT_DEFAULT 1.0f
#define BLDC_MOTORB_PARAMETERSKNOWN_DEFAULT false
#define BLDC_MOTORB_ZEROELECANGLE_DEFAULT -12345.0f // FOC NOT_SET

#define STATICIP_DEFAULT false
#define LOCALIP_DEFAULT "192.168.0.150"
#define GATEWAY_DEFAULT "192.168.0.1"
#define SUBNET_DEFAULT "255.255.255.0"
#define DNS1_DEFAULT "8.8.8.8"
#define DNS2_DEFAULT "8.8.4.4"
#define MDNS_ENABLED_DEFAULT true
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
#define INVERSE_TWIST_DEFAULT false
#define LUBE_AMOUNT_DEFAULT 255
#define LUBE_ENABLED_DEFAULT false
#define LUBE_BUTTON_PIN_MODE_DEFAULT (uint8_t)LubeButtonPinMode::PULL_UP
#define DISPLAY_ENABLED_DEFAULT false
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

#define SERVO_RESOLUTION_DEFAULT MAX_PWM_RESOLUTION
#define VIBE_RESOLUTION_DEFAULT 8
#define LUBE_RESOLUTION_DEFAULT 8

#define HEATER_THRESHOLD_DEFAULT 5.0f
#define HEATER_RESOLUTION_DEFAULT 8
// #define HEATER_FREQUENCY_DEFAULT 50
#define FAN_CONTROL_ENABLED_DEFAULT false
// #define CASE_FAN_FREQUENCY_DEFAULT 25
#define CASE_FAN_RESOLUTION_DEFAULT 10
#define CASE_FAN_MAX_PWM_DEFAULT 1023 // Set for default CASE_FAN_RESOLUTION_DEFAULT 10 bit
#define INTERNAL_TEMP_FOR_FAN_DEFAULT 30.0
#define INTERNAL_MAX_TEMP_DEFAULT 50.0
#define TEMP_INTERNAL_ENABLED_DEFAULT false
#define BATTERY_LEVEL_ENABLED_DEFAULT false
#define BATTERY_LEVEL_NUMERIC_DEFAULT false
#define BATTERY_VOLTAGE_MAX_DEFAULT 12.6
#define BATTERY_CAPACITY_MAX_DEFAULT 3500
#define POWER_MONITOR_3V3_DIVIDER_RATIO_DEFAULT 1.0f
#define POWER_MONITOR_5V_DIVIDER_RATIO_DEFAULT 1.0f
#define POWER_MONITOR_BATTERY_DIVIDER_RATIO_DEFAULT 1.0f
#define POWER_MONITOR_MOTOR_DIVIDER_RATIO_DEFAULT 1.0f
#define POWER_MONITOR_BUS_DIVIDER_RATIO_DEFAULT 1.0f
#define POWER_MONITOR_3V3_OFFSET_DEFAULT 0.0f
#define POWER_MONITOR_5V_OFFSET_DEFAULT 0.0f
#define POWER_MONITOR_BATTERY_OFFSET_DEFAULT 0.0f
#define POWER_MONITOR_MOTOR_OFFSET_DEFAULT 0.0f
#define POWER_MONITOR_BUS_OFFSET_DEFAULT 0.0f
#define POWER_MONITOR_VBUS_NOMINAL_DEFAULT 20.0f
#if MOTOR_TYPE == 1
#define POWER_MONITOR_VMOTOR_NOMINAL_DEFAULT 20.0f
#else
#define POWER_MONITOR_VMOTOR_NOMINAL_DEFAULT 9.0f
#endif
#define SERVO_VOLTAGE_ENABLE_PIN_DEFAULT -1
#define SERVO_VOLTAGE_ENABLE_STATE_DEFAULT true
#define VOICE_ENABLED_DEFAULT false
#define VOICE_MUTED_DEFAULT false
#define VOICE_WAKE_TIME_DEFAULT 10
#define VOICE_VOLUME_DEFAULT 5
// Arrays dont work like this. See Settingsfactory::loadDefaultVector for defaults workaround
// Use "" as placeholder — variant<int,char*,float,double,bool> cannot hold {}.
#define LOG_INCLUDETAGS_DEFAULT ""
#define LOG_EXCLUDETAGS_DEFAULT ""
#define DEBUG_INFO_LAST_BOOT_REASONS_DEFAULT {}

#define LAST_BOOT_REASONS_MAX "lastBootReasonsMax"
#define LAST_BOOT_REASONS_MAX_DEFAULT 50

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
#define WIFI_BAND_SETTING "wifiBand"
#define AP_MODE_SSID "apModeSSID"
#define AP_MODE_PASS "apModePass"
#define AP_MODE_HIDDEN "apModeHidden"
#define AP_MODE_CHANNEL "apModeChannel"
#define AP_MODE_IP "apModeIP"
#define AP_MODE_SUBNET "apModeSubnet"
#define AP_MODE_GATEWAY "apModeGateway"

#define BOARD_TYPE_SETTING "boardType"
#define LOG_LEVEL_SETTING "logLevel"
// #define FULL_BUILD "fullBuild"
#define TCODE_VERSION_SETTING "TCodeVersion"
#define UDP_SERVER_PORT "udpServerPort"
#define WEBSERVER_PORT "webServerPort"
#define HOST_NAME "hostname"
#define FRIENDLY_NAME "friendlyName"
#define BLUETOOTH_ENABLED "bluetoothEnabled"
#define BLE_ENABLED "bleEnabled"
#define BLE_DEVICE_TYPE "bleDeviceType"
#define BLE_LOVE_DEVICE_TYPE "bleLoveDeviceType"
// #define MS_PER_RAD "msPerRad"
#define MAX_SERVO_RANGE "maxServoRange"
#define CONTINUOUS_TWIST "continuousTwist"
#define FEEDBACK_TWIST "feedbackTwist"
#define ANALOG_TWIST "analogTwist"
#define BLDC_ENCODER "BLDC_Encoder"
#define BLDC_USEHALLSENSOR "BLDC_UseHallSensor"
#define BLDC_PULLEY_CIRCUMFERENCE "BLDC_Pulley_Circumference"
#define BLDC_MOTORA_VOLTAGE "BLDC_MotorA_VoltageLimit"
#define BLDC_MOTORA_SUPPLY "BLDC_MotorA_SupplyVoltage"
#define BLDC_MOTORA_CURRENT "BLDC_MotorA_Current"
#define BLDC_MOTORA_PARAMETERSKNOWN "BLDC_MotorA_ParametersKnown"
#define BLDC_MOTORA_ZEROELECANGLE "BLDC_MotorA_ZeroElecAngle"
#define BLDC_RAILLENGTH "BLDC_RailLength"
#define BLDC_STROKELENGTH "BLDC_StrokeLength"
#define BLDC_TWIST_MULTIPLIER "BLDC_TwistMultiplier"
#define BLDC_USEHALLSENSOR "BLDC_UseHallSensor"
#define BLDC_PID_PROPORTIONAL_CONST "BLDC_PIDProportionalConstant"
#define BLDC_LOWPASS_FILTER "BLDC_LowPassFilter"
#define BLDC_TWIST_LIMIT "BLDC_TwistLimit"

// MotorA aliases follow the pre-merge MillibyteProducts naming convention so
// existing saved settings load correctly. Do not redefine BLDC_MOTORA_VOLTAGE,
// BLDC_MOTORA_SUPPLY, BLDC_MOTORA_CURRENT, or BLDC_MOTORA_ZEROELECANGLE here;
// those are already defined above.
#define BLDC_MOTORA_ENCODER BLDC_ENCODER
#define BLDC_MOTORA_PULLEY_CIRCUMFERENCE BLDC_PULLEY_CIRCUMFERENCE

#define BLDC_MOTORB_ENCODER "BLDC_BEncoder"
#define BLDC_MOTORB_PULLEY_CIRCUMFERENCE "BLDC_BPulley_Circumference"
#define BLDC_MOTORB_VOLTAGE "BLDC_MotorB_VoltageLimit"
#define BLDC_MOTORB_SUPPLY  "BLDC_MotorB_SupplyVoltage"
#define BLDC_MOTORB_CURRENT "BLDC_MotorB_Current"
#define BLDC_MOTORB_ZEROELECANGLE "BLDC_MotorB_ZeroElecAngle"

#define STATICIP "staticIP"
#define LOCALIP "localIP"
#define GATEWAY "gateway"
#define SUBNET "subnet"
#define DNS1 "dns1"
#define DNS2 "dns2"
#define MDNS_ENABLED "mdnsEnabled"
// #define SR6MODE "sr6Mode"
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
#define INVERSE_TWIST "inverseTwist"
#define LUBE_AMOUNT "lubeAmount"
#define LUBE_ENABLED "lubeEnabled"
#define LUBE_BUTTON_PIN_MODE "lubeButtonPinMode"
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
#define CASE_FAN_MAX_PWM "caseFanMaxPWM"
#define DISPLAY_I2C_ADDRESS "Display_I2C_Address"
#define SERVO_RESOLUTION "servoResolution"
#define VIBE_RESOLUTION "vibeResolution"
#define LUBE_RESOLUTION "lubeResolution"
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
#define POWER_MONITOR_3V3_DIVIDER_RATIO "powerMonitor3v3DividerRatio"
#define POWER_MONITOR_5V_DIVIDER_RATIO "powerMonitor5vDividerRatio"
#define POWER_MONITOR_BATTERY_DIVIDER_RATIO "powerMonitorBatteryDividerRatio"
#define POWER_MONITOR_MOTOR_DIVIDER_RATIO "powerMonitorMotorDividerRatio"
#define POWER_MONITOR_BUS_DIVIDER_RATIO "powerMonitorBusDividerRatio"
#define POWER_MONITOR_3V3_OFFSET "powerMonitor3v3Offset"
#define POWER_MONITOR_5V_OFFSET "powerMonitor5vOffset"
#define POWER_MONITOR_BATTERY_OFFSET "powerMonitorBatteryOffset"
#define POWER_MONITOR_MOTOR_OFFSET "powerMonitorMotorOffset"
#define POWER_MONITOR_BUS_OFFSET "powerMonitorBusOffset"
#define POWER_MONITOR_VBUS_NOMINAL "powerMonitorVBusNominal"
#define POWER_MONITOR_VMOTOR_NOMINAL "powerMonitorVMotorNominal"
#define SERVO_VOLTAGE_ENABLE_PIN "servoVoltageEnablePin"
#define SERVO_VOLTAGE_ENABLE_STATE "servoVoltageEnableState"
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

#define CHANNEL_PROFILE "channelProfile"
#define CHANNEL_NAME "name"
#define CHANNEL_FRIENDLY_NAME "friendlyName"
#define CHANNEL_MIN "min"
#define CHANNEL_MID "mid"
#define CHANNEL_MAX "max"
#define CHANNEL_USER_MIN "userMin"
#define CHANNEL_USER_MID "userMid"
#define CHANNEL_USER_MAX "userMax"
#define CHANNEL_RANGE_LIMIT_ENABLED "rangeLimitEnabled"
#define CHANNEL_IS_SWITCH "isSwitch"
#define CHANNEL_SR6_ONLY "sr6Only"

#define ESP_H_TIMER0_FREQUENCY "ESP_H_TIMER0_FREQUENCY"
#define ESP_H_TIMER1_FREQUENCY "ESP_H_TIMER1_FREQUENCY"
#define ESP_H_TIMER2_FREQUENCY "ESP_H_TIMER2_FREQUENCY"
#define ESP_H_TIMER3_FREQUENCY "ESP_H_TIMER3_FREQUENCY"
#define ESP_L_TIMER0_FREQUENCY "ESP_L_TIMER0_FREQUENCY"
#define ESP_L_TIMER1_FREQUENCY "ESP_L_TIMER1_FREQUENCY"
#define ESP_L_TIMER2_FREQUENCY "ESP_L_TIMER2_FREQUENCY"
#define ESP_L_TIMER3_FREQUENCY "ESP_L_TIMER3_FREQUENCY"

#define ESP_H_TIMER0_DRIVER "ESP_H_TIMER0_DRIVER"
#define ESP_H_TIMER1_DRIVER "ESP_H_TIMER1_DRIVER"
#define ESP_H_TIMER2_DRIVER "ESP_H_TIMER2_DRIVER"
#define ESP_H_TIMER3_DRIVER "ESP_H_TIMER3_DRIVER"
#define ESP_L_TIMER0_DRIVER "ESP_L_TIMER0_DRIVER"
#define ESP_L_TIMER1_DRIVER "ESP_L_TIMER1_DRIVER"
#define ESP_L_TIMER2_DRIVER "ESP_L_TIMER2_DRIVER"
#define ESP_L_TIMER3_DRIVER "ESP_L_TIMER3_DRIVER"

// Readonly
#define DEBUG_INFO_LAST_BOOT_REASONS "lastBootReasons"

;
