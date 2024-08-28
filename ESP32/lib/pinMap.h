#pragma once

#include <stdint.h>

#ifdef WROOM32_MODULE
#include "pinDefaultsWROOM32.h"
#elif defined(S3_MODULE)
#include "pinDefaultsS3.h"
#endif

// Common
#define TWIST_FEEDBACK_PIN "TwistFeedBack_PIN"
#define VALVE_SERVO_PIN "ValveServo_PIN"
#define TWIST_SERVO_PIN "TwistServo_PIN"
#define SQUEEZE_PIN "Squeeze_PIN"
#define VIBE0_PIN "Vibe0_PIN"
#define VIBE1_PIN "Vibe1_PIN"
#define VIBE2_PIN "Vibe2_PIN"
#define VIBE3_PIN "Vibe3_PIN"
#define CASE_FAN_PIN "Case_Fan_PIN"
#define LUBE_BUTTON_PIN "LubeButton_PIN"
#define INTERNAL_TEMP_PIN "Internal_Temp_PIN"
#define I2C_SDA_PIN "i2cSdaPin"
#define I2C_SCL_PIN "i2cSclPin"
#define DISPLAY_RST_PIN "Display_Rst_PIN"
#define TEMP_PIN "Temp_PIN"
#define HEATER_PIN "Heater_PIN"
#define BUTTON_SET_PINS "Button_Set_PIN"

// OSR
#define RIGHT_SERVO_PIN "RightServo_PIN"
#define LEFT_SERVO_PIN "LeftServo_PIN"
#define PITCH_LEFT_SERVO_PIN "PitchLeftServo_PIN"

// SR6
#define PITCH_RIGHTSERVO_PIN "PitchRightServo_PIN"
#define RIGHT_UPPER_SERVO_PIN "RightUpperServo_PIN"
#define LEFT_UPPER_SERVO_PIN "LeftUpperServo_PIN"

// BLDC (SSR1)
#define BLDC_ENCODER_PIN "BLDC_Encoder_PIN"
#define BLDC_CHIPSELECT_PIN "BLDC_ChipSelect_PIN"
#define BLDC_ENABLE_PIN "BLDC_Enable_PIN"
#define BLDC_HALLEFFECT_PIN "BLDC_HallEffect_PIN"
#define BLDC_PWMCHANNEL1_PIN "BLDC_PWMchannel1_PIN"
#define BLDC_PWMCHANNEL2_PIN "BLDC_PWMchannel2_PIN"
#define BLDC_PWMCHANNEL3_PIN "BLDC_PWMchannel3_PIN"

class PinMap;

class PinMapInfo {
public:
    PinMapInfo(DeviceType deviceType, BoardType boardType, PinMap* pinMap): 
        m_deviceType(deviceType), 
        m_boardType(boardType), 
        m_pinMap(pinMap) { }

    DeviceType deviceType() {
        return m_deviceType;
    }

    BoardType boardType() {
        return m_boardType;
    }

    template<typename T, typename = std::enable_if<std::is_base_of<PinMap, T>::value>>
    const T pinMap() {
        return static_cast<const T>(m_pinMap);
    }  
    const PinMap* pinMap() {
        return m_pinMap;
    }
private:
    DeviceType m_deviceType;
    BoardType m_boardType;
    PinMap* m_pinMap;
};

// class PinMapFactory;
class PinMap {
public:
    PinMap(PinMap const&) = delete;
    void operator=(PinMap const&) = delete;

    int8_t twistFeedBack() const { return m_twistFeedBack; }
    void setTwistFeedBack(const int8_t &twistFeedBack) { m_twistFeedBack = twistFeedBack; }

    int8_t valve() const { return m_valve; }
    void setValve(const int8_t &valve) { m_valve = valve; }

    int8_t twist() const { return m_twist; }
    void setTwist(const int8_t &twist) { m_twist = twist; }

    int8_t squeeze() const { return m_squeeze; }
    void setSqueeze(const int8_t &squeeze) { m_squeeze = squeeze; }

    int8_t vibe0() const { return m_vibe0; }
    void setVibe0(const int8_t &vibe0) { m_vibe0 = vibe0; }

    int8_t vibe1() const { return m_vibe1; }
    void setVibe1(const int8_t &vibe1) { m_vibe1 = vibe1; }

    int8_t vibe2() const { return m_vibe2; }
    void setVibe2(const int8_t &vibe2) { m_vibe2 = vibe2; }

    int8_t vibe3() const { return m_vibe3; }
    void setVibe3(const int8_t &vibe3) { m_vibe3 = vibe3; }

    int8_t caseFan() const { return m_caseFan; }
    void setCaseFan(const int8_t &caseFan) { m_caseFan = caseFan; }

    int8_t lubeButton() const { return m_lubeButton; }
    void setLubeButton(const int8_t &lubeButton) { m_lubeButton = lubeButton; }

    int8_t internalTemp() const { return m_internalTemp; }
    void setInternalTemp(const int8_t &internalTemp) { m_internalTemp = internalTemp; }

    int8_t displayReset() const { return m_displayReset; }
    void setDisplayReset(const int8_t &displayReset) { m_displayReset = displayReset; }

    int8_t sleeveTemp() const { return m_sleeveTemp; }
    void setSleeveTemp(const int8_t &sleeveTemp) { m_sleeveTemp = sleeveTemp; }

    int8_t heater() const { return m_heater; }
    void setHeater(const int8_t &heater) { m_heater = heater; }

    int8_t buttonSetPin(int8_t index) const { return m_buttonSetPins[index]; }
    void setButtonSetPin(const int8_t pin, int8_t index) { 
        if(index >= MAX_BUTTON_SETS)
        {
            LogHandler::error("Pin_map", "Invalid index for button set %u", index);
            return;
        }
        m_buttonSetPins[index] = pin; 
    }

    int8_t i2cSda() const { return m_i2cSda; }
    void setI2cSda(const int8_t &i2cSda) { m_i2cSda = i2cSda; }

    int8_t i2cScl() const { return m_i2cScl; }
    void setI2cScl(const int8_t &i2cScl) { m_i2cScl = i2cScl; }
protected:
    PinMap(){}
private:
    int8_t m_twistFeedBack = TWIST_FEEDBACK_PIN_DEFAULT;
    int8_t m_valve = VALVE_SERVO_PIN_DEFAULT;
    int8_t m_twist = TWIST_SERVO_PIN_DEFAULT;
    int8_t m_squeeze = SQUEEZE_PIN_DEFAULT;
    int8_t m_vibe0 = VIBE0_PIN_DEFAULT;
    int8_t m_vibe1 = VIBE1_PIN_DEFAULT;
    int8_t m_vibe2 = VIBE2_PIN_DEFAULT;
    int8_t m_vibe3 = VIBE3_PIN_DEFAULT;
    int8_t m_caseFan = CASE_FAN_PIN_DEFAULT;
    int8_t m_lubeButton = LUBE_BUTTON_PIN_DEFAULT;
    int8_t m_internalTemp = INTERNAL_TEMP_PIN_DEFAULT;
    int8_t m_displayReset = DISPLAY_RST_PIN_DEFAULT;
    int8_t m_sleeveTemp = TEMP_PIN_DEFAULT;
    int8_t m_heater = HEATER_PIN_DEFAULT;
    int8_t m_i2cSda = I2C_SDA_PIN_DEFAULT;
    int8_t m_i2cScl = I2C_SCL_PIN_DEFAULT;
    int8_t m_buttonSetPins[MAX_BUTTON_SETS] = BUTTON_SET_PINS_DEFAULT;

    virtual void overideDefaults() =0;
};

class PinMapSSR1 : public PinMap {
public:
    static PinMapSSR1* getInstance()
    {
        static PinMapSSR1 instance;
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

private:
    int8_t m_encoder = BLDC_ENCODER_PIN_DEFAULT;
    int8_t m_chipSelect = BLDC_CHIPSELECT_PIN_DEFAULT;
    int8_t m_enable = BLDC_ENABLE_PIN_DEFAULT;
    int8_t m_hallEffect = BLDC_HALLEFFECT_PIN_DEFAULT;
    int8_t m_pwmChannel1 = BLDC_PWMCHANNEL1_PIN_DEFAULT;
    int8_t m_pwmChannel2 = BLDC_PWMCHANNEL2_PIN_DEFAULT;
    int8_t m_pwmChannel3 = BLDC_PWMCHANNEL3_PIN_DEFAULT;
    void overideDefaults() override {}

};

class PinMapOSR : public PinMap {
public:
    static PinMapOSR* getInstance()
    {
        static PinMapOSR instance;
        return &instance;
    }
    int8_t rightServo() const { return m_rightServo; }
    void setRightServo(const int8_t &rightServo) { m_rightServo = rightServo; }

    int8_t leftServo() const { return m_leftServo; }
    void setLeftServo(const int8_t &leftServo) { m_leftServo = leftServo; }

    int8_t pitchLeft() const { return m_pitchLeft; }
    void setPitchLeft(const int8_t &pitchLeft) { m_pitchLeft = pitchLeft; }

private:
    int8_t m_pitchLeft = PITCH_LEFT_SERVO_PIN_DEFAULT;
    int8_t m_rightServo = RIGHT_SERVO_PIN_DEFAULT;
    int8_t m_leftServo = LEFT_SERVO_PIN_DEFAULT;
    void overideDefaults() override {}
};

class PinMapSR6 : public PinMapOSR {
public:
    static PinMapSR6* getInstance()
    {
        static PinMapSR6 instance;
        return &instance;
    }
    int8_t pitchRight() const { return m_pitchRight; }
    void setPitchRight(const int8_t &pitchRight) { m_pitchRight = pitchRight; }

    int8_t rightUpperServo() const { return m_rightUpperServo; }
    void setRightUpperServo(const int8_t &rightUpperServo) { m_rightUpperServo = rightUpperServo; }

    int8_t leftUpperServo() const { return m_leftUpperServo; }
    void setLeftUpperServo(const int8_t &leftUpperServo) { m_leftUpperServo = leftUpperServo; }

private:
    int8_t m_pitchRight = PITCH_RIGHTSERVO_PIN_DEFAULT;
    int8_t m_rightUpperServo = RIGHT_UPPER_SERVO_PIN_DEFAULT;
    int8_t m_leftUpperServo = LEFT_UPPER_SERVO_PIN_DEFAULT;
    void overideDefaults() override {}
};

class PinMapINControl : public PinMapSR6 {
public:
    static PinMapINControl* getInstance()
    {
        static PinMapINControl instance;
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
        setInternalTemp(34);
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
};

class PinMapSR6MB : public PinMapSR6 {
public:
    static PinMapSR6MB* getInstance()
    {
        static PinMapSR6MB instance;
        return &instance;
    }
    void overideDefaults() override {
        setVibe3(26);
        setInternalTemp(32);

        // // EXT
        // //  EXT_Input2_PIN = 34;
        // //  EXT_Input3_PIN = 39;
        // //  EXT_Input4_PIN = 36;

        #warning How to handle other board setting overrides?
        // heaterResolution = json["heaterResolution"] | 8;
        // caseFanResolution = json["caseFanResolution"] | 10;
        // caseFanFrequency = json["caseFanFrequency"] | 25;
        // Display_Screen_Height = json["Display_Screen_Height"] | 32;

        setTwistFeedBack(0);

    }
};