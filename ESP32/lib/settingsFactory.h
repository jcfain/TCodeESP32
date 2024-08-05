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

#include "LogHandler.h"
#include "setting.h"
#include "constants.h"
#include "settingConstants.h"
#include "pinMap.h"


class SettingsFactory
{
public:
    SettingsFactory(SettingsFactory const&) = delete;
    void operator=(SettingsFactory const&) = delete;

    static SettingsFactory* getInstance() {
        static SettingsFactory factory;
        return &factory;
    }

    bool init() 
    {
        if(!loadWifi() || !loadCommon() || !loadPins())
            return false;
        m_initialized = true;
        loadCache();
        return true;
    }

    // Cached (Requires reboot)
    TCodeVersion getTcodeVersion() const { return tcodeVersion; }
    const char* getTcodeVersionString() const { 
        switch(getTcodeVersion()) {
            case TCodeVersion::v0_2:
                return "TCode v0.2\n";
                break;
            case TCodeVersion::v0_3:
                return "TCode v0.3\n";
                break;
            case TCodeVersion::v0_5:
                return "TCode v0.5\n";
                break;
            default:
                return "TCode v?\n";
                break;
        }
    }
    DeviceType getDeviceType() const { return m_deviceType; }
    int getUdpServerPort() const { return udpServerPort; }
    int getWebServerPort() const { return webServerPort; }
    const char* getHostname() const { return hostname; }
    const char* getFriendlyName() const { return friendlyName; }
    BoardType getBoardType() const { return m_boardType; }
    // MotorType getMotorType() const { return motorType; }
    // const char* getSSID() const { return ssid; }
    // const char* getWifiPass() const { return wifiPass; }
    // int getMsPerRad() const { return msPerRad; }
    // int getServoFrequency() const { return servoFrequency; }
    // int getPitchFrequency() const { return pitchFrequency; }
    // int getValveFrequency() const { return valveFrequency; }
    // int getTwistFrequency() const { return twistFrequency; }
    // int getSqueezeFrequency() const { return squeezeFrequency; }
    // bool getLubeEnabled() const { return lubeEnabled; }
    // bool getFeedbackTwist() const { return feedbackTwist; }
    // bool getAnalogTwist() const { return analogTwist; }
    // bool getBootButtonEnabled() const { return bootButtonEnabled; }
    // bool getButtonSetsEnabled() const { return buttonSetsEnabled; }
    // bool getBatteryLevelEnabled() const { return batteryLevelEnabled; }
    // bool getVoiceEnabled() const { return voiceEnabled; }
    // bool getTempSleeveEnabled() const { return tempSleeveEnabled; }
    // bool getTempInternalEnabled() const { return tempInternalEnabled; }
    // bool getStaticIP() const { return staticIP; }
    // const char* getLocalIP() const { return localIP; }
    // const char* getGateway() const { return gateway; }
    // const char* getSubnet() const { return subnet; }
    // const char* getDns1() const { return dns1; }
    // const char* getDns2() const { return dns2; }

    // Cached (Live update)
    bool getInverseStroke() const { return inverseStroke; }
    bool getInversePitch() const { return inversePitch; }
    bool getValveServo90Degrees() const { return valveServo90Degrees; }
    bool getAutoValve() const { return autoValve; }
    bool getInverseValve() const { return inverseValve; }
    bool getContinuousTwist() const { return continuousTwist; }
    int getLubeAmount() const { return lubeAmount; }
    int getBatteryCapacityMax() const { return batteryCapacityMax; }
    int getRightServo_ZERO() const { return RightServo_ZERO; }
    int getLeftServo_ZERO() const { return LeftServo_ZERO; }
    int getRightUpperServo_ZERO() const { return RightUpperServo_ZERO; }
    int getLeftUpperServo_ZERO() const { return LeftUpperServo_ZERO; }
    int getPitchLeftServo_ZERO() const { return PitchLeftServo_ZERO; }
    int getPitchRightServo_ZERO() const { return PitchRightServo_ZERO; }
    int getTwistServo_ZERO() const { return TwistServo_ZERO; }
    int getValveServo_ZERO() const { return ValveServo_ZERO; }
    int getSqueezeServo_ZERO() const { return SqueezeServo_ZERO; }
    const char* getBootButtonCommand() const { return bootButtonCommand; }
    uint16_t getButtonAnalogDebounce() const { return buttonAnalogDebounce; }
    bool getVersionDisplayed() const { return versionDisplayed; }
    bool getSleeveTempDisplayed() const { return sleeveTempDisplayed; }
    bool getInternalTempDisplayed() const { return internalTempDisplayed; }
    int getDisplayScreenWidth() const { return Display_Screen_Width; }
    int getDisplayScreenHeight() const { return Display_Screen_Height; }
    int getBatteryLevelNumeric() const { return batteryLevelNumeric; }
    int getTargetTemp() const { return targetTemp; }
    int getHeatPWM() const { return HeatPWM; }
    int getHoldPWM() const { return HoldPWM; }
    float getHeaterThreshold() const { return heaterThreshold; }
    double getInternalMaxTemp() const { return internalMaxTemp; }
    double getInternalTempForFanOn() const { return internalTempForFanOn; }
    

    // const std::vector<std::vector<Setting>*> settingsAll = {
    //     &m_wifiFileInfo.settings,
    //     &m_pinsFileInfo.settings,
    //     &m_commonFileInfo.settings
    // };

    const std::vector<SettingFileInfo*> AllSettings = {
        &m_wifiFileInfo,
        &m_pinsFileInfo,
        &m_commonFileInfo
    };

    const Setting* getSetting(const char* name) const
    { 
        if(!m_initialized)
            return 0;
        for(SettingFileInfo* settingsInfo : AllSettings)
        {
            std::vector<Setting>::const_iterator it = 
                    std::find_if(settingsInfo->settings.begin(), settingsInfo->settings.end(), 
                        [name](const Setting &setting) {
                            return setting.name == name;
                    });
            if(it != settingsInfo->settings.end()) {
                return it.base();
            }
        }
        return 0;
    }

    bool hasProfile(const char* name, const SettingProfile profile, const Setting* setting = 0) const
    {
        if(!m_initialized)
            return false;
        if(!setting)
            setting = getSetting(name);
        if(!setting)
            return false;
        return std::find_if(setting->profile.begin(), setting->profile.end(), 
                            [profile](const SettingProfile &profileIn) {
                                return profile == profileIn;
        }) != setting->profile.end();
    }

    template<typename T,
             typename = std::enable_if<!std::is_const<T>::value || std::is_integral<T>::value || std::is_enum<T>::value || std::is_floating_point<T>::value || std::is_same<T, bool>::value>>
    SettingFile getValue(const char* name, T &value)
    {
        if(!m_initialized)
            return SettingFile::NONE;
        if (m_wifiFileInfo.doc.containsKey(name)) 
        {
            xSemaphoreTake(m_wifiSemaphore, portTICK_PERIOD_MS);
            value = m_wifiFileInfo.doc[name].as<T>();
            xSemaphoreGive(m_wifiSemaphore);
            return SettingFile::Wifi;
        } 
        else if (m_commonFileInfo.doc.containsKey(name)) 
        {
            xSemaphoreTake(m_commonSemaphore, portTICK_PERIOD_MS);
            value = m_commonFileInfo.doc[name].as<T>();
            xSemaphoreGive(m_commonSemaphore);
            return SettingFile::Common;
        }    
        else if (m_pinsFileInfo.doc.containsKey(name)) 
        {
            xSemaphoreTake(m_commonSemaphore, portTICK_PERIOD_MS);
            value = m_pinsFileInfo.doc[name].as<T>();
            xSemaphoreGive(m_commonSemaphore);
            return SettingFile::Pins;
        }    
        else
        {
            LogHandler::error(m_TAG, "Get value key not found: ", name);
            return SettingFile::NONE;
        }
    }
    
    SettingFile getValue(const char* name, char* value, size_t len)
    {
        if(!m_initialized)
            return SettingFile::NONE;
        const char* constvalue = getValue(name);
        if(!constvalue) {
            return SettingFile::NONE;
        }
        strncpy(value, constvalue, len);
        if (m_wifiFileInfo.doc.containsKey(name)) 
        {
            return SettingFile::Wifi;
        } 
        else if (m_commonFileInfo.doc.containsKey(name)) 
        {
            return SettingFile::Common;
        }
    }
    
    const char* getValue(const char* name)
    {
        if(!m_initialized)
            return 0;
        if (m_wifiFileInfo.doc.containsKey(name)) 
        {
            xSemaphoreTake(m_wifiSemaphore, portTICK_PERIOD_MS);
            const char* constvalue = m_wifiFileInfo.doc[name];
            xSemaphoreGive(m_wifiSemaphore);
            return constvalue;
        } 
        else if (m_commonFileInfo.doc.containsKey(name)) 
        {
            xSemaphoreTake(m_commonSemaphore, portTICK_PERIOD_MS);
            const char* constvalue = m_commonFileInfo.doc[name];
            xSemaphoreGive(m_commonSemaphore);
            return constvalue;
        }    
        else
        {
            LogHandler::error(m_TAG, "Get value key not found: ", name);
        }
        return 0;
    }
    
    
    PinMapInfo getPins() 
    {
        return {m_deviceType, m_boardType, m_currentPinMap};
    }

    template<typename T,
             typename = std::enable_if<!std::is_const<T>::value || std::is_integral<T>::value || std::is_floating_point<T>::value || std::is_same<T, bool>::value>>
    SettingFile setValue(const char* name, const T &value) 
    {
        if(!m_initialized)
            return SettingFile::NONE;
        if (m_wifiFileInfo.doc.containsKey(name))
        {
            xSemaphoreTake(m_wifiSemaphore, portTICK_PERIOD_MS);
            toJson(getSetting(name), m_wifiFileInfo.doc);
            saveWifi();
            xSemaphoreGive(m_wifiSemaphore);
            return SettingFile::Wifi;
        }
        else if (m_commonFileInfo.doc.containsKey(name))
        {
            xSemaphoreTake(m_commonSemaphore, portTICK_PERIOD_MS);
            toJson(getSetting(name), m_commonFileInfo.doc);
            saveCommon();
            xSemaphoreGive(m_commonSemaphore);
            return SettingFile::Common;
        }
        else
            LogHandler::error(m_TAG, "Set value key not found: ", name);
            return SettingFile::NONE;
    }

    void defaultValue(const char* name) 
    {
        if(!m_initialized)
            return;
        const Setting* setting = getSetting(name);
        SettingFile file;
        file = setValue(name, setting->value);
        switch(file)
        {
            case SettingFile::Wifi:
            saveWifi();
            break;
            case SettingFile::Common:
            saveCommon();
            break;
            case SettingFile::ButtonSet:
            //saveButtonSet();
            break;
            case SettingFile::MotionProfile:
            //saveMotionProfiles();
            break;
            default:
                LogHandler::error(m_TAG, "Set default value key not found: ", name);
            break;
        }
    }

    bool saveCommon(JsonObject fromJson = JsonObject())
    {
        if(!m_initialized)
            return false;
        xSemaphoreTake(m_commonSemaphore, portTICK_PERIOD_MS);
        bool ret = save(m_commonFileInfo, fromJson);
        xSemaphoreGive(m_commonSemaphore);
        return ret;
    }
    bool resetCommon()
    {
        if(!m_initialized)
            return false;
        xSemaphoreTake(m_commonSemaphore, portTICK_PERIOD_MS);
        bool ret = loadDefault(m_commonFileInfo);
        xSemaphoreGive(m_commonSemaphore);
        return ret;
    }

    bool saveWifi(JsonObject fromJson = JsonObject())
    {
        if(!m_initialized)
            return false;
        xSemaphoreTake(m_wifiSemaphore, portTICK_PERIOD_MS);
        bool ret = save(m_wifiFileInfo, fromJson);
        xSemaphoreGive(m_wifiSemaphore);
        return ret;
    }
    bool resetWiFi()
    {
        if(!m_initialized)
            return false;
        xSemaphoreTake(m_wifiSemaphore, portTICK_PERIOD_MS);
        bool ret = loadDefault(m_wifiFileInfo);
        xSemaphoreGive(m_wifiSemaphore);
        return ret;
    }
    bool savePins(JsonObject fromJson = JsonObject())
    {
        if(!m_initialized)
            return false;
        xSemaphoreTake(m_pinSemaphore, portTICK_PERIOD_MS);
        bool ret = save(m_pinsFileInfo, fromJson);
        xSemaphoreGive(m_pinSemaphore);
        return ret;
    }
    bool resetPins()
    {
        if(!m_initialized)
            return false;
        xSemaphoreTake(m_pinSemaphore, portTICK_PERIOD_MS);
        bool ret = loadDefaultPins();
        xSemaphoreGive(m_pinSemaphore);
        return ret;
    }

    void loadCache() 
    {
        if(!m_initialized)
            return;
	    getValue(TCODE_VERSION_SETTING, tcodeVersion);
	    getValue(UDP_SERVER_PORT, udpServerPort);
	    getValue(WEBSERVER_PORT, webServerPort);
	    getValue(HOST_NAME, hostname, HOST_NAME_LEN);
	    getValue(FRIENDLY_NAME, friendlyName, FRIENDLY_NAME_LEN);
        getValue(DEVICE_TYPE, m_deviceType);
        getValue(BOARD_TYPE_SETTING, m_boardType);
        loadLiveCache();
    }

    void loadLiveCache() {
        if(!m_initialized)
            return;
        getValue(INVERSE_STROKE, inverseStroke);
        getValue(INVERSE_PITCH, inversePitch);
        getValue(VALVE_SERVO_90DEGREES, valveServo90Degrees);
        getValue(AUTO_VALVE, autoValve);
        getValue(INVERSE_VALVE, inverseValve);
        getValue(CONTINUOUS_TWIST, continuousTwist);
        getValue(LUBE_AMOUNT, lubeAmount);
        getValue(BATTERY_CAPACITY_MAX, batteryCapacityMax);
        getValue(RIGHT_SERVO_ZERO, RightServo_ZERO);
        getValue(LEFT_SERVO_ZERO, LeftServo_ZERO);
        getValue(RIGHT_UPPER_SERVO_ZERO, RightUpperServo_ZERO);
        getValue(LEFT_UPPER_SERVO_ZERO, LeftUpperServo_ZERO);
        getValue(PITCH_LEFT_SERVO_ZERO, PitchLeftServo_ZERO);
        getValue(PITCH_RIGHT_SERVO_ZERO, PitchRightServo_ZERO);
        getValue(TWIST_SERVO_ZERO, TwistServo_ZERO);
        getValue(VALVE_SERVO_ZERO, ValveServo_ZERO);
        getValue(SQUEEZE_ZERO, SqueezeServo_ZERO);
        getValue(BOOT_BUTTON_COMMAND, bootButtonCommand, sizeof(bootButtonCommand));
        getValue(VERSION_DISPLAYED, versionDisplayed);
        getValue(SLEEVE_TEMP_DISPLAYED, sleeveTempDisplayed);
        getValue(INTERNAL_TEMP_DISPLAYED, internalTempDisplayed);
        getValue(DISPLAY_SCREEN_WIDTH, Display_Screen_Width);
        getValue(DISPLAY_SCREEN_HEIGHT, Display_Screen_Height);
        getValue(BATTERY_LEVEL_NUMERIC, batteryLevelNumeric);
        getValue(TARGET_TEMP, targetTemp);
        getValue(HEAT_PWM, HeatPWM);
        getValue(HOLD_PWM, HoldPWM);
        getValue(HEATER_THRESHOLD, heaterThreshold);
        getValue(INTERNAL_MAX_TEMP, internalMaxTemp);
        getValue(INTERNAL_TEMP_FOR_FANON, internalTempForFanOn);
    }

private:
    const char* m_TAG = "SettingsFactory";
    PinMap* m_currentPinMap;
    const int m_commonDeserializeSize = 32768;
    const int m_commonSerializeSize = 24576;

    bool m_initialized = false;


    SemaphoreHandle_t m_wifiSemaphore;
    SemaphoreHandle_t m_commonSemaphore;
    SemaphoreHandle_t m_pinSemaphore;

    SettingFileInfo m_wifiFileInfo = 
    {
        WIFI_SETTINGS_PATH, SettingFile::Wifi, JsonDocument(), 
        {
            {SSID_SETTING, "Wifi ssid", "The ssid of the WiFi AP", SettingType::String, SSID_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
            {WIFI_PASS_SETTING, "Wifi pass", "The password for the WiFi AP", SettingType::String, WIFI_PASS_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}}
        }
    };

    SettingFileInfo m_commonFileInfo = 
    {
        COMMON_SETTINGS_PATH, SettingFile::Common, JsonDocument(), 
        {
            {DEVICE_TYPE, "Type of device", "The surrent selected device", SettingType::Number, DEVICE_TYPE_DEFAULT, RestartRequired::YES, {SettingProfile::System}},
            {MOTOR_TYPE_SETTING, "Motor type", "The current motor type", SettingType::Number, MOTOR_TYPE_DEFAULT, RestartRequired::YES, {SettingProfile::System}},
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
            {BLDC_ENCODER, "BLDC encoder type", "Select the type of bldc encoder installed", SettingType::Number, BLDC_ENCODER_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc}},
            {BLDC_USEHALLSENSOR, "Use hall sensor", "Use Hall sensor for BLDC sensor", SettingType::Boolean, BLDC_USEHALLSENSOR_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc}},
            {BLDC_PULLEY_CIRCUMFERENCE, "Pull circumference", "The pulley circumference for BLDC motor", SettingType::Number, BLDC_PULLEY_CIRCUMFERENCE_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc}},
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
            //{SR6MODE, "SR6 mode", "The current device mode", SettingType::Boolean, SR6MODE_DEFAULT, RestartRequired::YES, {SettingProfile::System}},
            {RIGHT_SERVO_ZERO, "Right servo zero", "The zero calibration for the right servo", SettingType::Number, RIGHT_SERVO_ZERO_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
            {LEFT_SERVO_ZERO, "Left servo zero", "The zero calibration for the left servo", SettingType::Number, LEFT_SERVO_ZERO_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
            {RIGHT_UPPER_SERVO_ZERO, "Right upper servo zero", "The zero calibration for the right upper servo", SettingType::Number, RIGHT_UPPER_SERVO_ZERO_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
            {LEFT_UPPER_SERVO_ZERO, "Left upper servo zero", "The zero calibration for the left upper servo", SettingType::Number, LEFT_UPPER_SERVO_ZERO_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
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
            {HEATER_THRESHOLD, "Heater thresh hold", "", SettingType::Float, HEATER_THRESHOLD_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},// TODo: what is this exactly
            {HEATER_RESOLUTION, "Heater resolution", "Resolution for the Heater PWM", SettingType::Number, HEATER_RESOLUTION_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
            {HEATER_FREQUENCY, "Heater frequency", "Frequence for the heater PWM", SettingType::Number, HEATER_FREQUENCY_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
            {FAN_CONTROL_ENABLED, "Fan control enabled", "Enable PWM fan", SettingType::Boolean, FAN_CONTROL_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
            {CASE_FAN_FREQUENCY, "Fan frequency", "Fan frequency", SettingType::Number, CASE_FAN_FREQUENCY_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
            {CASE_FAN_RESOLUTION, "Fan resolution", "Fan resolution", SettingType::Number, CASE_FAN_RESOLUTION_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
            {INTERNAL_TEMP_FOR_FANON, "Internal temp for fan", "The temp. threshold to turn the fan on", SettingType::Double, INTERNAL_TEMP_FOR_FAN_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
            {INTERNAL_MAX_TEMP, "Internal max temp", "Max temp for internal. The movement will shutdown if this value is reached.", SettingType::Double, INTERNAL_MAX_TEMP_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
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
            {BOOT_BUTTON_ENABLED, "Boot button enabled", "Enables the boot button function", SettingType::Boolean, BOOT_BUTTON_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::Button}},
            {BOOT_BUTTON_COMMAND, "Boot button command", "Command to execute when the boot button is pressed", SettingType::String, BOOT_BUTTON_COMMAND_DEFAULT, RestartRequired::NO, {SettingProfile::Button}},
            {BUTTON_SETS_ENABLED, "Button sets enabled", "Enables the button sets function", SettingType::Boolean, BUTTON_SETS_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::Button}},
            {BUTTON_ANALOG_DEBOUNCE, "Button debounce", "How long to debounce the button press in ms", SettingType::Number, BUTTON_ANALOG_DEBOUNCE_DEFAULT, RestartRequired::NO, {SettingProfile::Button}}  
        }
    };

    SettingFileInfo m_pinsFileInfo = 
    {
        PIN_SETTINGS_PATH, SettingFile::Pins, JsonDocument(), 
        {
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
            {DISPLAY_RST_PIN, "Display Rst PIN", "Reset pin for the display", SettingType::Number, DISPLAY_RST_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Display, SettingProfile::Pin}},
            {TEMP_PIN, "Temp pin", "Pin the sleeve temperture is on", SettingType::Number, TEMP_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Pin, SettingProfile::Analog}},
            {HEATER_PIN, "Heater PIN", "Pin the heater is on", SettingType::Number, HEATER_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature, SettingProfile::Pin, SettingProfile::PWM}},
            {I2C_SDA_PIN, "I2C SDA PIN", "", SettingType::Number, I2C_SDA_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::System, SettingProfile::Pin}},
            {I2C_SCL_PIN, "I2C SCL PIN", "", SettingType::Number, I2C_SCL_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::System, SettingProfile::Pin}},
            {BLDC_ENCODER_PIN, "Encoder PIN", "Pin the BLDC encoder is on", SettingType::Number, BLDC_ENCODER_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin}},
            {BLDC_CHIPSELECT_PIN, "Chipselect PIN", "Pin the BLDC chip select is on", SettingType::Number, BLDC_CHIPSELECT_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin}},
            {BLDC_ENABLE_PIN, "Enable PIN", "Pin the BLDC enable is on", SettingType::Number, BLDC_ENABLE_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin}},
            {BLDC_HALLEFFECT_PIN, "Halleffect PIN", "Pin the hall effect is on", SettingType::Number, BLDC_HALLEFFECT_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin}},
            {BLDC_PWMCHANNEL1_PIN, "PWM channel1 PIN", "Pin for the BLDC PWM 1", SettingType::Number, BLDC_PWMCHANNEL1_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin, SettingProfile::PWM}},
            {BLDC_PWMCHANNEL2_PIN, "PWM channel2 PIN", "Pin for the BLDC PWM 2", SettingType::Number, BLDC_PWMCHANNEL2_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin, SettingProfile::PWM}},
            {BLDC_PWMCHANNEL3_PIN, "PWM channel3 PIN", "Pin for the BLDC PWM 3", SettingType::Number, BLDC_PWMCHANNEL3_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin, SettingProfile::PWM}}
        }
    };


    SettingsFactory() {
        m_wifiSemaphore = xSemaphoreCreateMutex();
        m_commonSemaphore = xSemaphoreCreateMutex();
        m_pinSemaphore = xSemaphoreCreateMutex();
    };

    // Cached (Requires reboot)
    TCodeVersion tcodeVersion;
    int udpServerPort;
    int webServerPort;
    char hostname[HOST_NAME_LEN];
    char friendlyName[FRIENDLY_NAME_LEN];
    BoardType m_boardType;
    DeviceType m_deviceType;

    // Cached (Live update)
    bool inverseStroke;
    bool inversePitch;
    bool valveServo90Degrees;
    bool autoValve;
    bool inverseValve;
    bool continuousTwist;
    int lubeAmount;
    int batteryCapacityMax;
    int RightServo_ZERO;
    int LeftServo_ZERO;
    int RightUpperServo_ZERO;
    int LeftUpperServo_ZERO;
    int PitchLeftServo_ZERO;
    int PitchRightServo_ZERO;
    int TwistServo_ZERO;
    int ValveServo_ZERO;
    int SqueezeServo_ZERO;
    char bootButtonCommand[MAX_COMMAND];
    uint16_t buttonAnalogDebounce;
    bool versionDisplayed;
    bool sleeveTempDisplayed;
    bool internalTempDisplayed;
    int Display_Screen_Width;
    int Display_Screen_Height;
    int batteryLevelNumeric;
    int targetTemp;
    int HeatPWM;
    int HoldPWM;
    float heaterThreshold;
    double internalMaxTemp;
    double internalTempForFanOn;

    bool load(SettingFileInfo &fileInfo)
    {
        bool fileExists = LittleFS.exists(fileInfo.path);
        if(!fileExists)
        {
            LogHandler::info(m_TAG, "File %s did not exist, creating..", fileInfo.path);
        }
        File file = LittleFS.open(fileInfo.path, FILE_READ, !fileExists);
        if(!file) {
            LogHandler::error(m_TAG, "%s failed to open!", fileInfo.path);
            return false;
        }
        if(LogDeserializationError(deserializeJson(fileInfo.doc, file), file.name())) {
            file.close();
            return false;
        }
        file.close();
        //json = doc.as<JsonObject>();
        return true;
    }

    bool loadDefault()
    {
        return loadDefault(m_wifiFileInfo) &&
                loadDefault(m_commonFileInfo) &&
                loadDefault(m_pinsFileInfo);
    }

    bool loadDefault(SettingFile file) 
    {
        if(file == SettingFile::Wifi) 
        {
            return loadDefault(m_wifiFileInfo);
        }
        if(file == SettingFile::Common) 
        {
            return loadDefault(m_commonFileInfo);
        }
        if(file == SettingFile::Pins) 
        {
            return loadDefaultPins();
        }
    }

    //template <unsigned int N>
    bool loadDefault(SettingFileInfo &fileInfo)
    {
        for(const Setting& setting : fileInfo.settings)
        {
            toJson(&setting, fileInfo.doc);
        }
        return save(fileInfo);
    }

    bool loadDefaultPins() {
        BoardType boardType;
        getValue(BOARD_TYPE_SETTING, boardType);// Use setting from json
        switch(boardType)
        {
            case BoardType::CRIMZZON: {
                PinMapSR6MB* pinMap = PinMapSR6MB::getInstance();
                pinMap->overideDefaults();
                syncSR6Pins(pinMap);
                save(m_pinsFileInfo);
            }
            break;
            case BoardType::ISAAC: {
                PinMapINControl* pinMap = PinMapINControl::getInstance();
                pinMap->overideDefaults();
                syncSR6Pins(pinMap);
                save(m_pinsFileInfo);
            }
            break;
            default: {
                loadDefault(m_pinsFileInfo);
            }
        }
    }

    bool loadCommon()
    {
        xSemaphoreTake(m_commonSemaphore, portTICK_PERIOD_MS);
        auto ret = load(m_commonFileInfo);
        xSemaphoreGive(m_commonSemaphore);
        return ret;
    }
    bool loadWifi()
    {
        xSemaphoreTake(m_wifiSemaphore, portTICK_PERIOD_MS);
        bool ret = load(m_wifiFileInfo);;
        xSemaphoreGive(m_wifiSemaphore);
        return ret;
    }
    bool loadPins()
    {
        xSemaphoreTake(m_pinSemaphore, portTICK_PERIOD_MS);
        bool ret = load(m_pinsFileInfo);
        DeviceType deviceType;
        getValue(DEVICE_TYPE, deviceType);
        switch(deviceType) 
        {
            case DeviceType::SSR1:
                m_currentPinMap = loadSSR1Pins();
            break;
            case DeviceType::SR6:
                m_currentPinMap = loadSR6Pins();
            break;
            default:
                m_currentPinMap = loadOSRPins();

        }
        xSemaphoreGive(m_pinSemaphore);
        return ret;
    }

    //template <unsigned int N>
    bool save(SettingFileInfo &fileInfo, JsonObject fromJson = JsonObject())
    {
        if(!m_initialized)
            return false;
        if(!fromJson.isNull()) {
            LogHandler::debug(m_TAG, "Loading from input json: %s", fileInfo.path);
            fileInfo.doc.clear();
            fileInfo.doc.set(fromJson);
        }

        LogHandler::debug(m_TAG, "Doc overflowed: %u", fileInfo.doc.overflowed());
        LogHandler::debug(m_TAG, "Doc memory: %u", fileInfo.doc.memoryUsage());
        //LogHandler::debug(m_TAG, "Doc capacity: %u", fileInfo.doc.capacity());
        File file = LittleFS.open(fileInfo.path, FILE_WRITE);
        if (!file )
        {
            LogHandler::error(m_TAG, "Failed to open file: %s", fileInfo.path);
            return false;
        }
        if (!serializeJson(fileInfo.doc, file))
        {
            LogHandler::error(m_TAG, "Failed to write to file: %s", fileInfo.path);
            file.close();
            return false;
        }
        LogHandler::debug(m_TAG, "File contents: %s", file.readString().c_str());
        return true;
    }
    
    bool LogDeserializationError(DeserializationError error, const char* fileName) 
    {
        if (error)
        {
            LogHandler::error(m_TAG, "Error deserializing json: %s", fileName);
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
    void loadCommonPins(PinMap* pinMap) 
    {
        uint8_t pin = -1;
        getValue(TWIST_FEEDBACK_PIN, pin);
        pinMap->setTwistFeedBack(pin);
        getValue(VALVE_SERVO_PIN, pin);
        pinMap->setValve(pin);
        getValue(TWIST_SERVO_PIN, pin);
        pinMap->setTwist(pin);
        getValue(SQUEEZE_PIN, pin);
        pinMap->setSqueeze(pin);
        getValue(VIBE0_PIN, pin);
        pinMap->setVibe0(pin);
        getValue(VIBE1_PIN, pin);
        pinMap->setVibe1(pin);
        getValue(VIBE2_PIN, pin);
        pinMap->setVibe2(pin);
        getValue(VIBE3_PIN, pin);
        pinMap->setVibe3(pin);
        getValue(CASE_FAN_PIN, pin);
        pinMap->setCaseFan(pin);
        getValue(LUBE_BUTTON_PIN, pin);
        pinMap->setLubeButton(pin);
        getValue(INTERNAL_TEMP_PIN, pin);
        pinMap->setInternalTemp(pin);
        getValue(DISPLAY_RST_PIN, pin);
        pinMap->setDisplayReset(pin);
        getValue(TEMP_PIN, pin);
        pinMap->setSleeveTemp(pin);
        getValue(HEATER_PIN, pin);
        pinMap->setHeater(pin);
        // getValue(SQUEEZE_PIN, pin);
        // pinMap->setButtonSetPins(pin);
        getValue(I2C_SDA_PIN, pin);
        pinMap->setI2cSda(pin);
        getValue(I2C_SCL_PIN, pin);
        pinMap->setI2cScl(pin);
    }
    PinMapSSR1* loadSSR1Pins() 
    {
        PinMapSSR1* pinMap = PinMapSSR1::getInstance();
        loadCommonPins(pinMap);
        uint8_t pin = -1;
        getValue(BLDC_ENCODER_PIN, pin);
        pinMap->setEncoder(pin);
        getValue(BLDC_CHIPSELECT_PIN, pin);
        pinMap->setChipSelect(pin);
        getValue(BLDC_ENABLE_PIN, pin);
        pinMap->setEnable(pin);
        getValue(BLDC_HALLEFFECT_PIN, pin);
        pinMap->setHallEffect(pin);
        getValue(BLDC_PWMCHANNEL1_PIN, pin);
        pinMap->setPwmChannel1(pin);
        getValue(BLDC_PWMCHANNEL2_PIN, pin);
        pinMap->setPwmChannel2(pin);
        getValue(BLDC_PWMCHANNEL3_PIN, pin);
        pinMap->setPwmChannel3(pin);
        return pinMap;

    }
    PinMapOSR* loadOSRPins() 
    {
        PinMapOSR* pinMap = PinMapOSR::getInstance();
        loadCommonPins(pinMap);
        uint8_t pin = -1;
        getValue(RIGHT_SERVO_PIN, pin);
        pinMap->setRightServo(pin);
        getValue(LEFT_SERVO_PIN, pin);
        pinMap->setLeftServo(pin);
        getValue(PITCH_LEFT_SERVO_PIN, pin);
        pinMap->setPitchLeft(pin);
        return pinMap;
    }
    
    PinMapSR6* loadSR6Pins() 
    {
        PinMapSR6* pinMap = PinMapSR6::getInstance();
        loadCommonPins(pinMap);
        uint8_t pin = -1;
        getValue(RIGHT_SERVO_PIN, pin);
        pinMap->setRightServo(pin);
        getValue(LEFT_SERVO_PIN, pin);
        pinMap->setLeftServo(pin);
        getValue(PITCH_LEFT_SERVO_PIN, pin);
        pinMap->setPitchLeft(pin);
        getValue(PITCH_RIGHTSERVO_PIN, pin);
        pinMap->setPitchRight(pin);
        getValue(RIGHT_UPPER_SERVO_PIN, pin);
        pinMap->setRightUpperServo(pin);
        getValue(LEFT_UPPER_SERVO_PIN, pin);
        pinMap->setLeftUpperServo(pin);
        return pinMap;
    }
    
    void syncCommonPins(const PinMap* pinMap) 
    {
        setValue(TWIST_FEEDBACK_PIN, pinMap->twistFeedBack());
        setValue(VALVE_SERVO_PIN, pinMap->valve());
        setValue(TWIST_SERVO_PIN, pinMap->twist());
        setValue(SQUEEZE_PIN, pinMap->squeeze());
        setValue(VIBE0_PIN, pinMap->vibe0());
        setValue(VIBE1_PIN, pinMap->vibe1());
        setValue(VIBE2_PIN, pinMap->vibe2());
        setValue(VIBE3_PIN, pinMap->vibe3());
        setValue(CASE_FAN_PIN, pinMap->caseFan());
        setValue(LUBE_BUTTON_PIN, pinMap->lubeButton());
        setValue(INTERNAL_TEMP_PIN, pinMap->internalTemp());
        setValue(DISPLAY_RST_PIN, pinMap->displayReset());
        setValue(TEMP_PIN, pinMap->sleeveTemp());
        setValue(HEATER_PIN, pinMap->heater());
        //setValue(I2C_SDA_PIN, pinMap->setButtonSetPins());
        setValue(I2C_SDA_PIN, pinMap->i2cSda());
        setValue(I2C_SCL_PIN, pinMap->i2cScl());
    }

    void syncSSR1Pins(const PinMapSSR1* pinMap) 
    {
        syncCommonPins(pinMap);
        setValue(BLDC_ENCODER_PIN, pinMap->encoder());
        setValue(BLDC_CHIPSELECT_PIN, pinMap->chipSelect());
        setValue(BLDC_ENABLE_PIN, pinMap->enable());
        setValue(BLDC_HALLEFFECT_PIN, pinMap->hallEffect());
        setValue(BLDC_PWMCHANNEL1_PIN, pinMap->pwmChannel1());
        setValue(BLDC_PWMCHANNEL2_PIN, pinMap->pwmChannel2());
        setValue(BLDC_PWMCHANNEL3_PIN, pinMap->pwmChannel3());
    }

    void syncOSRPins(const PinMapOSR* pinMap) 
    {
        syncCommonPins(pinMap);
        setValue(RIGHT_SERVO_PIN, pinMap->rightServo());
        setValue(LEFT_SERVO_PIN, pinMap->leftServo());
        setValue(PITCH_LEFT_SERVO_PIN, pinMap->pitchLeft());
    }

    void syncSR6Pins(const PinMapSR6* pinMap) 
    {
        syncCommonPins(pinMap);
        setValue(RIGHT_SERVO_PIN, pinMap->rightServo());
        setValue(LEFT_SERVO_PIN, pinMap->leftServo());
        setValue(PITCH_LEFT_SERVO_PIN, pinMap->pitchLeft());
        setValue(PITCH_RIGHTSERVO_PIN, pinMap->pitchRight());
        setValue(RIGHT_UPPER_SERVO_PIN, pinMap->rightUpperServo());
        setValue(LEFT_UPPER_SERVO_PIN, pinMap->leftUpperServo());
    }

    void toJson(const Setting *setting, JsonDocument &doc) {
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
};
