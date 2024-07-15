#pragma once

#include <stdint.h>

// Common
#define TWIST_FEEDBACK_PIN_DEFAULT 26
#define VALVE_SERVO_PIN_DEFAULT 25
#define TWIST_SERVO_PIN_DEFAULT 27
#define SQUEEZE_PIN_DEFAULT 17
#define VIBE0_PIN_DEFAULT 18
#define VIBE1_PIN_DEFAULT 19
#define VIBE2_PIN_DEFAULT 23
#define VIBE3_PIN_DEFAULT 32
#define CASE_FAN_PIN_DEFAULT 16
#define LUBE_BUTTON_PIN_DEFAULT 35
#define INTERNAL_TEMP_PIN_DEFAULT 34
#define DISPLAY_RST_PIN_DEFAULT -1
#define TEMP_PIN_DEFAULT 5
#define HEATER_PIN_DEFAULT 33
#define BUTTON_SET_PIN_DEFAULT 39
#define I2C_SDA_PIN_DEFAULT 21
#define I2C_SCL_PIN_DEFAULT 22

// OSR
#define PITCH_LEFT_SERVO_PIN_DEFAULT 4
#define RIGHT_SERVO_PIN_DEFAULT 13
#define LEFT_SERVO_PIN_DEFAULT 15

// SR6
#define PITCH_RIGHTSERVO_PIN_DEFAULT 14
#define RIGHT_UPPER_SERVO_PIN_DEFAULT 12
#define LEFT_UPPER_SERVO_PIN_DEFAULT 2

// BLDC (SSR1)
#define BLDC_ENCODER_PIN_DEFAULT 33
#define BLDC_CHIPSELECT_PIN_DEFAULT 5
#define BLDC_ENABLE_PIN_DEFAULT 14
#define BLDC_HALLEFFECT_PIN_DEFAULT 35
#define BLDC_PWMCHANNEL1_PIN_DEFAULT 27
#define BLDC_PWMCHANNEL2_PIN_DEFAULT 26
#define BLDC_PWMCHANNEL3_PIN_DEFAULT 25

class PinMap {
public:
    uint8_t twistFeedBack() const { return m_twistFeedBack; }
    void setTwistFeedBack(const uint8_t &twistFeedBack) { m_twistFeedBack = twistFeedBack; }

    uint8_t valve() const { return m_valve; }
    void setValve(const uint8_t &valve) { m_valve = valve; }

    uint8_t twist() const { return m_twist; }
    void setTwist(const uint8_t &twist) { m_twist = twist; }

    uint8_t squeeze() const { return m_squeeze; }
    void setSqueeze(const uint8_t &squeeze) { m_squeeze = squeeze; }

    uint8_t vibe0() const { return m_vibe0; }
    void setVibe0(const uint8_t &vibe0) { m_vibe0 = vibe0; }

    uint8_t vibe1() const { return m_vibe1; }
    void setVibe1(const uint8_t &vibe1) { m_vibe1 = vibe1; }

    uint8_t vibe2() const { return m_vibe2; }
    void setVibe2(const uint8_t &vibe2) { m_vibe2 = vibe2; }

    uint8_t vibe3() const { return m_vibe3; }
    void setVibe3(const uint8_t &vibe3) { m_vibe3 = vibe3; }

    uint8_t caseFan() const { return m_caseFan; }
    void setCaseFan(const uint8_t &caseFan) { m_caseFan = caseFan; }

    uint8_t lubeButton() const { return m_lubeButton; }
    void setLubeButton(const uint8_t &lubeButton) { m_lubeButton = lubeButton; }

    uint8_t internalTemp() const { return m_internalTemp; }
    void setInternalTemp(const uint8_t &internalTemp) { m_internalTemp = internalTemp; }

    uint8_t displayReset() const { return m_displayReset; }
    void setDisplayReset(const uint8_t &displayReset) { m_displayReset = displayReset; }

    uint8_t sleeveTemp() const { return m_sleeveTemp; }
    void setSleeveTemp(const uint8_t &sleeveTemp) { m_sleeveTemp = sleeveTemp; }

    uint8_t heater() const { return m_heater; }
    void setHeater(const uint8_t &heater) { m_heater = heater; }

    uint8_t buttonSet() const { return m_buttonSet; }
    void setButtonSet(const uint8_t &buttonSet) { m_buttonSet = buttonSet; }

    uint8_t i2cSda() const { return m_i2cSda; }
    void setI2cSda(const uint8_t &i2cSda) { m_i2cSda = i2cSda; }

    uint8_t i2cScl() const { return m_i2cScl; }
    void setI2cScl(const uint8_t &i2cScl) { m_i2cScl = i2cScl; }

private:
    uint8_t m_twistFeedBack = TWIST_FEEDBACK_PIN_DEFAULT;
    uint8_t m_valve = VALVE_SERVO_PIN_DEFAULT;
    uint8_t m_twist = TWIST_SERVO_PIN_DEFAULT;
    uint8_t m_squeeze = SQUEEZE_PIN_DEFAULT;
    uint8_t m_vibe0 = VIBE0_PIN_DEFAULT;
    uint8_t m_vibe1 = VIBE1_PIN_DEFAULT;
    uint8_t m_vibe2 = VIBE2_PIN_DEFAULT;
    uint8_t m_vibe3 = VIBE3_PIN_DEFAULT;
    uint8_t m_caseFan = CASE_FAN_PIN_DEFAULT;
    uint8_t m_lubeButton = LUBE_BUTTON_PIN_DEFAULT;
    uint8_t m_internalTemp = INTERNAL_TEMP_PIN_DEFAULT;
    uint8_t m_displayReset = DISPLAY_RST_PIN_DEFAULT;
    uint8_t m_sleeveTemp = TEMP_PIN_DEFAULT;
    uint8_t m_heater = HEATER_PIN_DEFAULT;
    uint8_t m_buttonSet = BUTTON_SET_PIN_DEFAULT;
    uint8_t m_i2cSda = I2C_SDA_PIN_DEFAULT;
    uint8_t m_i2cScl = I2C_SCL_PIN_DEFAULT;
};

class PinMapSSR1 : public PinMap {
public:
    uint8_t encoder() const { return m_encoder; }
    void setEncoder(const uint8_t &encoder) { m_encoder = encoder; }

    uint8_t chipSelect() const { return m_chipSelect; }
    void setChipSelect(const uint8_t &chipSelect) { m_chipSelect = chipSelect; }

    uint8_t enable() const { return m_enable; }
    void setEnable(const uint8_t &enable) { m_enable = enable; }

    uint8_t hallEffect() const { return m_hallEffect; }
    void setHallEffect(const uint8_t &hallEffect) { m_hallEffect = hallEffect; }

    uint8_t pwmChannel1() const { return m_pwmChannel1; }
    void setPwmChannel1(const uint8_t &pwmChannel1) { m_pwmChannel1 = pwmChannel1; }

    uint8_t pwmChannel2() const { return m_pwmChannel2; }
    void setPwmChannel2(const uint8_t &pwmChannel2) { m_pwmChannel2 = pwmChannel2; }

    uint8_t pwmChannel3() const { return m_pwmChannel3; }
    void setPwmChannel3(const uint8_t &pwmChannel3) { m_pwmChannel3 = pwmChannel3; }

private:
    uint8_t m_encoder = BLDC_ENCODER_PIN_DEFAULT;
    uint8_t m_chipSelect = BLDC_CHIPSELECT_PIN_DEFAULT;
    uint8_t m_enable = BLDC_ENABLE_PIN_DEFAULT;
    uint8_t m_hallEffect = BLDC_HALLEFFECT_PIN_DEFAULT;
    uint8_t m_pwmChannel1 = BLDC_PWMCHANNEL1_PIN_DEFAULT;
    uint8_t m_pwmChannel2 = BLDC_PWMCHANNEL2_PIN_DEFAULT;
    uint8_t m_pwmChannel3 = BLDC_PWMCHANNEL3_PIN_DEFAULT;

};

class PinMapOSR : public PinMap {
public:
    uint8_t rightServo() const { return m_rightServo; }
    void setRightServo(const uint8_t &rightServo) { m_rightServo = rightServo; }

    uint8_t leftServo() const { return m_leftServo; }
    void setLeftServo(const uint8_t &leftServo) { m_leftServo = leftServo; }

    uint8_t pitchLeft() const { return m_pitchLeft; }
    void setPitchLeft(const uint8_t &pitchLeft) { m_pitchLeft = pitchLeft; }

private:
    uint8_t m_pitchLeft = PITCH_LEFT_SERVO_PIN_DEFAULT;
    uint8_t m_rightServo = RIGHT_SERVO_PIN_DEFAULT;
    uint8_t m_leftServo = LEFT_SERVO_PIN_DEFAULT;
};

class PinMapSR6 : public PinMapOSR {
public:
    uint8_t pitchRight() const { return m_pitchRight; }
    void setPitchRight(const uint8_t &pitchRight) { m_pitchRight = pitchRight; }

    uint8_t rightUpperServo() const { return m_rightUpperServo; }
    void setRightUpperServo(const uint8_t &rightUpperServo) { m_rightUpperServo = rightUpperServo; }

    uint8_t leftUpperServo() const { return m_leftUpperServo; }
    void setLeftUpperServo(const uint8_t &leftUpperServo) { m_leftUpperServo = leftUpperServo; }

private:
    uint8_t m_pitchRight = PITCH_RIGHTSERVO_PIN_DEFAULT;
    uint8_t m_rightUpperServo = RIGHT_UPPER_SERVO_PIN_DEFAULT;
    uint8_t m_leftUpperServo = LEFT_UPPER_SERVO_PIN_DEFAULT;
};


class PinMapINControl : PinMapSR6 {
    PinMapINControl() {
        setTwistFeedBack(32);// TODO finish map overrides
        // TwistFeedBack_PIN = json["TwistFeedBack_PIN"] | 32;
        // RightServo_PIN = json["RightServo_PIN"] | 4;
        // LeftServo_PIN = json["LeftServo_PIN"] | 13;
        // RightUpperServo_PIN = json["RightUpperServo_PIN"] | 16;
        // LeftUpperServo_PIN = json["LeftUpperServo_PIN"] | 27;
        // PitchLeftServo_PIN = json["PitchLeftServo_PIN"] | 26;
        // PitchRightServo_PIN = json["PitchRightServo_PIN"] | 17;
        // ValveServo_PIN = json["ValveServo_PIN"] | 18;
        // TwistServo_PIN = json["TwistServo_PIN"] | 25;
        // // Common motor
        // Squeeze_PIN = json["Squeeze_PIN"] | 19;
        // LubeButton_PIN = json["LubeButton_PIN"] | 34;
        // // Internal_Temp_PIN = json["Internal_Temp_PIN"] | 34;
        // Sleeve_Temp_PIN = json["Temp_PIN"] | 33;
        // // Case_Fan_PIN = json["Case_Fan_PIN"] | 16;
        // Vibe0_PIN = json["Vibe0_PIN"] | 15;
        // Vibe1_PIN = json["Vibe1_PIN"] | 2;
        // // Vibe2_PIN = json["Vibe2_PIN"] | 23;
        // // Vibe3_PIN = json["Vibe3_PIN"] | 32;
        // Heater_PIN = json["Heater_PIN"] | 5;
    }
};

class PinMapSR6MB : PinMapSR6 {
    PinMapSR6MB() {
        // Vibe3_PIN = json["Vibe3_PIN"] | 26;
        // Internal_Temp_PIN = json["Internal_Temp_PIN"] | 32;

        // // EXT
        // //  EXT_Input2_PIN = 34;
        // //  EXT_Input3_PIN = 39;
        // //  EXT_Input4_PIN = 36;

        // heaterResolution = json["heaterResolution"] | 8;
        // caseFanResolution = json["caseFanResolution"] | 10;
        // caseFanFrequency = json["caseFanFrequency"] | 25;
        // Display_Screen_Height = json["Display_Screen_Height"] | 32;
        // TwistFeedBack_PIN = json["TwistFeedBack_PIN"] | 0;

    }
};