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

#include "enum.h"
#include "jsonConverters.h"
// // #include "LogHandler.h"
#include "setting.h"
#include "constants.h"
#include "settingConstants.h"
#include "pinMap.h"
#include "espTimerMap.h"

using SETTING_STATE_FUNCTION_PTR_T = void (*)(const SettingProfile &profile, const char* settingThatChanged);

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
        //resetAll();
        if(!loadAllFromDisk())
            return false;
        loadCommonCache();
        loadPinCache();
        return true;
    }
    
    const std::vector<SettingFileInfo*> AllSettings = {
        &m_networkFileInfo,
        &m_commonFileInfo,
        &m_pinsFileInfo// Pins are dependent on common for now. Device type and board type
    };

    // Cached (Requires reboot)
    TCodeVersion getTcodeVersion() const { return tcodeVersion; }
    const char* getTcodeVersionString() const { 
        switch(getTcodeVersion()) {
            // case TCodeVersion::v0_2:
            //     return "TCode v0.2\n";
            //     break;
            case TCodeVersion::v0_3:
                return "TCode v0.3\n";
                break;
            case TCodeVersion::v0_4:
                return "TCode v0.4\n";
                break;
            default:
                return "TCode v?\n";
                break;
        }
    }
    // Cached requires restart
    // DeviceType getDeviceType() const { return m_deviceType; }
    int getUdpServerPort() const { return udpServerPort; }
    int getWebServerPort() const { return webServerPort; }
    const char* getHostname() const { return hostname; }
    const char* getFriendlyName() const { return friendlyName; }
    // BoardType getBoardType() const { return m_boardType; }

    // Cached (Live update)
    LogLevel getLogLevel() const { return logLevel; }
    std::vector<const char*> getLogIncludes() const { return logIncludes; }
    std::vector<const char*> getLogExcludes() const { return logExcludes; }
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
    bool getVoiceMuted() const { return voiceMuted; };
	int8_t getVoiceVolume() const { return voiceVolume; };
	int8_t getVoiceWakeTime() const { return voiceWakeTime; };
    bool getVersionDisplayed() const { return versionDisplayed; }
    bool getSleeveTempDisplayed() const { return sleeveTempDisplayed; }
    bool getInternalTempDisplayed() const { return internalTempDisplayed; }
    int getDisplayScreenWidth() const { return Display_Screen_Width; }
    int getDisplayScreenHeight() const { return Display_Screen_Height; }
    bool getBatteryLevelNumeric() const { return batteryLevelNumeric; }
    int getTargetTemp() const { return targetTemp; }
    int getHeatPWM() const { return HeatPWM; }
    int getHoldPWM() const { return HoldPWM; }
    float getHeaterThreshold() const { return heaterThreshold; }
    double getInternalMaxTemp() const { return internalMaxTemp; }
    double getInternalTempForFanOn() const { return internalTempForFanOn; }
    bool getVibTimeoutEnabled() const { return vibTimeoutEnabled; }
    int getVibTimeout() const { return vibTimeout; }

    void setMessageCallback(SETTING_STATE_FUNCTION_PTR_T f)
    {
        LogHandler::debug(m_TAG, "setMessageCallback");
        if (f == nullptr)
        {
            message_callback = 0;
        }
        else
        {
            message_callback = f;
        }
    }

    SettingFileInfo* getFile(const char* settingName) const
    { 
        for(SettingFileInfo* settingsInfo : AllSettings)
        {
            const Setting* setting = settingsInfo->getSetting(settingName);
            if(setting)
                return settingsInfo;
        }
        return 0;
    }

    JsonDocument getNetworkSettings() const
    { 
        return m_networkFileInfo.doc;
    }

    const Setting* getSetting(const char* name) const
    { 
        for(SettingFileInfo* settingsInfo : AllSettings)
        {
            const Setting* setting = settingsInfo->getSetting(name);
            if(setting)
                return setting;
        }
        LogHandler::error(m_TAG, "Setting '%s' not found when calling getSetting", name);
        return 0;
    }

    bool hasProfile(const char* name, const SettingProfile profile, const Setting* setting = 0) const
    {
        if(!setting)
            setting = getSetting(name);
        if(!setting)
            return false;
        return std::find_if(setting->profiles.begin(), setting->profiles.end(), 
                            [profile](const SettingProfile &profileIn) {
                                return profile == profileIn;
        }) != setting->profiles.end();
    }

    template<typename T,
             typename = std::enable_if<!std::is_const<T>::value || std::is_integral<T>::value || std::is_enum<T>::value || std::is_floating_point<T>::value || std::is_same<T, bool>::value>>
    SettingFile getValue(const char* name, T &value)
    {
        if (m_networkFileInfo.doc.containsKey(name)) 
        {
            if(!m_networkFileInfo.initialized) {
                LogHandler::error(m_TAG, "getValue T called before network file initialized");
                return SettingFile::NONE;
            }
            xSemaphoreTake(m_networkSemaphore, portTICK_PERIOD_MS);
            value = m_networkFileInfo.doc[name].as<T>();
            xSemaphoreGive(m_networkSemaphore);
            return SettingFile::Network;
        } 
        else if (m_commonFileInfo.doc.containsKey(name)) 
        {
            if(!m_commonFileInfo.initialized) {
                LogHandler::error(m_TAG, "getValue T called before common file initialized");
                return SettingFile::NONE;
            }
            xSemaphoreTake(m_commonSemaphore, portTICK_PERIOD_MS);
            value = m_commonFileInfo.doc[name].as<T>();
            xSemaphoreGive(m_commonSemaphore);
            return SettingFile::Common;
        }    
        else if (m_pinsFileInfo.doc.containsKey(name)) 
        {
            if(!m_pinsFileInfo.initialized) {
                LogHandler::error(m_TAG, "getValue T called before pins file initialized");
                return SettingFile::NONE;
            }
            xSemaphoreTake(m_commonSemaphore, portTICK_PERIOD_MS);
            value = m_pinsFileInfo.doc[name].as<T>();
            xSemaphoreGive(m_commonSemaphore);
            return SettingFile::Pins;
        }    
        else
        {
            LogHandler::error(m_TAG, "Get value key not found: %s", name);
            return SettingFile::NONE;
        }
    }
    
    SettingFile getValue(const char* name, char* value, size_t len)
    {
        const char* constvalue = getValue(name);
        if(!constvalue) {
            return SettingFile::NONE;
        }
        strncpy(value, constvalue, len);
        if (m_networkFileInfo.doc.containsKey(name)) 
        {
            if(!m_networkFileInfo.initialized) {
                LogHandler::error(m_TAG, "getValue char* len called before network file initialized");
                return SettingFile::NONE;
            }
            LogHandler::debug(m_TAG, "getValue char* len %s: value: %s", name, strcmp(name, WIFI_PASS_SETTING) || !strcmp(value, WIFI_PASS_DONOTCHANGE_DEFAULT) ? value : "<Redacted>");
            return SettingFile::Network;
        } 
        else if (m_commonFileInfo.doc.containsKey(name)) 
        {
            if(!m_commonFileInfo.initialized) {
                LogHandler::error(m_TAG, "getValue char* len called before common file initialized");
                return SettingFile::NONE;
            }
            LogHandler::debug(m_TAG, "getValue char* len %s: value: %s", name, value);
            return SettingFile::Common;
        }
        return SettingFile::NONE;
    }
    
    const char* getValue(const char* name)
    {
        if (m_networkFileInfo.doc.containsKey(name)) 
        {
            if(!m_networkFileInfo.initialized) {
                LogHandler::error(m_TAG, "getValue char* called before network file initialized");
                return 0;
            }
            xSemaphoreTake(m_networkSemaphore, portTICK_PERIOD_MS);
            const char* constvalue = m_networkFileInfo.doc[name];
            LogHandler::debug(m_TAG, "getValue char* wifi: %s: constvalue: %s", name, strcmp(name, WIFI_PASS_SETTING) || !strcmp(constvalue, WIFI_PASS_DONOTCHANGE_DEFAULT) ? constvalue : "<Redacted>");
            xSemaphoreGive(m_networkSemaphore);
            return constvalue;
        } 
        else if (m_commonFileInfo.doc.containsKey(name)) 
        {
            if(!m_commonFileInfo.initialized) {
                LogHandler::error(m_TAG, "getValue char* called before common file initialized");
                return 0;
            }
            xSemaphoreTake(m_commonSemaphore, portTICK_PERIOD_MS);
            const char* constvalue = m_commonFileInfo.doc[name];
            LogHandler::debug(m_TAG, "getValue char* common: %s: constvalue: %s", name, constvalue);
            xSemaphoreGive(m_commonSemaphore);
            return constvalue;
        }    
        else
        {
            LogHandler::error(m_TAG, "Get value key not found: %s", name);
        }
        return 0;
    }
    SettingFile getValueVector(const char* name, std::vector<const char*> &value)
    {
        LogHandler::debug("Getting vector string values: %s", name);
        SettingFileInfo* fileInfo = getFile(name);
        if(!fileInfo) {
            LogHandler::error("Key not found in settings: %s", name);
            return SettingFile::NONE;
        }
        if(!fileInfo->initialized) {
            LogHandler::error(m_TAG, "getValueVector char* called before initialized");
            return SettingFile::NONE;
        }
        //const Setting* setting = fileInfo->getSetting(name);
        JsonArray array = fileInfo->doc[name].as<JsonArray>();
        for (size_t i = 0; i < array.size(); i++)
        {
            value.push_back(array[i].as<const char*>());
        }
        
        return fileInfo->file;
    }
    SettingFile getValueVector(const char* name, std::vector<int> &value)
    {
        LogHandler::debug("Getting vector int values: %s", name);
        SettingFileInfo* fileInfo = getFile(name);
        if(!fileInfo) {
            LogHandler::error("Key not found in settings: %s", name);
            return SettingFile::NONE;
        }
        if(!fileInfo->initialized) {
            LogHandler::error(m_TAG, "getValueVector int called before initialized");
            return SettingFile::NONE;
        }
        //const Setting* setting = fileInfo->getSetting(name);
        JsonArray array = fileInfo->doc[name].as<JsonArray>();
        for (size_t i = 0; i < array.size(); i++)
        {
            value.push_back(array[i].as<int>());
        }
        
        return fileInfo->file;
    }
    
    PinMap* getPins() 
    {
        return m_currentPinMap;
    }

    template<typename T,
             typename = std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value || std::is_same<T, bool>::value>>
    SettingFile setValue(const char* name, const T &value) 
    {
        LogHandler::debug(m_TAG, "Enter setValue T: %s", name);
        if (m_networkFileInfo.doc.containsKey(name))
        {
            if(!m_networkFileInfo.initialized) {
                LogHandler::error(m_TAG, "setValue T called before network file initialized");
                return SettingFile::NONE;
            }
            T currentValue = m_networkFileInfo.doc[name].as<T>();
            if(currentValue != value) {
                LogHandler::debug(m_TAG, "Change wifi value T: %s", name);
                xSemaphoreTake(m_networkSemaphore, portTICK_PERIOD_MS);
                //const Setting* setting = getSetting(name);
                m_networkFileInfo.doc[name] = value;
                //loadWifiLiveCache(name); // Not needed now
                xSemaphoreGive(m_networkSemaphore);
            }
            return SettingFile::Network;
        }
        else if (m_commonFileInfo.doc.containsKey(name))
        {
            if(!m_commonFileInfo.initialized) {
                LogHandler::error(m_TAG, "setValue T called before common file initialized");
                return SettingFile::NONE;
            }
            T currentValue = m_commonFileInfo.doc[name].as<T>();
            if(currentValue != value) {
                LogHandler::debug(m_TAG, "Change common value T: %s", name);
                xSemaphoreTake(m_commonSemaphore, portTICK_PERIOD_MS);
                //const Setting* setting = getSetting(name);
                m_commonFileInfo.doc[name] = value;
                loadCommonLiveCache(name);
                xSemaphoreGive(m_commonSemaphore);
            }
            return SettingFile::Common;
        }
        else if (m_pinsFileInfo.doc.containsKey(name))
        {
            if(!m_pinsFileInfo.initialized) {
                LogHandler::error(m_TAG, "setValue T called before pins file initialized");
                return SettingFile::NONE;
            }
            T currentValue = m_pinsFileInfo.doc[name].as<T>();
            if(currentValue != value) {
                LogHandler::debug(m_TAG, "Change pin value T: %s", name);
                xSemaphoreTake(m_pinSemaphore, portTICK_PERIOD_MS);
                //const Setting* setting = getSetting(name);
                m_pinsFileInfo.doc[name] = value;
                //loadPinCache(); // Not needed now
                xSemaphoreGive(m_pinSemaphore);
            }
            return SettingFile::Pins;
        }
        LogHandler::error(m_TAG, "Set value key not found");
        return SettingFile::NONE;
    }

    SettingFile setValue(const char* name, const char* value) 
    {
        LogHandler::debug(m_TAG, "Enter setValue const char*: %s", name);
        SettingFileInfo* fileInfo = getFile(name);
        if(!fileInfo) {
            LogHandler::error(m_TAG, "Set value key not found");
            return SettingFile::NONE;
        }
        if(!fileInfo->initialized) {
            LogHandler::error(m_TAG, "setValue const char* called before initialized");
            return SettingFile::NONE;
        }
        const char* currentValue = fileInfo->doc[name].as<const char*>();
        if(fileInfo->doc[name].isNull() || strcmp(currentValue, value)) {
            LogHandler::debug(m_TAG, "Change value: %s old value: %s new value: %s", name, currentValue, strcmp(name, WIFI_PASS_SETTING) || !strcmp(value, WIFI_PASS_DONOTCHANGE_DEFAULT) ? value : "<Redacted>");
            fileInfo->doc[name] = value;
            if(fileInfo->file == SettingFile::Common) {
                loadCommonLiveCache(name);
            }
        }
        return fileInfo->file;
    }

    template <typename T>
    SettingFile setValue(const char* name, const std::vector<T> &value) 
    {
        LogHandler::debug(m_TAG, "Set vector value: %s", name);
        SettingFileInfo* fileInfo = getFile(name);
        if(!fileInfo->initialized) {
            LogHandler::error(m_TAG, "setValue vector T called before initialized");
            return SettingFile::NONE;
        }
        if(!fileInfo) {
            LogHandler::error(m_TAG, "Set value key not found");
            return SettingFile::NONE;
        }
        fileInfo->doc[name] = value;
        if(fileInfo->file == SettingFile::Common) {
            loadCommonLiveCache(name);
        }
        return fileInfo->file;
    }

    void defaultValue(const char* name) 
    {
        LogHandler::debug(m_TAG, "Default: %s", name);
        SettingFileInfo* fileInfo = getFile(name);
        if(!fileInfo->initialized) {
            LogHandler::error(m_TAG, "defaultValue called before initialized");
            return;
        }
        if(!fileInfo) {
            LogHandler::error(m_TAG, "Default value key not found");
            return;
        }
        const Setting* setting = fileInfo->getSetting(name);
        defaultToJson(setting, fileInfo->doc);
        if(fileInfo->file == SettingFile::Common) {
            loadCommonLiveCache(name);
        }
        switch(fileInfo->file)
        {
            case SettingFile::Network:
            saveWifi();
            break;
            case SettingFile::Common:
            saveCommon();
            break;
            case SettingFile::Pins:
            savePins();
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

    bool saveAllToDisk(JsonObject fromJson = JsonObject())
    {
        for(SettingFileInfo* settingsInfo : AllSettings)
        {
            if(!saveToDisk(*settingsInfo, fromJson))
                return false;
        }
        loadCommonLiveCache();
        loadPinCache();
        return true;;
    }
    bool resetAll() 
    {
        for(SettingFileInfo* settingsInfo : AllSettings)
        {
            if(!deleteJsonFile(settingsInfo->path))
                return false;
        }
        if(!deleteJsonFile(MOTION_PROFILE_SETTINGS_PATH))
            return false;
        if(!deleteJsonFile(BUTTON_SETTINGS_PATH))
            return false;
        // for(SettingFileInfo* settingsInfo : AllSettings)
        // {
        //     if(!loadDefault(*settingsInfo))
        //         return false;
        // }
        return true;
    }

    bool save(SettingFile file, JsonObject fromJson = JsonObject()) {
        switch(file)
        {
            case SettingFile::Common:
                if(!saveCommon(fromJson)) {
                    return false;
                }
                break;
            case SettingFile::Network:
                if(!saveWifi(fromJson)) {
                    return false;
                }
            case SettingFile::Pins:
                if(!savePins(fromJson)) {
                    return false;
                }
                break;
            default: {
                LogHandler::error(m_TAG, "Invalid or unsuported file: %ld", (int)file);
                return false;
            }
        }
        return true;
    }

    bool saveCommon(JsonObject fromJson = JsonObject())
    {
        xSemaphoreTake(m_commonSemaphore, portTICK_PERIOD_MS);
        bool ret = saveToDisk(m_commonFileInfo, fromJson);
        xSemaphoreGive(m_commonSemaphore);
        if(ret) {
            loadCommonLiveCache();
        }
        return ret;
    }
    bool resetCommon()
    {
        xSemaphoreTake(m_commonSemaphore, portTICK_PERIOD_MS);
        bool ret = loadDefault(m_commonFileInfo);
        xSemaphoreGive(m_commonSemaphore);
        if(ret)
            loadCommonLiveCache();
        return ret;
    }

    bool saveWifi(JsonObject fromJson = JsonObject())
    {
        xSemaphoreTake(m_networkSemaphore, portTICK_PERIOD_MS);
        if(!fromJson.isNull()) 
        {
            const char* pass = fromJson[WIFI_PASS_SETTING] | DECOY_PASS;
            if(!strcmp(pass, DECOY_PASS)) 
            {
                char passTemp[WIFI_PASS_LEN];
                getValue(WIFI_PASS_SETTING, passTemp, sizeof(passTemp));
                fromJson[WIFI_PASS_SETTING] = passTemp;
            }
        }
        bool ret = saveToDisk(m_networkFileInfo, fromJson);
        xSemaphoreGive(m_networkSemaphore);
        return ret;
    }
    bool resetWiFi()
    {
        xSemaphoreTake(m_networkSemaphore, portTICK_PERIOD_MS);
        bool ret = loadDefault(m_networkFileInfo);
        xSemaphoreGive(m_networkSemaphore);
        return ret;
    }
    
    bool savePins(JsonObject fromJson = JsonObject())
    {
        xSemaphoreTake(m_pinSemaphore, portTICK_PERIOD_MS);
        bool ret = saveToDisk(m_pinsFileInfo, fromJson);
        xSemaphoreGive(m_pinSemaphore);
        if(ret)
            loadPinCache();
        return ret;
    }
    bool resetPins()
    {
        xSemaphoreTake(m_pinSemaphore, portTICK_PERIOD_MS);
        bool ret = loadDefaultPins();
        xSemaphoreGive(m_pinSemaphore);
        if(ret)
            loadPinCache();
        return ret;
    }
    
    void loadCommonLiveCache(const char* name = 0) {
        if(!m_commonFileInfo.initialized) {
            LogHandler::error(m_TAG, "loadCommonLiveCache called before initialized");
            return;
        }
        bool targeted = !!name;
        if(!name || !strcmp(name, LOG_LEVEL_SETTING)) {
            getValue(LOG_LEVEL_SETTING, logLevel);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, LOG_INCLUDETAGS)) {
            getValue(LOG_INCLUDETAGS, logIncludes);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, LOG_INCLUDETAGS)) {
            getValue(LOG_EXCLUDETAGS, logExcludes);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, INVERSE_STROKE)) {
            getValue(INVERSE_STROKE, inverseStroke);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, INVERSE_PITCH)) {
            getValue(INVERSE_PITCH, inversePitch);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, VALVE_SERVO_90DEGREES)) {
            getValue(VALVE_SERVO_90DEGREES, valveServo90Degrees);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, AUTO_VALVE)) {
            getValue(AUTO_VALVE, autoValve);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, INVERSE_VALVE)) {
            getValue(INVERSE_VALVE, inverseValve);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, CONTINUOUS_TWIST)) {
            getValue(CONTINUOUS_TWIST, continuousTwist);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, LUBE_AMOUNT)) {
            getValue(LUBE_AMOUNT, lubeAmount);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, BATTERY_CAPACITY_MAX)) {
            getValue(BATTERY_CAPACITY_MAX, batteryCapacityMax);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, RIGHT_SERVO_ZERO)) {
            getValue(RIGHT_SERVO_ZERO, RightServo_ZERO);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, LEFT_SERVO_ZERO)) {
            getValue(LEFT_SERVO_ZERO, LeftServo_ZERO);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, RIGHT_UPPER_SERVO_ZERO)) {
            getValue(RIGHT_UPPER_SERVO_ZERO, RightUpperServo_ZERO);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, LEFT_UPPER_SERVO_ZERO)) {
            getValue(LEFT_UPPER_SERVO_ZERO, LeftUpperServo_ZERO);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, PITCH_LEFT_SERVO_ZERO)) {
            getValue(PITCH_LEFT_SERVO_ZERO, PitchLeftServo_ZERO);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, PITCH_RIGHT_SERVO_ZERO)) {
            getValue(PITCH_RIGHT_SERVO_ZERO, PitchRightServo_ZERO);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, TWIST_SERVO_ZERO)) {
            getValue(TWIST_SERVO_ZERO, TwistServo_ZERO);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, VALVE_SERVO_ZERO)) {
            getValue(VALVE_SERVO_ZERO, ValveServo_ZERO);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, SQUEEZE_ZERO)) {
            getValue(SQUEEZE_ZERO, SqueezeServo_ZERO);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, BOOT_BUTTON_COMMAND)) {
            bootButtonCommand[0] = {0};
            getValue(BOOT_BUTTON_COMMAND, bootButtonCommand, BOOT_BUTTON_COMMAND_LEN);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, VERSION_DISPLAYED)) {
            getValue(VERSION_DISPLAYED, versionDisplayed);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, SLEEVE_TEMP_DISPLAYED)) {
            getValue(SLEEVE_TEMP_DISPLAYED, sleeveTempDisplayed);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, INTERNAL_TEMP_DISPLAYED)) {
            getValue(INTERNAL_TEMP_DISPLAYED, internalTempDisplayed);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, DISPLAY_SCREEN_WIDTH)) {
            getValue(DISPLAY_SCREEN_WIDTH, Display_Screen_Width);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, DISPLAY_SCREEN_HEIGHT)) {
            getValue(DISPLAY_SCREEN_HEIGHT, Display_Screen_Height);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, BATTERY_LEVEL_NUMERIC)) {
            getValue(BATTERY_LEVEL_NUMERIC, batteryLevelNumeric);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, TARGET_TEMP)) {
            getValue(TARGET_TEMP, targetTemp);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, HEAT_PWM)) {
            getValue(HEAT_PWM, HeatPWM);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, HOLD_PWM)) {
            getValue(HOLD_PWM, HoldPWM);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, HEATER_THRESHOLD)) {
            getValue(HEATER_THRESHOLD, heaterThreshold);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, INTERNAL_MAX_TEMP)) {
            getValue(INTERNAL_MAX_TEMP, internalMaxTemp);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, INTERNAL_TEMP_FOR_FANON)) {
            getValue(INTERNAL_TEMP_FOR_FANON, internalTempForFanOn);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, VOICE_MUTED)) {
            getValue(VOICE_MUTED, voiceMuted);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, VOICE_VOLUME)) {
            getValue(VOICE_VOLUME, voiceVolume);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, VOICE_WAKE_TIME)) {
            getValue(VOICE_WAKE_TIME, voiceWakeTime);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, VIB_TIMEOUT)) {
            getValue(VIB_TIMEOUT, vibTimeout);
            if(targeted) {initCommonMessages(name); return;}
        }
        if(!name || !strcmp(name, VIB_TIMEOUT_ENABLED)) {
            getValue(VIB_TIMEOUT_ENABLED, vibTimeoutEnabled);
            if(targeted) {initCommonMessages(name); return;}
        }
        initCommonMessages();
    }

    
private:
    const char* m_TAG = TagHandler::SettingsFactory;
    PinMap* m_currentPinMap;
    // const int m_commonDeserializeSize = 32768;
    // const int m_commonSerializeSize = 24576;

    SETTING_STATE_FUNCTION_PTR_T message_callback = 0;


    SemaphoreHandle_t m_networkSemaphore;
    SemaphoreHandle_t m_commonSemaphore;
    SemaphoreHandle_t m_pinSemaphore;

    SettingFileInfo m_networkFileInfo = 
    {
        false, NETWORK_SETTINGS_PATH, SettingFile::Network, JsonDocument(), 
        {
            {SSID_SETTING, "Wifi ssid", "The ssid of the WiFi AP", SettingType::String, SSID_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi, SettingProfile::Wireless}},
            {WIFI_PASS_SETTING, "Wifi pass", "The password for the WiFi AP", SettingType::String, WIFI_PASS_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi, SettingProfile::Wireless}},
            {STATICIP, "Static IP", "Enable static IP for this device", SettingType::Boolean, STATICIP_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
            {LOCALIP, "Local IP", "The static IP of this device", SettingType::String, LOCALIP_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
            {GATEWAY, "Gateway", "The networks gateway", SettingType::String, GATEWAY_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
            {SUBNET, "Subnet", "The networks subnet", SettingType::String, SUBNET_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
            {DNS1, "DNS1", "The networks first DNS", SettingType::String, DNS1_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
            {DNS2, "DSN2", "The networks second DNS", SettingType::String, DNS2_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
            {UDP_SERVER_PORT, "Udp port", "The UDP port for TCode input", SettingType::Number, UDP_SERVER_PORT_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
            {WEBSERVER_PORT, "Web port", "The Web port for the web server", SettingType::Number, WEBSERVER_PORT_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
            {HOST_NAME, "Hostname", "The hostname for network com", SettingType::String, HOST_NAME_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
            {FRIENDLY_NAME, "Friendly name", "The friendly name displayed when connecting", SettingType::String, FRIENDLY_NAME_DEFAULT, RestartRequired::YES, {SettingProfile::Wifi}},
            {BLUETOOTH_ENABLED, "Bluetooth classic enabled", "Bluetooth classic TCode. Note: this disabled wifi and the website. Use the setting command to switch back", SettingType::Boolean, BLUETOOTH_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::Bluetooth, SettingProfile::Wireless}},
            {BLE_ENABLED, "BLE enabled", "BLE TCode. Note: this disabled wifi and the website. Use the setting command to switch back", SettingType::Boolean, BLE_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::Ble, SettingProfile::Wireless}},
            {BLE_DEVICE_TYPE, "BLE device type", "BLE device type", SettingType::Number, BLE_DEVICE_TYPE_DEFAULT, RestartRequired::YES, {SettingProfile::Bluetooth, SettingProfile::Wireless}},
            {BLE_LOVE_DEVICE_TYPE, "BLE love device type", "BLE love device type", SettingType::Number, BLE_LOVE_DEVICE_TYPE_DEFAULT, RestartRequired::YES, {SettingProfile::Bluetooth, SettingProfile::Wireless}}     
        }
    };

    SettingFileInfo m_commonFileInfo = 
    {
        false, COMMON_SETTINGS_PATH, SettingFile::Common, JsonDocument(), 
        {
            {DEVICE_TYPE, "Type of device", "The surrent selected device", SettingType::Number, DEVICE_TYPE_DEFAULT, RestartRequired::YES, {SettingProfile::System}},
            {MOTOR_TYPE_SETTING, "Motor type", "The current motor type", SettingType::Number, MOTOR_TYPE_DEFAULT, RestartRequired::YES, {SettingProfile::System}},
            {BOARD_TYPE_SETTING, "Board type", "The physical board type", SettingType::Number, BOARD_TYPE_DEFAULT, RestartRequired::YES, {SettingProfile::System}},
            {LOG_LEVEL_SETTING, "Log level", "The loglevel that will output", SettingType::Number, LOG_LEVEL_DEFAULT, RestartRequired::NO, {SettingProfile::System}},
            //{FULL_BUILD, "Full build", "", SettingType::Boolean, false, RestartRequired::YES, {SettingProfile::System}}, // Not sure what this was for. Doesnt appear to be used anywhere.
            {TCODE_VERSION_SETTING, "TCode version", "The version of TCode", SettingType::Number, TCODE_VERSION_DEFAULT, RestartRequired::YES, {SettingProfile::System}},
            {MS_PER_RAD, "Ms per rad", "Micro seconds per radian for servos", SettingType::Number, MS_PER_RAD_DEFAULT, RestartRequired::YES, {SettingProfile::Servo}},
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
            {VIB_TIMEOUT_ENABLED, "Vib timeout Enabled", "If disabled the vibs must be manually stopped", SettingType::Boolean, VIB_TIMEOUT_ENABLED_DEFAULT, RestartRequired::NO, {SettingProfile::Vib}},
            {VIB_TIMEOUT, "Vib timeout", "The time out the vib stops", SettingType::Number, VIB_TIMEOUT_DEFAULT, RestartRequired::NO, {SettingProfile::Vib}},
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
            {HEATER_THRESHOLD, "Heater thresh hold", "The HoldPWM will be sent while the temp less than or equal to TargetTemp + heaterThreshold", SettingType::Float, HEATER_THRESHOLD_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},// TODo: what is this exactly
            {HEATER_RESOLUTION, "Heater resolution", "Resolution for the Heater PWM", SettingType::Number, HEATER_RESOLUTION_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
            {FAN_CONTROL_ENABLED, "Fan control enabled", "Enable PWM fan", SettingType::Boolean, FAN_CONTROL_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
            {CASE_FAN_RESOLUTION, "Fan resolution", "Fan resolution", SettingType::Number, CASE_FAN_RESOLUTION_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
            {INTERNAL_TEMP_FOR_FANON, "Internal temp for fan", "The temp. threshold to turn the fan on", SettingType::Double, INTERNAL_TEMP_FOR_FAN_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
            {INTERNAL_MAX_TEMP, "Internal max temp", "Max temp for internal. The movement will shutdown if this value is reached.", SettingType::Double, INTERNAL_MAX_TEMP_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
            {TEMP_INTERNAL_ENABLED, "Temp Internal enabled", "Enable the internal temp sensor", SettingType::Boolean, TEMP_INTERNAL_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature}},
            {BATTERY_LEVEL_ENABLED, "Battery level enabled", "Enable the battery level", SettingType::Boolean, BATTERY_LEVEL_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::Battery}},
            {BATTERY_LEVEL_NUMERIC, "Battery level numeric", "Display the battery level in numeric form", SettingType::Boolean, BATTERY_LEVEL_NUMERIC_DEFAULT, RestartRequired::YES, {SettingProfile::Battery}},
            {BATTERY_VOLTAGE_MAX, "Battery voltage max", "The max voltage for the battery", SettingType::Double, BATTERY_VOLTAGE_MAX_DEFAULT, RestartRequired::YES, {SettingProfile::Battery}},
            {BATTERY_CAPACITY_MAX, "Battery capcity max", "The battery max capacity", SettingType::Number, BATTERY_CAPACITY_MAX_DEFAULT, RestartRequired::YES, {SettingProfile::Battery}},
            {VOICE_ENABLED, "Voice enabled", "Enable voice control", SettingType::Boolean, VOICE_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::Voice}},
            {VOICE_MUTED, "Voice muted", "Voice talk back muted", SettingType::Boolean, VOICE_MUTED_DEFAULT, RestartRequired::YES, {SettingProfile::Voice}},
            {VOICE_WAKE_TIME, "Voice wake time", "How long to keep the voice module awake listening for commands", SettingType::Number, VOICE_WAKE_TIME_DEFAULT, RestartRequired::YES, {SettingProfile::Voice}},
            {VOICE_VOLUME, "Voice volume", "The volume of the voice talk back", SettingType::Number, VOICE_VOLUME_DEFAULT, RestartRequired::YES, {SettingProfile::Voice}},
            {LOG_INCLUDETAGS, "Log included tags", "Log tags to be included in the output", SettingType::ArrayString, LOG_INCLUDETAGS_DEFAULT, RestartRequired::YES, {SettingProfile::System}},
            {LOG_EXCLUDETAGS, "Log excluded tags", "Log tags to be excluded in the output", SettingType::ArrayString, LOG_EXCLUDETAGS_DEFAULT, RestartRequired::YES, {SettingProfile::System}},
            {BOOT_BUTTON_ENABLED, "Boot button enabled", "Enables the boot button function", SettingType::Boolean, BOOT_BUTTON_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::Button}},
            {BOOT_BUTTON_COMMAND, "Boot button command", "Command to execute when the boot button is pressed", SettingType::String, BOOT_BUTTON_COMMAND_DEFAULT, RestartRequired::NO, {SettingProfile::Button}},
            {BUTTON_SETS_ENABLED, "Button sets enabled", "Enables the button sets function", SettingType::Boolean, BUTTON_SETS_ENABLED_DEFAULT, RestartRequired::YES, {SettingProfile::Button}},
            {BUTTON_ANALOG_DEBOUNCE, "Button debounce", "How long to debounce the button press in ms", SettingType::Number, BUTTON_ANALOG_DEBOUNCE_DEFAULT, RestartRequired::NO, {SettingProfile::Button}}
        }
    };

    SettingFileInfo m_pinsFileInfo = 
    {
        false, PIN_SETTINGS_PATH, SettingFile::Pins, JsonDocument(), 
        {
            // PWM
            {RIGHT_SERVO_PIN, "Right servo PIN", "Pin the right servo is on", SettingType::Number, RIGHT_SERVO_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
            {RIGHT_SERVO_CHANNEL, "Right servo channel", "Timer channel the right servo is on", SettingType::Number, RIGHT_SERVO_CHANNEL_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
            {LEFT_SERVO_PIN, "Left servo PIN", "Pin the left servo is on", SettingType::Number, LEFT_SERVO_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
            {LEFT_SERVO_CHANNEL, "Left servo channel", "Timer channel the left servo is on", SettingType::Number, LEFT_SERVO_CHANNEL_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
            {RIGHT_UPPER_SERVO_PIN, "Right upper servo PIN", "Pin the right upper servo is on", SettingType::Number, RIGHT_UPPER_SERVO_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
            {RIGHT_UPPER_SERVO_CHANNEL, "Right upper servo channel", "Timer channel the right upper servo is on", SettingType::Number, RIGHT_UPPER_SERVO_CHANNEL_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
            {LEFT_UPPER_SERVO_PIN, "Left upper servo PIN", "Pin the left servo is on", SettingType::Number, LEFT_UPPER_SERVO_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
            {LEFT_UPPER_SERVO_CHANNEL, "Left upper servo channel", "Timer channel the left servo is on", SettingType::Number, LEFT_UPPER_SERVO_CHANNEL_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
            {PITCH_LEFT_SERVO_PIN, "Pitch left servo PIN", "Pin the pitch left servo is on", SettingType::Number, PITCH_LEFT_SERVO_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
            {PITCH_LEFT_SERVO_CHANNEL, "Pitch left servo channel", "Timer channel the pitch left servo is on", SettingType::Number, PITCH_LEFT_SERVO_CHANNEL_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
            {PITCH_RIGHTSERVO_PIN, "Pitch right servo PIN", "Pin the pitch right servo is on", SettingType::Number, PITCH_RIGHTSERVO_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
            {PITCH_RIGHTSERVO_CHANNEL, "Pitch right servo channel", "Timer channel the pitch right servo is on", SettingType::Number, PITCH_RIGHTSERVO_CHANNEL_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
            {VALVE_SERVO_PIN, "Valve servo PIN", "Pin the valve servo is on", SettingType::Number, VALVE_SERVO_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
            {VALVE_SERVO_CHANNEL, "Valve servo channel", "Timer channel the valve servo is on", SettingType::Number, VALVE_SERVO_CHANNEL_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
            {TWIST_SERVO_PIN, "Twist servo PIN", "Pin the twist servo is on", SettingType::Number, TWIST_SERVO_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
            {TWIST_SERVO_CHANNEL, "Twist servo channel", "Timer channel the twist servo is on", SettingType::Number, TWIST_SERVO_CHANNEL_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
            {SQUEEZE_PIN, "Squeeze servo PIN", "Pin the squeeze servo is on", SettingType::Number, SQUEEZE_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
            {SQUEEZE_CHANNEL, "Squeeze servo channel", "Timer channel the squeeze servo is on", SettingType::Number, SQUEEZE_CHANNEL_DEFAULT, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
            {VIBE0_PIN, "Vibe1 PIN", "Pin the vibe 1 is on", SettingType::Number, VIBE0_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
            {VIBE0_CHANNEL, "Vibe1 channel", "Timer channel the vibe 1 is on", SettingType::Number, VIBE0_CHANNEL_DEFAULT, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
            {VIBE1_PIN, "Vibe2 PIN", "Pin the vibe 2 is on", SettingType::Number, VIBE1_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
            {VIBE1_CHANNEL, "Vibe2 channel", "Timer channel the vibe 2 is on", SettingType::Number, VIBE1_CHANNEL_DEFAULT, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
            {VIBE2_PIN, "Vibe3 PIN", "Pin the vibe 3 is on", SettingType::Number, VIBE2_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
            {VIBE2_CHANNEL, "Vibe3 channel", "Timer channel the vibe 3 is on", SettingType::Number, VIBE2_CHANNEL_DEFAULT, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
            {VIBE3_PIN, "Vibe4 PIN", "Pin the vibe 4 is on", SettingType::Number, VIBE3_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
            {VIBE3_CHANNEL, "Vibe4 channel", "Timer channel the vibe 4 is on", SettingType::Number, VIBE3_CHANNEL_DEFAULT, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
            {CASE_FAN_PIN, "Case fan PIN", "Pin the case fan is on", SettingType::Number, CASE_FAN_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
            {CASE_FAN_CHANNEL, "Case fan channel", "Timer channel the case fan is on", SettingType::Number, CASE_FAN_CHANNEL_DEFAULT, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
            {HEATER_PIN, "Heater PIN", "Pin the heater is on", SettingType::Number, HEATER_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature, SettingProfile::Pin, SettingProfile::PWM}},
            {HEATER_CHANNEL, "Heater channel", "Timer channel the heater is on", SettingType::Number, HEATER_CHANNEL_DEFAULT, RestartRequired::YES, {SettingProfile::Temperature, SettingProfile::Pin, SettingProfile::PWM}},
            // Analog
            {TWIST_FEEDBACK_PIN, "Twist feedback PIN", "The twist feedback pin", SettingType::Number, TWIST_FEEDBACK_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Pin}},
            {LUBE_BUTTON_PIN, "Lube button PIN", "Pin the lube button is on", SettingType::Number, LUBE_BUTTON_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
            {INTERNAL_TEMP_PIN, "Internal temp PIN", "Pin the internal temp sensor is on", SettingType::Number, INTERNAL_TEMP_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Analog, SettingProfile::Pin}},
            {DISPLAY_RST_PIN, "Display Rst PIN", "Reset pin for the display", SettingType::Number, DISPLAY_RST_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Display, SettingProfile::Pin}},
            {TEMP_PIN, "Temp pin", "Pin the sleeve temperture is on", SettingType::Number, TEMP_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Pin, SettingProfile::Analog}},
            {I2C_SDA_PIN, "I2C SDA PIN", "Pin of the I2C SDA", SettingType::Number, I2C_SDA_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::System, SettingProfile::Pin}},
            {I2C_SCL_PIN, "I2C SCL PIN", "Pin of the I2C SCL", SettingType::Number, I2C_SCL_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::System, SettingProfile::Pin}},
            {BUTTON_SET_PINS, "Button set pins", "Pins for each button set. (Max 4)", SettingType::ArrayInt, BUTTON_SET_PINS_DEFAULT, RestartRequired::YES, {SettingProfile::Pin, SettingProfile::Analog}},
            // BLDC
            {BLDC_ENCODER_PIN, "Encoder PIN", "Pin the BLDC encoder is on", SettingType::Number, BLDC_ENCODER_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin}},
            {BLDC_CHIPSELECT_PIN, "Chipselect PIN", "Pin the BLDC chip select is on", SettingType::Number, BLDC_CHIPSELECT_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin}},
            {BLDC_ENABLE_PIN, "Enable PIN", "Pin the BLDC enable is on", SettingType::Number, BLDC_ENABLE_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin}},
            {BLDC_HALLEFFECT_PIN, "Halleffect PIN", "Pin the hall effect is on", SettingType::Number, BLDC_HALLEFFECT_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin}},
            {BLDC_PWMCHANNEL1_PIN, "PWM channel1 PIN", "Pin for the BLDC PWM 1", SettingType::Number, BLDC_PWMCHANNEL1_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin, SettingProfile::PWM}},
            {BLDC_PWMCHANNEL2_PIN, "PWM channel2 PIN", "Pin for the BLDC PWM 2", SettingType::Number, BLDC_PWMCHANNEL2_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin, SettingProfile::PWM}},
            {BLDC_PWMCHANNEL3_PIN, "PWM channel3 PIN", "Pin for the BLDC PWM 3", SettingType::Number, BLDC_PWMCHANNEL3_PIN_DEFAULT, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin, SettingProfile::PWM}}
            #if CONFIG_IDF_TARGET_ESP32
            ,{ESP_H_TIMER0_FREQUENCY, "High timer 0 frequency", "Frequency for the high timer 0", SettingType::Number, ESP_TIMER_FREQUENCY_DEFAULT, RestartRequired::YES, {SettingProfile::Timer}}
            ,{ESP_H_TIMER1_FREQUENCY, "High timer 1 frequency", "Frequency for the high timer 1", SettingType::Number, ESP_TIMER_FREQUENCY_DEFAULT, RestartRequired::YES, {SettingProfile::Timer}}
            ,{ESP_H_TIMER2_FREQUENCY, "High timer 2 frequency", "Frequency for the high timer 2", SettingType::Number, ESP_TIMER_FREQUENCY_DEFAULT, RestartRequired::YES, {SettingProfile::Timer}}
            ,{ESP_H_TIMER3_FREQUENCY, "High timer 3 frequency", "Frequency for the high timer 3", SettingType::Number, ESP_TIMER_FREQUENCY_DEFAULT, RestartRequired::YES, {SettingProfile::Timer}} 
            #endif
            ,{ESP_L_TIMER0_FREQUENCY, "Low timer 0 frequency", "Frequency for the low timer 0", SettingType::Number, ESP_TIMER_FREQUENCY_DEFAULT, RestartRequired::YES, {SettingProfile::Timer}}
            ,{ESP_L_TIMER1_FREQUENCY, "Low timer 1 frequency", "Frequency for the low timer 1", SettingType::Number, ESP_TIMER_FREQUENCY_DEFAULT, RestartRequired::YES, {SettingProfile::Timer}}
            ,{ESP_L_TIMER2_FREQUENCY, "Low timer 2 frequency", "Frequency for the low timer 2", SettingType::Number, ESP_TIMER_FREQUENCY_DEFAULT, RestartRequired::YES, {SettingProfile::Timer}}
            ,{ESP_L_TIMER3_FREQUENCY, "Low timer 3 frequency", "Frequency for the low timer 3", SettingType::Number, ESP_TIMER_FREQUENCY_DEFAULT, RestartRequired::YES, {SettingProfile::Timer}}  
        }
    };


    SettingsFactory() {
        m_networkSemaphore = xSemaphoreCreateMutex();
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
    LogLevel logLevel;
    std::vector<const char*> logIncludes;
    std::vector<const char*> logExcludes;
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
    bool batteryLevelNumeric;
    int targetTemp;
    int HeatPWM;
    int HoldPWM;
    float heaterThreshold;
    double internalMaxTemp;
    double internalTempForFanOn;
    bool voiceMuted;
    int8_t voiceVolume;
    int8_t voiceWakeTime;
    int vibTimeout;
    bool vibTimeoutEnabled;

    bool load(SettingFileInfo &fileInfo)
    {
        LogHandler::info(m_TAG, "Loading file: %s", fileInfo.path);
        bool fileExists = LittleFS.exists(fileInfo.path);
        if(!fileExists)
        {
            LogHandler::info(m_TAG, "File %s did not exist", fileInfo.path);
            if(!createJsonFile(fileInfo.path))
                return false;
            if(!loadDefault(fileInfo.file))
                return false;
        } else {
            File file = LittleFS.open(fileInfo.path, FILE_READ, !fileExists);
            if(!file) {
                LogHandler::error(m_TAG, "%s failed to open!", fileInfo.path);
                return false;
            }
            if(LogDeserializationError(deserializeJson(fileInfo.doc, file), file.name())) {
                file.close();
                createJsonFile(fileInfo.path);
                return false;
            }
            file.close();
            fileInfo.initialized = true;
        }
        //json = doc.as<JsonObject>();
        return true;
    }
    
    // bool loadVector(SettingFileInfo &fileInfo, std::vector<const char*> vector)
    // {
    //     for (size_t i = 0; i < fileInfo.settings.size(); i++)
    //     {
    //         Setting setting = fileInfo.settings[i];
    //         if(setting.type == SettingType::ArrayString) 
    //         {
    //             JsonArray array = fileInfo.doc[setting.name];
    //             for (size_t j = 0; j < array.size(); j++)
    //             {
    //                 vector.push_back(array[j].as<const char*>());
    //             }
                
    //         } 
    //     }
        
    //     return true;
    // }

    bool loadDefault(SettingFile file) 
    {
        if(file == SettingFile::Network) 
        {
            return loadDefault(m_networkFileInfo);
        }
        if(file == SettingFile::Common) 
        {
            return loadDefault(m_commonFileInfo);
        }
        if(file == SettingFile::Pins) 
        {
            return loadDefaultPins();
        }
        LogHandler::error(m_TAG, "Unknown file loading default: %ld", (int)file);
        return false;
    }

    //template <unsigned int N>
    bool loadDefault(SettingFileInfo &fileInfo)
    {
        LogHandler::info(m_TAG, "Loading default: %s", fileInfo.path);
        for(const Setting& setting : fileInfo.settings)
        {
            defaultToJson(&setting, fileInfo.doc);
        }
        fileInfo.initialized = true;
        return saveToDisk(fileInfo);
    }

    bool loadDefaultVector(const Setting *setting, JsonDocument &doc) {
        
        if(!strcmp(setting->name, LOG_INCLUDETAGS)) {
            std::vector<const char*> includesVec;
            doc[LOG_INCLUDETAGS] = includesVec;
            loadCommonLiveCache(LOG_INCLUDETAGS);
            return true;
        } else if(!strcmp(setting->name, LOG_EXCLUDETAGS)) {
            std::vector<const char*> excludesVec;
            doc[LOG_EXCLUDETAGS] = excludesVec;
            loadCommonLiveCache(LOG_EXCLUDETAGS);
            return true;
        } else if(!strcmp(setting->name, BUTTON_SET_PINS)) {
            std::vector<int8_t> vec = { BUTTON_SET_PINS_1, BUTTON_SET_PINS_2, BUTTON_SET_PINS_3, BUTTON_SET_PINS_4 };
            doc[BUTTON_SET_PINS] = vec;
            return true;
        }
        LogHandler::error(m_TAG, "No default vector set for: %s", setting->name);
        return false;
    }

    bool loadDefaultPins() {
        BoardType boardType;
        getValue(BOARD_TYPE_SETTING, boardType);// Use setting from json
        switch(boardType)
        {
            case BoardType::CRIMZZON: {
                PinMapSR6MB* pinMap = PinMapSR6MB::getInstance();
                pinMap->overideDefaults();
                m_pinsFileInfo.initialized = true;
                syncSR6AndCommonPinsToDisk(pinMap);
                loadDefaultChannelsForDeviceType();
                return saveToDisk(m_pinsFileInfo);
            }
            break;
            case BoardType::ISAAC: {
                PinMapINControl* pinMap = PinMapINControl::getInstance();
                pinMap->overideDefaults();
                m_pinsFileInfo.initialized = true;
                syncSR6AndCommonPinsToDisk(pinMap);
                loadDefaultChannelsForDeviceType();
                return saveToDisk(m_pinsFileInfo);
            }
            break;
            default: {
                bool ret = loadDefault(m_pinsFileInfo);
                m_pinsFileInfo.initialized = true;
                loadDefaultChannelsForDeviceType();
                return saveToDisk(m_pinsFileInfo);
            }
        }
    }

    void loadDefaultChannelsForDeviceType() {
        DeviceType deviceType;
        getValue(DEVICE_TYPE, deviceType);
        switch(deviceType) {
            case DeviceType::OSR: {
                LogHandler::debug(m_TAG, "Loading default channels and pins for OSR");
#if CONFIG_IDF_TARGET_ESP32
                setValue(RIGHT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::HIGH0_CH0);
                setValue(LEFT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::HIGH0_CH1);
                setValue(RIGHT_UPPER_SERVO_PIN, -1);
                setValue(RIGHT_UPPER_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(LEFT_UPPER_SERVO_PIN, -1);
                setValue(LEFT_UPPER_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(PITCH_LEFT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::HIGH1_CH2);
                setValue(PITCH_RIGHTSERVO_PIN, -1);
                setValue(PITCH_RIGHTSERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(TWIST_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::HIGH1_CH3);
                setValue(VALVE_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::HIGH2_CH4);
                setValue(SQUEEZE_PIN, -1);
                setValue(SQUEEZE_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(VIBE0_CHANNEL, (int8_t)ESPTimerChannelNum::LOW0_CH0);
                setValue(VIBE1_CHANNEL, (int8_t)ESPTimerChannelNum::LOW0_CH1);
                setValue(VIBE2_CHANNEL, (int8_t)ESPTimerChannelNum::LOW1_CH2);
                setValue(VIBE3_CHANNEL, (int8_t)ESPTimerChannelNum::LOW1_CH3);
                setValue(ESP_L_TIMER0_FREQUENCY, ESP_VIB_TIMER_FREQUENCY_DEFAULT);
                setValue(ESP_L_TIMER1_FREQUENCY, ESP_VIB_TIMER_FREQUENCY_DEFAULT);
                setValue(CASE_FAN_CHANNEL, (int8_t)ESPTimerChannelNum::LOW2_CH4);
                setValue(HEATER_CHANNEL, (int8_t)ESPTimerChannelNum::LOW3_CH6);
#elif CONFIG_IDF_TARGET_ESP32S3
                setValue(RIGHT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::LOW0_CH0);
                setValue(LEFT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::LOW0_CH1);
                setValue(RIGHT_UPPER_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(LEFT_UPPER_SERVO_PIN, -1);
                setValue(LEFT_UPPER_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(PITCH_LEFT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::LOW1_CH2);
                setValue(PITCH_RIGHTSERVO_PIN, -1);
                setValue(PITCH_RIGHTSERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(TWIST_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::LOW1_CH3);
                setValue(VALVE_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::LOW2_CH5);
                setValue(SQUEEZE_PIN, -1);
                setValue(SQUEEZE_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(VIBE0_PIN, -1);
                setValue(VIBE0_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(VIBE1_PIN, -1);
                setValue(VIBE1_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(VIBE2_PIN, -1);
                setValue(VIBE2_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(VIBE3_PIN, -1);
                setValue(VIBE3_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(CASE_FAN_PIN, -1);
                setValue(CASE_FAN_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(HEATER_PIN, -1);
                setValue(HEATER_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
#endif
                break;
            }
            case DeviceType::SR6: {
                LogHandler::debug(m_TAG, "Loading default channels and pins for SR6");
#if CONFIG_IDF_TARGET_ESP32
                setValue(RIGHT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::HIGH0_CH0);
                setValue(LEFT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::HIGH0_CH1);
                setValue(RIGHT_UPPER_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::HIGH1_CH2);
                setValue(LEFT_UPPER_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::HIGH1_CH3);
                setValue(PITCH_LEFT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::HIGH2_CH4);
                setValue(PITCH_RIGHTSERVO_CHANNEL, (int8_t)ESPTimerChannelNum::HIGH2_CH5);
                setValue(TWIST_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::HIGH3_CH6);
                setValue(VALVE_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::HIGH3_CH7);
                setValue(SQUEEZE_PIN, -1);
                setValue(SQUEEZE_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(VIBE0_CHANNEL, (int8_t)ESPTimerChannelNum::LOW0_CH0);
                setValue(VIBE1_CHANNEL, (int8_t)ESPTimerChannelNum::LOW0_CH1);
                setValue(VIBE2_CHANNEL, (int8_t)ESPTimerChannelNum::LOW1_CH2);
                setValue(VIBE3_CHANNEL, (int8_t)ESPTimerChannelNum::LOW1_CH3);
                setValue(ESP_L_TIMER0_FREQUENCY, ESP_VIB_TIMER_FREQUENCY_DEFAULT);
                setValue(ESP_L_TIMER1_FREQUENCY, ESP_VIB_TIMER_FREQUENCY_DEFAULT);
                setValue(CASE_FAN_CHANNEL, (int8_t)ESPTimerChannelNum::LOW2_CH4);
                setValue(HEATER_CHANNEL, (int8_t)ESPTimerChannelNum::LOW3_CH6);
#elif CONFIG_IDF_TARGET_ESP32S3
                setValue(RIGHT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::LOW0_CH0);
                setValue(LEFT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::LOW0_CH1);
                setValue(RIGHT_UPPER_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::LOW1_CH2);
                setValue(LEFT_UPPER_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::LOW1_CH3);
                setValue(PITCH_LEFT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::LOW2_CH4);
                setValue(PITCH_RIGHTSERVO_CHANNEL, (int8_t)ESPTimerChannelNum::LOW2_CH5);
                setValue(TWIST_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::LOW3_CH6);
                setValue(VALVE_SERVO_PIN, -1);
                setValue(VALVE_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(SQUEEZE_PIN, -1);
                setValue(SQUEEZE_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(VIBE0_PIN, -1);
                setValue(VIBE0_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(VIBE1_PIN, -1);
                setValue(VIBE1_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(VIBE2_PIN, -1);
                setValue(VIBE2_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(VIBE3_PIN, -1);
                setValue(VIBE3_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(CASE_FAN_CHANNEL, (int8_t)ESPTimerChannelNum::LOW3_CH7);
                setValue(HEATER_PIN, -1);
                setValue(HEATER_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
#endif
                break;
            }
            case DeviceType::SSR1: {
                LogHandler::debug(m_TAG, "Loading default channels and pins for SSR1");
#if CONFIG_IDF_TARGET_ESP32
                setValue(RIGHT_SERVO_PIN, -1);
                setValue(RIGHT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(LEFT_SERVO_PIN, -1);
                setValue(LEFT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(RIGHT_UPPER_SERVO_PIN, -1);
                setValue(RIGHT_UPPER_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(LEFT_UPPER_SERVO_PIN, -1);
                setValue(LEFT_UPPER_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(PITCH_LEFT_SERVO_PIN, -1);
                setValue(PITCH_LEFT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(TWIST_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::HIGH0_CH0);
                setValue(VALVE_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::HIGH2_CH4);
                setValue(PITCH_RIGHTSERVO_CHANNEL, (int8_t)ESPTimerChannelNum::HIGH0_CH1);
                setValue(SQUEEZE_CHANNEL, (int8_t)ESPTimerChannelNum::HIGH1_CH2);
                setValue(VIBE0_CHANNEL, (int8_t)ESPTimerChannelNum::LOW0_CH0);
                setValue(VIBE1_CHANNEL, (int8_t)ESPTimerChannelNum::LOW0_CH1);
                setValue(VIBE2_CHANNEL, (int8_t)ESPTimerChannelNum::LOW1_CH2);
                setValue(VIBE3_CHANNEL, (int8_t)ESPTimerChannelNum::LOW1_CH3);
                setValue(ESP_L_TIMER0_FREQUENCY, ESP_VIB_TIMER_FREQUENCY_DEFAULT);
                setValue(ESP_L_TIMER1_FREQUENCY, ESP_VIB_TIMER_FREQUENCY_DEFAULT);
                setValue(CASE_FAN_CHANNEL, (int8_t)ESPTimerChannelNum::LOW2_CH4);
                setValue(HEATER_CHANNEL, (int8_t)ESPTimerChannelNum::LOW3_CH6);
#elif CONFIG_IDF_TARGET_ESP32S3
                setValue(RIGHT_SERVO_PIN, -1);
                setValue(RIGHT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(LEFT_SERVO_PIN, -1);
                setValue(LEFT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(RIGHT_UPPER_SERVO_PIN, -1);
                setValue(RIGHT_UPPER_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(LEFT_UPPER_SERVO_PIN, -1);
                setValue(LEFT_UPPER_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(PITCH_LEFT_SERVO_PIN, -1);
                setValue(PITCH_LEFT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(TWIST_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::LOW0_CH0);
                setValue(VALVE_SERVO_PIN, -1);
                setValue(VALVE_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(PITCH_RIGHTSERVO_CHANNEL, (int8_t)ESPTimerChannelNum::LOW0_CH1);
                setValue(SQUEEZE_PIN, -1);
                setValue(SQUEEZE_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(VIBE0_CHANNEL, (int8_t)ESPTimerChannelNum::LOW1_CH2);
                setValue(VIBE1_CHANNEL, (int8_t)ESPTimerChannelNum::LOW1_CH3);
                setValue(VIBE2_CHANNEL, (int8_t)ESPTimerChannelNum::LOW2_CH4);
                setValue(VIBE3_CHANNEL, (int8_t)ESPTimerChannelNum::LOW2_CH5);
                setValue(ESP_L_TIMER1_FREQUENCY, ESP_VIB_TIMER_FREQUENCY_DEFAULT);
                setValue(ESP_L_TIMER2_FREQUENCY, ESP_VIB_TIMER_FREQUENCY_DEFAULT);
                setValue(CASE_FAN_CHANNEL, (int8_t)ESPTimerChannelNum::LOW3_CH6);
                setValue(HEATER_CHANNEL, (int8_t)ESPTimerChannelNum::LOW3_CH7);
#endif
                break;
            }
            case DeviceType::TVIBE: {
                LogHandler::debug(m_TAG, "Loading default channels and pins for TVIBE");
                setValue(RIGHT_SERVO_PIN, -1);
                setValue(RIGHT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(LEFT_SERVO_PIN, -1);
                setValue(LEFT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(RIGHT_UPPER_SERVO_PIN, -1);
                setValue(RIGHT_UPPER_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(LEFT_UPPER_SERVO_PIN, -1);
                setValue(LEFT_UPPER_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(PITCH_LEFT_SERVO_PIN, -1);
                setValue(PITCH_LEFT_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(TWIST_SERVO_PIN, -1);
                setValue(TWIST_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(VALVE_SERVO_PIN, -1);
                setValue(VALVE_SERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(PITCH_RIGHTSERVO_PIN, -1);
                setValue(PITCH_RIGHTSERVO_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(SQUEEZE_PIN, -1);
                setValue(SQUEEZE_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
                setValue(VIBE0_CHANNEL, (int8_t)ESPTimerChannelNum::LOW0_CH0);
                setValue(VIBE1_CHANNEL, (int8_t)ESPTimerChannelNum::LOW0_CH1);
                setValue(VIBE2_CHANNEL, (int8_t)ESPTimerChannelNum::LOW1_CH2);
                setValue(VIBE3_CHANNEL, (int8_t)ESPTimerChannelNum::LOW1_CH3);
                setValue(ESP_L_TIMER0_FREQUENCY, ESP_VIB_TIMER_FREQUENCY_DEFAULT);
                setValue(ESP_L_TIMER1_FREQUENCY, ESP_VIB_TIMER_FREQUENCY_DEFAULT);
                setValue(CASE_FAN_CHANNEL, (int8_t)ESPTimerChannelNum::LOW2_CH4);
                setValue(HEATER_PIN, -1);
                setValue(HEATER_CHANNEL, (int8_t)ESPTimerChannelNum::NONE);
            }
        }
    }
            
    bool loadAllFromDisk()
    {
        for(SettingFileInfo* settingsInfo : AllSettings)
        {
            if(!load(*settingsInfo))
                return false;
        }
        return true;
    }

    bool loadCommonFromDisk()
    {
        xSemaphoreTake(m_commonSemaphore, portTICK_PERIOD_MS);
        auto ret = load(m_commonFileInfo);
        xSemaphoreGive(m_commonSemaphore);
        return ret;
    }
    bool loadWifiFromDisk()
    {
        xSemaphoreTake(m_networkSemaphore, portTICK_PERIOD_MS);
        bool ret = load(m_networkFileInfo);;
        xSemaphoreGive(m_networkSemaphore);
        return ret;
    }
    bool loadPinsFromDisk()
    {
        xSemaphoreTake(m_pinSemaphore, portTICK_PERIOD_MS);
        bool ret = load(m_pinsFileInfo);
        xSemaphoreGive(m_pinSemaphore);
        return ret;
    }

    //template <unsigned int N>
    bool saveToDisk(SettingFileInfo &fileInfo, JsonObject fromJson = JsonObject())
    {
        LogHandler::info(m_TAG, "Save file: %s", fileInfo.path);
        if(!fromJson.isNull()) {
            LogHandler::debug(m_TAG, "Saving from override json");
            fileInfo.doc.clear();
            fileInfo.doc.set(fromJson);
        }

        LogHandler::debug(m_TAG, "Doc overflowed: %u", fileInfo.doc.overflowed());
        //LogHandler::debug(m_TAG, "Doc memory: %u", fileInfo.doc.memoryUsage());
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
        file.close();
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

    void loadCommonCache() 
    {
        if(!m_commonFileInfo.initialized) {
            LogHandler::error(m_TAG, "loadCommonCache called before initialized");
            return;
        }
	    getValue(TCODE_VERSION_SETTING, tcodeVersion);
	    getValue(UDP_SERVER_PORT, udpServerPort);
	    getValue(WEBSERVER_PORT, webServerPort);
	    getValue(HOST_NAME, hostname, HOST_NAME_LEN);
	    getValue(FRIENDLY_NAME, friendlyName, FRIENDLY_NAME_LEN);
        getValue(DEVICE_TYPE, m_deviceType);
        getValue(BOARD_TYPE_SETTING, m_boardType);
        loadCommonLiveCache();
    }

    void loadPinCache() {
        if(!m_pinsFileInfo.initialized) {
            LogHandler::error(m_TAG, "loadPinCache called before initialized");
            return;
        }
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
    }

    void loadCommonPins(PinMap* pinMap) 
    {
        if(!m_pinsFileInfo.initialized) {
            LogHandler::error(m_TAG, "loadCommonPins called before initialized");
            return;
        }
        int8_t pin = -1;
        int8_t channel = -1;
        getValue(VALVE_SERVO_PIN, pin);
        pinMap->setValve(pin);
        getValue(VALVE_SERVO_CHANNEL, channel);
        pinMap->setValveChannel(channel);
        getValue(TWIST_SERVO_PIN, pin);
        pinMap->setTwist(pin);
        getValue(TWIST_SERVO_CHANNEL, channel);
        pinMap->setTwistChannel(channel);
        getValue(SQUEEZE_PIN, pin);
        pinMap->setSqueeze(pin);
        getValue(SQUEEZE_CHANNEL, channel);
        pinMap->setSqueezeChannel(channel);
        getValue(VIBE0_PIN, pin);
        pinMap->setVibe0(pin);
        getValue(VIBE0_CHANNEL, channel);
        pinMap->setVibe0Channel(channel);
        getValue(VIBE1_PIN, pin);
        pinMap->setVibe1(pin);
        getValue(VIBE1_CHANNEL, channel);
        pinMap->setVibe1Channel(channel);
        getValue(VIBE2_PIN, pin);
        pinMap->setVibe2(pin);
        getValue(VIBE2_CHANNEL, channel);
        pinMap->setVibe2Channel(channel);
        getValue(VIBE3_PIN, pin);
        pinMap->setVibe3(pin);
        getValue(VIBE3_CHANNEL, channel);
        pinMap->setVibe3Channel(channel);
        getValue(CASE_FAN_PIN, pin);
        pinMap->setCaseFan(pin);
        getValue(CASE_FAN_CHANNEL, channel);
        pinMap->setCaseFanChannel(channel);
        getValue(HEATER_PIN, pin);
        pinMap->setHeater(pin);
        getValue(HEATER_CHANNEL, channel);
        pinMap->setHeaterChannel(channel);

        getValue(TWIST_FEEDBACK_PIN, pin);
        pinMap->setTwistFeedBack(pin);
        getValue(LUBE_BUTTON_PIN, pin);
        pinMap->setLubeButton(pin);
        getValue(INTERNAL_TEMP_PIN, pin);
        pinMap->setInternalTemp(pin);
        getValue(DISPLAY_RST_PIN, pin);
        pinMap->setDisplayReset(pin);
        getValue(TEMP_PIN, pin);
        pinMap->setSleeveTemp(pin);
        getValue(I2C_SDA_PIN, pin);
        pinMap->setI2cSda(pin);
        getValue(I2C_SCL_PIN, pin);
        pinMap->setI2cScl(pin);
        std::vector<int> vec;
        getValueVector(BUTTON_SET_PINS, vec);
        for (size_t i = 0; i < vec.size(); i++)
        {
            pinMap->setButtonSetPin(vec[i], i);
        }

        int timerFreq = -1;
#if CONFIG_IDF_TARGET_ESP32
        getValue(ESP_H_TIMER0_FREQUENCY, timerFreq);
        pinMap->setTimerFrequency(0, timerFreq);
        getValue(ESP_H_TIMER1_FREQUENCY, timerFreq);
        pinMap->setTimerFrequency(1, timerFreq);
        getValue(ESP_H_TIMER2_FREQUENCY, timerFreq);
        pinMap->setTimerFrequency(2, timerFreq);
        getValue(ESP_H_TIMER3_FREQUENCY, timerFreq);
        pinMap->setTimerFrequency(3, timerFreq);
        getValue(ESP_L_TIMER0_FREQUENCY, timerFreq);
        pinMap->setTimerFrequency(4, timerFreq);
        getValue(ESP_L_TIMER1_FREQUENCY, timerFreq);
        pinMap->setTimerFrequency(5, timerFreq);
        getValue(ESP_L_TIMER2_FREQUENCY, timerFreq);
        pinMap->setTimerFrequency(6, timerFreq);
        getValue(ESP_L_TIMER3_FREQUENCY, timerFreq);
        pinMap->setTimerFrequency(7, timerFreq);
#elif CONFIG_IDF_TARGET_ESP32S3
        getValue(ESP_L_TIMER0_FREQUENCY, timerFreq);
        pinMap->setTimerFrequency(0, timerFreq);
        getValue(ESP_L_TIMER1_FREQUENCY, timerFreq);
        pinMap->setTimerFrequency(1, timerFreq);
        getValue(ESP_L_TIMER2_FREQUENCY, timerFreq);
        pinMap->setTimerFrequency(2, timerFreq);
        getValue(ESP_L_TIMER3_FREQUENCY, timerFreq);
        pinMap->setTimerFrequency(3, timerFreq);
#endif
    }
    PinMapSSR1* loadSSR1Pins() 
    {
        if(!m_pinsFileInfo.initialized) {
            LogHandler::error(m_TAG, "loadSSR1Pins called before initialized");
            return 0;
        }
        PinMapSSR1* pinMap = PinMapSSR1::getInstance();
        loadCommonPins(pinMap);
        int8_t pin = -1;
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
        if(!m_pinsFileInfo.initialized) {
            LogHandler::error(m_TAG, "loadSSR1Pins called before initialized");
            return 0;
        }
        PinMapOSR* pinMap = PinMapOSR::getInstance();
        loadCommonPins(pinMap);
        int8_t pin = -1;
        int8_t channel = -1;
        getValue(RIGHT_SERVO_PIN, pin);
        pinMap->setRightServo(pin);
        getValue(RIGHT_SERVO_CHANNEL, channel);
        pinMap->setRightServoChannel(channel);
        getValue(LEFT_SERVO_PIN, pin);
        pinMap->setLeftServo(pin);
        getValue(LEFT_SERVO_CHANNEL, channel);
        pinMap->setLeftServoChannel(channel);
        getValue(PITCH_LEFT_SERVO_PIN, pin);
        pinMap->setPitchLeft(pin);
        getValue(PITCH_LEFT_SERVO_CHANNEL, channel);
        pinMap->setPitchLeftChannel(channel);
        return pinMap;
    }
    
    PinMapSR6* loadSR6Pins() 
    {
        if(!m_pinsFileInfo.initialized) {
            LogHandler::error(m_TAG, "loadSR6Pins called before initialized");
            return 0;
        }
        PinMapSR6* pinMap = PinMapSR6::getInstance();
        loadCommonPins(pinMap);
        int8_t pin = -1;
        int8_t channel = -1;
        getValue(RIGHT_SERVO_PIN, pin);
        pinMap->setRightServo(pin);
        getValue(RIGHT_SERVO_CHANNEL, channel);
        pinMap->setRightServoChannel(channel);
        getValue(LEFT_SERVO_PIN, pin);
        pinMap->setLeftServo(pin);
        getValue(LEFT_SERVO_CHANNEL, channel);
        pinMap->setLeftServoChannel(channel);
        getValue(PITCH_LEFT_SERVO_PIN, pin);
        pinMap->setPitchLeft(pin);
        getValue(PITCH_LEFT_SERVO_CHANNEL, channel);
        pinMap->setPitchLeftChannel(channel);
        getValue(PITCH_RIGHTSERVO_PIN, pin);
        pinMap->setPitchRight(pin);
        getValue(PITCH_RIGHTSERVO_CHANNEL, channel);
        pinMap->setPitchRightChannel(channel);
        getValue(RIGHT_UPPER_SERVO_PIN, pin);
        pinMap->setRightUpperServo(pin);
        getValue(RIGHT_UPPER_SERVO_CHANNEL, channel);
        pinMap->setRightUpperServoChannel(channel);
        getValue(LEFT_UPPER_SERVO_PIN, pin);
        pinMap->setLeftUpperServo(pin);
        getValue(LEFT_UPPER_SERVO_CHANNEL, channel);
        pinMap->setLeftUpperServoChannel(channel);
        return pinMap;
    }
    
    void syncCommonPinsToDoc(const PinMap* pinMap) 
    {
        if(!m_pinsFileInfo.initialized) {
            LogHandler::error(m_TAG, "syncCommonPinsToDoc called before initialized");
            return;
        }
        setValue(VALVE_SERVO_PIN, pinMap->valve());
        setValue(VALVE_SERVO_CHANNEL, pinMap->valveChannel());
        setValue(TWIST_SERVO_PIN, pinMap->twist());
        setValue(TWIST_SERVO_CHANNEL, pinMap->twistChannel());
        setValue(SQUEEZE_PIN, pinMap->squeeze());
        setValue(SQUEEZE_CHANNEL, pinMap->squeezeChannel());
        setValue(VIBE0_PIN, pinMap->vibe0());
        setValue(VIBE0_CHANNEL, pinMap->vibe0Channel());
        setValue(VIBE1_PIN, pinMap->vibe1());
        setValue(VIBE1_CHANNEL, pinMap->vibe1Channel());
        setValue(VIBE2_PIN, pinMap->vibe2());
        setValue(VIBE2_CHANNEL, pinMap->vibe2Channel());
        setValue(VIBE3_PIN, pinMap->vibe3());
        setValue(VIBE3_CHANNEL, pinMap->vibe3Channel());
        setValue(CASE_FAN_PIN, pinMap->caseFan());
        setValue(CASE_FAN_CHANNEL, pinMap->caseFanChannel());
        setValue(HEATER_PIN, pinMap->heater());
        setValue(HEATER_CHANNEL, pinMap->heaterChannel());

        setValue(TWIST_FEEDBACK_PIN, pinMap->twistFeedBack());
        setValue(LUBE_BUTTON_PIN, pinMap->lubeButton());
        setValue(INTERNAL_TEMP_PIN, pinMap->internalTemp());
        setValue(DISPLAY_RST_PIN, pinMap->displayReset());
        setValue(TEMP_PIN, pinMap->sleeveTemp());
        setValue(I2C_SDA_PIN, pinMap->i2cSda());
        setValue(I2C_SCL_PIN, pinMap->i2cScl());
#if CONFIG_IDF_TARGET_ESP32
        setValue(ESP_H_TIMER0_FREQUENCY, pinMap->getTimerFrequency(0));
        setValue(ESP_H_TIMER1_FREQUENCY, pinMap->getTimerFrequency(1));
        setValue(ESP_H_TIMER2_FREQUENCY, pinMap->getTimerFrequency(2));
        setValue(ESP_H_TIMER3_FREQUENCY, pinMap->getTimerFrequency(3));
        setValue(ESP_L_TIMER0_FREQUENCY, pinMap->getTimerFrequency(4));
        setValue(ESP_L_TIMER1_FREQUENCY, pinMap->getTimerFrequency(5));
        setValue(ESP_L_TIMER2_FREQUENCY, pinMap->getTimerFrequency(6));
        setValue(ESP_L_TIMER3_FREQUENCY, pinMap->getTimerFrequency(7));
#elif CONFIG_IDF_TARGET_ESP32S3
        setValue(ESP_L_TIMER0_FREQUENCY, pinMap->getTimerFrequency(0));
        setValue(ESP_L_TIMER1_FREQUENCY, pinMap->getTimerFrequency(1));
        setValue(ESP_L_TIMER2_FREQUENCY, pinMap->getTimerFrequency(2));
        setValue(ESP_L_TIMER3_FREQUENCY, pinMap->getTimerFrequency(3));
#endif
    }

    void syncSSR1AndCommonPinsToDisk(const PinMapSSR1* pinMap) 
    {
        if(!m_pinsFileInfo.initialized) {
            LogHandler::error(m_TAG, "syncSSR1AndCommonPinsToDisk called before initialized");
            return;
        }
        syncCommonPinsToDoc(pinMap);
        setValue(BLDC_ENCODER_PIN, pinMap->encoder());
        setValue(BLDC_CHIPSELECT_PIN, pinMap->chipSelect());
        setValue(BLDC_ENABLE_PIN, pinMap->enable());
        setValue(BLDC_HALLEFFECT_PIN, pinMap->hallEffect());
        setValue(BLDC_PWMCHANNEL1_PIN, pinMap->pwmChannel1());
        setValue(BLDC_PWMCHANNEL2_PIN, pinMap->pwmChannel2());
        setValue(BLDC_PWMCHANNEL3_PIN, pinMap->pwmChannel3());
        savePins();
    }

    void syncOSRAndCommonPinsToDisk(const PinMapOSR* pinMap) 
    {
        if(!m_pinsFileInfo.initialized) {
            LogHandler::error(m_TAG, "syncOSRAndCommonPinsToDisk called before initialized");
            return;
        }
        syncCommonPinsToDoc(pinMap);
        setValue(RIGHT_SERVO_PIN, pinMap->rightServo());
        setValue(RIGHT_SERVO_CHANNEL, pinMap->rightServoChannel());
        setValue(LEFT_SERVO_PIN, pinMap->leftServo());
        setValue(LEFT_SERVO_CHANNEL, pinMap->leftServoChannel());
        setValue(PITCH_LEFT_SERVO_PIN, pinMap->pitchLeft());
        setValue(PITCH_LEFT_SERVO_CHANNEL, pinMap->pitchLeftChannel());
        savePins();
    }

    void syncSR6AndCommonPinsToDisk(const PinMapSR6* pinMap) 
    {
        if(!m_pinsFileInfo.initialized) {
            LogHandler::error(m_TAG, "syncSR6AndCommonPinsToDisk called before initialized");
            return;
        }
        syncCommonPinsToDoc(pinMap);
        setValue(RIGHT_SERVO_PIN, pinMap->rightServo());
        setValue(RIGHT_SERVO_CHANNEL, pinMap->rightServoChannel());
        setValue(LEFT_SERVO_PIN, pinMap->leftServo());
        setValue(LEFT_SERVO_CHANNEL, pinMap->leftServoChannel());
        setValue(PITCH_LEFT_SERVO_PIN, pinMap->pitchLeft());
        setValue(PITCH_LEFT_SERVO_CHANNEL, pinMap->pitchLeftChannel());
        setValue(PITCH_RIGHTSERVO_PIN, pinMap->pitchRight());
        setValue(PITCH_RIGHTSERVO_CHANNEL, pinMap->pitchRightChannel());
        setValue(RIGHT_UPPER_SERVO_PIN, pinMap->rightUpperServo());
        setValue(RIGHT_UPPER_SERVO_CHANNEL, pinMap->rightUpperServoChannel());
        setValue(LEFT_UPPER_SERVO_PIN, pinMap->leftUpperServo());
        setValue(LEFT_UPPER_SERVO_CHANNEL, pinMap->leftUpperServoChannel());
        savePins();
    }

    void initMessages() {
        for(SettingFileInfo* settingsInfo : AllSettings)
        {
            for(const Setting& setting : settingsInfo->settings)
            {
                sendMessage(setting.profiles.front(), setting.name);
            }
        }
    }
    void initCommonMessages(const char* name = 0) {
        if(name) {
            const Setting* setting = m_commonFileInfo.getSetting(name);
            if(!setting) {
                return;
            }
            sendMessage(setting->profiles.front(), name);
        } else {
            for(const Setting& setting : m_commonFileInfo.settings)
            {
                sendMessage(setting.profiles.front(), setting.name);
            }
        }
    }
    void initWifiMessages(const char* name = 0) {
        if(name) {
            const Setting* setting = m_networkFileInfo.getSetting(name);
            if(!setting) {
                return;
            }
            sendMessage(setting->profiles.front(), name);
        } else {
            for(const Setting& setting : m_networkFileInfo.settings)
            {
                sendMessage(setting.profiles.front(), setting.name);
            }
        }
    }
    void initPinsMessages(const char* name = 0) {
        if(name) {
            const Setting* setting = m_pinsFileInfo.getSetting(name);
            if(!setting) {
                return;
            }
            sendMessage(setting->profiles.front(), name);
        } else {
            for(const Setting& setting : m_pinsFileInfo.settings)
            {
                sendMessage(setting.profiles.front(), setting.name);
            }
        }
    }

    // void toJsonVector(const Setting *setting, const std::vector<int> &value, JsonDocument &doc) 
    // {
    //     doc[setting->name] = value;
    //     // JsonArray array = doc[setting->name].add<JsonArray>();
    //     // for (size_t i = 0; i < value.size(); i++)
    //     // {
    //     //     array.add(value[i]);
    //     // }
    // }
    // void toJsonVector(const Setting *setting, const std::vector<const char*> &value, JsonDocument &doc) 
    // {
    //     doc[setting->name] = value;
    //     // JsonArray array = doc[setting->name].add<JsonArray>();
    //     // for (size_t i = 0; i < value.size(); i++)
    //     // {
    //     //     array.add(value[i]);
    //     // }
    // }
    
    template<typename T,
             typename = 
                std::enable_if
                <
                    !std::is_const<T>::value || 
                    std::is_integral<T>::value || 
                    std::is_floating_point<T>::value || 
                    std::is_same<T, bool>::value 
                    // || std::is_same<T, std::vector<const char*>>::value 
                    // || std::is_same<T, std::vector<const int>>::value
                >
            >
    void toJson(const Setting *setting, JsonDocument &doc, T value) {
        switch(setting->type)
        {
            case SettingType::Boolean: {
                doc[setting->name] = static_cast<bool>(value);
                LogHandler::verbose(m_TAG, "Load bool: %s, value: %ld", setting->name, doc[setting->name].as<bool>());
            }
            break;
            case SettingType::Number: {
                doc[setting->name] = static_cast<int>(value);
                LogHandler::verbose(m_TAG, "Load number: %s, value: %ld", setting->name, doc[setting->name].as<int>());
            }
            break;
            case SettingType::Double: {
                doc[setting->name] = static_cast<double>(value);
                LogHandler::verbose(m_TAG, "Load double: %s, value: %f", setting->name, doc[setting->name].as<double>());
            }
            break;
            case SettingType::Float: {
                doc[setting->name] = static_cast<float>(value);
                LogHandler::verbose(m_TAG, "Load float: %s, value: %f", setting->name, doc[setting->name].as<float>());
            }
            break;
            case SettingType::String: {
                doc[setting->name] = static_cast<char*>(value);
                LogHandler::verbose(m_TAG, "Load string: %s, value: %s", setting->name, doc[setting->name].as<const char*>());
            }
            break;
            // case SettingType::ArrayString: {
            //     doc[setting->name] = static_cast<const std::vector<const char*>>(value);
            //     LogHandler::verbose(m_TAG, "Load array string: %s, value size: %ld", setting->name, doc[setting->name].as<const std::vector<const char*>>().size());
            // }
            // break;
            // case SettingType::ArrayInt: {
            //     doc[setting->name] = static_cast<const std::vector<int>>(value);
            //     LogHandler::verbose(m_TAG, "Load array int: %s, value size: %ld", setting->name, doc[setting->name].as<const std::vector<const int>>().size());
            // }
            break;
        }
    }
    void defaultToJson(const Setting *setting, JsonDocument &doc) {
        switch(setting->type)
        {
            case SettingType::Boolean: {
                doc[setting->name] = mpark::get<const bool>(setting->defaultValue);
                LogHandler::verbose(m_TAG, "Load default bool: %s, value: %ld", setting->name, doc[setting->name].as<bool>());
            }
            break;
            case SettingType::Number: {
                doc[setting->name] = mpark::get<const int>(setting->defaultValue);
                LogHandler::verbose(m_TAG, "Load default number: %s, value: %ld", setting->name, doc[setting->name].as<int>());
            }
            break;
            case SettingType::Double: {
                doc[setting->name] = mpark::get<const double>(setting->defaultValue);
                LogHandler::verbose(m_TAG, "Load default double: %s, value: %f", setting->name, doc[setting->name].as<double>());
            }
            break;
            case SettingType::Float: {
                doc[setting->name] = mpark::get<const float>(setting->defaultValue);
                LogHandler::verbose(m_TAG, "Load default float: %s, value: %f", setting->name, doc[setting->name].as<float>());
            }
            break;
            case SettingType::String: {
                doc[setting->name] = mpark::get<const char*>(setting->defaultValue);
                LogHandler::verbose(m_TAG, "Load default string: %s, value: %s", setting->name, doc[setting->name].as<const char*>());
            }
            break;
            case SettingType::ArrayString: {
                loadDefaultVector(setting, doc);
                LogHandler::verbose(m_TAG, "Load default string vector: %s, value size: %ld", setting->name, doc[setting->name].as<JsonArray>().size());
            }
            break;
            case SettingType::ArrayInt: {
                loadDefaultVector(setting, doc);
                LogHandler::verbose(m_TAG, "Load default int vector: %s, value size: %ld", setting->name, doc[setting->name].as<JsonArray>().size());
            }
            break;
            case SettingType::NONE: 
            break;
            case SettingType::MAX: 
            break;
        }
    }
    
    void sendMessage(const SettingProfile &profile, const char *message)
    {
        if (message_callback)
        {
            //LogHandler::debug(m_TAG, "sendMessage: message_callback profile %ld: %s", (int)profile, message);
            message_callback(profile, message);
        }
        else
        {
            LogHandler::warning(m_TAG, "sendMessage: message_callback 0");
        }
    }

    bool createJsonFile(const char* path) {
        LogHandler::info(m_TAG, "Creating file %s", path);
        if(LittleFS.exists(path)) {
            LogHandler::info(m_TAG, "File exists, Deleting file %s", path);
            if(!LittleFS.remove(path)) {
                LogHandler::error(m_TAG, "Error deleting %s!", path);
                return false;
            }
        }
        File newFile = LittleFS.open(path, FILE_WRITE, true);
        if(!newFile) {
            LogHandler::error(m_TAG, "Error creating %s!", path);
            return false;
        }
        newFile.print("{}");
        newFile.flush();
        newFile.close();
        return true;
    }
    
    bool deleteJsonFile(const char* path) {
        if(LittleFS.exists(path)) {
            LogHandler::info(m_TAG, "Deleting file %s", path);
            if(!LittleFS.remove(path)) {
                LogHandler::error(m_TAG, "Error deleting %s!", path);
                return false;
            }
        }
        return true;
    }
};
