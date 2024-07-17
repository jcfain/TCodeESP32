/* MIT License

Copyright (c) 2024 Jason C. Fain

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#pragma once
#include <algorithm>

#include <ArduinoJson.h>
#include <LittleFS.h>

#include "setting.h"
#include "settingConstants.h"
#include "pinMap.h"

class SettingMap
{
public:
    const std::vector<Setting> settings = {
        {SSID_SETTING, "Wifi ssid", "The ssid of the WiFi AP", SettingType::String, SSID_DEFAULT_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
        {WIFI_PASS_SETTING, "Wifi pass", "The password for the WiFi AP", SettingType::String, WIFI_PASS_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
        {BOARD_TYPE_SETTING, "Board type", "The physical board type", SettingType::Number, BOARD_TYPE_DEFAULT, RestartRequired::YES, {SettingProfile::System}},
        {LOG_LEVEL_SETTING, "Log level", "The loglevel that will output", SettingType::Number, LOG_LEVEL_DEFAULT, RestartRequired::NO, {SettingProfile::System}},
        //{FULL_BUILD, "Full build", "", SettingType::Boolean, false, RestartRequired::YES, {SettingProfile::System}},
        {TCODE_VERSION_SETTING, "TCode version", "The version of TCode", SettingType::Number, TCODE_VERSION_DEFAULT, RestartRequired::YES, {SettingProfile::System}},
        {UDP_SERVER_PORT, "Udp port", "The UDP port for TCode input", SettingType::Number, UDP_SERVER_PORT_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
        {WEBSERVER_PORT, "Web port", "The Web port for the web server", SettingType::Number, WEBSERVER_PORT_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
        {HOST_NAME, "Hostname", "The hostname for network com", SettingType::String, HOST_NAME_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
        {FRIENDLY_NAME, "Friendly name", "The friendly name displayed when connecting", SettingType::String, FRIENDLY_NAME_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
        {BLUETOOTH_ENABLED, "BLE enabled", "BLE tcode", SettingType::Boolean, BLUETOOTH_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::Bluetooth}},
        {PITCH_FREQUENCY_IS_DIFFERENT, "Pitch frewuency is different", "", SettingType::Boolean, PITCH_FREQUENCY_IS_DIFFERENT_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {MS_PER_RAD, "Ms per rad", "Micro seconds per radian for servos", SettingType::Number, MS_PER_RAD_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {SERVO_FREQUENCY, "Servo frequency", "Base servo frequenbcy", SettingType::Number, SERVO_FREQUENCY_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {PITCH_FREQUENCY, "Pitch frequency", "Pitch servo frequency if different than base", SettingType::Number, PITCH_FREQUENCY_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {VALVE_FREQUENCY, "Valve frequency", "Valve servo frequency", SettingType::Number, VALVE_FREQUENCY_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {TWIST_FREQUENCY, "Twist frequency", "Twist servo frequency if different than base", SettingType::Number, TWIST_FREQUENCY_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {SQUEEZE_FREQUENCY, "Squeeze frequency", "Squeeze servo frequency if different than base", SettingType::Number, SQUEEZE_FREQUENCY_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {CONTINUOUS_TWIST, "Continous twist", "Ignores any feedback signal", SettingType::Boolean, CONTINUOUS_TWIST_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {FEEDBACK_TWIST, "Feedback twist", "For feed back servos", SettingType::Boolean, FEEDBACK_TWIST_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {ANALOG_TWIST, "Analog twist", "Analog feedback servo", SettingType::Boolean, ANALOG_TWIST_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {TWIST_FEEDBACK_PIN, "Twist feedback PIN", "Feedback servo pin", SettingType::Number, TWIST_FEEDBACK_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Pin}},
        {RIGHT_SERVO_PIN, "Right servo PIN", "Pin the right servo is on", SettingType::Number, RIGHT_SERVO_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {LEFT_SERVO_PIN, "Left servo PIN", "Pin the left servo is on", SettingType::Number, LEFT_SERVO_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {RIGHT_UPPER_SERVO_PIN, "Right upper servo PIN", "Pin the right upper servo is on", SettingType::Number, RIGHT_UPPER_SERVO_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {LEFT_UPPER_SERVO_PIN, "Left upper servo PIN", "Pin the left servo is on", SettingType::Number, LEFT_UPPER_SERVO_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {PITCH_LEFT_SERVO_PIN, "Pitch left servo PIN", "Pin the pitch left servo is on", SettingType::Number, PITCH_LEFT_SERVO_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {PITCH_RIGHTSERVO_PIN, "Pitch right servo PIN", "Pin the pitch right servo is on", SettingType::Number, PITCH_RIGHTSERVO_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {VALVE_SERVO_PIN, "Valve servo PIN", "Pin the valve servo is on", SettingType::Number, VALVE_SERVO_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {TWIST_SERVO_PIN, "Twist servo PIN", "Pin the twist servo is on", SettingType::Number, TWIST_SERVO_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {SQUEEZE_PIN, "Squeeze servo PIN", "Pin the squeeze servo is on", SettingType::Number, SQUEEZE_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {VIBE0_PIN, "Vibe1 PIN", "Pin the vibe 1 is on", SettingType::Number, VIBE0_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
        {VIBE1_PIN, "Vibe2 PIN", "Pin the vibe 2 is on", SettingType::Number, VIBE1_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
        {VIBE2_PIN, "Vibe3 PIN", "Pin the vibe 3 is on", SettingType::Number, VIBE2_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
        {VIBE3_PIN, "Vibe4 PIN", "Pin the vibe 4 is on", SettingType::Number, VIBE3_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
        {CASE_FAN_PIN, "Case fan PIN", "Pin the case fan is on", SettingType::Number, CASE_FAN_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
        {LUBE_BUTTON_PIN, "Lube button PIN", "Pin the lube button is on", SettingType::Number, LUBE_BUTTON_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
        {INTERNAL_TEMP_PIN, "Internal temp PIN", "Pin the internal temp sensor is on", SettingType::Number, INTERNAL_TEMP_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Analog, SettingProfile::Pin}},
        {BLDC_USEPWM, "Use PWM", "Use PWM for BLDC sensor", SettingType::Boolean, BLDC_USEPWM_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc}},
        {BLDC_USEMT6701, "Use MT6701", "Use MT6701 for BLDC sensor", SettingType::Boolean, BLDC_USEMT6701_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc}},
        {BLDC_USEHALLSENSOR, "Use hall sensor", "Use Hall sensor for BLDC sensor", SettingType::Boolean, BLDC_USEHALLSENSOR_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc}},
        {BLDC_PULLEY_CIRCUMFERENCE, "Pull circumference", "The pulley circumference for BLDC motor", SettingType::Number, BLDC_PULLEY_CIRCUMFERENCE_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc}},
        {BLDC_ENCODER_PIN, "Encoder PIN", "Pin the BLDC encoder is on", SettingType::Number, BLDC_ENCODER_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin}},
        {BLDC_CHIPSELECT_PIN, "Chipselect PIN", "Pin the BLDC chip select is on", SettingType::Number, BLDC_CHIPSELECT_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin}},
        {BLDC_ENABLE_PIN, "Enable PIN", "Pin the BLDC enable is on", SettingType::Number, BLDC_ENABLE_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin}},
        {BLDC_HALLEFFECT_PIN, "Halleffect PIN", "Pin the hall effect is on", SettingType::Number, BLDC_HALLEFFECT_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin}},
        {BLDC_PWMCHANNEL1_PIN, "PWM channel1 PIN", "Pin for the BLDC PWM 1", SettingType::Number, BLDC_PWMCHANNEL1_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin, SettingProfile::PWM}},
        {BLDC_PWMCHANNEL2_PIN, "PWM channel2 PIN", "Pin for the BLDC PWM 2", SettingType::Number, BLDC_PWMCHANNEL2_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin, SettingProfile::PWM}},
        {BLDC_PWMCHANNEL3_PIN, "PWM channel3 PIN", "Pin for the BLDC PWM 3", SettingType::Number, BLDC_PWMCHANNEL3_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin, SettingProfile::PWM}},
        {BLDC_MOTORA_VOLTAGE, "Motor A voltage", "BLDC Motor A voltage", SettingType::Float, BLDC_MOTORA_VOLTAGE_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc}},
        {BLDC_MOTORA_CURRENT, "Motor A current", "BLDC Motor A current", SettingType::Float, BLDC_MOTORA_CURRENT_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc}},
        {BLDC_MOTORA_PARAMETERSKNOWN, "Motor A parameters known", "BLDC Motor A params known", SettingType::Boolean, BLDC_MOTORA_PARAMETERSKNOWN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc}},
        {BLDC_MOTORA_ZEROELECANGLE, "Motor A ZeroElecAngle", "BLDC Motor A ZeroElecAngle", SettingType::Float, BLDC_MOTORA_ZEROELECANGLE_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc}},
        {BLDC_RAILLENGTH, "Rail length", "SSR1 rail length", SettingType::Number, BLDC_RAILLENGTH_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc}},
        {BLDC_STROKELENGTH, "Stroke length", "SSR1 stroke length", SettingType::Number, BLDC_STROKELENGTH_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc}},
        {STATICIP, "Static IP", "Enable static IP for this device", SettingType::Boolean, STATICIP_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
        {LOCALIP, "Local IP", "The static IP of this device", SettingType::String, LOCALIP_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
        {GATEWAY, "Gateway", "The networks gateway", SettingType::String, GATEWAY_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
        {SUBNET, "Subnet", "The networks subnet", SettingType::String, SUBNET_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
        {DNS1, "DNS1", "The networks first DNS", SettingType::String, DNS1_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
        {DNS2, "DSN2", "The networks second DNS", SettingType::String, DNS2_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
        {SR6MODE, "SR6 mode", "The current device mode", SettingType::Boolean, SR6MODE_DEFAULT, RestartRequired::YES, {SettingProfile::System}},
        {RIGHT_SERVO_ZERO, "Right servo zero", "The zero calibration for the right servo", SettingType::Number, RIGHT_SERVO_ZERO_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {LEFT_SERVO_ZERO, "Left servo zero", "The zero calibration for the left servo", SettingType::Number, LEFT_SERVO_ZERO_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {RIGHT_UPPER_SERVO_ZERO, "Right upper servo zero", "The zero calibration for the right upper servo", SettingType::Number, RIGHT_UPPER_SERVO_ZERO_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {LEFT_UPPE_RSERVO_ZERO, "Left upper servo zero", "The zero calibration for the left upper servo", SettingType::Number, LEFT_UPPER_SERVO_ZERO_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {PITCH_LEFT_SERVO_ZERO, "Pitch left servo zero", "The zero calibration for the pitch left servo", SettingType::Number, PITCH_LEFT_SERVO_ZERO_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {PITCH_RIGHT_SERVO_ZERO, "Pitch right servo zero", "The zero calibration for the pitch right servo", SettingType::Number, PITCH_RIGHT_SERVO_ZERO_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {TWIST_SERVO_ZERO, "Twist servo zero", "The zero calibration for the twist servo", SettingType::Number, TWIST_SERVO_ZERO_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {VALVE_SERVO_ZERO, "Valve servo zero", "The zero calibration for the valve servo", SettingType::Number, VALVE_SERVO_ZERO_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {SQUEEZE_ZERO, "Squeeze servo zero", "The zero calibration for the squeeze servo", SettingType::Number, SQUEEZE_ZERO_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {AUTO_VALVE, "Auto valve", "Enable valve auto behavior", SettingType::Boolean, AUTO_VALVE_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {INVERSE_VALVE, "Inverse valve", "Inverse the valve movement", SettingType::Boolean, INVERSE_VALVE_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {VALVE_SERVO_90DEGREES, "Valve 90 degree servo", "The valve is 90 degrees max", SettingType::Boolean, VALVE_SERVO_90DEGREES_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {INVERSE_STROKE, "Inverse stroke", "Inverse stroke", SettingType::Boolean, INVERSE_STROKE_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {INVERSE_PITCH, "Inverse pitch", "Inverse pitch", SettingType::Boolean, INVERSE_PITCH_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
        {LUBE_AMOUNT, "Lube amount", "Amount of lube in PWM", SettingType::Number, LUBE_AMOUNT_DEFAULT, RestartRequired::YES, {SettingProfile::System}},
        {LUBE_ENABLED, "Lube enabled", "Enable lube", SettingType::Boolean, LUBE_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::System}},
        {DISPLAY_ENABLED, "Display enabled", "Enable the OLED display", SettingType::Boolean, DISPLAY_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::Display}},
        {SLEEVE_TEMP_DISPLAYED, "Sleeve temp displayed", "Display the sleeve temp on the OLED", SettingType::Boolean, SLEEVE_TEMP_DISPLAYED_DEFAULT, RestartRequired::YES, {SettingProfile::Display}},
        {VERSION_DISPLAYED, "Version displayed", "Display the version on the OLED", SettingType::Boolean, VERSION_DISPLAYED_DEFAULT, RestartRequired::YES, {SettingProfile::Display}},
        {INTERNAL_TEMP_DISPLAYED, "Internal temp displayed", "Display the internal temp on the OLED", SettingType::Boolean, INTERNAL_TEMP_DISPLAYED_DEFAULT, RestartRequired::YES, {SettingProfile::Display}},
        {TEMP_SLEEVE_ENABLED, "Temp sleeve enabled", "Enable sleeve temp", SettingType::Boolean, TEMP_SLEEVE_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
        {DISPLAY_SCREEN_WIDTH, "Display screen width", "Screen width of the OLED", SettingType::Number, DISPLAY_SCREEN_WIDTH_DEFAULT, RestartRequired::YES, {SettingProfile::Display}},
        {DISPLAY_SCREEN_HEIGHT, "Display screen height", "Screen height of the OLED", SettingType::Number, DISPLAY_SCREEN_HEIGHT_DEFAULT, RestartRequired::YES, {SettingProfile::Display}},
        {TARGET_TEMP, "Target sleeve temp", "Target temp for the sleeve", SettingType::Float, TARGET_TEMP_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
        {HEAT_PWM, "Heat PWM", "PWM when the sleeve needs heating", SettingType::Number, HEAT_PWM_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
        {HOLD_PWM, "Hold PWM", "PWM when the sleeve is at target", SettingType::Number, HOLD_PWM_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
        {DISPLAY_I2C_ADDRESS, "Display I2C address", "I2C address of the display", SettingType::String, DISPLAY_I2C_ADDRESS_DEFAULT, RestartRequired::YES, {SettingProfile::Display}},
        {DISPLAY_RST_PIN, "Display Rst PIN", "Reset pin for the display", SettingType::Number, DISPLAY_RST_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Display, SettingProfile::Pin}},
        {TEMP_PIN, "Temp pin", "Pin the sleeve temperture is on", SettingType::Number, TEMP_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Pin, SettingProfile::Analog}},
        {HEATER_PIN, "Heater PIN", "Pin the heater is on", SettingType::Number, HEATER_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature, SettingProfile::Pin, SettingProfile::PWM}},
        {HEATER_THRESHOLD, "Heater thresh hold", "", SettingType::Float, HEATER_THRESHOLD_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},// TODo: what is this exactly
        {HEATER_RESOLUTION, "Heater resolution", "Resolution for the Heater PWM", SettingType::Number, HEATER_RESOLUTION_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
        {HEATER_FREQUENCY, "Heater frequency", "Frequence for the heater PWM", SettingType::Number, HEATER_FREQUENCY_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
        {FAN_CONTROL_ENABLED, "Fan control enabled", "Enable PWM fan", SettingType::Boolean, FAN_CONTROL_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
        {CASE_FAN_FREQUENCY, "Fan frequency", "Fan frequency", SettingType::Number, CASE_FAN_FREQUENCY_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
        {CASE_FAN_RESOLUTION, "Fan resolution", "Fan resolution", SettingType::Number, CASE_FAN_RESOLUTION_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
        {INTERNAL_TEMP_FOR_FAN, "Internal temp for fan", "Fan temp sensor", SettingType::Double, INTERNAL_TEMP_FOR_FAN_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
        {INTERNAL_MAX_TEMP, "Internal max temp", "Max temp for the fan temp", SettingType::Double, INTERNAL_MAX_TEMP_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
        {TEMP_INTERNAL_ENABLED, "Temp Internal enabled", "Enable the internal temp sensor", SettingType::Boolean, TEMP_INTERNAL_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
        {BATTERY_LEVEL_ENABLED, "Battery level enabled", "Enable the battery level", SettingType::Boolean, BATTERY_LEVEL_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::Battery}},
        {BATTERY_LEVEL_NUMERIC, "Battery level numeric", "", SettingType::Number, BATTERY_LEVEL_NUMERIC_DEFAULT, RestartRequired::YES, {SettingProfile::Battery}},// TODO what is this exactly?
        {BATTERY_VOLTAGE_MAX, "Battery voltage max", "The max voltage for the battery", SettingType::Double, BATTERY_VOLTAGE_MAX_DEFAULT, RestartRequired::YES, {SettingProfile::Battery}},
        {BATTERY_CAPACITY_MAX, "Battery capcity max", "The battery max capacity", SettingType::Number, BATTERY_CAPACITY_MAX_DEFAULT, RestartRequired::YES, {SettingProfile::Battery}},
        {VOICE_ENABLED, "Voice enabled", "Enable voice control", SettingType::Boolean, VOICE_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::Voice}},
        {VOICE_MUTED, "Voice muted", "Voice talk back muted", SettingType::Boolean, VOICE_MUTED_DEFAULT, RestartRequired::YES, {SettingProfile::Voice}},
        {VOICE_WAKE_TIME, "Voice wake time", "How long to keep the voice module awake listening for commands", SettingType::Number, VOICE_WAKE_TIME_DEFAULT, RestartRequired::YES, {SettingProfile::Voice}},
        {VOICE_VOLUME, "Voice volume", "The volume of the voice talk back", SettingType::Number, VOICE_VOLUME_DEFAULT, RestartRequired::YES, {SettingProfile::Voice}},
        {LOG_INCLUDETAGS, "Log included tags", "Log tags to be included in the output", SettingType::Array, LOG_INCLUDETAGS_DEFAULT, RestartRequired::YES, {SettingProfile::System}},
        {LOG_EXCLUDETAGS, "Log excluded tags", "Log tags to be excluded in the output", SettingType::Array, LOG_EXCLUDETAGS_DEFAULT, RestartRequired::YES, {SettingProfile::System}},
        // {BUTTON_SET_PIN, "Button set PIN", "", SettingType::Number, BUTTON_SET_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::System, SettingProfile::Pin, SettingProfile::Analog}},
        // {I2C_SDA_PIN, "I2C SDA PIN", "", SettingType::Number, I2C_SDA_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::System, SettingProfile::Pin}},
        // {I2C_SCL_PIN, "I2C SCL PIN", "", SettingType::Number, I2C_SCL_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::System, SettingProfile::Pin}}
        
    };

    const Setting* getSetting(const char* name) const
    {
        std::vector<Setting>::const_iterator it = 
                std::find_if(settings.begin(), settings.end(), 
                    [name](const Setting &setting) {
                        return setting.name == name;
                });
        if(it != settings.end()) {
            return it.base();
        }
        return 0;
    }

    bool hasProfile(const char* name, const SettingProfile profile, const Setting* setting = 0) const
    {
        if(!setting)
            setting = getSetting(name);
        if(!setting)
            return false;
        return std::find_if(setting->profile.begin(), setting->profile.end(), 
                            [profile](const SettingProfile &profileIn) {
                                return profile == profileIn;
        }) != setting->profile.end();
    }

    template<typename T>
    void getValue(const char* name, T &value) const
    {
        if (m_wifiDoc.containsKey(name))
            value = m_wifiDoc[name];
        else if (m_commonDoc.containsKey(name))
            value = m_commonDoc[name];
        else
            LogHandler::error(m_TAG, "Get value key not found: ", name);
    }

    template<typename T>
    void setValue(const char* name, const T &value) const 
    {
        if (m_wifiDoc.containsKey(name))
            m_wifiDoc[name] = value;
        else if (m_commonDoc.containsKey(name))
            m_commonDoc[name] = value;
        else
            LogHandler::error(m_TAG, "Set value key not found: ", name);
    }

    bool loadCommon()
    {
        return load(userSettingsFilePath, m_commonDoc);
    }
    bool loadWifi()
    {
        return load(wifiFilePath, m_wifiDoc);
    }

    bool saveCommon(JsonObject fromJson = JsonObject())
    {
        return save(userSettingsFilePath, m_commonDoc, fromJson);
    }
    bool saveWifi(JsonObject fromJson = JsonObject())
    {
        return save(wifiFilePath, m_wifiDoc, fromJson);
    }
    bool resetCommon()
    {
        return resetDefault(userSettingsFilePath, m_commonDoc);
    }
    bool resetWiFi()
    {
        return resetDefault(wifiFilePath, m_wifiDoc);
    }
    // bool defaultCommon(JsonObject fromJson = JsonObject())
    // {
    //     return save("/settings.json", m_commonDoc, fromJson);
    // }
    // bool defaultWifi(JsonObject fromJson = JsonObject())
    // {
    //     //m_wifiDoc[]
    //     return save("/wifi.json", m_wifiDoc, fromJson);
    // }

private:
    const char* m_TAG = "SettingsFactory";
    const int m_commonDeserializeSize = 32768;
    const int m_commonSerializeSize = 24576;
    
    const char *userSettingsFilePath = "/userSettings.json";
    const char *wifiFilePath = "/wifiInfo.json";
    const char *buttonsFilePath = "/buttons.json";
    const char *motionProfilesFilePath = "/motionProfiles.json";
    StaticJsonDocument<100> m_wifiDoc;
    StaticJsonDocument<32768> m_commonDoc;

    template <unsigned int N>
    bool load(const char* path, StaticJsonDocument<N> &doc)
    {
        bool fileExists = LittleFS.exists(path);
        if(!fileExists)
        {
            LogHandler::info(m_TAG, "File %s did not exist, creating..", path);
        }
        File file = LittleFS.open(path, FILE_READ, !fileExists);
        if(!file) {
            LogHandler::error(m_TAG, "%s failed to open!", path);
            return false;
        }
        if(LogDeserializationError(deserializeJson(doc, file), file.name())) {
            file.close();
            return false;
        }
        file.close();
        //json = doc.as<JsonObject>();
        return true;
    }

    template <unsigned int N>
    bool resetDefault(const char* path, StaticJsonDocument<N> &doc)
    {
        bool fileExists = LittleFS.exists(path);
        if(!fileExists)
        {
            LogHandler::info(m_TAG, "File %s did not exist, creating..", path);
        }
        for(int i=0;i<settings.size();i++)
        {
            const Setting* setting = &settings[i];
            if(doc.containsKey(setting->name))
            {
                switch(setting->type)
                {
                    case SettingType::Boolean:
                        doc[setting->name] = mpark::get<const bool>(setting->value);
                    break;
                    case SettingType::Number:
                        doc[setting->name] = mpark::get<const int>(setting->value);
                    break;
                    case SettingType::Double:
                        doc[setting->name] = mpark::get<const double>(setting->value);
                    break;
                    case SettingType::Float:
                        doc[setting->name] = mpark::get<const float>(setting->value);
                    break;
                    case SettingType::String:
                        doc[setting->name] = mpark::get<const char*>(setting->value);
                    break;
                    case SettingType::Array:
                        // int[]
                        // doc[setting->name] = mpark::get<Array>(setting->value);
                    break;
                }
            }
        }
        return save(path, doc);
    }

    template <unsigned int N>
    bool save(const char* path, StaticJsonDocument<N> &doc, JsonObject fromJson = JsonObject())
    {
        if(!fromJson.isNull()) {
            LogHandler::debug(m_TAG, "Loading from input json: %s", path);
            doc.clear();
            doc.set(fromJson);
        }

        LogHandler::debug(m_TAG, "Doc overflowed: %u", doc.overflowed());
        LogHandler::debug(m_TAG, "Doc memory: %u", doc.memoryUsage());
        LogHandler::debug(m_TAG, "Doc capacity: %u", doc.capacity());
        File file = LittleFS.open(path, FILE_WRITE);
        if (!file )
        {
            LogHandler::error(m_TAG, "Failed to open file: %s", path);
            return false;
        }
        if (!serializeJson(doc, file))
        {
            LogHandler::error(m_TAG, "Failed to write to file: %s", path);
            file.close();
            return false;
        }
        LogHandler::debug(m_TAG, "File contents: %s", file.readString().c_str());
        return true;
    }
    
    bool LogDeserializationError(DeserializationError error, const char* filename) {
        if (error)
        {
            LogHandler::error(m_TAG, "Error deserializing json: %s", filename);
            switch (error.code())
            {
            case DeserializationError::Code::Ok:
                LogHandler::error(m_TAG, "Code: Ok");
                break;
            case DeserializationError::Code::EmptyInput:
                LogHandler::error(m_TAG, "Code: EmptyInput");
                break;
            case DeserializationError::Code::IncompleteInput:
                LogHandler::error(m_TAG, "Code: IncompleteInput");
                break;
            case DeserializationError::Code::InvalidInput:
                LogHandler::error(m_TAG, "Code: InvalidInput");
                break;
            case DeserializationError::Code::NoMemory:
                LogHandler::error(m_TAG, "Code: NoMemory");
                break;
            case DeserializationError::Code::TooDeep:
                LogHandler::error(m_TAG, "Code: TooDeep");
                break;
            }
            return true;
        }
        return false;
    }
};