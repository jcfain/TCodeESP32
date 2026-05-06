#pragma once

#include <stdint.h>
#include "LogHandler.h"
#include "TagHandler.h"
#include "espTimerMap.h"
#if CONFIG_IDF_TARGET_ESP32
    #include "ESP32/pinDefaults.h"
#elif CONFIG_IDF_TARGET_ESP32S3
    #ifdef S3_ZERO
        #include "S3/pinDefaultsZero.h"
    #else
        #include "S3/pinDefaults.h"
    #endif
#elif CONFIG_IDF_TARGET_ESP32C5
    #include "C5/pinDefaults.h"
#elif CONFIG_IDF_TARGET_ESP32C6
    #include "C6/pinDefaults.h"
#elif CONFIG_IDF_TARGET_ESP32C61
    #include "E22/pinDefaults.h"
#endif

// Common PWM
#define VALVE_SERVO_PIN "ValveServo_PIN"
#define VALVE_SERVO_CHANNEL "ValveServo_CHANNEL"
#define TWIST_SERVO_PIN "TwistServo_PIN"
#define TWIST_SERVO_CHANNEL "TwistServo_CHANNEL"
#define SQUEEZE_PIN "Squeeze_PIN"
#define SQUEEZE_CHANNEL "Squeeze_CHANNEL"
#define VIBE0_PIN "Vibe0_PIN"
#define VIBE0_CHANNEL "Vibe0_CHANNEL"
#define VIBE1_PIN "Vibe1_PIN"
#define VIBE1_CHANNEL "Vibe1_CHANNEL"
#define VIBE2_PIN "Vibe2_PIN"
#define VIBE2_CHANNEL "Vibe2_CHANNEL"
#define VIBE3_PIN "Vibe3_PIN"
#define VIBE3_CHANNEL "Vibe3_CHANNEL"
#define CASE_FAN_PIN "Case_Fan_PIN"
#define CASE_FAN_CHANNEL "Case_Fan_CHANNEL"
#define HEATER_PIN "Heater_PIN"
#define HEATER_CHANNEL "Heater_CHANNEL"

// Common Analog
#define LUBE_BUTTON_PIN "LubeButton_PIN"
#define INTERNAL_TEMP_PIN "Internal_Temp_PIN"
#define I2C_SDA_PIN "i2cSda_PIN"
#define I2C_SCL_PIN "i2cScl_PIN"
#define DISPLAY_RST_PIN "Display_Rst_PIN"
#define TEMP_PIN "Temp_PIN"
#define TWIST_FEEDBACK_PIN "TwistFeedBack_PIN"
#define BUTTON_SET_PINS "Button_Set_PIN"
#define VOLTAGE_3V3_PIN "Voltage_3V3_PIN"
#define VOLTAGE_5V_PIN "Voltage_5V_PIN"
#define VOLTAGE_BATTERY_PIN "Voltage_Battery_PIN"
#define VOLTAGE_MOTOR_PIN "Voltage_Motor_PIN"
#define VOLTAGE_BUS_PIN "Voltage_Bus_PIN"

// OSR
#define RIGHT_SERVO_PIN "RightServo_PIN"
#define RIGHT_SERVO_CHANNEL "RightServo_CHANNEL"
#define LEFT_SERVO_PIN "LeftServo_PIN"
#define LEFT_SERVO_CHANNEL "LeftServo_CHANNEL"
#define PITCH_LEFT_SERVO_PIN "PitchLeftServo_PIN"
#define PITCH_LEFT_SERVO_CHANNEL "PitchLeftServo_CHANNEL"

// SR6
#define PITCH_RIGHTSERVO_PIN "PitchRightServo_PIN"
#define PITCH_RIGHTSERVO_CHANNEL "PitchRightServo_CHANNEL"
#define RIGHT_UPPER_SERVO_PIN "RightUpperServo_PIN"
#define RIGHT_UPPER_SERVO_CHANNEL "RightUpperServo_CHANNEL"
#define LEFT_UPPER_SERVO_PIN "LeftUpperServo_PIN"
#define LEFT_UPPER_SERVO_CHANNEL "LeftUpperServo_CHANNEL"

// Stroke BLDC (SSR1)
#define BLDC_ENCODER_PIN "BLDC_Encoder_PIN"
#define BLDC_CHIPSELECT_PIN "BLDC_ChipSelect_PIN"
#define BLDC_ENABLE_PIN "BLDC_Enable_PIN"
#define BLDC_HALLEFFECT_PIN "BLDC_HallEffect_PIN"
#define BLDC_PWMCHANNEL1_PIN "BLDC_PWMchannel1_PIN"
#define BLDC_PWMCHANNEL2_PIN "BLDC_PWMchannel2_PIN"
#define BLDC_PWMCHANNEL3_PIN "BLDC_PWMchannel3_PIN"

// Twist BLDC (SSR2)
#define BLDC_B_ENCODER_PIN "BLDC_BEncoder_PIN"
#define BLDC_B_CHIPSELECT_PIN "BLDC_BChipSelect_PIN"
#define BLDC_B_ENABLE_PIN "BLDC_BEnable_PIN"
#define BLDC_B_HALLEFFECT_PIN "BLDC_BHallEffect_PIN"
#define BLDC_B_PWMCHANNEL1_PIN "BLDC_BPWMchannel1_PIN"
#define BLDC_B_PWMCHANNEL2_PIN "BLDC_BPWMchannel2_PIN"
#define BLDC_B_PWMCHANNEL3_PIN "BLDC_BPWMchannel3_PIN"

// class PinMap;

// class PinMapInfo {
// public:
//     PinMapInfo(DeviceType deviceType, BoardType boardType, PinMap* pinMap):
//         m_deviceType(deviceType),
//         m_boardType(boardType),
//         m_pinMap(pinMap) { }

//     DeviceType deviceType() {
//         return m_deviceType;
//     }

//     BoardType boardType() {
//         return m_boardType;
//     }

//     template<typename T, typename = std::enable_if<std::is_base_of<PinMap, T>::value>>
//     const T pinMap() {
//         return static_cast<const T>(m_pinMap);
//     }
//     const PinMap* pinMap() {
//         return m_pinMap;
//     }
// private:
//     DeviceType m_deviceType;
//     BoardType m_boardType;
//     PinMap* m_pinMap;
// };

class PinMap {
public:
    PinMap(PinMap const&) = delete;
    void operator=(PinMap const&) = delete;

    DeviceType deviceType() { return m_deviceType; }
    virtual void setDeviceType(DeviceType deviceType) {  m_deviceType = deviceType; }
    BoardType boardType() { return m_boardType; }
    void setBoardType(BoardType boardType) { m_boardType = boardType; }

    int8_t valve() const { return m_valve; }
    void setValve(const int8_t &valve) { m_valve = valve; }
    int8_t valveChannel() const { return m_valveChannel; }
    void setValveChannel(const int8_t &channel) { m_valveChannel = channel; }

    int8_t twist() const { return m_twist; }
    void setTwist(const int8_t &twist) { m_twist = twist; }
    int8_t twistChannel() const { return m_twistChannel; }
    void setTwistChannel(const int8_t &channel) { m_twistChannel = channel; }

    int8_t squeeze() const { return m_squeeze; }
    void setSqueeze(const int8_t &squeeze) { m_squeeze = squeeze; }
    int8_t squeezeChannel() const { return m_squeezeChannel; }
    void setSqueezeChannel(const int8_t &channel) { m_squeezeChannel = channel; }

    int8_t vibe0() const { return m_vibe0; }
    void setVibe0(const int8_t &vibe0) { m_vibe0 = vibe0; }
    int8_t vibe0Channel() const { return m_vibe0Channel; }
    void setVibe0Channel(const int8_t &channel) { m_vibe0Channel = channel; }

    int8_t vibe1() const { return m_vibe1; }
    void setVibe1(const int8_t &vibe1) { m_vibe1 = vibe1; }
    int8_t vibe1Channel() const { return m_vibe1Channel; }
    void setVibe1Channel(const int8_t &channel) { m_vibe1Channel = channel; }

    int8_t vibe2() const { return m_vibe2; }
    void setVibe2(const int8_t &vibe2) { m_vibe2 = vibe2; }
    int8_t vibe2Channel() const { return m_vibe2Channel; }
    void setVibe2Channel(const int8_t &channel) { m_vibe2Channel = channel; }

    int8_t vibe3() const { return m_vibe3; }
    void setVibe3(const int8_t &vibe3) { m_vibe3 = vibe3; }
    int8_t vibe3Channel() const { return m_vibe3Channel; }
    void setVibe3Channel(const int8_t &channel) { m_vibe3Channel = channel; }

    int8_t caseFan() const { return m_caseFan; }
    void setCaseFan(const int8_t &caseFan) { m_caseFan = caseFan; }
    int8_t caseFanChannel() const { return m_caseFanChannel; }
    void setCaseFanChannel(const int8_t &channel) { m_caseFanChannel = channel; }

    int8_t heater() const { return m_heater; }
    void setHeater(const int8_t &heater) { m_heater = heater; }
    int8_t heaterChannel() const { return m_heaterChannel; }
    void setHeaterChannel(const int8_t &channel) { m_heaterChannel = channel; }

    int8_t twistFeedBack() const { return m_twistFeedBack; }
    void setTwistFeedBack(const int8_t &twistFeedBack) { m_twistFeedBack = twistFeedBack; }

    int8_t lubeButton() const { return m_lubeButton; }
    void setLubeButton(const int8_t &lubeButton) { m_lubeButton = lubeButton; }

    int8_t internalTemp() const { return m_internalTemp; }
    void setInternalTemp(const int8_t &internalTemp) { m_internalTemp = internalTemp; }

    int8_t displayReset() const { return m_displayReset; }
    void setDisplayReset(const int8_t &displayReset) { m_displayReset = displayReset; }

    int8_t sleeveTemp() const { return m_sleeveTemp; }
    void setSleeveTemp(const int8_t &sleeveTemp) { m_sleeveTemp = sleeveTemp; }

    int8_t buttonSetPin(int8_t index) const { return m_buttonSetPins[index]; }
    void setButtonSetPin(const int8_t pin, int8_t index) {
        if(index >= MAX_BUTTON_SETS)
        {
            LogHandler::error(Tags::PinMap, "Invalid index for button set %d", index);
            return;
        }
        m_buttonSetPins[index] = pin;
    }

    int8_t i2cSda() const { return m_i2cSda; }
    void setI2cSda(const int8_t &i2cSda) { m_i2cSda = i2cSda; }

    int8_t i2cScl() const { return m_i2cScl; }
    void setI2cScl(const int8_t &i2cScl) { m_i2cScl = i2cScl; }

    int8_t voltageMonitor(VoltageMonitors source) const {
        const int8_t index = static_cast<int8_t>(source);
        if (index < 0 || index >= static_cast<int8_t>(VoltageMonitors::MAX)) {
            LogHandler::error(Tags::PinMap, "Invalid voltage monitor source %d", index);
            return -1;
        }
        return m_voltageMonitors[index];
    }
    void setVoltageMonitor(VoltageMonitors source, const int8_t& pin) {
        const int8_t index = static_cast<int8_t>(source);
        if (index < 0 || index >= static_cast<int8_t>(VoltageMonitors::MAX)) {
            LogHandler::error(Tags::PinMap, "Invalid voltage monitor source %d", index);
            return;
        }
        m_voltageMonitors[index] = pin;
    }

    int8_t servoVoltageEnable() const { return m_servoVoltageEnable; }
    void setServoVoltageEnable(const int8_t& pin) { m_servoVoltageEnable = pin; }

    // void setChannelFrequency(ESPTimerChannelNum channel, int frequency) {
    //     int8_t timer = getTimer(channel);
    //     if(timer > MAX_TIMERS - 1 || timer < 0) {
    //         LogHandler::error(Tags::PinMap, "Invalid channel '%d' when setting frequency: %d", (int8_t)channel, frequency);
    //         return;
    //     }
    //     if(frequency < 0) {
    //         LogHandler::error(Tags::PinMap, "Invalid frequency '%d' when setting frequency for channel: %d", frequency, (int8_t)channel);
    //         return;
    //     }
    //     m_timerFreq[timer] = frequency;
    // }
    int getChannelFrequency(int8_t channel) const {
        // A channel value of -1 means "not assigned in settings" — this is
        // expected when the user clears a timer-channel cell in the web GUI
        // or when no channel was ever stored. Return a safe servo-grade
        // default (50 Hz) silently so the calling handler can still attach
        // the pin via PwmManager (which auto-allocates the actual hardware
        // channel/timer). For higher-frequency LEDC consumers (vibe/lube)
        // PwmManager will reuse a matching timer if 50 Hz isn't ideal — the
        // unified solver in a follow-up will pick a per-output default.
        if (channel < 0) {
            return ESP_TIMER_FREQUENCY_DEFAULT > 0 ? ESP_TIMER_FREQUENCY_DEFAULT : 50;
        }
        if (channel >= (MAX_TIMERS << 1)) {
            LogHandler::error(Tags::PinMap, "Invalid channel '%d' when getting frequency", channel);
            return -1;
        }
        const ESPTimer* timer = getTimer(static_cast<ESPTimerChannelNum>(channel));
        if(!timer) {
            LogHandler::error(Tags::PinMap, "Invalid channel '%d' when getting frequency", channel);
            return -1;
        }
        return timer->frequency;
    }

    void setTimerFrequency(int8_t timerIndex, int frequency) {
        if(timerIndex > MAX_TIMERS - 1 || timerIndex < 0) {
            LogHandler::error(Tags::PinMap, "Invalid timer index '%d' when setting frequency: %d", timerIndex, frequency);
            return;
        }
        if(frequency < 0) {
            LogHandler::error(Tags::PinMap, "Invalid frequency '%d' when setting for timer index: %d", frequency, timerIndex);
            return;
        }
        m_timers[timerIndex].frequency = frequency;
    }
    int getTimerFrequency(int8_t timerIndex) const {
        if(timerIndex > MAX_TIMERS - 1 || timerIndex < 0) {
            //LogHandler::error(Tags::PinMap, "Invalid timer '%d' when getting frequency", timer);
            return -1;
        }
        return m_timers[timerIndex].frequency;
    }
    ESPTimer* getTimer(uint8_t timerIndex) {
        if(timerIndex >= MAX_TIMERS || timerIndex < 0) {
            return 0;
        }
        return &m_timers[timerIndex];
    }
    const ESPTimer* getTimer(uint8_t timerIndex) const {
        if (timerIndex >= MAX_TIMERS || timerIndex < 0) {
            return 0;
        }
        return &m_timers[timerIndex];
    }
    void setTimerDriver(int8_t timerIndex, PwmDriver driver) {
        if (timerIndex < 0 || timerIndex >= MAX_TIMERS) {
            LogHandler::error(Tags::PinMap, "Invalid timer index '%d' when setting driver", timerIndex);
            return;
        }
        m_timers[timerIndex].pwmDriver = driver;
    }
    /**
     * Returns the configured PWM driver for the timer that owns 'channel'.
     * Defaults to MCPWM for HIGH timers, LEDC for LOW timers.
     * When `channel < 0` (unset), defaults to MCPWM so servo paths still
     * function — LEDC is auto-selected as a fallback by PwmManager when
     * MCPWM is full. Pure-LEDC consumers (vibe/lube) call attachLedcPin
     * directly so this hint isn't consulted on their path.
     */
    PwmDriver getTimerDriverForChannel(int8_t channel) const {
        if (channel < 0) return PwmDriver::MCPWM;
        const ESPTimer* timer = getTimer(static_cast<ESPTimerChannelNum>(channel));
        if (!timer) return PwmDriver::LEDC;
        return timer->pwmDriver;
    }
protected:
    PinMap(DeviceType deviceType, BoardType boardType) {
        m_deviceType = deviceType;
        m_boardType = boardType;
    }
    const Tags::tag_t m_TAG = Tags::PinMap;
    DeviceType m_deviceType;
    BoardType m_boardType;
    ESPTimer m_timers[MAX_TIMERS] = {
#if CONFIG_IDF_TARGET_ESP32
        // HIGH timers default to MCPWM (servo-grade precision)
        { ESP_H_TIMER0_FREQUENCY, "High 0", ESP_TIMER_FREQUENCY_DEFAULT, {
                {"High 0 CH0", ESPTimerChannelNum::HIGH0_CH0},
                {"High 0 CH1", ESPTimerChannelNum::HIGH0_CH1}
            }, PwmDriver::MCPWM
        },
        { ESP_H_TIMER1_FREQUENCY, "High 1", ESP_TIMER_FREQUENCY_DEFAULT, {
                {"High 1 CH2", ESPTimerChannelNum::HIGH1_CH2},
                {"High 1 CH3", ESPTimerChannelNum::HIGH1_CH3}
            }, PwmDriver::MCPWM
        },
        { ESP_H_TIMER2_FREQUENCY, "High 2", ESP_TIMER_FREQUENCY_DEFAULT, {
                {"High 2 CH4", ESPTimerChannelNum::HIGH2_CH4},
                {"High 2 CH5", ESPTimerChannelNum::HIGH2_CH5}
            }, PwmDriver::MCPWM
        },
        { ESP_H_TIMER3_FREQUENCY, "High 3", ESP_TIMER_FREQUENCY_DEFAULT, {
                {"High 3 CH6", ESPTimerChannelNum::HIGH3_CH6},
                {"High 3 CH7", ESPTimerChannelNum::HIGH3_CH7}
            }, PwmDriver::MCPWM
        },
        // LOW timers default to LEDC (vibe/misc)
#endif
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S3
        { ESP_L_TIMER0_FREQUENCY, "Low 0", ESP_TIMER_FREQUENCY_DEFAULT, {
                {"Low 0 CH0", ESPTimerChannelNum::LOW0_CH0},
                {"Low 0 CH1", ESPTimerChannelNum::LOW0_CH1}
            }, PwmDriver::LEDC
        },
        { ESP_L_TIMER1_FREQUENCY, "Low 1", ESP_TIMER_FREQUENCY_DEFAULT, {
                {"Low 1 CH2", ESPTimerChannelNum::LOW1_CH2},
                {"Low 1 CH3", ESPTimerChannelNum::LOW1_CH3}
            }, PwmDriver::LEDC
        },
        { ESP_L_TIMER2_FREQUENCY, "Low 2", ESP_TIMER_FREQUENCY_DEFAULT, {
                {"Low 2 CH4", ESPTimerChannelNum::LOW2_CH4},
                {"Low 2 CH5", ESPTimerChannelNum::LOW2_CH5}
            }, PwmDriver::LEDC
        },
        { ESP_L_TIMER3_FREQUENCY, "Low 3", ESP_TIMER_FREQUENCY_DEFAULT, {
                {"Low 3 CH6", ESPTimerChannelNum::LOW3_CH6},
                {"Low 3 CH7", ESPTimerChannelNum::LOW3_CH7}
            }, PwmDriver::LEDC
        }
#if CONFIG_IDF_TARGET_ESP32S3
        ,
        // ESP32-S3: HIGH timers default to MCPWM. Listed AFTER the LOW timers
        // so the channel numbers used by LEDC stay in the 0..7 range supported
        // by S3 LEDC hardware. MCPWM ignores the raw channel value, so HIGH
        // channels 8..15 are simply unique identifiers.
        { ESP_H_TIMER0_FREQUENCY, "High 0", ESP_TIMER_FREQUENCY_DEFAULT, {
                {"High 0 CH0", ESPTimerChannelNum::HIGH0_CH0},
                {"High 0 CH1", ESPTimerChannelNum::HIGH0_CH1}
            }, PwmDriver::MCPWM
        },
        { ESP_H_TIMER1_FREQUENCY, "High 1", ESP_TIMER_FREQUENCY_DEFAULT, {
                {"High 1 CH2", ESPTimerChannelNum::HIGH1_CH2},
                {"High 1 CH3", ESPTimerChannelNum::HIGH1_CH3}
            }, PwmDriver::MCPWM
        },
        { ESP_H_TIMER2_FREQUENCY, "High 2", ESP_TIMER_FREQUENCY_DEFAULT, {
                {"High 2 CH4", ESPTimerChannelNum::HIGH2_CH4},
                {"High 2 CH5", ESPTimerChannelNum::HIGH2_CH5}
            }, PwmDriver::MCPWM
        },
        { ESP_H_TIMER3_FREQUENCY, "High 3", ESP_TIMER_FREQUENCY_DEFAULT, {
                {"High 3 CH6", ESPTimerChannelNum::HIGH3_CH6},
                {"High 3 CH7", ESPTimerChannelNum::HIGH3_CH7}
            }, PwmDriver::MCPWM
        }
#endif
    };
#endif
#if CONFIG_IDF_TARGET_ESP32C5 || CONFIG_IDF_TARGET_ESP32C6
        { ESP_L_TIMER0_FREQUENCY, "Low 0", ESP_TIMER_FREQUENCY_DEFAULT, {
                {"Low 0 CH0", ESPTimerChannelNum::LOW0_CH0},
                {"Low 0 CH1", ESPTimerChannelNum::LOW0_CH1}
            }
        },
        { ESP_L_TIMER1_FREQUENCY, "Low 1", ESP_TIMER_FREQUENCY_DEFAULT, {
                {"Low 1 CH2", ESPTimerChannelNum::LOW1_CH2},
                {"Low 1 CH3", ESPTimerChannelNum::LOW1_CH3}
            }
        },
        { ESP_L_TIMER2_FREQUENCY, "Low 2", ESP_TIMER_FREQUENCY_DEFAULT, {
                {"Low 2 CH4", ESPTimerChannelNum::LOW2_CH4}
            }
        },
        { ESP_L_TIMER3_FREQUENCY, "Low 3", ESP_TIMER_FREQUENCY_DEFAULT, {
                {"Low 3 CH5", ESPTimerChannelNum::LOW2_CH5},
            }
        }
    };
#endif

    // void setCommonTimers() {
    //     m_timers.timerH3 = {
    //         ESP_TIMER_FREQUENCY_DEFAULT,
    //         {
    //             {SQUEEZE_PIN, SqueezeServo_PWM, squeeze()},
    //             {TWIST_SERVO_PIN, TwistServo_PWM, twist()}
    //         }
    //     };

    // }
private:
    // PWM
    int8_t m_valve = VALVE_SERVO_PIN_DEFAULT;
    int8_t m_valveChannel = VALVE_SERVO_CHANNEL_DEFAULT;
    int8_t m_twist = TWIST_SERVO_PIN_DEFAULT;
    int8_t m_twistChannel = TWIST_SERVO_CHANNEL_DEFAULT;
    int8_t m_squeeze = SQUEEZE_PIN_DEFAULT;
    int8_t m_squeezeChannel = SQUEEZE_CHANNEL_DEFAULT;
    int8_t m_vibe0 = VIBE0_PIN_DEFAULT;
    int8_t m_vibe0Channel = VIBE0_CHANNEL_DEFAULT;
    int8_t m_vibe1 = VIBE1_PIN_DEFAULT;
    int8_t m_vibe1Channel = VIBE1_CHANNEL_DEFAULT;
    int8_t m_vibe2 = VIBE2_PIN_DEFAULT;
    int8_t m_vibe2Channel = VIBE2_CHANNEL_DEFAULT;
    int8_t m_vibe3 = VIBE3_PIN_DEFAULT;
    int8_t m_vibe3Channel = VIBE3_CHANNEL_DEFAULT;
    int8_t m_caseFan = CASE_FAN_PIN_DEFAULT;
    int8_t m_caseFanChannel = CASE_FAN_CHANNEL_DEFAULT;
    int8_t m_heater = HEATER_PIN_DEFAULT;
    int8_t m_heaterChannel = HEATER_CHANNEL_DEFAULT;
    // Analog
    int8_t m_twistFeedBack = TWIST_FEEDBACK_PIN_DEFAULT;
    int8_t m_lubeButton = LUBE_BUTTON_PIN_DEFAULT;
    int8_t m_internalTemp = INTERNAL_TEMP_PIN_DEFAULT;
    int8_t m_displayReset = DISPLAY_RST_PIN_DEFAULT;
    int8_t m_sleeveTemp = TEMP_PIN_DEFAULT;
    int8_t m_i2cSda = I2C_SDA_PIN_DEFAULT;
    int8_t m_i2cScl = I2C_SCL_PIN_DEFAULT;
    int8_t m_voltageMonitors[static_cast<int8_t>(VoltageMonitors::MAX)] = { -1, -1, -1, -1, -1 };
    int8_t m_servoVoltageEnable = SERVO_VOLTAGE_ENABLE_PIN_DEFAULT;
    int8_t m_buttonSetPins[MAX_BUTTON_SETS] = BUTTON_SET_PINS_DEFAULT;
    int8_t m_brushedMotorA = -1;
    int8_t m_brushedMotorB = -1;

    virtual void overideDefaults() =0;

    const ESPTimer* getTimer(ESPTimerChannelNum channel) const {
        if(channel == ESPTimerChannelNum::NONE) {
            return 0;
        }
        int8_t channelInt = static_cast<int8_t>(channel);
        int8_t timerIndex = channelInt >> 1;
        return &m_timers[timerIndex];
    }
};

class PinMapSSR : public PinMap {
public:
    static PinMapSSR* getInstance()
    {
        static PinMapSSR instance(DeviceType::SSR1, BoardType::DEVKIT);
        return &instance;
    }
    int8_t encoder() const { return m_encoder; }
    void setEncoder(const int8_t &encoder) { m_encoder = encoder; }

    int8_t chipSelect() const { return m_chipSelect; }
    void setChipSelect(const int8_t &chipSelect) { m_chipSelect = chipSelect; }

    int8_t enable() const { return m_enable; }
    void setEnable(const int8_t &enable) { m_enable = enable; }

    int8_t hallEffect() const { return m_hallEffect; }
    void setHallEffect(const int8_t &hallEffect) { m_hallEffect = hallEffect; }

    int8_t pwmChannel1() const { return m_pwmChannel1; }
    void setPwmChannel1(const int8_t &pwmChannel1) { m_pwmChannel1 = pwmChannel1; }

    int8_t pwmChannel2() const { return m_pwmChannel2; }
    void setPwmChannel2(const int8_t &pwmChannel2) { m_pwmChannel2 = pwmChannel2; }

    int8_t pwmChannel3() const { return m_pwmChannel3; }
    void setPwmChannel3(const int8_t &pwmChannel3) { m_pwmChannel3 = pwmChannel3; }

    int8_t motorBEncoder() const { return m_twistEncoder; }
    void setLeftEncoder(const int8_t &encoder) { m_twistEncoder = encoder; }

    int8_t motorBChipSelect() const { return m_twistChipSelect; }
    void setLeftChipSelect(const int8_t &chipSelect) { m_twistChipSelect = chipSelect; }

    int8_t motorBEnable() const { return m_twistEnable; }
    void setLeftEnable(const int8_t &enable) { m_twistEnable = enable; }

    int8_t motorBPwmChannel1() const { return m_twistPwmChannel1; }
    void setLeftPwmChannel1(const int8_t &pwmChannel1) { m_twistPwmChannel1 = pwmChannel1; }

    int8_t motorBPwmChannel2() const { return m_twistPwmChannel2; }
    void setLeftPwmChannel2(const int8_t &pwmChannel2) { m_twistPwmChannel2 = pwmChannel2; }

    int8_t motorBPwmChannel3() const { return m_twistPwmChannel3; }
    void setLeftPwmChannel3(const int8_t &pwmChannel3) { m_twistPwmChannel3 = pwmChannel3; }

    void setDeviceType(DeviceType type) override
    {
        if (type == DeviceType::NONE)
        {
            LogHandler::info(m_TAG, "[PinMapSSR.setDevice Setting device NONE");
        }
        else if(type == DeviceType::SSR1)
        {
            m_deviceType = type;
            LogHandler::debug(m_TAG, "[PinMapSSR.setDevice] SSR1, Device type %i", (int)m_deviceType);
            setEncoder(BLDC_ENCODER_PIN_DEFAULT);
            setChipSelect(BLDC_CHIPSELECT_PIN_DEFAULT);
            setEnable(BLDC_ENABLE_PIN_DEFAULT);
            setPwmChannel1(BLDC_PWMCHANNEL1_PIN_DEFAULT);
            setPwmChannel2(BLDC_PWMCHANNEL2_PIN_DEFAULT);
            setPwmChannel3(BLDC_PWMCHANNEL3_PIN_DEFAULT);
        }
        else if(type == DeviceType::SSR2)
        {
            m_deviceType = type;
            LogHandler::debug(m_TAG, "[PinMapSSR.setDevice] SSR2, Device type %i", (int)m_deviceType);
            setEncoder(-1);
            setChipSelect(5);
            setEnable(26);
            setPwmChannel1(25);
            setPwmChannel2(33);
            setPwmChannel3(32);

            setValve(-1);
            setTwist(-1);
            setSqueeze(-1);
            setVibe0(-1);
            setVibe1(-1);
            setVibe2(-1);
            setVibe3(-1);
            setSleeveTemp(-1);
            setInternalTemp(-1);
            setCaseFan(-1);
            setHeater(-1);
            setTwistFeedBack(-1);
        }
        else
        {
            LogHandler::error(m_TAG, "[PinMapSSR.setDevice Invalid device type %i", (int)type);
        }
    }
protected:
    PinMapSSR(DeviceType deviceType, BoardType boardType) : PinMap(deviceType, boardType) {}
private:
    int8_t m_encoder = BLDC_ENCODER_PIN_DEFAULT;
    int8_t m_chipSelect = BLDC_CHIPSELECT_PIN_DEFAULT;
    int8_t m_enable = BLDC_ENABLE_PIN_DEFAULT;
    int8_t m_hallEffect = BLDC_HALLEFFECT_PIN_DEFAULT;
    int8_t m_pwmChannel1 = BLDC_PWMCHANNEL1_PIN_DEFAULT;
    int8_t m_pwmChannel2 = BLDC_PWMCHANNEL2_PIN_DEFAULT;
    int8_t m_pwmChannel3 = BLDC_PWMCHANNEL3_PIN_DEFAULT;

    int8_t m_twistEncoder = BLDC_B_ENCODER_PIN_DEFAULT;
    int8_t m_twistChipSelect = BLDC_B_CHIPSELECT_PIN_DEFAULT;
    int8_t m_twistEnable = BLDC_B_ENABLE_PIN_DEFAULT;
    int8_t m_twistPwmChannel1 = BLDC_B_PWMCHANNEL1_PIN_DEFAULT;
    int8_t m_twistPwmChannel2 = BLDC_B_PWMCHANNEL2_PIN_DEFAULT;
    int8_t m_twistPwmChannel3 = BLDC_B_PWMCHANNEL3_PIN_DEFAULT;
    void overideDefaults() override {}

};

class PinMapOSR : public PinMap {
public:
    static PinMapOSR* getInstance()
    {
        static PinMapOSR instance(DeviceType::OSR, BoardType::DEVKIT);
        return &instance;
    }
    int8_t rightServo() const { return m_rightServo; }
    void setRightServo(const int8_t &rightServo) { m_rightServo = rightServo; }
    int8_t rightServoChannel() const { return m_rightServoChannel; }
    void setRightServoChannel(const int8_t &rightServoChannel) { m_rightServoChannel = rightServoChannel; }

    int8_t leftServo() const { return m_leftServo; }
    void setLeftServo(const int8_t &leftServo) { m_leftServo = leftServo; }
    int8_t leftServoChannel() const { return m_leftServoChannel; }
    void setLeftServoChannel(const int8_t &leftServoChannel) { m_leftServoChannel = leftServoChannel; }

    int8_t pitchLeft() const { return m_pitchLeft; }
    void setPitchLeft(const int8_t &pitchLeft) { m_pitchLeft = pitchLeft; }
    int8_t pitchLeftChannel() const { return m_pitchLeftChannel; }
    void setPitchLeftChannel(const int8_t &pitchLeftChannel) { m_pitchLeftChannel = pitchLeftChannel; }

protected:
    PinMapOSR(DeviceType deviceType, BoardType boardType) : PinMap(deviceType, boardType) {}
private:
    int8_t m_pitchLeft = PITCH_LEFT_SERVO_PIN_DEFAULT;
    int8_t m_pitchLeftChannel = PITCH_LEFT_SERVO_CHANNEL_DEFAULT;
    int8_t m_rightServo = RIGHT_SERVO_PIN_DEFAULT;
    int8_t m_rightServoChannel = RIGHT_SERVO_CHANNEL_DEFAULT;
    int8_t m_leftServo = LEFT_SERVO_PIN_DEFAULT;
    int8_t m_leftServoChannel = LEFT_SERVO_CHANNEL_DEFAULT;
    void overideDefaults() override {
        // setTwistChannel((int8_t)ESPTimerChannelNum::HIGH2_CH4);
        // setSqueezeChannel((int8_t)ESPTimerChannelNum::HIGH2_CH5);
        // setValveChannel((int8_t)ESPTimerChannelNum::HIGH3_CH6);
        // setVibe0Channel((int8_t)ESPTimerChannelNum::LOW0_CH0);
        // setVibe1Channel((int8_t)ESPTimerChannelNum::LOW0_CH1);
        // setVibe2Channel((int8_t)ESPTimerChannelNum::LOW1_CH2);
        // setVibe3Channel((int8_t)ESPTimerChannelNum::LOW1_CH3);
        // setHeaterChannel((int8_t)ESPTimerChannelNum::LOW2_CH4);
        // setCaseFanChannel((int8_t)ESPTimerChannelNum::LOW3_CH6);
    }
};

    // #define LowerLeftServo_PWM 0     // Lower Left Servo
    // #define UpperLeftServo_PWM 1     // Upper Left Servo

    // #define LowerRightServo_PWM 2    // Lower Right Servo
    // #define UpperRightServo_PWM 3    // Upper Right Servo

    // #define LeftPitchServo_PWM 4     // Left Pitch Servo
    // #define RightPitchServo_PWM 5    // Right Pitch Servo

    // #define TwistServo_PWM 6         // Twist Servo
    // #define SqueezeServo_PWM 7

    // #define Vibe0_PWM 8              // Vibration motor 1
    // #define Vibe1_PWM 9            // Vibration motor 2

    // #define Vibe2_PWM 10
    // #define Vibe3_PWM 11

    // #define Heater_PWM 12             // Heating pad
    // #define CaseFan_PWM 13

    // #define ValveServo_PWM 14         // Valve Servo

class PinMapSR6 : public PinMapOSR {
public:
    static PinMapSR6* getInstance()
    {
        static PinMapSR6 instance(DeviceType::SR6, BoardType::DEVKIT);
        return &instance;
    }
    int8_t pitchRight() const { return m_pitchRight; }
    void setPitchRight(const int8_t &pitchRight) { m_pitchRight = pitchRight; }
    int8_t pitchRightChannel() const { return m_pitchRightChannel; }
    void setPitchRightChannel(const int8_t &channel) { m_pitchRightChannel = channel; }

    int8_t rightUpperServo() const { return m_rightUpperServo; }
    void setRightUpperServo(const int8_t &rightUpperServo) { m_rightUpperServo = rightUpperServo; }
    int8_t rightUpperServoChannel() const { return m_rightUpperServoChannel; }
    void setRightUpperServoChannel(const int8_t &channel) { m_rightUpperServoChannel = channel; }

    int8_t leftUpperServo() const { return m_leftUpperServo; }
    void setLeftUpperServo(const int8_t &leftUpperServo) { m_leftUpperServo = leftUpperServo; }
    int8_t leftUpperServoChannel() const { return m_leftUpperServoChannel; }
    void setLeftUpperServoChannel(const int8_t &channel) { m_leftUpperServoChannel = channel; }

protected:
    PinMapSR6(DeviceType deviceType, BoardType boardType) : PinMapOSR(deviceType, boardType) {}
private:
    int8_t m_pitchRight = PITCH_RIGHTSERVO_PIN_DEFAULT;
    int8_t m_pitchRightChannel = PITCH_RIGHTSERVO_CHANNEL_DEFAULT;
    int8_t m_rightUpperServo = RIGHT_UPPER_SERVO_PIN_DEFAULT;
    int8_t m_rightUpperServoChannel = RIGHT_UPPER_SERVO_CHANNEL_DEFAULT;
    int8_t m_leftUpperServo = LEFT_UPPER_SERVO_PIN_DEFAULT;
    int8_t m_leftUpperServoChannel = LEFT_UPPER_SERVO_CHANNEL_DEFAULT;
    void overideDefaults() override {}
};

class PinMapINControl : public PinMapSR6 {
public:
    static PinMapINControl* getInstance()
    {
        static PinMapINControl instance(DeviceType::SR6, BoardType::ISAAC);
        return &instance;
    }
    void overideDefaults() override {
        setTwistFeedBack(32);
        setRightServo(4);
        setLeftServo(13);
        setRightUpperServo(16);
        setLeftUpperServo(27);
        setPitchLeft(26);
        setPitchRight(17);
        setValve(18);
        setTwist(25);
        // Common motor
        setSqueeze(19);
        setLubeButton(34);
        setInternalTemp(35);
        setSleeveTemp(33);
        setCaseFan(16);
        // // Case_Fan_PIN = json["Case_Fan_PIN"] | 16;
        setVibe0(15);
        setVibe1(2);
        setVibe2(23);
        // // Vibe2_PIN = json["Vibe2_PIN"] | 23;
        setVibe3(32);
        // // Vibe3_PIN = json["Vibe3_PIN"] | 32;
        setHeater(5);
    }
protected:
    PinMapINControl(DeviceType deviceType, BoardType boardType) : PinMapSR6(deviceType, boardType) {}
};

class PinMapSR6MB : public PinMapSR6 {
public:
    static PinMapSR6MB* getInstance()
    {
        static PinMapSR6MB instance(DeviceType::SR6, BoardType::CRIMZZON);
        return &instance;
    }
    void overideDefaults() override {
        setVibe3(26);
        setInternalTemp(32);
        // Feed back was on 0 but this firmware has the potental to use that as an action button so disable feedback by default.
        setTwistFeedBack(-1);


        // // EXT
        // //  EXT_Input2_PIN = 34;
        // //  EXT_Input3_PIN = 39;
        // //  EXT_Input4_PIN = 36;

        // TODO: Add per-board defaults for heater/fan/display settings if needed.
        // heaterResolution = json["heaterResolution"] | 8;
        // caseFanResolution = json["caseFanResolution"] | 10;
        // caseFanFrequency = json["caseFanFrequency"] | 25;
        // Display_Screen_Height = json["Display_Screen_Height"] | 32;
    }
protected:
    PinMapSR6MB(DeviceType deviceType, BoardType boardType) : PinMapSR6(deviceType, boardType) {}
};


class PinMapSSR1PCB : public PinMapSSR {
    public:
        static PinMapSSR1PCB* getInstance()
        {
            static PinMapSSR1PCB instance(DeviceType::SSR1, BoardType::SSR1PCB);
            return &instance;
        }
        void overideDefaults() override {
            setEnable(4);
            setPwmChannel1(2);
            setPwmChannel2(16);
            setPwmChannel3(17);
            setHallEffect(14);

            setI2cSda(22);
            setI2cScl(21);

            setValve(-1);
            setTwist(-1);
            setSqueeze(-1);
            setVibe0(-1);
            setVibe1(-1);
            setVibe2(-1);
            setVibe3(-1);
            setSleeveTemp(-1);
            setInternalTemp(-1);
            setCaseFan(-1);
            setHeater(-1);
            setTwistFeedBack(-1);
        }
    protected:
        PinMapSSR1PCB(DeviceType deviceType, BoardType boardType) : PinMapSSR(deviceType, boardType) {}
};

class PinMapSR6PCB : public PinMapSR6 {
    public:
        static PinMapSR6PCB* getInstance()
        {
            static PinMapSR6PCB instance(DeviceType::SR6, BoardType::SR6PCB);
            return &instance;
        }
        void overideDefaults() override {
            setI2cSda(2);
            setI2cScl(1);

            setLeftServo(38);
            setRightServo(35);
            setLeftUpperServo(37);
            setRightUpperServo(48);
            setPitchLeft(36);
            setPitchRight(47);
            setValve(21);
            setTwist(14);
            setSqueeze(-1);
            setVibe0(-1);
            setVibe1(8);
            setVibe2(-1);
            setVibe3(18);
            setLubeButton(13);
            setSleeveTemp(-1);
            setInternalTemp(-1);
            setCaseFan(-1);
            setHeater(-1);
            setTwistFeedBack(-1);

            setVoltageMonitor(VoltageMonitors::VOLTAGE_3V3, 7);
            setVoltageMonitor(VoltageMonitors::VOLTAGE_5V, 6);
            setVoltageMonitor(VoltageMonitors::VOLTAGE_BUS, 5);
            setVoltageMonitor(VoltageMonitors::VOLTAGE_MOTOR, 4);
            setVoltageMonitor(VoltageMonitors::VOLTAGE_BATTERY, -1);

            setServoVoltageEnable(15);

        }
    protected:
        PinMapSR6PCB(DeviceType deviceType, BoardType boardType) : PinMapSR6(deviceType, boardType) {}
};