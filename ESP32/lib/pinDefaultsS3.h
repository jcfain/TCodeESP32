#pragma once
// IMPORTANT: The pins in this file may be overriden in th pinMap class depending on the board/device type selected

#include "enum.h"
// https://www.waveshare.com/wiki/ESP32-S3-Zero
// Only 8 PWM channels on S3 chip..choose wisely
// In ESP32-S3-Zero, GPIO33 to GPIO37 pins are not exposed; these pins are used for Octal PSRAM.
// ESP32-S3-Zero uses GPIO21 to connect with WS2812 RGB LED. Please refer to this link "https://files.waveshare.com/wiki/ESP32-S3-Zero/XL-0807RGBC-WS2812B.pdf" for WS2812 specifications.
// ESP32-S3-Zero does not employ a USB to UART chip. When flashing firmware, press and hold the BOOT button (GPIO0) and then connect the Type-C cable.
// The "TX" and "RX" markings on the board indicate the default UART0 pins for ESP32-S3-Zero. Specifically, TX is GPIO43, and RX is GPIO44.


// Common PWM
#define VALVE_SERVO_PIN_DEFAULT -1
#define VALVE_SERVO_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::NONE
#define TWIST_SERVO_PIN_DEFAULT 8
#define TWIST_SERVO_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::NONE
#define SQUEEZE_PIN_DEFAULT -1
#define SQUEEZE_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::NONE
#define VIBE0_PIN_DEFAULT -1
#define VIBE0_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::NONE
#define VIBE1_PIN_DEFAULT -1
#define VIBE1_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::NONE
#define VIBE2_PIN_DEFAULT -1
#define VIBE2_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::NONE
#define VIBE3_PIN_DEFAULT -1
#define VIBE3_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::NONE
#define CASE_FAN_PIN_DEFAULT -1
#define CASE_FAN_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::NONE
#define HEATER_PIN_DEFAULT -1
#define HEATER_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::NONE

// Common Analog
#define TWIST_FEEDBACK_PIN_DEFAULT -1
#define LUBE_BUTTON_PIN_DEFAULT -1
#define INTERNAL_TEMP_PIN_DEFAULT -1
#define DISPLAY_RST_PIN_DEFAULT -1
#define TEMP_PIN_DEFAULT -1
#define I2C_SDA_PIN_DEFAULT 13
#define I2C_SCL_PIN_DEFAULT 12
#define BUTTON_SET_PINS_DEFAULT {}// Arrays dont work like this. See Settingsfactory::loadDefaultVector for defaults workaround
#define BUTTON_SET_PINS_1 11
#define BUTTON_SET_PINS_2 -1
#define BUTTON_SET_PINS_3 -1
#define BUTTON_SET_PINS_4 -1
#define PD_CFG1_PIN_DEFAULT -1
#define PD_CFG2_PIN_DEFAULT -1
#define PD_CFG3_PIN_DEFAULT -1
#define SERVO_VOLTAGE_EN_PIN_DEFAULT -1
#define SERVO_VOLTAGE_PIN_DEFAULT -1
#define INPUT_VOLTAGE_PIN_DEFAULT -1

// OSR
#define RIGHT_SERVO_PIN_DEFAULT 1
#define RIGHT_SERVO_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::LOW0_CH0
#define LEFT_SERVO_PIN_DEFAULT 2
#define LEFT_SERVO_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::LOW0_CH1
#define PITCH_LEFT_SERVO_PIN_DEFAULT 3
#define PITCH_LEFT_SERVO_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::LOW1_CH2

// SR6
#define RIGHT_UPPER_SERVO_PIN_DEFAULT 4
#define RIGHT_UPPER_SERVO_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::LOW2_CH4
#define LEFT_UPPER_SERVO_PIN_DEFAULT 5
#define LEFT_UPPER_SERVO_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::LOW2_CH5
#define PITCH_RIGHTSERVO_PIN_DEFAULT 6
#define PITCH_RIGHTSERVO_CHANNEL_DEFAULT (int8_t)ESPTimerChannelNum::LOW1_CH3

// BLDC (SSR1)
#define BLDC_ENCODER_PIN_DEFAULT 1
#define BLDC_CHIPSELECT_PIN_DEFAULT 2
#define BLDC_ENABLE_PIN_DEFAULT 3
#define BLDC_HALLEFFECT_PIN_DEFAULT 4
#define BLDC_PWMCHANNEL1_PIN_DEFAULT 5
#define BLDC_PWMCHANNEL2_PIN_DEFAULT 6
#define BLDC_PWMCHANNEL3_PIN_DEFAULT 7
