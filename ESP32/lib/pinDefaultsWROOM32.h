#pragma once
#include "enum.h"

// Common PWM
#define VALVE_SERVO_PIN_DEFAULT 25
#define VALVE_SERVO_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::HIGH3_CH6
#define TWIST_SERVO_PIN_DEFAULT 27
#define TWIST_SERVO_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::HIGH2_CH4
#define SQUEEZE_PIN_DEFAULT 17
#define SQUEEZE_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::HIGH2_CH5
#define VIBE0_PIN_DEFAULT 18
#define VIBE0_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::LOW0_CH0
#define VIBE1_PIN_DEFAULT 19
#define VIBE1_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::LOW0_CH1
#define VIBE2_PIN_DEFAULT 23
#define VIBE2_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::LOW1_CH2
#define VIBE3_PIN_DEFAULT 32
#define VIBE3_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::LOW2_CH4
#define CASE_FAN_PIN_DEFAULT 16
#define CASE_FAN_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::LOW3_CH6
#define HEATER_PIN_DEFAULT 33
#define HEATER_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::LOW2_CH4

// Common Analog
#define TWIST_FEEDBACK_PIN_DEFAULT 26
#define LUBE_BUTTON_PIN_DEFAULT 35
#define INTERNAL_TEMP_PIN_DEFAULT 34
#define DISPLAY_RST_PIN_DEFAULT -1
#define TEMP_PIN_DEFAULT 5
#define I2C_SDA_PIN_DEFAULT 21
#define I2C_SCL_PIN_DEFAULT 22
#define BUTTON_SET_PINS_DEFAULT {}// Arrays dont work like this. See Settingsfactory::loadDefaultVector for defaults workaround
#define BUTTON_SET_PINS_1 39
#define BUTTON_SET_PINS_2 -1
#define BUTTON_SET_PINS_3 -1
#define BUTTON_SET_PINS_4 -1

// OSR
#define RIGHT_SERVO_PIN_DEFAULT 13
#define RIGHT_SERVO_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::HIGH0_CH0
#define LEFT_SERVO_PIN_DEFAULT 15
#define LEFT_SERVO_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::HIGH0_CH1
#define PITCH_LEFT_SERVO_PIN_DEFAULT 4
#define PITCH_LEFT_SERVO_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::HIGH1_CH2

// SR6
#define RIGHT_UPPER_SERVO_PIN_DEFAULT 12
#define RIGHT_UPPER_SERVO_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::HIGH2_CH4
#define LEFT_UPPER_SERVO_PIN_DEFAULT 2
#define LEFT_UPPER_SERVO_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::HIGH2_CH5
#define PITCH_RIGHTSERVO_PIN_DEFAULT 14
#define PITCH_RIGHTSERVO_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::HIGH1_CH3

// BLDC (SSR1)
#define BLDC_ENCODER_PIN_DEFAULT 33
#define BLDC_CHIPSELECT_PIN_DEFAULT 5
#define BLDC_ENABLE_PIN_DEFAULT 14
#define BLDC_HALLEFFECT_PIN_DEFAULT 35
#define BLDC_PWMCHANNEL1_PIN_DEFAULT 27
#define BLDC_PWMCHANNEL2_PIN_DEFAULT 26
#define BLDC_PWMCHANNEL3_PIN_DEFAULT 25