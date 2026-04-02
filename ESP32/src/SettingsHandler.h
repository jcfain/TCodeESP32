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

#include <sstream>
#include <WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include <Wire.h>
#include "soc/rtc.h"
// // #include "LogHandler.h"
#include "utils.h"
#include "TagHandler.h"
#include "struct/voice.h"
#include "struct/motionProfile.h"
#include "struct/channel.h"
#include "struct/motionChannel.h"
#include "struct/buttonSet.h"
#include "enum.h"
#include "constants.h"
#include "channelMap.hpp"
#include "settingConstants.h"
#include "settingsFactory.h"
#include "callback.h"

#define DESERIALIZE_SIZE 32768
#define SERIALIZE_SIZE 24576

//using SETTING_STATE_FUNCTION_PTR_T = void (*)(const char *group, const char *settingNameThatChanged);

class SettingsHandler
{
public:
    static bool initialized;
    static int restartInSecs;
    static bool saving;
    static bool motionPaused;
    static bool fullBuild;
    static inline bool channelRangesEnabled = true;
    static LogLevel logLevel;
    static std::vector<int> systemI2CAddresses;

    static ChannelMap channelMap;
    static BuildFeature buildFeatures[(int)BuildFeature::MAX_FEATURES];

    static inline MotionProfile* motionProfiles;
    static inline ButtonSet* buttonSets;
    
    // static bool staticIP;
    static char currentIP[IP_ADDRESS_LEN];
    static char currentGateway[IP_ADDRESS_LEN];
    static char currentSubnet[IP_ADDRESS_LEN];
    static char currentDns1[IP_ADDRESS_LEN];
    static char currentDns2[IP_ADDRESS_LEN];

    static bool apMode;

    static void init()
    {
        m_settingsFactory = SettingsFactory::getInstance();
        motionProfiles = m_settingsFactory->getMotionProfiles();
        buttonSets = m_settingsFactory->getButtonSets();
        setBuildFeatures();
        setMotorType();

        // loadWifiInfo(false);
        // loadSettings(false);
        loadChannels(false);
        loadMotionProfiles(false);
        loadButtons(false);


        LogHandler::debug(_TAG, "Last reset reason: %s", machine_reset_cause());
        initialized = true;
    }

    static void setMessageCallback(SettingsChangeCallback f)
    {
        LogHandler::debug(_TAG, "setMessageCallback");
        if (f == nullptr)
        {
            message_callback = 0;
        }
        else
        {
            message_callback = f;
        }
    }

    static void printFree(bool forcePrint = false) {
        if(forcePrint || LogHandler::getLogLevel() == LogLevel::DEBUG)
        {
            uint32_t freeHEap = ESP.getFreeHeap();
            uint32_t heapSize = ESP.getHeapSize();
            //https://esp32.com/viewtopic.php?t=27780
            //https://github.com/espressif/esp-idf/blob/master/components/heap/include/esp_heap_caps.h#L20-L37
            //esp_get_free_internal_heap_size
            Serial.printf("Used heap INTERNAL: %u/%u Free: %u\n", heapSize - freeHEap, heapSize, freeHEap);
            Serial.printf("Free psram: %u\n", ESP.getFreePsram());
            Serial.printf("Total Psram: %u\n", ESP.getPsramSize());
            Serial.printf("LittleFS used: %i\n", LittleFS.usedBytes());
            Serial.printf("LittleFS total: %i\n", LittleFS.totalBytes());
            //LogHandler::debug(_TAG, "Used Psram: %u/%u", ESP.getPsramSize() - ESP.getFreePsram(), ESP.getPsramSize());
            Serial.printf("Sketch size: %u\n", ESP.getSketchSize());
            Serial.printf("Sketch free space: %u\n", ESP.getFreeSketchSpace());
            Serial.printf("DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
            Serial.printf("IRAM %u\n", heap_caps_get_free_size(MALLOC_CAP_32BIT));
            Serial.printf("FREE_HEAP Default %u\n", esp_get_free_heap_size());
            Serial.printf("MIN_FREE_HEAP %u\n", esp_get_minimum_free_heap_size() );
            //uxTaskGetStackHighWaterMark
        }
    }

	static void restart(const int delayInSec = 0) {
		LogHandler::info(_TAG, "Schedule device restart in %ld seconds", delayInSec);
        // Restart in main task loop
		restartInSecs = delayInSec;
	}

    static void printWebAddress(const char* hostAddress) 
    {
        char webServerportString[6];
        int webServerPort = 0;
        m_settingsFactory->getValue(WEBSERVER_PORT, webServerPort);
        sprintf(webServerportString, ":%d", webServerPort);
        LogHandler::info(_TAG, "Web address: http://%s%s", hostAddress, webServerPort == 80 ? "" : webServerportString);
    }
    
    static bool saveAll(JsonObject obj = JsonObject()) 
    {
        if(!m_settingsFactory->saveAllToDisk(obj) || !saveMotionProfiles(obj) || !saveButtons(obj))
            return false;
        return true;
    }
    
    static bool saveAll(const String& data)
    {
        LogHandler::debug(_TAG, "Save frome string");
        printFree();
        JsonDocument doc;
    
        DeserializationError error = deserializeJson(doc, data);
        if (error)
        {
            LogHandler::error(_TAG, "Settings save: Deserialize error: %s", error.c_str());
            return false;
        }
        printFree();
        JsonObject obj = doc.as<JsonObject>();
        if (!saveAll(obj))
        {
            LogHandler::error(_TAG, "Settings save: save error");
            return false;
        }
        return true;
    }

    static void getWifiInfo(char* buf)
    {
        JsonDocument doc; //100

        JsonDocument wifiDoc = m_settingsFactory->getNetworkSettings();

        doc.set(wifiDoc);
        const char* wifiPass = doc[WIFI_PASS_SETTING];
        if(strcmp(wifiPass, WIFI_PASS_DONOTCHANGE_DEFAULT)) {
            doc[WIFI_PASS_SETTING] = DECOY_PASS; // Never set to actual password
        } else {
            doc[WIFI_PASS_SETTING] = WIFI_PASS_DONOTCHANGE_DEFAULT;
        }
        const char* apPass = doc[AP_MODE_PASS];
        if(strcmp(apPass, AP_MODE_PASS_DEFAULT)) {
            doc[AP_MODE_PASS] = DECOY_PASS; // Never set to actual password
        } else {
            doc[AP_MODE_PASS] = AP_MODE_PASS_DEFAULT;
        }

        String output;
        serializeJson(doc, output);
        doc.clear();
        if (LogHandler::getLogLevel() == LogLevel::VERBOSE)
            Serial.printf("Network Info: %s\n", output.c_str());
        buf[0] = {0};
        strcpy(buf, output.c_str());
    }

    static void getSystemInfo(String &buf)
    {
        JsonDocument doc; // 3500

        doc["esp32Version"] = FIRMWARE_VERSION_NAME;
        doc["esp32VersionNum"] = FIRMWARE_VERSION;
        doc["TCodeVersion"] = m_settingsFactory->getTcodeVersion();
        doc["lastRebootReason"] = machine_reset_cause();
        doc["channelRangesEnabled"] = getChannelRangesEnabled();

        JsonArray logLevels = doc["logLevels"].to<JsonArray>();
        JsonObject logLevelNone = logLevels.add<JsonObject>();
        logLevelNone["name"] = "None";
        logLevelNone["value"] = LogLevel::NONE;
        JsonObject logLevelError= logLevels.add<JsonObject>();
        logLevelError["name"] = "Error";
        logLevelError["value"] = LogLevel::ERROR;
        JsonObject logLevelWarning = logLevels.add<JsonObject>();
        logLevelWarning["name"] = "Warning";
        logLevelWarning["value"] = LogLevel::WARNING;
        JsonObject logLevelInfo = logLevels.add<JsonObject>();
        logLevelInfo["name"] = "info";
        logLevelInfo["value"] = LogLevel::INFO;
        JsonObject logLevelDebug = logLevels.add<JsonObject>();
        logLevelDebug["name"] = "Debug";
        logLevelDebug["value"] = LogLevel::DEBUG;
        JsonObject logLevelVerbose = logLevels.add<JsonObject>();
        logLevelVerbose["name"] = "Verbose";
        logLevelVerbose["value"] = LogLevel::VERBOSE;
        
        JsonArray tcodeVersions = doc["tcodeVersions"].to<JsonArray>();
        JsonObject v03 = tcodeVersions.add<JsonObject>();
        v03["name"] = "v0.3";
        v03["value"] = TCodeVersion::v0_3;
        // JsonObject v04 = tcodeVersions.add<JsonObject>();
        // v04["name"] = "v0.4 (Experimental)";
        // v04["value"] = TCodeVersion::v0_4;
        JsonArray boardTypes = doc["boardTypes"].to<JsonArray>();
#if CONFIG_IDF_TARGET_ESP32
        JsonObject devkit = boardTypes.add<JsonObject>();
        devkit["name"] = "Devkit";
        devkit["value"] = (uint8_t)BoardType::DEVKIT;
    #ifdef MOTOR_TYPE_SERVO
        JsonObject SR6MB = boardTypes.add<JsonObject>();
        SR6MB["name"] = "SR6MB";
        SR6MB["value"] = (uint8_t)BoardType::CRIMZZON;
        JsonObject INControl = boardTypes.add<JsonObject>();
        INControl["name"] = "IN-Control";
        INControl["value"] = (uint8_t)BoardType::ISAAC;
    #elif defined MOTOR_TYPE_BLDC
        JsonObject SSR1PCB = boardTypes.add<JsonObject>();
        SSR1PCB["name"] = "SSR1PCB";
        SSR1PCB["value"] = (uint8_t)BoardType::SSR1PCB;
    #endif
#elif CONFIG_IDF_TARGET_ESP32S3
    #ifdef S3_ZERO
        JsonObject S3_Zero = boardTypes.add<JsonObject>();
        S3_Zero["name"] = "S3 Zero";
        S3_Zero["value"] = (uint8_t)BoardType::ZERO;
    #else
        JsonObject N8R8 = boardTypes.add<JsonObject>();
        N8R8["name"] = "S3 N8R8";
        N8R8["value"] = (uint8_t)BoardType::N8R8;
    #endif
#endif
        int motorType = MOTOR_TYPE_DEFAULT;
        m_settingsFactory->getValue(MOTOR_TYPE_SETTING, motorType);
        doc["motorType"] = motorType;
        JsonArray buildFeaturesJsonArray = doc["buildFeatures"].to<JsonArray>();
        for (BuildFeature value : buildFeatures)
        {
            buildFeaturesJsonArray.add((int)value);
        }
        doc["moduleType"] = (int)MODULE_CURRENT;

        JsonArray availableTagsJsonArray = doc["availableTags"].to<JsonArray>();
        for (const char *tag : TagHandler::AvailableTags)
        {
            availableTagsJsonArray.add(tag);
        }
        JsonArray systemI2CAddressesJsonArray = doc["systemI2CAddresses"].to<JsonArray>();
        systemI2CAddressesJsonArray.add("0x0");
        for (int value : systemI2CAddresses) {
			char buf[10];
            hexToString(value, buf);
            systemI2CAddressesJsonArray.add(buf);
        }

        int deviceType = DEVICE_TYPE_DEFAULT;
        m_settingsFactory->getValue(DEVICE_TYPE, deviceType);
        JsonArray deviceTypes = doc["deviceTypes"].to<JsonArray>();
        JsonObject defaultDevice = deviceTypes.add<JsonObject>();
    #ifdef MOTOR_TYPE_SERVO
        defaultDevice["name"] = DEVICE_TYPE_NAME_DEFAULT;
        defaultDevice["value"] = DEVICE_TYPE_DEFAULT;
        JsonObject SR6 = deviceTypes.add<JsonObject>();
        SR6["name"] = "SR6";
        SR6["value"] = DeviceType::SR6;
        JsonObject TVIBE = deviceTypes.add<JsonObject>();
        TVIBE["name"] = "TVIBE";
        TVIBE["value"] = DeviceType::TVIBE;
    #elif defined MOTOR_TYPE_BLDC
        defaultDevice["name"] = DEVICE_TYPE_NAME_DEFAULT;
        defaultDevice["value"] = DEVICE_TYPE_DEFAULT;
        JsonObject SSR1 = deviceTypes.add<JsonObject>();
        SSR1["name"] = "SSR1";
        SSR1["value"] = DeviceType::SSR1;
        JsonObject SSR2 = deviceTypes.add<JsonObject>();
        SSR2["name"] = "SSR2";
        SSR2["value"] = DeviceType::SSR2;
        JsonArray encoderTypes = doc["encoderTypes"].to<JsonArray>();
        JsonObject defaultEncoder = encoderTypes.add<JsonObject>();
        defaultEncoder["name"] = "NONE";
        defaultEncoder["value"] = BLDCEncoderType::NONE;
        if(static_cast<DeviceType>(deviceType) != DeviceType::SSR2)
        {
            JsonObject MT6701 = encoderTypes.add<JsonObject>();
            MT6701["name"] = "MT6701 SSI";
            MT6701["value"] = BLDCEncoderType::MT6701;
            JsonObject PWM = encoderTypes.add<JsonObject>();
            PWM["name"] = "PWM";
            PWM["value"] = BLDCEncoderType::PWM;
        }
        JsonObject SPI = encoderTypes.add<JsonObject>();
        SPI["name"] = "SPI";
        SPI["value"] = BLDCEncoderType::SPI;
    #endif

        JsonArray bleDeviceTypes = doc["bleDeviceTypes"].to<JsonArray>();
        JsonObject defaultBleDevice = bleDeviceTypes.add<JsonObject>();
        defaultBleDevice["name"] = "TCode";
        defaultBleDevice["value"] = BLEDeviceType::TCODE;
        JsonObject loveDevice = bleDeviceTypes.add<JsonObject>();
        loveDevice["name"] = "Love";
        loveDevice["value"] = BLEDeviceType::LOVE;
        JsonObject hcDevice = bleDeviceTypes.add<JsonObject>();
        // HC has an unknown formatting.
        hcDevice["name"] = "HC";
        hcDevice["value"] = BLEDeviceType::HC;

        JsonArray bleLoveDevices = doc["bleLoveDeviceTypes"].to<JsonArray>();
        JsonObject defaultLoveDevice = bleLoveDevices.add<JsonObject>();
        defaultLoveDevice["name"] = "Edge";
        defaultLoveDevice["value"] = BLELoveDeviceType::EDGE;
        

        JsonArray availableChannels = doc["availableChannels"].to<JsonArray>();
        channelMap.serialize(availableChannels);
        doc[MOTION_ENABLED] = getMotionEnabled();
        // int motionProfileSelectedIndex = MOTION_PROFILE_SELECTED_INDEX_DEFAULT;
        // m_settingsFactory->getValue(MOTION_PROFILE_SELECTED_INDEX, motionProfileSelectedIndex);
        doc[MOTION_PROFILE_SELECTED_INDEX] = motionSelectedProfileIndex; 

        JsonArray availableTimers = doc["availableTimers"].to<JsonArray>();
        JsonArray timerChannels = doc["timerChannels"].to<JsonArray>();
        JsonObject timerChannelNoneObj = timerChannels.add<JsonObject>();
        timerChannelNoneObj["name"] = "None";
        timerChannelNoneObj["value"] = ESPTimerChannelNum::NONE;
        PinMap* pinMap = m_settingsFactory->getPins();
        for (size_t i = 0; i < MAX_TIMERS; i++)
        {
            JsonObject timerObj = availableTimers.add<JsonObject>();
            ESPTimer* timer = pinMap->getTimer(i);
            timerObj["id"] = timer->id;
            timerObj["name"] = timer->name;
            timerObj["value"] = i;
            for (size_t j = 0; j < 2; j++)
            {
                JsonObject timerChannelObj = timerChannels.add<JsonObject>();
                timerChannelObj["name"] = timer->channels[j].name;
                timerChannelObj["value"] = timer->channels[j].channel;
            }
        }

        doc["localIP"] = currentIP;
        doc["gateway"] = currentGateway;
        doc["subnet"] = currentSubnet;
        doc["dns1"] = currentDns1;
        doc["dns2"] = currentDns2;// Not being used currently
        char macTemp[18] = {0};
		#ifdef ESP_ARDUINO3
        strlcpy(macTemp, Network.macAddress().c_str(), sizeof(macTemp));
		#else
        strlcpy(macTemp, WiFi.macAddress().c_str(), sizeof(macTemp));
		#endif
		doc["mac"] = macTemp;

        doc["chipModel"] = ESP.getChipModel();
        doc["chipRevision"] = ESP.getChipRevision();
        doc["chipCores"] = ESP.getChipCores();
        uint32_t chipId = 0;
        for (int i = 0; i < 17; i = i + 8)
        {
            chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
        }
        doc["chipID"] = chipId;

        doc["maxPWMResolution"] = MAX_PWM_RESOLUTION;
        doc["apbClockFrequency"] =  rtc_clk_apb_freq_get();
        doc["decoyPass"] = DECOY_PASS;
        doc["apMode"] = apMode;
        doc["defaultIP"] = m_settingsFactory->getAPModeIP();
        doc["restartRequired"] = m_settingsFactory->restartRequired();
        //String output;
        serializeJson(doc, buf);
        doc.clear();
        if (LogHandler::getLogLevel() == LogLevel::VERBOSE)
            Serial.printf("SystemInfo: %s\n", buf.c_str());
        // buf[0] = {0};
        // strcpy(buf, output.c_str());
    }

    static bool loadButtons(bool loadDefault, JsonObject json = JsonObject()) {
        LogHandler::info(_TAG, "Loading buttons");
        return loadSettingsJson(BUTTON_SETTINGS_PATH, loadDefault, m_buttonsMutex, [](const JsonObject json, bool& mutableLoadDefault) -> bool {
            
            // const bool bootButtonEnabled = SettingsHandler::getValue<const bool>(BOOT_BUTTON_ENABLED);
            // const bool buttonSetsEnabled = SettingsHandler::getValue<const bool>(BUTTON_SETS_ENABLED);;
            // const char* bootButtonCommand =  SettingsHandler::getValue<const char*>(BOOT_BUTTON_COMMAND);;
            // const int buttonAnalogDebounce = SettingsHandler::getValue<const int>(BUTTON_ANALOG_DEBOUNCE);
            // setValue(json, bootButtonEnabled, "buttonCommand", "bootButtonEnabled", BOOT_BUTTON_ENABLED_DEFAULT);
            // setValue(json, buttonSetsEnabled, "buttonCommand", "buttonSetsEnabled", BOOT_BUTTON_COMMAND_DEFAULT);
            // setValue(json, bootButtonCommand, "buttonCommand", "bootButtonCommand", BUTTON_SETS_ENABLED_DEFAULT);
            // setValue(json, buttonAnalogDebounce, "buttonCommand", "buttonAnalogDebounce", BUTTON_ANALOG_DEBOUNCE_DEFAULT);
            if(!json.isNull())
            {
                bool bootButtonEnabled = json[BOOT_BUTTON_ENABLED] | BOOT_BUTTON_ENABLED_DEFAULT;
                m_settingsFactory->setValue(BOOT_BUTTON_ENABLED, bootButtonEnabled);
                bool buttonSetsEnabled = json[BUTTON_SETS_ENABLED] | BUTTON_SETS_ENABLED_DEFAULT;
                m_settingsFactory->setValue(BUTTON_SETS_ENABLED, buttonSetsEnabled);
                const char* bootButtonCommand = json[BOOT_BUTTON_COMMAND] | BOOT_BUTTON_COMMAND_DEFAULT;
                m_settingsFactory->setValue(BOOT_BUTTON_COMMAND, bootButtonCommand);
                int buttonAnalogDebounce = json[BUTTON_ANALOG_DEBOUNCE] | BUTTON_ANALOG_DEBOUNCE_DEFAULT;
                m_settingsFactory->setValue(BUTTON_ANALOG_DEBOUNCE, buttonAnalogDebounce);
                m_settingsFactory->saveCommon();
            }

            JsonArray buttonSetsObj = json["buttonSets"].as<JsonArray>();
            if(buttonSetsObj.isNull()) {
                LogHandler::info(_TAG, "No button sets stored, loading default");
                mutableLoadDefault = true;
                const PinMap* pinMap = m_settingsFactory->getPins();
                for(int i = 0; i < MAX_BUTTON_SETS; i++) {
                    buttonSets[i] = ButtonSet();
                    buttonSets[i].pin = pinMap->buttonSetPin(i);
                        
                    sprintf(buttonSets[i].name, "Button set %u", i+1);
                    LogHandler::debug(_TAG, "Default buttonset name: %s, index: %u, pin: %ld", buttonSets[i].name, i, buttonSets[i].pin);
                    for(int j = 0; j < MAX_BUTTONS; j++) {
                        buttonSets[i].buttons[j] = ButtonModel();
                        buttonSets[i].buttons[j].loadDefault(j);
                        LogHandler::debug(_TAG, "Default button name: %s, index: %u, command: %s", buttonSets[i].name, buttonSets[i].buttons[j].index, buttonSets[i].buttons[j].command);

                    }
                }
            } else {
                std::vector<int> pins; 
                for(int i = 0; i < MAX_BUTTON_SETS; i++) {
                    auto set = ButtonSet();
                    set.fromJson(buttonSetsObj[i].as<JsonObject>());
                    pins.push_back(set.pin);
                    LogHandler::debug(_TAG, "Loaded button set '%s', pin: %ld", set.name, set.pin);
                    buttonSets[i] = set;
                    for(int j = 0; j < MAX_BUTTONS; j++) {
                        LogHandler::debug(_TAG, "Loaded button, name: %s, index: %u, command: %s", buttonSets[i].name, buttonSets[i].buttons[j].index, buttonSets[i].buttons[j].command);
                    }
                }
                m_settingsFactory->setValue(BUTTON_SET_PINS, pins);
                m_settingsFactory->savePins();
            }
            
            if(initialized)
                sendMessage(SettingProfile::Button, "analogButtonCommands");
            
            return true;
        }, saveButtons, json);
    }

    static bool saveButtons(JsonObject json = JsonObject()) {
        LogHandler::info(_TAG, "Save buttons file");
        uint16_t docSize = 2000;
        // for(int i = 0; i < MAX_BUTTON_SETS; i++) {
        //     LogHandler::debug(_TAG, "Save buttonSets[i] '%s', pin: %ld",  buttonSets[i].name, buttonSets[i].pin);
        //     for(int j = 0; j < MAX_BUTTONS; j++) {
        //         LogHandler::debug(_TAG, "Save buttonSets[i].buttons, name: %s,  index: %ld, command: %s", buttonSets[i].name, buttonSets[i].buttons[j].index, buttonSets[i].buttons[j].command);
        //     }
        // }
        return saveSettingsJson(BUTTON_SETTINGS_PATH, m_buttonsMutex, docSize, [](JsonDocument& doc) -> bool {

            bool bootButtonEnabled = BOOT_BUTTON_ENABLED_DEFAULT;
            m_settingsFactory->getValue(BOOT_BUTTON_ENABLED, bootButtonEnabled);
            doc[BOOT_BUTTON_ENABLED] = bootButtonEnabled; 
            bool buttonSetsEnabled = BUTTON_SETS_ENABLED_DEFAULT;
            m_settingsFactory->getValue(BUTTON_SETS_ENABLED, buttonSetsEnabled);
            doc[BUTTON_SETS_ENABLED] = buttonSetsEnabled;
            // char bootButtonCommand[BOOT_BUTTON_COMMAND_LEN] = {0};
            // m_settingsFactory->getValue(BOOT_BUTTON_COMMAND, bootButtonCommand, BOOT_BUTTON_COMMAND_LEN);
            const char* bootButtonCommand = m_settingsFactory->getBootButtonCommand();
            doc[BOOT_BUTTON_COMMAND] = bootButtonCommand; 
            int buttonAnalogDebounce = BUTTON_ANALOG_DEBOUNCE_DEFAULT;
            m_settingsFactory->getValue(BUTTON_ANALOG_DEBOUNCE, buttonAnalogDebounce);
            doc[BUTTON_ANALOG_DEBOUNCE] = buttonAnalogDebounce;

            //auto buttonSetArray = doc["buttonSets"].as<JsonArray>();
            std::vector<int> pins;
            for (size_t i = 0; i < MAX_BUTTON_SETS; i++)
            {
                //JsonObject obj;
                doc["buttonSets"][i]["name"] = buttonSets[i].name;
                
                doc["buttonSets"][i]["pin"] = buttonSets[i].pin;
                pins.push_back(buttonSets[i].pin);
                doc["buttonSets"][i]["pullMode"] = (uint8_t)buttonSets[i].pullMode;
                LogHandler::debug(_TAG, "Saving button set '%s' from settings, pin: %ld",  doc["buttonSets"][i]["name"].as<const  char*>(), doc["buttonSets"][i]["pin"].as<int>());
                for(size_t j = 0; j < MAX_BUTTONS; j++) {
                    doc["buttonSets"][i]["buttons"][j]["name"] = buttonSets[i].buttons[j].name;
                    doc["buttonSets"][i]["buttons"][j]["index"] = buttonSets[i].buttons[j].index;
                    doc["buttonSets"][i]["buttons"][j]["command"] = buttonSets[i].buttons[j].command;
                    LogHandler::debug(_TAG, "Saving button, name: %s, index: %u, command: %s", doc["buttonSets"][i]["buttons"][j]["name"].as<const char*>(), doc["buttonSets"][i]["buttons"][j]["index"].as<int>(), doc["buttonSets"][i]["buttons"][j]["command"].as<const char*>());
                }
                //buttonSetArray.add(obj);
            }
            m_settingsFactory->setValue(BUTTON_SET_PINS, pins);
            m_settingsFactory->savePins();
            return true;
        }, loadButtons, json);
    }

    static bool loadMotionProfiles(bool loadDefault, JsonObject json = JsonObject()) {
        LogHandler::info(_TAG, "Loading motion profiles");
        // bool mutableLoadDefault = loadDefault;
        // JsonDocument doc; //deserializeSize
        // if(mutableLoadDefault || json.isNull()) {
		//     xSemaphoreTake(m_motionMutex, portMAX_DELAY);
        //     if(!checkForFileAndLoad(MOTION_PROFILE_SETTINGS_PATH, doc, mutableLoadDefault)) {
        //         saving = false;
        //         xSemaphoreGive(m_motionMutex);
        //         return false;
        //     }
        //     json = doc.as<JsonObject>();
        // }
        return loadSettingsJson(MOTION_PROFILE_SETTINGS_PATH, loadDefault, m_motionMutex, [](const JsonObject json, bool& mutableLoadDefault) -> bool {
            motionDefaultProfileIndex = json[MOTION_PROFILE_DEFAULT_INDEX] | MOTION_PROFILE_SELECTED_INDEX_DEFAULT;
            if(!initialized)
                motionSelectedProfileIndex = motionDefaultProfileIndex;
                
            JsonArray motionProfilesObj = json[MOTION_PROFILES].as<JsonArray>();
            if(motionProfilesObj.isNull()) {
                LogHandler::info(_TAG, "No motion profiles stored, loading default");
                mutableLoadDefault = true;
                for(int i = 0; i < MAX_MOTION_PROFILE_COUNT; i++) {
                    motionProfiles[i] = MotionProfile(i + 1);
                    motionProfiles[i].addDefaultChannel("L0");
                    LogHandler::debug(_TAG, "Added new Motion profile for: %s", motionProfiles[i].channels.back().name);
                }
            } else {
                int i = 0;
                for (JsonObject profileObj : motionProfilesObj) {
                    auto profile = MotionProfile();
                    profile.fromJson(profileObj);
                    LogHandler::debug(_TAG, "Loading motion profile '%s' from settings", profile.motionProfileName);
                    motionProfiles[i] = profile;
                    i++;
                }
            }
        //xSemaphoreGive(m_motionMutex);
        // if(mutableLoadDefault)
        //     saveMotionProfiles();
        return true;
        }, saveMotionProfiles, json);
    }

    static bool saveMotionProfiles(JsonObject json = JsonObject()) {
        LogHandler::info(_TAG, "Save motion profiles file");
        saving = true;
		xSemaphoreTake(m_motionMutex, portMAX_DELAY);
        if (!LittleFS.exists(MOTION_PROFILE_SETTINGS_PATH)) {
            LogHandler::error(_TAG, "Motion profile file did not exist whan saving.");
            saving = false;
            xSemaphoreGive(m_motionMutex);
            return false;
        }
        if(!json.isNull()) { // If passed in, load the json into memory before flushing it to disk.
            // WARNING: watchout for the mutex taken in this method. Changing these parameters below may result in hard locks.
            loadMotionProfiles(false, json); // DO NOT PASS loadDefault as true else infinit loop
        }
        JsonDocument doc; //serializeSize
        doc[MOTION_PROFILE_DEFAULT_INDEX] = motionDefaultProfileIndex;
        LogHandler::debug(_TAG, "motion profiles index: %ld", motionDefaultProfileIndex);

        for (int i=0; i < MAX_MOTION_PROFILE_COUNT; i++) {
            //if(motionProfiles[i].edited) { // TODO: this does not work because doc is empty and needs to be loaded from disk first bedore modifying sections of it.

                LogHandler::debug(_TAG, "Edited motion profile name: %s", motionProfiles[i].motionProfileName);
                doc[MOTION_PROFILES][i]["name"] = motionProfiles[i].motionProfileName;
                for (size_t j = 0; j < motionProfiles[i].channels.size(); j++) {
                    //if(motionProfiles[i].channels[j].edited) {
                        LogHandler::debug(_TAG, "motion profile channel: %s", motionProfiles[i].channels[j].name);
                        doc[MOTION_PROFILES][i]["channels"][j]["name"] = motionProfiles[i].channels[j].name;
                        doc[MOTION_PROFILES][i]["channels"][j]["update"] = motionProfiles[i].channels[j].motionUpdateGlobal;
                        doc[MOTION_PROFILES][i]["channels"][j]["period"] = motionProfiles[i].channels[j].motionPeriodGlobal;
                        doc[MOTION_PROFILES][i]["channels"][j]["amp"] = motionProfiles[i].channels[j].motionAmplitudeGlobal;
                        doc[MOTION_PROFILES][i]["channels"][j]["offset"] = motionProfiles[i].channels[j].motionOffsetGlobal;
                        doc[MOTION_PROFILES][i]["channels"][j]["phase"] = motionProfiles[i].channels[j].motionPhaseGlobal;
                        doc[MOTION_PROFILES][i]["channels"][j]["reverse"] = motionProfiles[i].channels[j].motionReversedGlobal;
                        doc[MOTION_PROFILES][i]["channels"][j]["periodRan"] = motionProfiles[i].channels[j].motionPeriodGlobalRandom;
                        doc[MOTION_PROFILES][i]["channels"][j]["periodMin"] = motionProfiles[i].channels[j].motionPeriodGlobalRandomMin;
                        doc[MOTION_PROFILES][i]["channels"][j]["periodMax"] = motionProfiles[i].channels[j].motionPeriodGlobalRandomMax;
                        doc[MOTION_PROFILES][i]["channels"][j]["ampRan"] = motionProfiles[i].channels[j].motionAmplitudeGlobalRandom;
                        doc[MOTION_PROFILES][i]["channels"][j]["ampMin"] = motionProfiles[i].channels[j].motionAmplitudeGlobalRandomMin;
                        doc[MOTION_PROFILES][i]["channels"][j]["ampMax"] = motionProfiles[i].channels[j].motionAmplitudeGlobalRandomMax;
                        doc[MOTION_PROFILES][i]["channels"][j]["offsetRan"] = motionProfiles[i].channels[j].motionOffsetGlobalRandom;
                        doc[MOTION_PROFILES][i]["channels"][j]["offsetMin"] = motionProfiles[i].channels[j].motionOffsetGlobalRandomMin;
                        doc[MOTION_PROFILES][i]["channels"][j]["offsetMax"] = motionProfiles[i].channels[j].motionOffsetGlobalRandomMax;
                        doc[MOTION_PROFILES][i]["channels"][j]["phaseRan"] = motionProfiles[i].channels[j].motionPhaseRandom;
                        doc[MOTION_PROFILES][i]["channels"][j]["phaseMin"] = motionProfiles[i].channels[j].motionPhaseRandomMin;
                        doc[MOTION_PROFILES][i]["channels"][j]["phaseMax"] = motionProfiles[i].channels[j].motionPhaseRandomMax;
                        doc[MOTION_PROFILES][i]["channels"][j]["ranMin"] = motionProfiles[i].channels[j].motionRandomChangeMin;
                        doc[MOTION_PROFILES][i]["channels"][j]["ranMax"] = motionProfiles[i].channels[j].motionRandomChangeMax;
                        motionProfiles[i].channels[j].edited = false;
                    //}
                }
                if(initialized && motionSelectedProfileIndex == i) {
                    sendMessage(SettingProfile::MotionProfile, MOTION_PROFILES);
                }
                motionProfiles[i].edited = false;
            //}
        }
        File file = LittleFS.open(MOTION_PROFILE_SETTINGS_PATH, FILE_WRITE);
        if (serializeJson(doc, file) == 0) {
            LogHandler::error(_TAG, "Failed to write to motion profiles file");
            file.close();
            xSemaphoreGive(m_motionMutex);
            saving = false;
            return false;
        }
        
        xSemaphoreGive(m_motionMutex);
        saving = false;
        return true;
    }

    static bool loadChannels(bool loadDefault, JsonObject json = JsonObject()) {

        MotorType motorType;
        DeviceType deviceType;
        m_settingsFactory->getValue(MOTOR_TYPE_SETTING, motorType);
        m_settingsFactory->getValue(DEVICE_TYPE, deviceType);
        // Init motor type BEFORE loading the channels else it will load the defaults
        channelMap.init(m_settingsFactory->getTcodeVersion(), motorType, deviceType);

        LogHandler::info(_TAG, "Loading channel profile");
        return loadSettingsJson(CHANNELS_SETTINGS_PATH, loadDefault, m_channelsMutex, [](const JsonObject json, bool& mutableLoadDefault) -> bool {
                
            JsonArray channelProfileObj = json[CHANNEL_PROFILE].as<JsonArray>();
            
            if(channelProfileObj.isNull()) {
                LogHandler::info(_TAG, "No channel profile stored, loading default");
                mutableLoadDefault = true;
            } else {
                for (JsonObject profileObj : channelProfileObj) {
                    const char* name = profileObj[CHANNEL_NAME];
                    Channel* channel = channelMap.get(name);
                    if(!channel)
                    {
                        LogHandler::error(_TAG, "Channel missing from stored profile: %s", name);
                        continue;
                    }
                    channel->userMin = profileObj[CHANNEL_USER_MIN] | TCODE_MIN;
                    channel->userMid = profileObj[CHANNEL_USER_MID] | TCODE_MID;
                    channel->userMax = profileObj[CHANNEL_USER_MAX] | TCODE_MAX;
                    channel->rangeLimitEnabled = profileObj[CHANNEL_RANGE_LIMIT_ENABLED];
                    LogHandler::debug(_TAG, "Loading channel profile '%s' from settings", name);
                }
            }
        return true;
        }, saveChannels, json);
    }

    static bool saveChannels(JsonObject json = JsonObject()) {
        LogHandler::info(_TAG, "Save channel profile file");
        saving = true;
		xSemaphoreTake(m_channelsMutex, portMAX_DELAY);
        if (!LittleFS.exists(CHANNELS_SETTINGS_PATH)) {
            LogHandler::error(_TAG, "Channel profile file did not exist whan saving.");
            saving = false;
            xSemaphoreGive(m_channelsMutex);
            return false;
        }
        if(!json.isNull()) { // If passed in, load the json into memory before flushing it to disk.
            // WARNING: watchout for the mutex taken in this method. Changing these parameters below may result in hard locks.
            loadChannels(false, json); // DO NOT PASS loadDefault as true else infinit loop
        }
        JsonDocument doc; //serializeSize
        for (int i=0; i < channelMap.count(); i++) {
            Channel* channel = channelMap.get(i);
            doc[CHANNEL_PROFILE][i][CHANNEL_NAME] = channel->Name;
            doc[CHANNEL_PROFILE][i][CHANNEL_USER_MIN] = channel->userMin;
            doc[CHANNEL_PROFILE][i][CHANNEL_USER_MID] = channel->userMid;
            doc[CHANNEL_PROFILE][i][CHANNEL_USER_MAX] = channel->userMax;
            doc[CHANNEL_PROFILE][i][CHANNEL_RANGE_LIMIT_ENABLED] = channel->rangeLimitEnabled;
            if(initialized) {
                sendMessage(SettingProfile::ChannelRanges, CHANNEL_PROFILE);
            }
        }
        File file = LittleFS.open(CHANNELS_SETTINGS_PATH, FILE_WRITE);
        if (serializeJson(doc, file) == 0) {
            LogHandler::error(_TAG, "Failed to write to channel profile file");
            file.close();
            xSemaphoreGive(m_channelsMutex);
            saving = false;
            return false;
        }
        
        xSemaphoreGive(m_channelsMutex);
        saving = false;
        return true;
    }

    static std::vector<MotionChannel>& getMotionChannels()
    {
        return motionProfiles[motionSelectedProfileIndex].channels;
    }
    
    static bool getMotionEnabled()
    {
        return motionEnabled;
    }
    static void setMotionEnabled(const bool& newValue)
    {
        setValue(newValue, motionEnabled, SettingProfile::MotionProfile, MOTION_ENABLED);
    }
    static bool getMotionPaused()
    {
        return motionPaused;
    }
    static void setMotionPaused(const bool& newValue)
    {
        setValue(newValue, motionPaused, SettingProfile::MotionProfile, MOTION_PAUSED);
    }

    static int getMotionDefaultProfileIndex() 
    {
        return motionDefaultProfileIndex;
    }
    static void setMotionProfileName(const char newValue[MAX_MOTION_PROFILE_NAME_LENGTH])
    {
        strcpy(motionProfiles[motionSelectedProfileIndex].motionProfileName, newValue);
    }

    // static int getMotionUpdateGlobal(const char name[3])
    // {
    //     return motionProfiles[motionSelectedProfileIndex].motionUpdateGlobal;
    // }
    // static void setMotionUpdateGlobal(const int& newValue)
    // {
    //     setValue(newValue, motionProfiles[motionSelectedProfileIndex].motionUpdateGlobal, "motionGenerator", "motionUpdateGlobal");
    // }
    // static int getMotionPeriodGlobal()
    // {
    //     return motionProfiles[motionSelectedProfileIndex].motionPeriodGlobal;
    // }
    // static void setMotionPeriodGlobal(const int& newValue)
    // {
    //     setValue(newValue, motionProfiles[motionSelectedProfileIndex].motionPeriodGlobal, "motionGenerator", "motionPeriodGlobal");
    // }
    // static int getMotionAmplitudeGlobal()
    //  {
    //     return motionProfiles[motionSelectedProfileIndex].motionAmplitudeGlobal;
    // }
    // static void setMotionAmplitudeGlobal(const int& newValue)
    // {
    //     setValue(newValue, motionProfiles[motionSelectedProfileIndex].motionAmplitudeGlobal, "motionGenerator", "motionAmplitudeGlobal");
    // }
    // static int getMotionOffsetGlobal()
    // {
    //     return motionProfiles[motionSelectedProfileIndex].motionOffsetGlobal;
    // }
    // static void setMotionOffsetGlobal(const int& newValue)
    // {
    //     setValue(newValue, motionProfiles[motionSelectedProfileIndex].motionOffsetGlobal, "motionGenerator", "motionOffsetGlobal");
    // }
    // static float getMotionPhaseGlobal()
    // {
    //     return motionProfiles[motionSelectedProfileIndex].motionPhaseGlobal;
    // }
    // static void setMotionPhaseGlobal(const float& newValue)
    // {
    //     setValue(newValue, motionProfiles[motionSelectedProfileIndex].motionPhaseGlobal, "motionGenerator", "motionPhaseGlobal");
    // }
    // static bool getMotionReversedGlobal()
    // {
    //     return motionProfiles[motionSelectedProfileIndex].motionReversedGlobal;
    // }
    // static void setMotionReversedGlobal(const bool& newValue)
    // {
    //     setValue(newValue, motionProfiles[motionSelectedProfileIndex].motionReversedGlobal, "motionGenerator", "motionReversedGlobal");
    // }
    // static bool getMotionPeriodGlobalRandom()
    // {
    //     return motionProfiles[motionSelectedProfileIndex].motionPeriodGlobalRandom;
    // }
    // static void setMotionPeriodGlobalRandom(const bool& newValue)
    // {
    //     setValue(newValue, motionProfiles[motionSelectedProfileIndex].motionPeriodGlobalRandom, "motionGenerator", "motionPeriodGlobalRandom");
    // }
    // static int getMotionPeriodGlobalRandomMin()
    // {
    //     return motionProfiles[motionSelectedProfileIndex].motionPeriodGlobalRandomMin;
    // }
    // static void setMotionPeriodGlobalRandomMin(const int& newValue)
    // {
    //     setValue(newValue, motionProfiles[motionSelectedProfileIndex].motionPeriodGlobalRandomMin, "motionGenerator", "motionPeriodGlobalRandomMin");
    // }
    // static int getMotionPeriodGlobalRandomMax()
    // {
    //     return motionProfiles[motionSelectedProfileIndex].motionPeriodGlobalRandomMax;
    // }
    // static void setMotionPeriodGlobalRandomMax(const int& newValue)
    // {
    //     setValue(newValue, motionProfiles[motionSelectedProfileIndex].motionPeriodGlobalRandomMax, "motionGenerator", "motionPeriodGlobalRandomMax");
    // }
    // static bool getMotionAmplitudeGlobalRandom()
    // {
    //     return motionProfiles[motionSelectedProfileIndex].motionAmplitudeGlobalRandom ;
    // }
    // static void setMotionAmplitudeGlobalRandom(const bool& newValue)
    // {
    //     setValue(newValue, motionProfiles[motionSelectedProfileIndex].motionAmplitudeGlobalRandom, "motionGenerator", "motionAmplitudeGlobalRandom");
    // }
    // static int getMotionAmplitudeGlobalRandomMin()
    // {
    //     return motionProfiles[motionSelectedProfileIndex].motionAmplitudeGlobalRandomMin;
    // }
    // static void setMotionAmplitudeGlobalRandomMin(const int& newValue)
    // {
    //     setValue(newValue, motionProfiles[motionSelectedProfileIndex].motionAmplitudeGlobalRandomMin = newValue, "motionGenerator", "motionAmplitudeGlobalRandomMin");
    // }
    // static int getMotionAmplitudeGlobalRandomMax()
    // {
    //     return motionProfiles[motionSelectedProfileIndex].motionAmplitudeGlobalRandomMax;
    // }
    // static void setMotionAmplitudeGlobalRandomMax(const int& newValue)
    // {
    //     setValue(newValue, motionProfiles[motionSelectedProfileIndex].motionAmplitudeGlobalRandomMax = newValue, "motionGenerator", "motionAmplitudeGlobalRandomMax");
    // }
    // static bool getMotionOffsetGlobalRandom()
    // {
    //     return motionProfiles[motionSelectedProfileIndex].motionOffsetGlobalRandom;
    // }
    // static void setMotionOffsetGlobalRandom(const bool& newValue)
    // {
    //     setValue(newValue, motionProfiles[motionSelectedProfileIndex].motionOffsetGlobalRandom = newValue, "motionGenerator", "motionOffsetGlobalRandom");
    // }
    // static int getMotionOffsetGlobalRandomMin()
    // {
    //     return motionProfiles[motionSelectedProfileIndex].motionOffsetGlobalRandomMin;
    // }
    // static void setMotionOffsetGlobalRandomMin(const int& newValue)
    // {
    //     setValue(newValue, motionProfiles[motionSelectedProfileIndex].motionOffsetGlobalRandomMin, "motionGenerator", "motionOffsetGlobalRandomMin");
    // }
    // static int getMotionOffsetGlobalRandomMax()
    // {
    //     return motionProfiles[motionSelectedProfileIndex].motionOffsetGlobalRandomMax;
    // }
    // static void setMotionOffsetGlobalRandomMax(const int& newValue)
    // {
    //     setValue(newValue, motionProfiles[motionSelectedProfileIndex].motionOffsetGlobalRandomMax = newValue, "motionGenerator", "motionOffsetGlobalRandomMax");
    // }
    // static int getMotionRandomChangeMin()
    // {
    //     return motionProfiles[motionSelectedProfileIndex].motionRandomChangeMin;
    // }
    // static void setMotionRandomChangeMin(const int& newValue)
    // {
    //     setValue(newValue, motionProfiles[motionSelectedProfileIndex].motionRandomChangeMin, "motionGenerator", "motionRandomChangeMin");
    // }
    // static int getMotionRandomChangeMax()
    // {
    //     return motionProfiles[motionSelectedProfileIndex].motionRandomChangeMax;
    // }
    // static void setMotionRandomChangeMax(const int& newValue)
    // {
    //     setValue(newValue, motionProfiles[motionSelectedProfileIndex].motionRandomChangeMax, "motionGenerator", "motionRandomChangeMax");
    // }

    static int motionProfileExists(const char* profile) {
        for (size_t i = 0; i < MAX_MOTION_PROFILE_COUNT; i++)
        {
            if (strcmp(motionProfiles[i].motionProfileName, profile) == 0)
                return (int)i;
        }
        return -1;
    }

    static void setMotionDefaults() {
        setMotionEnabled(false);
        auto motionProfile = MotionProfile(motionSelectedProfileIndex + 1);
        setMotionProfile(motionProfile, motionSelectedProfileIndex);
    }

    static void setMotionProfile(const char profile[MAX_MOTION_PROFILE_NAME_LENGTH]) {
        auto index = motionProfileExists(profile);
        if(index < 0) {
            LogHandler::error(_TAG, "Motion profile %s does not exist", profile);
            return;
        }
        setMotionProfile(index);
    }

    static void setMotionProfile(const int& index) {
        if(index < 0 || index > MAX_MOTION_PROFILE_COUNT - 1) {
            LogHandler::error(_TAG, "Invalid motion profile index: %ld", index);
            return;
        }
        auto newProfile = motionProfiles[index];
        setMotionProfile(newProfile, index);
    }

    static void setMotionProfile(const MotionProfile& profile, int profileIndex) {
        if(profileIndex < 0 || profileIndex > MAX_MOTION_PROFILE_COUNT - 1) {
            LogHandler::error(_TAG, "Invalid motion profile index: %ld", profileIndex);
            return;
        }
        //m_settingsFactory->setValue(MOTION_PROFILE_SELECTED_INDEX, profileIndex);
        setValue(profileIndex, motionSelectedProfileIndex, SettingProfile::MotionProfile, MOTION_PROFILE_SELECTED_INDEX);
    }
    
    static void cycleMotionProfile() {
        if(!getMotionEnabled()) {
            setMotionEnabled(true);
            return;
        }
        uint8_t newProfileIndex = motionSelectedProfileIndex + 1;
        if(newProfileIndex > MAX_MOTION_PROFILE_COUNT - 1) {
            newProfileIndex = 0;
            setMotionEnabled(false);
        }
        auto newProfile = motionProfiles[newProfileIndex];
        setMotionProfile(newProfile, newProfileIndex);
    }

    static const bool readFile(char* &buf, const char* path) {
        if(!LittleFS.exists(path)) {
            LogHandler::error(_TAG, "Path did not exist when reading contents: %s", path);
            return false;
        }
        File file = LittleFS.open(path, "r");
        printFree();
        String fileStr = file.readString();
        //buf = static_cast<char*>(malloc(fileStr.length() + 1));
        printFree();
        LogHandler::info(_TAG, "Create buffer: %u", fileStr.length());
        buf = new char[fileStr.length() + 1];
        strcpy(buf, fileStr.c_str());
        return true;
    }

    static const int getDeserializeSize() {
        return deserializeSize;
    }

    // static void processTCodeJson(char *outbuf, const char *tcodeJson)
    // {
    //     StaticJsonDocument<512> doc;
    //     DeserializationError error = deserializeJson(doc, tcodeJson);
    //     if (error)
    //     {
    //         LogHandler::error(_TAG, "Failed to read udp jsonobject, using default configuration");
    //         outbuf[0] = {0};
    //         return;
    //     }
    //     JsonArray arr = doc.as<JsonArray>();
    //     char buffer[MAX_COMMAND] = "";
    //     for (JsonObject repo : arr)
    //     {
    //         const char *channel = repo["c"];
    //         int value = repo["v"];
    //         if (channel != nullptr && value > 0)
    //         {
    //             if (buffer[0] == '\0')
    //             {
    //                 // Serial.println("tcode empty");
    //                 strcpy(buffer, channel);
    //             }
    //             else
    //             {
    //                 strcat(buffer, channel);
    //             }
    //             // Serial.print("channel: ");
    //             // Serial.print(channel);
    //             // Serial.print(" value: ");
    //             // Serial.println(value);
    //             char integer_string[4];
    //             sprintf(integer_string, SettingsHandler::TCodeVersionEnum == TCodeVersion::v0_2 ? "%03d" : "%04d", SettingsHandler::calculateRange(name, value));
    //             // pad(integer_string);
    //             // sprintf(integer_string, "%d", SettingsHandler::calculateRange(name, value));
    //             // Serial.print("integer_string");
    //             // Serial.println(integer_string);
    //             strcat(buffer, integer_string);
    //             int speed = repo["s"];
    //             int interval = repo["i"];
    //             if (interval > 0)
    //             {
    //                 char interval_string[5];
    //                 sprintf(interval_string, "%d", interval);
    //                 strcat(buffer, "I");
    //                 strcat(buffer, interval_string);
    //             }
    //             else if (speed > 0)
    //             {
    //                 char speed_string[5];
    //                 sprintf(speed_string, "%d", speed);
    //                 strcat(buffer, "S");
    //                 strcat(buffer, speed_string);
    //             }
    //             strcat(buffer, " ");
    //             // Serial.print("buffer ");
    //             // Serial.println(buffer);
    //         }
    //     }
    //     strcpy(outbuf, buffer);
    //     strcat(outbuf, "\n");
    //     // Serial.print("outbuf ");
    //     // Serial.println(outbuf);
    // }

    static bool waitForI2CDevices(const int& i2cAddress = 0) {
        int tries = 0;
        if(i2cAddress)
            LogHandler::info(_TAG, "Looking for I2c address: %ld", i2cAddress);
        while((systemI2CAddresses.size() == 0 || i2cAddress) && tries <= 3) {
            tries++;
            I2CScan();
            if(i2cAddress && std::find(systemI2CAddresses.begin(), systemI2CAddresses.end(), i2cAddress) != systemI2CAddresses.end()) {
                return true;
            } else if(i2cAddress) {
                LogHandler::info(_TAG, "I2c address: %ld not found. trying again...", i2cAddress);
            } else if(systemI2CAddresses.size() == 0) {
                LogHandler::info(_TAG, "No I2C devices found in system, trying again...");
            }
            if(tries >= 3){
                if (i2cAddress) {
                    LogHandler::error(_TAG, "I2c address: %ld timed out.", i2cAddress);
                } else {
                    LogHandler::error(_TAG, "No I2C devices found in system");
                }
                return false;
            }
            vTaskDelay(1000/portTICK_PERIOD_MS);
        }
        return true;
    }

	static bool I2CScan() 
	{
        systemI2CAddresses.clear();
		byte error, address;
		int nDevices;
		LogHandler::info(_TAG, "Scanning for I2C...");
		nDevices = 0;
        int8_t sdaPin = I2C_SDA_PIN_DEFAULT;
        m_settingsFactory->getValue(I2C_SDA_PIN, sdaPin);
        int8_t sclPin = I2C_SCL_PIN_DEFAULT;
        m_settingsFactory->getValue(I2C_SCL_PIN, sclPin);
        if(sdaPin < 0 || sclPin < 0) {
		    LogHandler::debug(_TAG, "SDA or SCL is disabled when scaning for I2C devices sdaPin: %d, sclPin: %d", sdaPin, sclPin);
            return false;
        }
		Wire.begin(sdaPin, sclPin);
		for(address = 1; address < 127; address++ ) 
		{
			Wire.beginTransmission(address);
			error = Wire.endTransmission();
			if (error == 0) 
			{
				//Serial.print("I2C device found at address 0x");
				// if (address<16) 
				// {
				// 	Serial.print("0");
				// }
				// Serial.println(address,HEX);

                // std::stringstream I2C_Address_String;
                // I2C_Address_String << "0x" << std::hex << address;
                // std::string foundAddress = I2C_Address_String.str();
                
				char buf[10];
				hexToString(address, buf);
				LogHandler::info(_TAG, "I2C device found at address %s, byte %ld", buf, address);

				systemI2CAddresses.push_back((int)address);
				nDevices++;
			}
			else if (error==4) 
			{
				Serial.print("Unknow error at address 0x");
				if (address<16) 
				{
					Serial.print("0");
				}
				Serial.println(address,HEX);
                // std::stringstream I2C_Address_String;
                // I2C_Address_String << "0x" << std::hex << address;
                // std::string foundAddress = I2C_Address_String.str();
				// LogHandler::error(_TAG, "Unknow error at address %s", foundAddress);
			}    
		}
		if (nDevices == 0) {
			LogHandler::info(_TAG, "No I2C devices found");
			return false;
		}
		return true;
	}
    
    static Channel* getChannel(const char *name) 
    {
        return channelMap.get(name);
    }

    static uint16_t getChannelMin(const char *name) 
    {
        Channel* channelProfile = channelMap.get(name);
        if(!channelProfile)
        {
            LogHandler::error(_TAG, "[getChannelMin] Invalid name for current map: %s", name);
            return TCODE_MIN;
        }
        return channelProfile->min;
    }

    static uint16_t getChannelMax(const char *name) 
    {
        Channel* channelProfile = channelMap.get(name);
        if(!channelProfile)
        {
            LogHandler::error(_TAG, "[getChannelMax] Invalid name for current map: %s", name);
            return TCODE_MAX;
        }
        return channelProfile->max;
    }

    static uint16_t getChannelUserMin(const char *name) 
    {
        Channel* channelProfile = channelMap.get(name);
        if(!channelProfile)
        {
            LogHandler::error(_TAG, "[getChannelUserMin] Invalid name for current map: %s", name);
            return TCODE_MIN;
        }
        return channelProfile->userMin;
    }

    static uint16_t getChannelUserMax(const char *name) 
    {
        Channel* channelProfile = channelMap.get(name);
        if(!channelProfile)
        {
            LogHandler::error(_TAG, "[getChannelUserMax] Invalid name for current map: %s", name);
            return TCODE_MAX;
        }
        return channelProfile->userMax;
    }

    static void setChannelMin(const char *name, uint16_t value) 
    {
        Channel* channelProfile = channelMap.get(name);
        if(!channelProfile)
        {
            LogHandler::error(_TAG, "[setChannelMin] Invalid name for current map: %s", name);
            return;
        }
        channelProfile->userMin = value;
    }

    static void setChannelMax(const char *name, uint16_t value) 
    {
        Channel* channelProfile = channelMap.get(name);
        if(!channelProfile)
        {
            LogHandler::error(_TAG, "[setChannelMax] Invalid name for current map: %s", name);
            return;
        }
        channelProfile->userMax = value;
    }

    static void setChannelRangesEnabled(bool enabled) {
        channelRangesEnabled = enabled;
        sendMessage(SettingProfile::ChannelRanges, "channelRangesEnabled");
    }
    static bool getChannelRangesEnabled() {
        return channelRangesEnabled;
    }

private:
    static const char *_TAG;
    
    static SettingsFactory* m_settingsFactory;
	static SemaphoreHandle_t m_motionMutex;
    static SemaphoreHandle_t m_channelsMutex;
	static SemaphoreHandle_t m_wifiMutex;
    static SemaphoreHandle_t m_buttonsMutex;
	static SemaphoreHandle_t m_settingsMutex;
    static inline SettingsChangeCallback message_callback = 0;
    // Use http://arduinojson.org/assistant to compute the capacity.
    static const int deserializeSize = 32768;
    static const int serializeSize = 24576;

    static bool motionEnabled;
    static int motionSelectedProfileIndex;
    static int motionDefaultProfileIndex;

    /// @brief Locks the mutex checks for an existing file and creates  it if it doesnt exist. Calls the callback function and gives the mutex.
    /// @param filepath 
    /// @param mutableLoadDefault 
    /// @param mutex 
    /// @param jsonSize 
    /// @param loadFunction 
    /// @param json 
    /// @return 
    static bool loadSettingsJson(const char* filepath, bool loadDefault, SemaphoreHandle_t& mutex, std::function<bool(const JsonObject, bool& mutableLoadDefault)> loadFunction, std::function<bool(JsonObject)> saveFunction, JsonObject json = JsonObject()) {
        JsonDocument doc; //jsonSize
        bool mutableLoadDefault = loadDefault;
        if(mutableLoadDefault || json.isNull()) {
		    xSemaphoreTake(mutex, portMAX_DELAY);
            if(!checkForFileAndLoad(filepath, doc, mutableLoadDefault)) {
                xSemaphoreGive(mutex);
                return false;
            }
            json = doc.as<JsonObject>();
        }
        if(!loadFunction(json, mutableLoadDefault)) {
            xSemaphoreGive(mutex);
            return false;
        }
        xSemaphoreGive(mutex);
        if(mutableLoadDefault)
            saveFunction(JsonObject());
        return true;
    }

    /// @brief Locks the mutex and validates the file exists. calls the calback and serializes the data in a file to disk. Releases the mutex.
    /// @param filepath 
    /// @param mutex 
    /// @param jsonSize 
    /// @param saveFunction 
    /// @param json 
    /// @return 
    static bool saveSettingsJson(const char* filepath, SemaphoreHandle_t& mutex, int jsonSize, std::function<bool(JsonDocument&)> saveFunction, std::function<bool(bool, JsonObject)> loadFunction, JsonObject json = JsonObject()) {
        saving = true;
		xSemaphoreTake(mutex, portMAX_DELAY);
        LogHandler::debug(_TAG, "Saving File: %s", filepath);
        bool loadBeforeSetting = false;
        if (!LittleFS.exists(filepath)) {
            LogHandler::error(_TAG, "File did not exist whan saving: %s", filepath);
            xSemaphoreGive(mutex);
            saving = false;
            return false;
        } else {
            if(!json.isNull()) {
                LogHandler::debug(_TAG, "Loading from input json: %s", filepath);
                xSemaphoreGive(mutex);
                if(!loadFunction(false, json)){
                    LogHandler::error(_TAG, "File loading input json failed: %s", filepath);
                    return false;
                }
		        xSemaphoreTake(mutex, portMAX_DELAY);
            }
            LogHandler::debug(_TAG, "jsonSize: %ld", jsonSize);
            JsonDocument doc; //jsonSize
            if(!saveFunction(doc)) {
                LogHandler::error(_TAG, "Failed to compile JSON object: %s", filepath);
                xSemaphoreGive(mutex);
                saving = false;
                return false;
            }
            LogHandler::debug(_TAG, "Doc overflowed: %u", doc.overflowed());
            //LogHandler::debug(_TAG, "Doc memory: %u", doc.memoryUsage());
            //LogHandler::debug(_TAG, "Doc capacity: %u", doc.capacity());
            File file = LittleFS.open(filepath, FILE_WRITE);
            if (serializeJson(doc, file) == 0)
            {
                LogHandler::error(_TAG, "Failed to write to file: %s", filepath);
                file.close();
                xSemaphoreGive(mutex);
                saving = false;
                return false;
            }
            LogHandler::debug(_TAG, "File contents: %s", file.readString().c_str());
            file.close();
            printFree();
        }
        saving = false;
        xSemaphoreGive(mutex);
        return true;
    }

    static bool checkForFileAndLoad(const char* path, JsonDocument &doc, bool &loadDefault) {
        if(!LittleFS.exists(path)) {
            loadDefault = true;
        }
        if(loadDefault) {
            defaultJsonFile(path);
        }
        return loadJsonFromFile(path, doc);
    }

    static bool defaultJsonFile(const char* path) {
        LogHandler::debug(_TAG, "Defaulting file %s", path);
        if(LittleFS.exists(path)) {
            LogHandler::debug(_TAG, "Deleting file %s", path);
            if(!LittleFS.remove(path)) {
                LogHandler::error(_TAG, "Error deleting %s!", path);
                return false;
            }
        }
        LogHandler::debug(_TAG, "Creating file %s", path);
        File newFile = LittleFS.open(path, FILE_WRITE, true);
        if(!newFile) {
            LogHandler::error(_TAG, "Error creating %s!", path);
            return false;
        }
        newFile.print("{}");
        newFile.flush();
        newFile.close();
        return true;
    }

    static bool loadJsonFromFile(const char* path, JsonDocument &doc) {
        LogHandler::debug(_TAG, "Loading json file %s", path);
        if (!LittleFS.exists(path)) {
            LogHandler::error(_TAG, "%s did not exist!", path);
            return false;
        }

        File file = LittleFS.open(path, FILE_READ);
        if(!file) {
            LogHandler::error(_TAG, "%s failed to open!", path);
            return false;
        }
        if(LogDeserializationError(deserializeJson(doc, file), file.name())) {
            file.close();
            return false;
        }
        file.close();
        return true;
    }

    static bool LogDeserializationError(DeserializationError error, const char* filename) {
        if (error)
        {
            LogHandler::error(_TAG, "Error deserializing json: %s", filename);
            switch (error.code())
            {
            case DeserializationError::Code::Ok:
                LogHandler::error(_TAG, "Code: Ok");
                break;
            case DeserializationError::Code::EmptyInput:
                LogHandler::error(_TAG, "Code: EmptyInput");
                break;
            case DeserializationError::Code::IncompleteInput:
                LogHandler::error(_TAG, "Code: IncompleteInput");
                break;
            case DeserializationError::Code::InvalidInput:
                LogHandler::error(_TAG, "Code: InvalidInput");
                break;
            case DeserializationError::Code::NoMemory:
                LogHandler::error(_TAG, "Code: NoMemory");
                break;
            case DeserializationError::Code::TooDeep:
                LogHandler::error(_TAG, "Code: TooDeep");
                break;
            }
            return true;
        }
        return false;
    }

    static void setBuildFeatures()
    {
        int index = 0;
#if WIFI_TCODE
        LogHandler::debug("setBuildFeatures", "WIFI_TCODE");
        buildFeatures[index] = BuildFeature::WIFI;
        index++;
#endif
#if BLUETOOTH_TCODE
        LogHandler::debug("setBuildFeatures", "BLUETOOTH_TCODE");
        buildFeatures[index] = BuildFeature::BLUETOOTH;
        index++;
#endif
#if BLE_TCODE
        LogHandler::debug("setBuildFeatures", "BLE_TCODE");
        buildFeatures[index] = BuildFeature::BLE;
        index++;
#endif
#if DEBUG_BUILD
        LogHandler::debug("setBuildFeatures", "DEBUG_BUILD");
        buildFeatures[index] = BuildFeature::DEBUG;
        index++;
#endif
#ifdef ESP32_DA
        LogHandler::debug("setBuildFeatures", "ESP32_DA");
        buildFeatures[index] = BuildFeature::DA;
        index++;
#endif
#if BUILD_TEMP
        LogHandler::debug("setBuildFeatures", "BUILD_TEMP");
        buildFeatures[index] = BuildFeature::TEMP;
        index++;
#endif
#if BUILD_DISPLAY
        LogHandler::debug("setBuildFeatures", "BUILD_DISPLAY");
        buildFeatures[index] = BuildFeature::DISPLAY_;
        index++;
#endif
// #if TCODE_V2
//         LogHandler::debug("setBuildFeatures", "TCODE_V2");
//         buildFeatures[index] = BuildFeature::HAS_TCODE_V2;
//         index++;
// #endif
#if SECURE_WEB
        LogHandler::debug("setBuildFeatures", "HTTPS");
        buildFeatures[index] = BuildFeature::HTTPS;
        index++;
#endif
#if COEXIST
        LogHandler::debug("setBuildFeatures", "COEXIST");
        buildFeatures[index] = BuildFeature::COEXIST_FEATURE;
        index++;
#endif
        buildFeatures[(int)BuildFeature::MAX_FEATURES - 1] = {};
    }

    static void setMotorType()
    {
#ifdef MOTOR_TYPE_SERVO
       m_settingsFactory->setValue(MOTOR_TYPE_SETTING, (int)MotorType::Servo);
#elif defined MOTOR_TYPE_BLDC
       m_settingsFactory->setValue(MOTOR_TYPE_SETTING, (int)MotorType::BLDC);
#endif
    }

    static void sendMessage(const SettingProfile &profile, const char *message)
    {
        if (message_callback)
        {
            LogHandler::debug(_TAG, "sendMessage: message_callback %s", message);
            message_callback(profile, message);
        }
        else
        {
            LogHandler::debug(_TAG, "sendMessage: message_callback 0");
        }
    }

    static void setValue(JsonObject json, bool &variable, const SettingProfile &profile, const char *propertyName, bool defaultValue)
    {
        bool newValue = json[propertyName] | defaultValue;
        setValue(newValue, variable, profile, propertyName);
    }

    template<size_t n> 
    static void setValue(JsonObject json, char (&variable)[n], const SettingProfile &profile, const char *propertyName, const char *defaultValue)
    {
        const char *newValue = json[propertyName] | defaultValue;
        setValue(newValue, variable, profile, propertyName);
    }

    static void setValue(JsonObject json, int &variable, const SettingProfile &profile, const char *propertyName, int defaultValue)
    {
        int newValue = json[propertyName] | defaultValue;
        setValue(newValue, variable, profile, propertyName);
    }
    static void setValue(JsonObject json, uint8_t &variable, const SettingProfile &profile, const char *propertyName, uint8_t defaultValue)
    {
        uint8_t newValue = json[propertyName] | defaultValue;
        setValue(newValue, variable, profile, propertyName);
    }
    static void setValue(JsonObject json, uint16_t &variable, const SettingProfile &profile, const char *propertyName, uint16_t defaultValue)
    {
        uint16_t newValue = json[propertyName] | defaultValue;
        setValue(newValue, variable, profile, propertyName);
    }

    static void setValue(JsonObject json, float &variable, const SettingProfile &profile, const char *propertyName, float defaultValue)
    {
        float newValue = json[propertyName] | defaultValue;
        setValue(newValue, variable, profile, propertyName);
    }

    static void setValue(JsonObject json, std::vector<const char*> &variable, const SettingProfile &profile, const char *propertyName)
    {
        variable.clear();
        if(json[propertyName].isNull()) {
            return;
        }
        JsonArray jsonArray = json[propertyName].as<JsonArray>();
        for (int i = 0; i < jsonArray.size(); i++)
        {
            variable.push_back(jsonArray[i]);
        }
        if(initialized)
            sendMessage(profile, propertyName);
    }

    static void setValue(JsonObject json, std::vector<String> &variable, const SettingProfile &profile, const char *propertyName)
    {
        variable.clear();
        if(json[propertyName].isNull()) {
            return;
        }
        JsonArray jsonArray = json[propertyName].as<JsonArray>();
        for (int i = 0; i < jsonArray.size(); i++)
        {
            variable.push_back(jsonArray[i]);
        }
        if(initialized)
            sendMessage(profile, propertyName);
    }

    static void setValue(bool newValue, bool &variable, const SettingProfile &profile, const char *propertyName)
    {
        bool valueChanged = initialized && variable != newValue;
        LogHandler::debug(TagHandler::SettingsHandler, "Set bool '%s' oldValue '%ld' newValue '%ld' changed: '%ld'", propertyName, variable, newValue, valueChanged);
        variable = newValue;
        if (valueChanged)
            sendMessage(profile, propertyName);
    }
    
    template<size_t n> 
    static void setValue(const char *newValue, char (&variable)[n], const SettingProfile &profile, const char *propertyName)
    {
        bool valueChanged = initialized && strcmp(variable, newValue) != -1;
        LogHandler::debug(TagHandler::SettingsHandler, "Set char* '%s' oldValue '%s' newValue '%s' changed: '%ld'", propertyName, variable, newValue, valueChanged);
        strcpy(variable, newValue);
        if (valueChanged)
            sendMessage(profile, propertyName);
    }

    static void setValue(int newValue, int &variable, const SettingProfile &profile, const char *propertyName)
    {
        bool valueChanged = initialized && variable != newValue;
        LogHandler::debug(TagHandler::SettingsHandler, "Set int '%s' oldValue '%ld' newValue '%ld' changed: '%ld'", propertyName, variable, newValue, valueChanged);
        variable = newValue;
        if (valueChanged)
            sendMessage(profile, propertyName);
    }

    static void setValue(uint8_t newValue, uint8_t &variable, const SettingProfile &profile, const char *propertyName)
    {
        bool valueChanged = initialized && variable != newValue;
        LogHandler::debug(TagHandler::SettingsHandler, "Set int '%s' oldValue '%u' newValue '%u' changed: '%ld'", propertyName, variable, newValue, valueChanged);
        variable = newValue;
        if (valueChanged)
            sendMessage(profile, propertyName);
    }

    static void setValue(uint16_t newValue, uint16_t &variable, const SettingProfile &profile, const char *propertyName)
    {
        bool valueChanged = initialized && variable != newValue;
        LogHandler::debug(TagHandler::SettingsHandler, "Set int '%s' oldValue '%u' newValue '%u' changed: '%ld'", propertyName, variable, newValue, valueChanged);
        variable = newValue;
        if (valueChanged)
            sendMessage(profile, propertyName);
    }

    static void setValue(float newValue, float &variable, const SettingProfile &profile, const char *propertyName)
    {
        bool valueChanged = initialized && variable != newValue;
        LogHandler::debug(TagHandler::SettingsHandler, "Set float '%s' oldValue '%f' newValue '%f' changed: '%ld'", propertyName, variable, newValue, valueChanged);
        variable = newValue;
        if (valueChanged)
            sendMessage(profile, propertyName);
    }

    static u_int16_t calculateRange(const char *channel, int value)
    {
        return constrain(value, getChannelMin(channel), getChannelMax(channel));
    }

    // Function that gets current epoch time
    static unsigned long getTime()
    {
        time_t now;
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo))
        {
            // Serial.println("Failed to obtain time");
            return (0);
        }
        time(&now);
        return now;
    }

    static const char *machine_reset_cause()
    {
        switch (esp_reset_reason())
        {
        case ESP_RST_POWERON:
            return "Reset due to power-on event";
            break;
        case ESP_RST_BROWNOUT:
            return "Brownout reset (software or hardware)";
            break;
        case ESP_RST_INT_WDT:
            return "Reset (software or hardware) due to interrupt watchdog";
            break;
        case ESP_RST_TASK_WDT:
            return "Reset due to task watchdog";
            break;
        case ESP_RST_WDT:
            return "Reset due to other watchdogs";
            break;
        case ESP_RST_DEEPSLEEP:
            return "Reset after exiting deep sleep mode";
            break;
        case ESP_RST_SW:
            return "Software reset via esp_restart";
            break;
        case ESP_RST_PANIC:
            return "Software reset due to exception/panic";
            break;
        case ESP_RST_EXT: // Comment in ESP-IDF: "For ESP32, ESP_RST_EXT is never returned"
            return "Reset by external pin (not applicable for ESP32)";
            break;
        case ESP_RST_SDIO:
            return "Reset over SDIO";
            break;
        case ESP_RST_UNKNOWN:
            return "Reset reason can not be determined";
            break;
        default:
            return "";
            break;
        }
    }
};

SettingsFactory* SettingsHandler::m_settingsFactory;
SemaphoreHandle_t SettingsHandler::m_motionMutex = xSemaphoreCreateMutex();
SemaphoreHandle_t SettingsHandler::m_channelsMutex = xSemaphoreCreateMutex();
SemaphoreHandle_t SettingsHandler::m_wifiMutex = xSemaphoreCreateMutex();
SemaphoreHandle_t SettingsHandler::m_buttonsMutex = xSemaphoreCreateMutex();
SemaphoreHandle_t SettingsHandler::m_settingsMutex = xSemaphoreCreateMutex();
bool SettingsHandler::initialized = false;
int SettingsHandler::restartInSecs = -1;
bool SettingsHandler::saving = false;
bool SettingsHandler::motionPaused = false;
bool SettingsHandler::fullBuild = false;
bool SettingsHandler::apMode = false;

BuildFeature SettingsHandler::buildFeatures[(int)BuildFeature::MAX_FEATURES];
const char *SettingsHandler::_TAG = TagHandler::SettingsHandler;
std::vector<int> SettingsHandler::systemI2CAddresses;
ChannelMap SettingsHandler::channelMap;

char SettingsHandler::currentIP[IP_ADDRESS_LEN] = LOCALIP_DEFAULT;
char SettingsHandler::currentGateway[IP_ADDRESS_LEN] = GATEWAY_DEFAULT;
char SettingsHandler::currentSubnet[IP_ADDRESS_LEN] = SUBNET_DEFAULT;
char SettingsHandler::currentDns1[IP_ADDRESS_LEN] = DNS1_DEFAULT;
char SettingsHandler::currentDns2[IP_ADDRESS_LEN] = DNS2_DEFAULT;

bool SettingsHandler::motionEnabled = false;
int SettingsHandler::motionSelectedProfileIndex = 0;
int SettingsHandler::motionDefaultProfileIndex = 0;
