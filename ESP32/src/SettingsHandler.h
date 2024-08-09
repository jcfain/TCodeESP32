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
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include <Wire.h>
#include "LogHandler.h"
#include "utils.h"
#include "TagHandler.h"
#include "../lib/struct/voice.h"
#include "../lib/struct/motionProfile.h"
#include "../lib/struct/channel.h"
#include "../lib/struct/motionChannel.h"
#include "../lib/struct/buttonSet.h"
#include "../lib/enum.h"
#include "../lib/constants.h"
#include "../lib/channelMap.hpp"
#include "../lib/settingConstants.h"
#include "../lib/settingsFactory.h"

#define DESERIALIZE_SIZE 32768
#define SERIALIZE_SIZE 24576

//using SETTING_STATE_FUNCTION_PTR_T = void (*)(const char *group, const char *settingNameThatChanged);

class SettingsHandler
{
public:
    static bool initialized;
    static bool restartRequired;
    static bool saving;
    static bool motionPaused;
    static bool fullBuild;
    static LogLevel logLevel;
    static std::vector<int> systemI2CAddresses;

    static ChannelMap channelMap;
    static std::vector<Channel> currentChannels;
    static BuildFeature buildFeatures[featureCount];

    
    // // // Cached (Requires reboot)
    // static TCodeVersion getTcodeVersion() { return m_settingsFactory->getTcodeVersion(); }
    // static DeviceType getDeviceType() { return m_settingsFactory->getDeviceType(); }
    // static int getWebServerPort() { return m_settingsFactory->getWebServerPort(); }
    // static int getUdpServerPort() { return m_settingsFactory->getUdpServerPort(); }
    // static const char* getHostname() { return m_settingsFactory->getHostname(); }
    // static const char* getFriendlyName() { return m_settingsFactory->getFriendlyName(); }
    // // static MotorType getMotorType() { return m_settingsFactory->getMotorType(); }
    // // static BoardType getBoardType() { return m_settingsFactory->getBoardType(); }
    // // static const char* getSSID() { return m_settingsFactory->getSSID(); }
    // // static const char* getWifiPass() { return m_settingsFactory->getWifiPass(); }
    // // static int getMsPerRad() { return m_settingsFactory->getMsPerRad(); }
    // // static int getServoFrequency() { return m_settingsFactory->getServoFrequency(); }
    // // static int getPitchFrequency() { return m_settingsFactory->getPitchFrequency(); }
    // // static int getValveFrequency() { return m_settingsFactory->getValveFrequency(); }
    // // static int getTwistFrequency() { return m_settingsFactory->getTwistFrequency(); }
    // // static int getSqueezeFrequency() { return m_settingsFactory->getSqueezeFrequency(); }
    // // static bool getLubeEnabled() { return m_settingsFactory->getLubeEnabled(); }
    // // static bool getFeedbackTwist() { return m_settingsFactory->getFeedbackTwist(); }
    // // static bool getAnalogTwist() { return m_settingsFactory->getAnalogTwist(); }
    // // static bool getBootButtonEnabled() { return m_settingsFactory->getBootButtonEnabled(); }
    // // static bool getButtonSetsEnabled() { return m_settingsFactory->getButtonSetsEnabled();
    // // static bool getBatteryLevelEnabled() { return m_settingsFactory->getBatteryLevelEnabled(); }
    // // static bool getVoiceEnabled() { return m_settingsFactory->getVoiceEnabled(); }
    // // static bool getTempSleeveEnabled() { return m_settingsFactory->getTempSleeveEnabled(); }
    // // static bool getTempInternalEnabled() { return m_settingsFactory->getTempInternalEnabled(); }
    // // static bool getStaticIP() { return m_settingsFactory->getStaticIP(); }
    // // static const char* getLocalIP() { return m_settingsFactory->getLocalIP(); }
    // // static const char* getGateway() { return m_settingsFactory->getGateway(); }
    // // static const char* getSubnet() { return m_settingsFactory->getSubnet(); }
    // // static const char* getDns1() { return m_settingsFactory->getDns1(); }
    // // static const char* getDns2() { return m_settingsFactory->getDns2(); }

    // // Cached (Live update)
    // static bool getInverseStroke() { return m_settingsFactory->getInverseStroke(); }
    // static bool getInversePitch() { return m_settingsFactory->getInversePitch(); }
    // static bool getValveServo90Degrees() { return m_settingsFactory->getValveServo90Degrees(); }
    // static bool getAutoValve() { return m_settingsFactory->getAutoValve(); }
    // static bool getInverseValve() { return m_settingsFactory->getInverseValve(); }
    // static bool getContinuousTwist() { return m_settingsFactory->getContinuousTwist(); }
    // static int getLubeAmount() { return m_settingsFactory->getLubeAmount(); }
    // static int getBatteryCapacityMax() { return m_settingsFactory->getBatteryCapacityMax(); }
    // static int getRightServo_ZERO() { return m_settingsFactory->getRightServo_ZERO(); }
    // static int getLeftServo_ZERO() { return m_settingsFactory->getLeftServo_ZERO(); }
    // static int getRightUpperServo_ZERO() { return m_settingsFactory->getRightUpperServo_ZERO(); }
    // static int getLeftUpperServo_ZERO() { return m_settingsFactory->getLeftUpperServo_ZERO(); }
    // static int getPitchLeftServo_ZERO() { return m_settingsFactory->getPitchLeftServo_ZERO(); }
    // static int getPitchRightServo_ZERO() { return m_settingsFactory->getPitchRightServo_ZERO(); }
    // static int getTwistServo_ZERO() { return m_settingsFactory->getTwistServo_ZERO(); }
    // static int getValveServo_ZERO() { return m_settingsFactory->getValveServo_ZERO(); }
    // static int getSqueezeServo_ZERO() { return m_settingsFactory->getSqueezeServo_ZERO(); }
    // static const char* getBootButtonCommand() { return m_settingsFactory->getBootButtonCommand(); }
    // static uint16_t getButtonAnalogDebounce() { return m_settingsFactory->getButtonAnalogDebounce(); }
    // static bool getVersionDisplayed() { return m_settingsFactory->getVersionDisplayed(); }
    // static bool getSleeveTempDisplayed() { return m_settingsFactory->getSleeveTempDisplayed(); }
    // static bool getInternalTempDisplayed() { return m_settingsFactory->getInternalTempDisplayed(); }
    // static int getDisplayScreenWidth() { return m_settingsFactory->getDisplayScreenWidth(); }
    // static int getDisplayScreenHeight() { return m_settingsFactory->getDisplayScreenHeight(); }
    // static int getBatteryLevelNumeric() { return m_settingsFactory->getBatteryLevelNumeric(); }
    // static int getTargetTemp() { return m_settingsFactory->getTargetTemp(); }
    // static int getHeatPWM() { return m_settingsFactory->getHeatPWM(); }
    // static int getHoldPWM() { return m_settingsFactory->getHoldPWM(); }
    // static float getHeaterThreshold() { return m_settingsFactory->getHeaterThreshold(); }
    // static double getInternalMaxTemp() { return m_settingsFactory->getInternalMaxTemp(); }
    // static double getInternalTempForFanOn() { return m_settingsFactory->getInternalTempForFanOn(); }
    

    // static TCodeVersion TCodeVersionEnum;
    // static MotorType motorType;
    // static const char defaultIP[15];
    // static const char defaultGateWay[15];
    // static const char defaultSubnet[15];
    // const static char HandShakeChannel[4];
    // const static char SettingsChannel[4];
    // static bool bluetoothEnabled;
    // static bool isTcp;
    // static char ssid[32];
    // static char wifiPass[63];
    // static int udpServerPort;
    // static int webServerPort;
    // static char hostname[63];
    // static char friendlyName[100];
    // static int TwistFeedBack_PIN; // Vibe 4?
    // static int RightServo_PIN;
    // static int LeftServo_PIN;
    // static int RightUpperServo_PIN;
    // static int LeftUpperServo_PIN;
    // static int PitchLeftServo_PIN;
    // static int PitchRightServo_PIN;
    // static int ValveServo_PIN;
    // static int TwistServo_PIN;

    // static bool BLDC_UsePWM;
    // static bool BLDC_UseMT6701;
    // static bool BLDC_UseHallSensor;
    // static int BLDC_Pulley_Circumference;
    // static int BLDC_Encoder_PIN;
    // static int BLDC_HallEffect_PIN;
    // static int BLDC_ChipSelect_PIN; 
    // static int BLDC_Enable_PIN;
    // static int BLDC_PWMchannel1_PIN;
    // static int BLDC_PWMchannel2_PIN;
    // static int BLDC_PWMchannel3_PIN;
    // static float BLDC_MotorA_Voltage;
    // static float BLDC_MotorA_Current; 
    // static bool BLDC_MotorA_ParametersKnown;
    // static float BLDC_MotorA_ZeroElecAngle;
    // static int BLDC_RailLength;
    // static int BLDC_StrokeLength;

    // static int Vibe0_PIN;
    // static int Vibe1_PIN;
    // static int Vibe2_PIN;
    // static int Vibe3_PIN;
    // static int LubeButton_PIN;
    // static int Squeeze_PIN;
    // static int msPerRad;
    // static int servoFrequency;
    // static int pitchFrequency;
    // static int valveFrequency;
    // static int twistFrequency;
    // static int squeezeFrequency;
    // static bool pitchFrequencyIsDifferent;
    // static bool feedbackTwist;
    // static bool continuousTwist;
    // static bool analogTwist;
    // static bool staticIP;
    static char currentIP[IP_ADDRESS_LEN];
    static char currentGateway[IP_ADDRESS_LEN];
    static char currentSubnet[IP_ADDRESS_LEN];
    static char currentDns1[IP_ADDRESS_LEN];
    static char currentDns2[IP_ADDRESS_LEN];
    // static bool sr6Mode;
    // static DeviceType deviceType;
    // static int RightServo_ZERO;
    // static int LeftServo_ZERO;
    // static int RightUpperServo_ZERO;
    // static int LeftUpperServo_ZERO;
    // static int PitchLeftServo_ZERO;
    // static int PitchRightServo_ZERO;
    // static int TwistServo_ZERO;
    // static int ValveServo_ZERO;
    // static int SqueezeServo_ZERO;
    // static bool autoValve;
    // static bool inverseValve;
    // static bool inverseStroke;
    // static bool inversePitch;
    // static bool valveServo90Degrees;
    // static bool lubeEnabled;
    // static int lubeAmount;
    // static bool displayEnabled;
    // static bool sleeveTempDisplayed;
    // static bool internalTempDisplayed;
    // static bool versionDisplayed;
    // static bool tempSleeveEnabled;
    // static bool tempInternalEnabled;
    // static bool fanControlEnabled;
    // static bool batteryLevelEnabled;

    // //static int Battery_Voltage_PIN;

    // static bool batteryLevelNumeric;
    // static double batteryVoltageMax;
    // static int batteryCapacityMax;
    // static int Display_Screen_Width;
    // static int Display_Screen_Height;
    // static int Internal_Temp_PIN;
    // static int Case_Fan_PIN;
    // static int Sleeve_Temp_PIN;
    // static int caseFanMaxDuty;
    // static double internalTempForFan; // C
    // static double internalMaxTemp;    // C
    // static int Heater_PIN;
    // //static int HeatLED_PIN;
    // static int I2C_SDA_PIN_obsolete;
    // static int I2C_SCL_PIN_obsolete;
    // static int TargetTemp; // Desired Temp in degC
    // static int HeatPWM;    // Heating PWM setting 0-255
    // static int HoldPWM;    // Hold heat PWM setting 0-255
    // static int Display_I2C_Address;
    // static int Display_Rst_PIN;
    // // static long heaterFailsafeTime;
    // static float heaterThreshold;
    // static int heaterResolution;
    // static int heaterFrequency;
    // static int caseFanFrequency;
    // static int caseFanResolution;

    // static bool bootButtonEnabled;
    // static bool buttonSetsEnabled;
    // static char bootButtonCommand[MAX_COMMAND];
    static ButtonSet buttonSets[MAX_BUTTON_SETS];
    // static uint8_t defaultButtonSetPin;
    // static uint16_t buttonAnalogDebounce;

    // static VoiceCommand voiceCommands[147];

    static const char *lastRebootReason;

    // static const char *userSettingsFilePath;
    // static const char *wifiPassFilePath;
    // static const char *buttonsFilePath;
    // static const char *motionProfilesFilePath;
    // static const char *logPath;
    // static const char *defaultWifiPass;
    // static const char *decoyPass;
    static bool apMode;

    
    // template<typename T,
    //          typename = std::enable_if<!std::is_const<T>::value || std::is_integral<T>::value || std::is_enum<T>::value || std::is_floating_point<T>::value || std::is_same<T, bool>::value>>
    // static void getValue(const char* name, T &value)
    // {
    //     m_settingsFactory->getValue(name, value);
    // }
    
    // static void getValue(const char* name, char* value, size_t len)
    // {
    //     m_settingsFactory->getValue(name, value, len);
    // }

    // static void defaultValue(const char* name) 
    // {
    //     m_settingsFactory->defaultValue(name);
    // }

    static void init()
    {
        setBuildFeatures();
        setMotorType();

        // loadWifiInfo(false);
        // loadSettings(false);
        loadMotionProfiles(false);
        loadButtons(false);

        initialized = true;
    }

    static void setMessageCallback(SETTING_STATE_FUNCTION_PTR_T f)
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

    // static bool isBoardType(BoardType value) {
    //     return m_settingsFactory->getBoardType() == value;
    // }

    static void printFree() {
        //https://esp32.com/viewtopic.php?t=27780
        //https://github.com/espressif/esp-idf/blob/master/components/heap/include/esp_heap_caps.h#L20-L37
        uint32_t freeHEap = ESP.getFreeHeap();
        uint32_t heapSize = ESP.getHeapSize();
        //esp_get_free_internal_heap_size
        LogHandler::debug(_TAG, "Used heap INTERNAL: %u/%u Free: %u", heapSize - freeHEap, heapSize, freeHEap);
        //LogHandler::debug(_TAG, "Used Psram: %u/%u", ESP.getPsramSize() - ESP.getFreePsram(), ESP.getPsramSize());
        LogHandler::debug(_TAG, "Used sketch size: %u/%u", ESP.getSketchSize(), ESP.getSketchSize() + ESP.getFreeSketchSpace());
        LogHandler::debug(_TAG, "DRAM %u", heap_caps_get_free_size(MALLOC_CAP_8BIT));
        LogHandler::debug(_TAG, "IRAM %u", heap_caps_get_free_size(MALLOC_CAP_32BIT));
        LogHandler::debug(_TAG, "FREE_HEAP Default %u", esp_get_free_heap_size());
        LogHandler::debug(_TAG, "MIN_FREE_HEAP %u", esp_get_minimum_free_heap_size() );
    }

	static void restart() {
		LogHandler::info(_TAG, "Schedule device restart...");
		//ESP.restart();
		restartRequired = true;
		delay(1000);
	}

    static void printMemory()
    {
        LogHandler::debug(_TAG, "Free heap: %u", ESP.getFreeHeap());
        LogHandler::debug(_TAG, "Total heap: %u", ESP.getHeapSize());
        LogHandler::debug(_TAG, "Free psram: %u", ESP.getFreePsram());
        LogHandler::debug(_TAG, "Total Psram: %u", ESP.getPsramSize());
        LogHandler::debug(_TAG, "LittleFS used: %i", LittleFS.usedBytes());
        LogHandler::debug(_TAG, "LittleFS total: %i", LittleFS.totalBytes());
    }

    static void printWebAddress(const char* hostAddress) 
    {
        char webServerportString[6];
        int webServerPort = 0;
        m_settingsFactory->getValue(WEBSERVER_PORT, webServerPort);
        sprintf(webServerportString, ":%ld", webServerPort);
        LogHandler::info(_TAG, "Web address: http://%s%s", hostAddress, webServerPort == 80 ? "" : webServerportString);
    }

    static bool defaultAll()
    {
        return m_settingsFactory->resetAll() && loadMotionProfiles(true) && loadButtons(true); 
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
        printMemory();
        JsonDocument doc;
    
        DeserializationError error = deserializeJson(doc, data);
        if (error)
        {
            LogHandler::error(_TAG, "Settings save: Deserialize error: %s", error.c_str());
            return false;
        }
        JsonObject obj = doc.as<JsonObject>();
        if (!saveAll(obj))
        {
            LogHandler::error(_TAG, "Settings save: save error");
            return false;
        }
        return true;
    }

    static bool defaultPinout() {
		//setBoardPinout();
        if(!m_settingsFactory->resetPins())
            return false;
        const PinMap* pinMap = m_settingsFactory->getPins().pinMap();
        for(int i = 0; i < MAX_BUTTON_SETS; i++) {
            buttonSets[i].pin = pinMap->buttonSetPin(i);
        }
        return m_settingsFactory->savePins();
    }

    static void getWifiInfo(char buf[100])
    {
        JsonDocument doc; //100

        char ssid[SSID_LEN];
        char wifiPass[WIFI_PASS_LEN];
        m_settingsFactory->getValue(SSID_SETTING, ssid, SSID_LEN);
        m_settingsFactory->getValue(WIFI_PASS_SETTING, wifiPass, WIFI_PASS_LEN);
        doc["ssid"] = ssid;
        
        if(strcmp(wifiPass, WIFI_PASS_DEFAULT) != 0) {
            doc["wifiPass"] = DECOY_PASS; // Never set to actual password
        } else {
            doc["wifiPass"] = WIFI_PASS_DEFAULT;
        }

        String output;
        serializeJson(doc, output);
        doc.clear();
        if (LogHandler::getLogLevel() == LogLevel::VERBOSE)
            Serial.printf("WifiInfo: %s\n", output.c_str());
        buf[0] = {0};
        strcpy(buf, output.c_str());
    }

    static void getSystemInfo(String &buf)
    {
        JsonDocument doc; // 3500

        doc["esp32Version"] = FIRMWARE_VERSION_NAME;
        doc["TCodeVersion"] = m_settingsFactory->getTcodeVersion();
        doc["lastRebootReason"] = lastRebootReason;
        
        JsonArray boardTypes = doc["boardTypes"].to<JsonArray>();
        JsonObject devkit = boardTypes.add<JsonObject>();
        devkit["name"] = "Devkit/NexusPRO";
        devkit["value"] = (uint8_t)BoardType::DEVKIT;
        JsonObject SR6MB = boardTypes.add<JsonObject>();
        SR6MB["name"] = "SR6MB";
        SR6MB["value"] = (uint8_t)BoardType::CRIMZZON;
        JsonObject INControl = boardTypes.add<JsonObject>();
        INControl["name"] = "IN-Control";
        INControl["value"] = (uint8_t)BoardType::ISAAC;
        int motorType = MOTOR_TYPE_DEFAULT;
        m_settingsFactory->getValue(MOTOR_TYPE_SETTING, motorType);
        doc["motorType"] = motorType;
        JsonArray buildFeaturesJsonArray = doc["buildFeatures"].to<JsonArray>();
        for (BuildFeature value : buildFeatures)
        {
            buildFeaturesJsonArray.add((int)value);
        }

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

        JsonArray deviceTypes = doc["deviceTypes"].to<JsonArray>();
        JsonObject defaultDevice = deviceTypes.add<JsonObject>();
    #if MOTOR_TYPE == 0
        defaultDevice["name"] = "OSR";
        defaultDevice["value"] = (uint8_t)DeviceType::OSR;
        JsonObject SR6 = deviceTypes.add<JsonObject>();
        SR6["name"] = "SR6";
        SR6["value"] = (uint8_t)DeviceType::SR6;
    #elif MOTOR_TYPE == 1
        defaultDevice["name"] = "SSR1";
        defaultDevice["value"] = (uint8_t)DeviceType::SSR1;
        JsonArray encoderTypes = doc["encoderTypes"].to<JsonArray>();
        JsonObject defaultEncoder = encoderTypes.add<JsonObject>();
        defaultEncoder["name"] = "MT6701 SSI";
        defaultEncoder["value"] = (uint8_t)BLDCEncoderType::MT6701;
        JsonObject PWM = encoderTypes.add<JsonObject>();
        PWM["name"] = "PWM";
        PWM["value"] = (uint8_t)BLDCEncoderType::PWM;
        JsonObject SPI = encoderTypes.add<JsonObject>();
        SPI["name"] = "SPI";
        SPI["value"] = (uint8_t)BLDCEncoderType::SPI;
    #endif

        JsonArray availableChannels = doc["availableChannels"].to<JsonArray>();
        channelMap.serialize(availableChannels);
        doc[MOTION_ENABLED] = getMotionEnabled();
        // int motionProfileSelectedIndex = MOTION_PROFILE_SELECTED_INDEX_DEFAULT;
        // m_settingsFactory->getValue(MOTION_PROFILE_SELECTED_INDEX, motionProfileSelectedIndex);
        doc[MOTION_PROFILE_SELECTED_INDEX] = motionSelectedProfileIndex; 
        
        doc["localIP"] = currentIP;
        doc["gateway"] = currentGateway;
        doc["subnet"] = currentSubnet;
        doc["dns1"] = currentDns1;
        doc["dns2"] = currentDns2;// Not being used currently

        doc["chipModel"] = ESP.getChipModel();
        doc["chipRevision"] = ESP.getChipRevision();
        doc["chipCores"] = ESP.getChipCores();
        uint32_t chipId = 0;
        for (int i = 0; i < 17; i = i + 8)
        {
            chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
        }
        doc["chipID"] = chipId;

        doc["decoyPass"] = DECOY_PASS;
        doc["apMode"] = apMode;
        doc["defaultIP"] = DEFAULT_IP;
        //String output;
        serializeJson(doc, buf);
        doc.clear();
        if (LogHandler::getLogLevel() == LogLevel::VERBOSE)
            Serial.printf("SystemInfo: %s\n", buf.c_str());
        // buf[0] = {0};
        // strcpy(buf, output.c_str());
    }

    // static bool loadSettings(bool loadDefault, JsonObject json = JsonObject()) {
    //     LogHandler::info(_TAG, "Loading common settings");
    //     return loadSettingsJson(userSettingsFilePath, loadDefault, m_settingsMutex, deserializeSize, [](const JsonObject json, bool& mutableLoadDefault) -> bool {
    //         if(!update(json)) {
    //             return false;
    //         }
    //         LogUpdateDebug();
    //         return true;
    //     }, saveSettings, json);
    // }

    // static bool saveSettings(JsonObject json = JsonObject()) {
    //     saving = true;
    //     LogHandler::info(_TAG, "Save common settings file");
    //     return saveSettingsJson(userSettingsFilePath, m_settingsMutex, serializeSize, [](DynamicJsonDocument& doc) -> bool {
    //         if(!compileCommonJsonDocument(doc)) {
    //             return false;
    //         }
    //         LogSaveDebug(doc);
    //         return true;
    //     }, loadSettings, json);
    // }

    // static bool loadWifiInfo(bool loadDefault, JsonObject json = JsonObject()) {
    //     LogHandler::info(_TAG, "Loading wifi info");
    //     bool mutableLoadDefault = loadDefault;
    //     DynamicJsonDocument doc(100);
    //     if(mutableLoadDefault || json.isNull()) {
	// 	    xSemaphoreTake(m_wifiMutex, portMAX_DELAY);
    //         if(!checkForFileAndLoad(wifiPassFilePath, json, doc, mutableLoadDefault)) {
    //             xSemaphoreGive(m_wifiMutex);
    //             return false;
    //         }
    //     }
    //     const char *ssidConst = json["ssid"] | "YOUR SSID HERE";
    //     if (ssid != nullptr)
    //     {
    //         strcpy(ssid, ssidConst);
    //     }
    //     const char *wifiPassConst = json["wifiPass"] | defaultWifiPass;
    //     //LogHandler::info(_TAG, "Pass: %s",wifiPassConst);
    //     if (strcmp(wifiPassConst, SettingsHandler::decoyPass) != 0)
    //     {
    //         strcpy(wifiPass, wifiPassConst);
    //     }
    //     xSemaphoreGive(m_wifiMutex);
    //     if(mutableLoadDefault)
    //         saveWifiInfo();
    //     return true;
    // }

    // static bool saveWifiInfo(JsonObject json = JsonObject()) {
    //     LogHandler::info(_TAG, "Save Wifi info file");
    //     saving = true;
	// 	xSemaphoreTake(m_wifiMutex, portMAX_DELAY);
    //     if (!LittleFS.exists(wifiPassFilePath)) {
    //         LogHandler::error(_TAG, "Wifi info file did not exist whan saving.");
    //         xSemaphoreGive(m_wifiMutex);
    //         saving = false;
    //         return false;
    //     } else {
    //         if(!json.isNull()) {
    //             xSemaphoreGive(m_wifiMutex);
    //             if(!loadWifiInfo(false, json)){
    //                 LogHandler::error(_TAG, "File loading wifi input json failed");
    //                 return false;
    //             }
	// 	        xSemaphoreTake(m_wifiMutex, portMAX_DELAY);
    //         }

    //         DynamicJsonDocument doc(100);
    //         File file = LittleFS.open(wifiPassFilePath, FILE_WRITE);
    //         //LogHandler::info(_TAG, "Pass: %s",wifiPass);
    //         doc["ssid"] = ssid;
    //         doc["wifiPass"] = wifiPass;
    //         if (serializeJson(doc, file) == 0)
    //         {
    //             LogHandler::error(_TAG, "Failed to write to password file");
    //             doc["wifiPass"] = defaultWifiPass;
    //             file.close();
    //             xSemaphoreGive(m_wifiMutex);
    //             saving = false;
    //             return false;
    //         }
    //         file.close();
    //     }
    //     saving = false;
    //     xSemaphoreGive(m_wifiMutex);
    //     return true;
    // }

    static bool loadButtons(bool loadDefault, JsonObject json = JsonObject()) {
        LogHandler::info(_TAG, "Loading buttons");
        uint16_t docSize = 3000;
        return loadSettingsJson(BUTTON_SETTINGS_PATH, loadDefault, m_buttonsMutex, docSize, [](const JsonObject json, bool& mutableLoadDefault) -> bool {
            
            // const bool bootButtonEnabled = SettingsHandler::getValue<const bool>(BOOT_BUTTON_ENABLED);
            // const bool buttonSetsEnabled = SettingsHandler::getValue<const bool>(BUTTON_SETS_ENABLED);;
            // const char* bootButtonCommand =  SettingsHandler::getValue<const char*>(BOOT_BUTTON_COMMAND);;
            // const int buttonAnalogDebounce = SettingsHandler::getValue<const int>(BUTTON_ANALOG_DEBOUNCE);
            // setValue(json, bootButtonEnabled, "buttonCommand", "bootButtonEnabled", BOOT_BUTTON_ENABLED_DEFAULT);
            // setValue(json, buttonSetsEnabled, "buttonCommand", "buttonSetsEnabled", BOOT_BUTTON_COMMAND_DEFAULT);
            // setValue(json, bootButtonCommand, "buttonCommand", "bootButtonCommand", BUTTON_SETS_ENABLED_DEFAULT);
            // setValue(json, buttonAnalogDebounce, "buttonCommand", "buttonAnalogDebounce", BUTTON_ANALOG_DEBOUNCE_DEFAULT);
            bool bootButtonEnabled = json[BOOT_BUTTON_ENABLED] | BOOT_BUTTON_ENABLED_DEFAULT;
            m_settingsFactory->setValue(BOOT_BUTTON_ENABLED, bootButtonEnabled);
            bool buttonSetsEnabled = json[BUTTON_SETS_ENABLED] | BUTTON_SETS_ENABLED_DEFAULT;
            m_settingsFactory->setValue(BUTTON_SETS_ENABLED, buttonSetsEnabled);
            const char* bootButtonCommand = json[BOOT_BUTTON_COMMAND] | BOOT_BUTTON_COMMAND_DEFAULT;
            m_settingsFactory->setValue(BOOT_BUTTON_COMMAND, bootButtonCommand);
            int buttonAnalogDebounce = json[BUTTON_ANALOG_DEBOUNCE] | BUTTON_ANALOG_DEBOUNCE_DEFAULT;
            m_settingsFactory->setValue(BUTTON_ANALOG_DEBOUNCE, buttonAnalogDebounce);
            m_settingsFactory->saveCommon();

            JsonArray buttonSetsObj = json["buttonSets"].as<JsonArray>();
            if(buttonSetsObj.isNull()) {
                LogHandler::info(_TAG, "No button sets stored, loading default");
                mutableLoadDefault = true;
                
                const PinMap* pinMap = m_settingsFactory->getPins().pinMap();
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
        bool mutableLoadDefault = loadDefault;
        JsonDocument doc; //deserializeSize
        if(mutableLoadDefault || json.isNull()) {
		    xSemaphoreTake(m_motionMutex, portMAX_DELAY);
            if(!checkForFileAndLoad(MOTION_PROFILE_SETTINGS_PATH, json, doc, mutableLoadDefault)) {
                saving = false;
                xSemaphoreGive(m_motionMutex);
                return false;
            }
        }
        motionDefaultProfileIndex = json[MOTION_PROFILE_DEFAULT_INDEX] | MOTION_PROFILE_SELECTED_INDEX_DEFAULT;
        if(!initialized)
            motionSelectedProfileIndex = motionDefaultProfileIndex;
            
        JsonArray motionProfilesObj = json[MOTION_PROFILES].as<JsonArray>();
        
        if(motionProfilesObj.isNull()) {
            LogHandler::info(_TAG, "No motion profiles stored, loading default");
            mutableLoadDefault = true;
            for(int i = 0; i < maxMotionProfileCount; i++) {
                motionProfiles[i] = MotionProfile(i + 1);
                motionProfiles[i].addDefaultChannel("L0");
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
        xSemaphoreGive(m_motionMutex);
        if(mutableLoadDefault)
            saveMotionProfiles();
        return true;
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
        int len = sizeof(motionProfiles)/sizeof(motionProfiles[0]);
        LogHandler::debug(_TAG, "motion profiles length: %ld", len);
        for (int i=0; i < len; i++) {
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

    static std::vector<MotionChannel> getMotionChannels()
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
    static void setMotionProfileName(const char newValue[maxMotionProfileNameLength])
    {
        strcpy(motionProfiles[motionSelectedProfileIndex].motionProfileName, newValue);
    }

    static bool getRestartRequired() 
    {
        return restartRequired;
    }
    static void setRestartRequired(const bool& newValue)
    {
        setValue(newValue, restartRequired, SettingProfile::System, "restartRequired");
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
        int len = sizeof(motionProfiles)/sizeof(motionProfiles[0]);
        for (size_t i = 0; i < len; i++)
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

    static void setMotionProfile(const char profile[maxMotionProfileNameLength]) {
        auto index = motionProfileExists(profile);
        if(index < 0) {
            LogHandler::error(_TAG, "Motion profile %s does not exist", profile);
            return;
        }
        setMotionProfile(index);
    }

    static void setMotionProfile(const int& index) {
        int len = sizeof(motionProfiles)/sizeof(motionProfiles[0]);
        if(index < 0 || index > len - 1) {
            LogHandler::error(_TAG, "Invalid motion profile index: %ld", index);
            return;
        }
        auto newProfile = motionProfiles[index];
        setMotionProfile(newProfile, index);
    }

    static void setMotionProfile(const MotionProfile& profile, int profileIndex) {
        int len = sizeof(motionProfiles)/sizeof(motionProfiles[0]);
        if(profileIndex < 0 || profileIndex > len - 1) {
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
        int len = sizeof(motionProfiles)/sizeof(motionProfiles[0]);
        uint8_t newProfileIndex = motionSelectedProfileIndex + 1;
        if(newProfileIndex > len - 1) {
            newProfileIndex = 0;
            setMotionEnabled(false);
        }
        auto newProfile = motionProfiles[newProfileIndex];
        setMotionProfile(newProfile, newProfileIndex);
    }

    static ButtonSet* getButtonSets() {
        return buttonSets;
    }

    // static bool getVoiceEnabled() {
    //     return voiceEnabled;
    // }

    // static void setVoiceEnabled(bool value) {
    //     setValue(value, voiceEnabled, "voiceHandler", "voiceEnabled");
    // }

    // static bool getVoiceMuted() {
    //     return voiceMuted;
    // }

    // static void setVoiceMuted(bool value) {
    //     setValue(value, voiceMuted, "voiceHandler", "voiceMuted");
    // }

    // static int getVoiceVolume() {
    //     return voiceVolume;
    // }

    // static void setVoiceVolume(int value) {
    //     setValue(value, voiceVolume, "voiceHandler", "voiceVolume");
    // }

    // static int getVoiceWakeTime() {
    //     return voiceWakeTime;
    // }

    // static void setVoiceWakeTime(int value) {
    //     setValue(value, voiceWakeTime, "voiceHandler", "voiceWakeTime");
    // }

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
    //             sprintf(integer_string, SettingsHandler::TCodeVersionEnum == TCodeVersion::v0_2 ? "%03d" : "%04d", SettingsHandler::calculateRange(channel, value));
    //             // pad(integer_string);
    //             // sprintf(integer_string, "%d", SettingsHandler::calculateRange(channel, value));
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
		    LogHandler::debug(_TAG, "SDA or SCL is disabled when scaning for I2C devices sdaPin: %ld, sclPin: %ld", sdaPin, sclPin);
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
    
    static u_int16_t getChannelMin(const char *channel) 
    {
        for (size_t i = 0; i < currentChannels.size(); i++)
        {
            if(strcmp(currentChannels[i].Name, channel) == 0)
                return currentChannels[i].min;
        }
        return 1;
    }

    static u_int16_t getChannelMax(const char *channel) 
    {
        for (size_t i = 0; i < currentChannels.size(); i++) {
            if(strcmp(currentChannels[i].Name, channel) == 0)
                return currentChannels[i].max;
        }
        return 9999;
    }

    static void setChannelMin(const char *channel, u_int16_t value) 
    {
        for (size_t i = 0; i < currentChannels.size(); i++) {
            if(strcmp(currentChannels[i].Name, channel) == 0) {
                currentChannels[i].min = value;
                return;
            }
        }
    }

    static void setChannelMax(const char *channel, u_int16_t value) 
    {
        for (size_t i = 0; i < currentChannels.size(); i++) {
            if(strcmp(currentChannels[i].Name, channel) == 0) {
                currentChannels[i].max = value;
                return;
            }
        }
    }

private:
    static const char *_TAG;
    
    static SettingsFactory* m_settingsFactory;
	static SemaphoreHandle_t m_motionMutex;
	static SemaphoreHandle_t m_wifiMutex;
    static SemaphoreHandle_t m_buttonsMutex;
	static SemaphoreHandle_t m_settingsMutex;
    static SETTING_STATE_FUNCTION_PTR_T message_callback;
    // Use http://arduinojson.org/assistant to compute the capacity.
    // static const size_t readCapacity = JSON_OBJECT_SIZE(100) + 2000;
    // static const size_t saveCapacity = JSON_OBJECT_SIZE(100);
    static const int deserializeSize = 32768;
    static const int serializeSize = 24576;
    // 3072

    static bool motionEnabled;
    static int motionSelectedProfileIndex;
    static int motionDefaultProfileIndex;
    static MotionProfile motionProfiles[maxMotionProfileCount];
    
    // static bool voiceEnabled;
    // static bool voiceMuted;
    // static int voiceWakeTime ;
    // static int voiceVolume;

//     static bool update(JsonObject json)
//     {
//         logLevel = (LogLevel)(json["logLevel"] | 2);
//         LogHandler::setLogLevel(logLevel);
//         LogHandler::debug(_TAG, "Load Json: Memory usage: %u bytes", json.memoryUsage());

//         boardType = (BoardType)(json["boardType"] | (uint8_t)BoardType::DEVKIT);

//         if(isBoardType(BoardType::CRIMZZON) || isBoardType(BoardType::ISAAC)) {
//             TCodeVersionEnum = TCodeVersion::v0_3;
//             TCodeVersionName = TCodeVersionMapper(TCodeVersionEnum);
//         }
        // std::vector<String> includesVec;
        // setValue(json, includesVec, "log", "log-include-tags");
        // LogHandler::setIncludes(includesVec);

        // std::vector<String> excludesVec;
        // setValue(json, excludesVec, "log", "log-exclude-tags");
        // LogHandler::setExcludes(excludesVec);

//         if(!isBoardType(BoardType::CRIMZZON)) {
//             TCodeVersionEnum = (TCodeVersion)(json["TCodeVersion"] | 1);
//             TCodeVersionName = TCodeVersionMapper(TCodeVersionEnum);
//         }

//         channelMap.init(TCodeVersionEnum, motorType);

// #if MOTOR_TYPE == 1
//         for (auto x : ChannelMapBLDC) {
//             currentChannels.push_back(x);
//         }
// #else
//         currentChannels.clear();
//         if(TCodeVersionEnum == TCodeVersion::v0_2) {
//             // for (size_t i = 0; i < (sizeof(ChannelMapV2)/sizeof(Channel)); i++) {
//             //     currentChannels.push_back(ChannelMapV2[i]);
//             // }
//             for (auto x : ChannelMapV2) {
//                 currentChannels.push_back(x);
//             }
//         } else {
//             for (auto x : ChannelMapV3) {
//                 currentChannels.push_back(x);
//             }
//         }  
// #endif


//         int tcodeMax = TCodeVersionEnum == TCodeVersion::v0_2 ? 999 : 9999;
//         for (size_t i = 0; i < currentChannels.size(); i++)
//         {
//             uint16_t min = json["channelRanges"][currentChannels[i].Name]["min"].as<uint16_t>();
//             uint16_t max = json["channelRanges"][currentChannels[i].Name]["max"].as<uint16_t>();
//             currentChannels[i].min = !min ? 1 : min;
//             currentChannels[i].max = !max ? tcodeMax : max;
//         }
        
//         sendMessage("channelRanges", "channelRanges");// TODO: channelranges should be in its own json

//         udpServerPort = json["udpServerPort"] | 8000;
//         webServerPort = json["webServerPort"] | 80;
//         const char *hostnameTemp = json["hostname"] | "tcode";
//         if (hostnameTemp != nullptr)
//             strcpy(hostname, hostnameTemp);
//         const char *friendlyNameTemp = json["friendlyName"] | "ESP32 TCode";
//         if (friendlyNameTemp != nullptr)
//             strcpy(friendlyName, friendlyNameTemp);

//         bluetoothEnabled = json["bluetoothEnabled"] | false;

//         // Servo motors//////////////////////////////////////////////////////////////////////////////////
//         pitchFrequencyIsDifferent = json["pitchFrequencyIsDifferent"];
//         msPerRad = json["msPerRad"] | 637;
//         servoFrequency = json["servoFrequency"] | 50;
//         pitchFrequency = json[pitchFrequencyIsDifferent ? "pitchFrequency" : "servoFrequency"] | servoFrequency;
//         sr6Mode = json["sr6Mode"];

//         RightServo_ZERO = json["RightServo_ZERO"] | 1500;
//         LeftServo_ZERO = json["LeftServo_ZERO"] | 1500;
//         RightUpperServo_ZERO = json["RightUpperServo_ZERO"] | 1500;
//         LeftUpperServo_ZERO = json["LeftUpperServo_ZERO"] | 1500;
//         PitchLeftServo_ZERO = json["PitchLeftServo_ZERO"] | 1500;
//         PitchRightServo_ZERO = json["PitchRightServo_ZERO"] | 1500;

//         BLDC_UsePWM = json["BLDC_UsePWM"] | false; // Must be before pinout is set
//         BLDC_UseMT6701 = json["BLDC_UseMT6701"] | true;
//         BLDC_UseHallSensor = json["BLDC_UseHallSensor"] | false;
//         BLDC_Pulley_Circumference = json["BLDC_Pulley_Circumference"] | 60;
//         BLDC_MotorA_Voltage = round2(json["BLDC_MotorA_Voltage"] | 20.0);
//         BLDC_MotorA_Current = round2(json["BLDC_MotorA_Current"] | 1.0);
//         BLDC_MotorA_ParametersKnown = json["BLDC_MotorA_ParametersKnown"] | false;
//         BLDC_MotorA_ZeroElecAngle = round2(json["BLDC_MotorA_ZeroElecAngle"] | 0.00);
//         BLDC_RailLength = json["BLDC_RailLength"] | 125;
//         BLDC_StrokeLength = json["BLDC_StrokeLength"] | 120;

//         setBoardPinout(json);
        
//         if(isBoardType(BoardType::CRIMZZON)) {
//             heaterResolution = json["heaterResolution"] | 8;
//             caseFanResolution = json["caseFanResolution"] | 10;
//             caseFanFrequency = json["caseFanFrequency"] | 25;
//             Display_Screen_Height = json["Display_Screen_Height"] | 32;
//         }

//         twistFrequency = json["twistFrequency"] | 50;
//         squeezeFrequency = json["squeezeFrequency"] | 50;
//         valveFrequency = json["valveFrequency"] | 50;
//         continuousTwist = json["continuousTwist"];
//         feedbackTwist = json["feedbackTwist"];
//         analogTwist = json["analogTwist"];
//         TwistServo_ZERO = json["TwistServo_ZERO"] | 1500;
//         ValveServo_ZERO = json["ValveServo_ZERO"] | 1500;
//         SqueezeServo_ZERO = json["Squeeze_ZERO"] | 1500;

//         staticIP = json["staticIP"];
//         const char *localIPTemp = json["localIP"] | "192.168.0.150";
//         if (localIPTemp != nullptr)
//             strcpy(localIP, localIPTemp);
//         const char *gatewayTemp = json["gateway"] | "192.168.0.1";
//         if (gatewayTemp != nullptr)
//             strcpy(gateway, gatewayTemp);
//         const char *subnetTemp = json["subnet"] | "255.255.255.0";
//         if (subnetTemp != nullptr)
//             strcpy(subnet, subnetTemp);
//         const char *dns1Temp = json["dns1"] | "8.8.8.8";
//         if (dns1Temp != nullptr)
//             strcpy(dns1, dns1Temp);
//         const char *dns2Temp = json["dns2"] | "8.8.4.4";
//         if (dns2Temp != nullptr)
//             strcpy(dns2, dns2Temp);

//         autoValve = json["autoValve"];
//         inverseValve = json["inverseValve"];
//         valveServo90Degrees = json["valveServo90Degrees"];
//         inverseStroke = json["inverseStroke"];
//         inversePitch = json["inversePitch"];
//         lubeEnabled = json["lubeEnabled"];
//         lubeAmount = json["lubeAmount"] | 255;
//         displayEnabled = json["displayEnabled"] | true;
//         sleeveTempDisplayed = json["sleeveTempDisplayed"];
//         internalTempDisplayed = json["internalTempDisplayed"];
//         versionDisplayed = json["versionDisplayed"] | true;
//         Display_Screen_Width = json["Display_Screen_Width"] | 128;
//         if(!isBoardType(BoardType::CRIMZZON)) {
//             Display_Screen_Height = json["Display_Screen_Height"] | 64;
//         }
//         const char *Display_I2C_AddressTemp = json["Display_I2C_Address"] | "0x3c";
//         if (Display_I2C_AddressTemp != nullptr)
//             Display_I2C_Address = (int)strtol(Display_I2C_AddressTemp, NULL, 0);
//         Display_Rst_PIN = json["Display_Rst_PIN"] | -1;

//         tempSleeveEnabled = json["tempSleeveEnabled"];
//         heaterThreshold = json["heaterThreshold"] | 5.0;
//         heaterFrequency = json["heaterFrequency"] | 50;
//         if(!isBoardType(BoardType::CRIMZZON)) {
//             heaterResolution = json["heaterResolution"] | 8;
//         }
//         TargetTemp = json["TargetTemp"] | 40.0;
//         HeatPWM = json["HeatPWM"] | 255;
//         HoldPWM = json["HoldPWM"] | 110;

//         tempInternalEnabled = json["tempInternalEnabled"];
//         fanControlEnabled = json["fanControlEnabled"];
//         internalTempForFan = json["internalTempForFan"] | 30.0;
//         internalMaxTemp = json["internalMaxTemp"] | 50.0;

//         batteryLevelEnabled = json["batteryLevelEnabled"];
//         batteryLevelNumeric = json["batteryLevelNumeric"];
//         batteryVoltageMax = json["batteryVoltageMax"] | 12.6;
//         batteryCapacityMax = json["batteryCapacityMax"] | 3500;

//         if(!isBoardType(BoardType::CRIMZZON)) {
//             caseFanFrequency = json["caseFanFrequency"] | 25;
//             caseFanResolution = json["caseFanResolution"] | 10;
//         }
//         caseFanMaxDuty = pow(2, caseFanResolution) - 1;
        
//         lubeEnabled = json["lubeEnabled"];

//         setValue(json, voiceEnabled, "voiceHandler", "voiceEnabled", false);
//         setValue(json, voiceMuted, "voiceHandler", "voiceMuted", false);
//         setValue(json, voiceVolume, "voiceHandler", "voiceVolume", 0);
//         setValue(json, voiceWakeTime, "voiceHandler", "voiceWakeTime", 10);

//         lastRebootReason = machine_reset_cause();
//         LogHandler::debug(_TAG, "Last reset reason: %s", SettingsHandler::lastRebootReason);

//         LogUpdateDebug();
//         return true;
//     }

//     static bool compileCommonJsonDocument(DynamicJsonDocument& doc)
//     {
//         // LogHandler::info(_TAG, "Save settings");
//         // Delete existing file, otherwise the configuration is appended to the file
//         // Serial.print("LittleFS used: ");
//         // Serial.println(LittleFS.usedBytes() + "/" + LittleFS.totalBytes());
//         // if (!LittleFS.remove(userSettingsFilePath))
//         // {
//         //     LogHandler::error(_TAG, "Failed to remove settings file: %s", userSettingsFilePath);
//         // }
//         // File file = LittleFS.open(userSettingsFilePath, FILE_WRITE);
//         // if (!file)
//         // {
//         //     LogHandler::error(_TAG, "Failed to create settings file: %s", userSettingsFilePath);
//         //     return false;
//         // }

//         // // Allocate a temporary docdocument
//         // DynamicdocDocument doc(serializeSize);

//         doc["boardType"] = (uint8_t)boardType;
//         doc["logLevel"] = (int)logLevel;
//         LogHandler::setLogLevel(logLevel);
//         // Serial.println("logLevel: ");
//         // Serial.println((int)logLevel);
//         // Serial.println( doc["logLevel"] .as<int>());
//         // Serial.println((int)doc["logLevel"]);

//         // std::vector<const char*> tags;
//         //  tags.push_back(TagHandler::DisplayHandler);
//         //  tags.push_back(TagHandler::BLDCHandler);
//         //  tags.push_back(TagHandler::ServoHandler3);
//         // LogHandler::setTags(tags);
        
//         for (size_t i = 0; i < currentChannels.size(); i++)
//         {
//             doc["channelRanges"][currentChannels[i].Name]["min"] = currentChannels[i].min;
//             doc["channelRanges"][currentChannels[i].Name]["max"] = currentChannels[i].max;
//             // LogHandler::debug(_TAG, "save %s min: %i", currentChannels[i].Name, doc["channelRanges"][currentChannels[i].Name]["min"].as<int>());
//             // LogHandler::debug(_TAG, "save %s max: %i", currentChannels[i].Name, doc["channelRanges"][currentChannels[i].Name]["max"].as<int>());
//         }

//         if (!tempInternalEnabled)
//         {
//             internalTempDisplayed = false;
//             fanControlEnabled = false;
//         }
//         if (!tempSleeveEnabled)
//         {
//             sleeveTempDisplayed = false;
//         }
//         doc["fullBuild"] = fullBuild;
//         doc["TCodeVersion"] = (int)TCodeVersionEnum;
        
//         doc["udpServerPort"] = udpServerPort;
//         doc["webServerPort"] = webServerPort;
//         doc["hostname"] = hostname;
//         doc["friendlyName"] = friendlyName;
//         doc["bluetoothEnabled"] = bluetoothEnabled;
//         doc["pitchFrequencyIsDifferent"] = pitchFrequencyIsDifferent;
//         doc["msPerRad"] = msPerRad;
//         doc["servoFrequency"] = servoFrequency;
//         doc["pitchFrequency"] = pitchFrequency;
//         doc["valveFrequency"] = valveFrequency;
//         doc["twistFrequency"] = twistFrequency;
//         doc["squeezeFrequency"] = squeezeFrequency;
//         doc["continuousTwist"] = continuousTwist;
//         doc["feedbackTwist"] = feedbackTwist;
//         doc["analogTwist"] = analogTwist;
//         doc["TwistFeedBack_PIN"] = TwistFeedBack_PIN;
//         doc["RightServo_PIN"] = RightServo_PIN;
//         doc["LeftServo_PIN"] = LeftServo_PIN;
//         doc["RightUpperServo_PIN"] = RightUpperServo_PIN;
//         doc["LeftUpperServo_PIN"] = LeftUpperServo_PIN;
//         doc["PitchLeftServo_PIN"] = PitchLeftServo_PIN;
//         doc["PitchRightServo_PIN"] = PitchRightServo_PIN;
//         doc["ValveServo_PIN"] = ValveServo_PIN;
//         doc["TwistServo_PIN"] = TwistServo_PIN;
//         doc["Squeeze_PIN"] = Squeeze_PIN;
//         doc["Vibe0_PIN"] = Vibe0_PIN;
//         doc["Vibe1_PIN"] = Vibe1_PIN;
//         doc["Vibe2_PIN"] = Vibe2_PIN;
//         doc["Vibe3_PIN"] = Vibe3_PIN;
//         doc["Case_Fan_PIN"] = Case_Fan_PIN;
//         doc["LubeButton_PIN"] = LubeButton_PIN;
//         doc["Internal_Temp_PIN"] = Internal_Temp_PIN;

//         doc["BLDC_UsePWM"] = BLDC_UsePWM;
//         doc["BLDC_UseMT6701"] = BLDC_UseMT6701;
//         doc["BLDC_UseHallSensor"] = BLDC_UseHallSensor;
//         doc["BLDC_Pulley_Circumference"] = BLDC_Pulley_Circumference;
//         doc["BLDC_Encoder_PIN"] = BLDC_Encoder_PIN;
//         doc["BLDC_ChipSelect_PIN"] = BLDC_ChipSelect_PIN;
//         doc["BLDC_Enable_PIN"] = BLDC_Enable_PIN;
//         doc["BLDC_HallEffect_PIN"] = BLDC_HallEffect_PIN;
//         doc["BLDC_PWMchannel1_PIN"] = BLDC_PWMchannel1_PIN;
//         doc["BLDC_PWMchannel2_PIN"] = BLDC_PWMchannel2_PIN;
//         doc["BLDC_PWMchannel3_PIN"] = BLDC_PWMchannel3_PIN;
//         doc["BLDC_MotorA_Voltage"] = round2(BLDC_MotorA_Voltage);
//         doc["BLDC_MotorA_Current"] = round2(BLDC_MotorA_Current);
//         doc["BLDC_MotorA_ParametersKnown"] = BLDC_MotorA_ParametersKnown;
//         doc["BLDC_MotorA_ZeroElecAngle"] = round2(BLDC_MotorA_ZeroElecAngle);
//         doc["BLDC_RailLength"] = BLDC_RailLength;
//         doc["BLDC_StrokeLength"] = BLDC_StrokeLength;
        
//         LogHandler::debug(_TAG, "save %s max: %f", "BLDC_MotorA_Voltage", doc["BLDC_MotorA_Current"].as<float>());

//         doc["staticIP"] = staticIP;
//         doc["localIP"] = localIP;
//         doc["gateway"] = gateway;
//         doc["subnet"] = subnet;
//         doc["dns1"] = dns1;
//         doc["dns2"] = dns2;

//         doc["sr6Mode"] = sr6Mode;
//         doc["RightServo_ZERO"] = RightServo_ZERO;
//         doc["LeftServo_ZERO"] = LeftServo_ZERO;
//         doc["RightUpperServo_ZERO"] = RightUpperServo_ZERO;
//         doc["LeftUpperServo_ZERO"] = LeftUpperServo_ZERO;
//         doc["PitchLeftServo_ZERO"] = PitchLeftServo_ZERO;
//         doc["PitchRightServo_ZERO"] = PitchRightServo_ZERO;
//         doc["TwistServo_ZERO"] = TwistServo_ZERO;
//         doc["ValveServo_ZERO"] = ValveServo_ZERO;
//         doc["Squeeze_ZERO"] = SqueezeServo_ZERO;
//         doc["autoValve"] = autoValve;
//         doc["inverseValve"] = inverseValve;
//         doc["valveServo90Degrees"] = valveServo90Degrees;
//         doc["inverseStroke"] = inverseStroke;
//         doc["inversePitch"] = inversePitch;
//         doc["lubeAmount"] = lubeAmount;
//         doc["lubeEnabled"] = lubeEnabled;
//         doc["displayEnabled"] = displayEnabled;
//         doc["sleeveTempDisplayed"] = sleeveTempDisplayed;
//         doc["versionDisplayed"] = versionDisplayed;
//         doc["internalTempDisplayed"] = internalTempDisplayed;
//         doc["tempSleeveEnabled"] = tempSleeveEnabled;
//         doc["Display_Screen_Width"] = Display_Screen_Width;
//         doc["Display_Screen_Height"] = Display_Screen_Height;
//         doc["TargetTemp"] = TargetTemp;
//         doc["HeatPWM"] = HeatPWM;
//         doc["HoldPWM"] = HoldPWM;
//         std::stringstream Display_I2C_Address_String;
//         Display_I2C_Address_String << "0x" << std::hex << Display_I2C_Address;
//         doc["Display_I2C_Address"] = Display_I2C_Address_String.str();
//         doc["Display_Rst_PIN"] = Display_Rst_PIN;
//         doc["Temp_PIN"] = Sleeve_Temp_PIN;
//         doc["Heater_PIN"] = Heater_PIN;
//         // doc["heaterFailsafeTime"] = String(heaterFailsafeTime);
//         doc["heaterThreshold"] = heaterThreshold;
//         doc["heaterResolution"] = heaterResolution;
//         doc["heaterFrequency"] = heaterFrequency;
//         doc["fanControlEnabled"] = fanControlEnabled;
//         doc["caseFanFrequency"] = caseFanFrequency;
//         doc["caseFanResolution"] = caseFanResolution;
//         doc["internalTempForFan"] = internalTempForFan;
//         doc["internalMaxTemp"] = internalMaxTemp;
//         doc["tempInternalEnabled"] = tempInternalEnabled;

//         doc["batteryLevelEnabled"] = batteryLevelEnabled;
//         //doc["Battery_Voltage_PIN"] = Battery_Voltage_PIN;
//         doc["batteryLevelNumeric"] = batteryLevelNumeric;
//         doc["batteryVoltageMax"] = round2(batteryVoltageMax);
//         doc["batteryCapacityMax"] = batteryCapacityMax;

//         doc["voiceEnabled"] = voiceEnabled;
//         doc["voiceMuted"] = voiceMuted;
//         doc["voiceWakeTime"] = voiceWakeTime;
//         doc["voiceVolume"] = voiceVolume;


//         JsonArray includes = doc.createNestedArray("log-include-tags");
//         std::vector<String> includesVec = LogHandler::getIncludes();
//         for (int i = 0; i < includesVec.size(); i++)
//         {
//             includes.add(includesVec[i]);
//         }

//         JsonArray excludes = doc.createNestedArray("log-exclude-tags");
//         std::vector<String> excludesVec = LogHandler::getExcludes();
//         for (int i = 0; i < excludesVec.size(); i++)
//         {
//             excludes.add(excludesVec[i]);
//         }

//         LogHandler::debug(_TAG, "isNull: %u", doc.isNull());
//         if (doc.isNull())
//         {
//             LogHandler::error(_TAG, "document is null!");
//             // file.close();
//             return false;
//         }

//         LogHandler::debug(_TAG, "Memory usage: %u bytes", doc.memoryUsage());
//         if (doc.memoryUsage() == 0)
//         {
//             LogHandler::error(_TAG, "document is empty!");
//             // file.close();
//             return false;
//         }

//         LogHandler::debug(_TAG, "Is overflowed: %u", doc.overflowed());
//         if (doc.overflowed())
//         {
//             LogHandler::error(_TAG, "document is overflowed! Increase serialize size: %u", doc.memoryUsage());
//             // file.close();
//             return false;
//         }

//         return true;
//     }

    /// @brief Locks the mutex checks for an existing file and creates  it if it doesnt exist. Calls the callback function and gives the mutex.
    /// @param filepath 
    /// @param mutableLoadDefault 
    /// @param mutex 
    /// @param jsonSize 
    /// @param loadFunction 
    /// @param json 
    /// @return 
    static bool loadSettingsJson(const char* filepath, bool loadDefault, SemaphoreHandle_t& mutex, const int jsonSize, std::function<bool(const JsonObject, bool& mutableLoadDefault)> loadFunction, std::function<bool(JsonObject)> saveFunction, JsonObject json = JsonObject()) {
        JsonDocument doc; //jsonSize
        bool mutableLoadDefault = loadDefault;
        if(mutableLoadDefault || json.isNull()) {
		    xSemaphoreTake(mutex, portMAX_DELAY);
            if(!checkForFileAndLoad(filepath, json, doc, mutableLoadDefault)) {
                xSemaphoreGive(mutex);
                return false;
            }
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
            printMemory();
        }
        saving = false;
        xSemaphoreGive(mutex);
        return true;
    }

    static bool checkForFileAndLoad(const char* path, JsonObject &json, JsonDocument &doc, bool &loadDefault) {
        if(!LittleFS.exists(path)) {
            loadDefault = true;
        }
        if(loadDefault) {
            defaultJsonFile(path);
        }
        return loadJsonFromFile(path, json, doc);
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

    static bool loadJsonFromFile(const char* path, JsonObject &json, JsonDocument &doc) {
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
        json = doc.as<JsonObject>();
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
//     /** If the parameter json is ommited or the pin value doesnt exist on the object then the pins are set to default. */
//     static void setBoardPinout(JsonObject json = JsonObject()) {
    
// #if MOTOR_TYPE == 0
//     if(isBoardType(BoardType::ISAAC)) {
//         // RightServo_PIN = 2;
//         // LeftServo_PIN = 13;
//         // PitchLeftServo_PIN = 14;
//         // ValveServo_PIN = 5;
//         // TwistServo_PIN = 27;
//         // TwistFeedBack_PIN = 33;
//         // Vibe0_PIN = 15;
//         // Vibe1_PIN = 16;
//         // LubeButton_PIN = 36;
//         // RightUpperServo_PIN = 4;
//         // LeftUpperServo_PIN = 12;
//         // PitchRightServo_PIN = 17;
//         // Sleeve_Temp_PIN = 25;
//         // Heater_PIN = 19;
//         // Squeeze_PIN = 26;
//         TwistFeedBack_PIN = json["TwistFeedBack_PIN"] | 32;
//         RightServo_PIN = json["RightServo_PIN"] | 4;
//         LeftServo_PIN = json["LeftServo_PIN"] | 13;
//         RightUpperServo_PIN = json["RightUpperServo_PIN"] | 16;
//         LeftUpperServo_PIN = json["LeftUpperServo_PIN"] | 27;
//         PitchLeftServo_PIN = json["PitchLeftServo_PIN"] | 26;
//         PitchRightServo_PIN = json["PitchRightServo_PIN"] | 17;
//         ValveServo_PIN = json["ValveServo_PIN"] | 18;
//         TwistServo_PIN = json["TwistServo_PIN"] | 25;
//         // Common motor
//         Squeeze_PIN = json["Squeeze_PIN"] | 19;
//         LubeButton_PIN = json["LubeButton_PIN"] | 34;
//         // Internal_Temp_PIN = json["Internal_Temp_PIN"] | 34;
//         Sleeve_Temp_PIN = json["Temp_PIN"] | 33;
//         // Case_Fan_PIN = json["Case_Fan_PIN"] | 16;
//         Vibe0_PIN = json["Vibe0_PIN"] | 15;
//         Vibe1_PIN = json["Vibe1_PIN"] | 2;
//         // Vibe2_PIN = json["Vibe2_PIN"] | 23;
//         // Vibe3_PIN = json["Vibe3_PIN"] | 32;
//         Heater_PIN = json["Heater_PIN"] | 5;
//     } else if(isBoardType(BoardType::CRIMZZON)) {

//         Vibe3_PIN = json["Vibe3_PIN"] | 26;
//         Internal_Temp_PIN = json["Internal_Temp_PIN"] | 32;

//         // EXT
//         //  EXT_Input2_PIN = 34;
//         //  EXT_Input3_PIN = 39;
//         //  EXT_Input4_PIN = 36;

//         heaterResolution = json["heaterResolution"] | 8;
//         caseFanResolution = json["caseFanResolution"] | 10;
//         caseFanFrequency = json["caseFanFrequency"] | 25;
//         Display_Screen_Height = json["Display_Screen_Height"] | 32;
//         TwistFeedBack_PIN = json["TwistFeedBack_PIN"] | 0;
//     }
//     if(!isBoardType(BoardType::ISAAC)) { // Devkit v1 pins
//         if(!isBoardType(BoardType::CRIMZZON)) {
//             TwistFeedBack_PIN = json["TwistFeedBack_PIN"] | 26;
//         }
//         // Common motor
//         Squeeze_PIN = json["Squeeze_PIN"] | 17;
//         LubeButton_PIN = json["LubeButton_PIN"] | 35;
//         if(!isBoardType(BoardType::CRIMZZON)) {
//             Internal_Temp_PIN = json["Internal_Temp_PIN"] | 34;
//         }
//         Case_Fan_PIN = json["Case_Fan_PIN"] | 16;

//         //Stock servo motors
//         RightServo_PIN = json["RightServo_PIN"] | 13;
//         LeftServo_PIN = json["LeftServo_PIN"] | 15;
//         RightUpperServo_PIN = json["RightUpperServo_PIN"] | 12;
//         LeftUpperServo_PIN = json["LeftUpperServo_PIN"] | 2;
//         PitchLeftServo_PIN = json["PitchLeftServo_PIN"] | 4;
//         PitchRightServo_PIN = json["PitchRightServo_PIN"] | 14;

//         Heater_PIN = json["Heater_PIN"] | 33;
//         ValveServo_PIN = json["ValveServo_PIN"] | 25;
//         TwistServo_PIN = json["TwistServo_PIN"] | 27;
//         Sleeve_Temp_PIN = json["Temp_PIN"] | 5;
//         Vibe0_PIN = json["Vibe0_PIN"] | 18;
//         Vibe1_PIN = json["Vibe1_PIN"] | 19;
//         Vibe2_PIN = json["Vibe2_PIN"] | 23;
//         if(!isBoardType(BoardType::CRIMZZON)) {
//             Vibe3_PIN = json["Vibe3_PIN"] | 32;
//         }
//     }
// #elif MOTOR_TYPE == 1 
//         // BLDC motor
//         BLDC_Encoder_PIN = json["BLDC_Encoder_PIN"] | 33;
//         BLDC_ChipSelect_PIN = json["BLDC_ChipSelect_PIN"] | 5;
//         BLDC_Enable_PIN = json["BLDC_Enable_PIN"] | 14;
//         BLDC_PWMchannel1_PIN = json["BLDC_PWMchannel1_PIN"] | 27;
//         BLDC_PWMchannel2_PIN = json["BLDC_PWMchannel2_PIN"] | 26;
//         BLDC_PWMchannel3_PIN = json["BLDC_PWMchannel3_PIN"] | 25;

//         // PWM
//         Heater_PIN = json["Heater_PIN"] | 15;
//         Sleeve_Temp_PIN = json["Temp_PIN"] | 36;
//         Vibe0_PIN = json["Vibe0_PIN"] | 2;
//         Vibe1_PIN = json["Vibe1_PIN"] | 4;
//         Vibe2_PIN = json["Vibe2_PIN"] | -1;
//         Vibe3_PIN = json["Vibe3_PIN"] | -1;
//         Case_Fan_PIN = json["Case_Fan_PIN"] | 16;

//         // PWM servo
//         ValveServo_PIN = json["ValveServo_PIN"] | 12;
//         TwistServo_PIN = json["TwistServo_PIN"] | 13;
//         Squeeze_PIN = json["Squeeze_PIN"] | 17;

//         // Input
//         TwistFeedBack_PIN = json["TwistFeedBack_PIN"] | 26;
//         LubeButton_PIN = json["LubeButton_PIN"] | 35;
//         Internal_Temp_PIN = json["Internal_Temp_PIN"] | 34;
//         BLDC_HallEffect_PIN = json["BLDC_HallEffect_PIN"] | 35;
// #endif
// }

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
#if DEBUG_BUILD
        LogHandler::debug("setBuildFeatures", "DEBUG_BUILD");
        buildFeatures[index] = BuildFeature::DEBUG;
        index++;
#endif
#if ESP32_DA
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
        buildFeatures[featureCount - 1] = {};
    }

    static void setMotorType()
    {
#if MOTOR_TYPE == 0
       m_settingsFactory->setValue(MOTOR_TYPE_SETTING, (int)MotorType::Servo);
#elif MOTOR_TYPE == 1
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

    // static bool LogDeserializationError(DeserializationError error, const char* filename) {
    //     if (error)
    //     {
    //         LogHandler::error(_TAG, "Error deserializing json: %s", filename);
    //         switch (error.code())
    //         {
    //         case DeserializationError::Code::Ok:
    //             LogHandler::error(_TAG, "Code: Ok");
    //             break;
    //         case DeserializationError::Code::EmptyInput:
    //             LogHandler::error(_TAG, "Code: EmptyInput");
    //             break;
    //         case DeserializationError::Code::IncompleteInput:
    //             LogHandler::error(_TAG, "Code: IncompleteInput");
    //             break;
    //         case DeserializationError::Code::InvalidInput:
    //             LogHandler::error(_TAG, "Code: InvalidInput");
    //             break;
    //         case DeserializationError::Code::NoMemory:
    //             LogHandler::error(_TAG, "Code: NoMemory");
    //             break;
    //         case DeserializationError::Code::TooDeep:
    //             LogHandler::error(_TAG, "Code: TooDeep");
    //             break;
    //         }
    //         return true;
    //     }
    //     return false;
    // }

    // static void LogSaveDebug(const DynamicJsonDocument doc)
    // {
        //Commented out due to error in logger on hostname.
        // LogHandler::debug(_TAG, "save TCodeVersionEnum: %i", doc["TCodeVersion"].as<int>());
        // LogHandler::debug(_TAG, "save ssid: %s", doc["ssid"].as<String>());
        // // LogHandler::debug(_TAG, "save pass: %s", doc["wifiPass"].as<String>());
        // LogHandler::debug(_TAG, "save udpServerPort: %i", doc["udpServerPort"].as<int>());
        // LogHandler::debug(_TAG, "save webServerPort: %i", doc["webServerPort"].as<int>());
        // LogHandler::debug(_TAG, "save hostname: %s", doc["hostname"].as<const char*>());
        // LogHandler::debug(_TAG, "save friendlyName: %s", doc["friendlyName"].as<const char*>());
        // LogHandler::debug(_TAG, "save pitchFrequencyIsDifferent ", doc["pitchFrequencyIsDifferent"].as<bool>());
        // LogHandler::debug(_TAG, "save msPerRad: %i", doc["msPerRad"].as<int>());
        // LogHandler::debug(_TAG, "save servoFrequency: %i", doc["servoFrequency"].as<int>());
        // LogHandler::debug(_TAG, "save  pitchFrequency: %i", doc["pitchFrequency"].as<int>());
        // LogHandler::debug(_TAG, "save valveFrequency: %i", doc["valveFrequency"].as<int>());
        // LogHandler::debug(_TAG, "save twistFrequency: %i", doc["twistFrequency"].as<int>());
        // LogHandler::debug(_TAG, "save continuousTwist: %i", doc["continuousTwist"].as<bool>());
        // LogHandler::debug(_TAG, "save feedbackTwist: %i", doc["feedbackTwist"].as<bool>());
        // LogHandler::debug(_TAG, "save analogTwist: %i", doc["analogTwist"].as<bool>());
        // LogHandler::debug(_TAG, "save TwistFeedBack_PIN: %i", doc["TwistFeedBack_PIN"].as<int>());
        // LogHandler::debug(_TAG, "save RightServo_PIN: %i", doc["RightServo_PIN"].as<int>());
        // LogHandler::debug(_TAG, "save LeftServo_PIN: %i", doc["LeftServo_PIN"].as<int>());
        // LogHandler::debug(_TAG, "save RightUpperServo_PIN: %i", doc["RightUpperServo_PIN"].as<int>());
        // LogHandler::debug(_TAG, "save LeftUpperServo_PIN: %i", doc["LeftUpperServo_PIN"].as<int>());
        // LogHandler::debug(_TAG, "save PitchLeftServo_PIN: %i", doc["PitchLeftServo_PIN"].as<int>());
        // LogHandler::debug(_TAG, "save PitchRightServo_PIN: %i", doc["PitchRightServo_PIN"].as<int>());
        // LogHandler::debug(_TAG, "save ValveServo_PIN: %i", doc["ValveServo_PIN"].as<int>());
        // LogHandler::debug(_TAG, "save TwistServo_PIN: %i", doc["TwistServo_PIN"].as<int>());
        // LogHandler::debug(_TAG, "save Vibe0_PIN: %i", doc["Vibe0_PIN"].as<int>());
        // LogHandler::debug(_TAG, "save Vibe1_PIN: %i", doc["Vibe1_PIN"].as<int>());
        // LogHandler::debug(_TAG, "save Lube_Pin: %i", doc["Lube_Pin"].as<int>());
        // LogHandler::debug(_TAG, "save LubeButton_PIN: %i", doc["LubeButton_PIN"].as<int>());
        // LogHandler::debug(_TAG, "save staticIP: %i", doc["staticIP"].as<bool>());
        // LogHandler::debug(_TAG, "save localIP: %s", doc["localIP"].as<const char*>());
        // LogHandler::debug(_TAG, "save gateway: %s", doc["gateway"].as<const char*>());
        // LogHandler::debug(_TAG, "save subnet: %s", doc["subnet"].as<const char*>());
        // LogHandler::debug(_TAG, "save dns1: %s", doc["dns1"].as<const char*>());
        // LogHandler::debug(_TAG, "save dns2: %s", doc["dns2"].as<const char*>());
        // LogHandler::debug(_TAG, "save sr6Mode: %i", doc["sr6Mode"].as<bool>());
        // LogHandler::debug(_TAG, "save RightServo_ZERO: %i", doc["RightServo_ZERO"].as<int>());
        // LogHandler::debug(_TAG, "save LeftServo_ZERO: %i", doc["LeftServo_ZERO"].as<int>());
        // LogHandler::debug(_TAG, "save RightUpperServo_ZERO: %i", doc["RightUpperServo_ZERO"].as<int>());
        // LogHandler::debug(_TAG, "save LeftUpperServo_ZERO: %i", doc["LeftUpperServo_ZERO"].as<int>());
        // LogHandler::debug(_TAG, "save PitchLeftServo_ZERO: %i", doc["PitchLeftServo_ZERO"].as<int>());
        // LogHandler::debug(_TAG, "save PitchRightServo_ZERO: %i", doc["PitchRightServo_ZERO"].as<int>());
        // LogHandler::debug(_TAG, "save TwistServo_ZERO: %i", doc["TwistServo_ZERO"].as<int>());
        // LogHandler::debug(_TAG, "save ValveServo_ZERO: %i", doc["ValveServo_ZERO"].as<int>());
        // LogHandler::debug(_TAG, "save autoValve: %i", doc["autoValve"].as<bool>());
        // LogHandler::debug(_TAG, "save inverseValve: %i", doc["inverseValve"].as<bool>());
        // LogHandler::debug(_TAG, "save valveServo90Degrees: %i", doc["valveServo90Degrees"].as<bool>());
        // LogHandler::debug(_TAG, "save inverseStroke: %i", doc["inverseStroke"].as<bool>());
        // LogHandler::debug(_TAG, "save inversePitch: %i", doc["inversePitch"].as<bool>());
        // LogHandler::debug(_TAG, "save lubeEnabled: %i", doc["lubeEnabled"].as<bool>());
        // LogHandler::debug(_TAG, "save lubeAmount: %i", doc["lubeAmount"].as<int>());
        // LogHandler::debug(_TAG, "save Temp_PIN: %i", doc["Temp_PIN"].as<int>());
        // LogHandler::debug(_TAG, "save Heater_PIN: %i", doc["Heater_PIN"].as<int>());
        // LogHandler::debug(_TAG, "save displayEnabled: %i", doc["displayEnabled"].as<bool>());
        // LogHandler::debug(_TAG, "save sleeveTempDisplayed: %i", doc["sleeveTempDisplayed"].as<bool>());
        // LogHandler::debug(_TAG, "save internalTempDisplayed: %i", doc["internalTempDisplayed"].as<bool>());
        // LogHandler::debug(_TAG, "save tempSleeveEnabled: %i", doc["tempSleeveEnabled"].as<bool>());
        // LogHandler::debug(_TAG, "save tempInternalEnabled: %i", doc["tempInternalEnabled"].as<bool>());
        // LogHandler::debug(_TAG, "save Display_Screen_Width: %i", doc["Display_Screen_Width"].as<int>());
        // LogHandler::debug(_TAG, "save internalMaxTemp: %f", doc["internalMaxTemp"].as<double>());
        // LogHandler::debug(_TAG, "save internalTempForFan: %f", doc["internalTempForFan"].as<double>());
        // LogHandler::debug(_TAG, "save Display_Screen_Height: %i", doc["Display_Screen_Height"].as<int>());
        // LogHandler::debug(_TAG, "save TargetTemp: %f", doc["TargetTemp"].as<double>());
        // LogHandler::debug(_TAG, "save HeatPWM: %i", doc["HeatPWM"].as<int>());
        // LogHandler::debug(_TAG, "save HoldPWM: %i", doc["HoldPWM"].as<int>());
        // LogHandler::debug(_TAG, "save Display_I2C_Address: %i", doc["Display_I2C_Address"].as<int>());
        // LogHandler::debug(_TAG, "save Display_Rst_PIN: %i", doc["Display_Rst_PIN"].as<int>());
        // LogHandler::debug(_TAG, "save heaterFailsafeTime: %ld", doc["heaterFailsafeTime"].as<long>());
        // LogHandler::debug(_TAG, "save heaterThreshold: %i", doc["heaterThreshold"].as<int>());
        // LogHandler::debug(_TAG, "save heaterResolution: %i", doc["heaterResolution"].as<int>());
        // LogHandler::debug(_TAG, "save heaterFrequency: %i", doc["heaterFrequency"].as<int>());
        // LogHandler::debug(_TAG, "save newtoungeHatExists: %i", doc["newtoungeHatExists"].as<bool>());
        // LogHandler::debug(_TAG, "save logLevel: %i", doc["logLevel"].as<int>());
        // LogHandler::debug(_TAG, "save bluetoothEnabled: %i", doc["bluetoothEnabled"].as<bool>());
    // }

    // static void LogUpdateDebug()
    // {
    //     LogHandler::debug(_TAG, "update TCodeVersionEnum: %i", TCodeVersionEnum);
    //     LogHandler::debug(_TAG, "update ssid: %s", ssid);
    //     //LogHandler::debug(_TAG, "update wifiPass: %s", wifiPass);
    //     LogHandler::debug(_TAG, "update udpServerPort: %i", udpServerPort);
    //     LogHandler::debug(_TAG, "update webServerPort: %i", webServerPort);
    //     LogHandler::debug(_TAG, "update hostname: %s", hostname);
    //     LogHandler::debug(_TAG, "update friendlyName: %s", friendlyName);
    //     LogHandler::debug(_TAG, "update pitchFrequencyIsDifferent: %i", pitchFrequencyIsDifferent);
    //     LogHandler::debug(_TAG, "update msPerRad: %i", msPerRad);
    //     LogHandler::debug(_TAG, "update servoFrequency: %i", servoFrequency);
    //     LogHandler::debug(_TAG, "update pitchFrequency: %i", pitchFrequency);
    //     LogHandler::debug(_TAG, "update valveFrequency: %i", valveFrequency);
    //     LogHandler::debug(_TAG, "update twistFrequency: %i", twistFrequency);
    //     LogHandler::debug(_TAG, "update continuousTwist: %i", continuousTwist);
    //     LogHandler::debug(_TAG, "update feedbackTwist: %i", feedbackTwist);
    //     LogHandler::debug(_TAG, "update analogTwist: %i", analogTwist);
    //     LogHandler::debug(_TAG, "update TwistFeedBack_PIN: %i", TwistFeedBack_PIN);
    //     LogHandler::debug(_TAG, "update RightServo_PIN: %i", RightServo_PIN);
    //     LogHandler::debug(_TAG, "update LeftServo_PIN: %i", LeftServo_PIN);
    //     LogHandler::debug(_TAG, "update RightUpperServo_PIN: %i", RightUpperServo_PIN);
    //     LogHandler::debug(_TAG, "update LeftUpperServo_PIN: %i", LeftUpperServo_PIN);
    //     LogHandler::debug(_TAG, "update PitchLeftServo_PIN: %i", PitchLeftServo_PIN);
    //     LogHandler::debug(_TAG, "update PitchRightServo_PIN: %i", PitchRightServo_PIN);
    //     LogHandler::debug(_TAG, "update ValveServo_PIN: %i", ValveServo_PIN);
    //     LogHandler::debug(_TAG, "update TwistServo_PIN: %i", TwistServo_PIN);
    //     LogHandler::debug(_TAG, "update Vibe0_PIN: %i", Vibe0_PIN);
    //     LogHandler::debug(_TAG, "update Vibe1_PIN: %i", Vibe1_PIN);
    //     LogHandler::debug(_TAG, "update LubeButton_PIN: %i", LubeButton_PIN);
    //     LogHandler::debug(_TAG, "update staticIP: %i", staticIP);
    //     LogHandler::debug(_TAG, "update localIP: %s", localIP);
    //     LogHandler::debug(_TAG, "update gateway: %s", gateway);
    //     LogHandler::debug(_TAG, "update subnet: %s", subnet);
    //     LogHandler::debug(_TAG, "update dns1: %s", dns1);
    //     LogHandler::debug(_TAG, "update dns2: %s", dns2);
    //     LogHandler::debug(_TAG, "update sr6Mode: %i", sr6Mode);
    //     LogHandler::debug(_TAG, "update RightServo_ZERO: %i", RightServo_ZERO);
    //     LogHandler::debug(_TAG, "update LeftServo_ZERO: %i", LeftServo_ZERO);
    //     LogHandler::debug(_TAG, "update RightUpperServo_ZERO: %i", RightUpperServo_ZERO);
    //     LogHandler::debug(_TAG, "update LeftUpperServo_ZERO: %i", LeftUpperServo_ZERO);
    //     LogHandler::debug(_TAG, "update PitchLeftServo_ZERO: %i", PitchLeftServo_ZERO);
    //     LogHandler::debug(_TAG, "update PitchRightServo_ZERO: %i", PitchRightServo_ZERO);
    //     LogHandler::debug(_TAG, "update TwistServo_ZERO: %i", TwistServo_ZERO);
    //     LogHandler::debug(_TAG, "update ValveServo_ZERO: %i", ValveServo_ZERO);
    //     LogHandler::debug(_TAG, "update autoValve: %i", autoValve);
    //     LogHandler::debug(_TAG, "update inverseValve: %i", inverseValve);
    //     LogHandler::debug(_TAG, "update valveServo90Degrees: %i", valveServo90Degrees);
    //     LogHandler::debug(_TAG, "update inverseStroke: %i", inverseStroke);
    //     LogHandler::debug(_TAG, "update inversePitch: %i", inversePitch);
    //     LogHandler::debug(_TAG, "update lubeEnabled: %i", lubeEnabled);
    //     LogHandler::debug(_TAG, "update lubeAmount: %i", lubeAmount);
    //     LogHandler::debug(_TAG, "update displayEnabled: %i", displayEnabled);
    //     LogHandler::debug(_TAG, "update sleeveTempDisplayed: %i", sleeveTempDisplayed);
    //     LogHandler::debug(_TAG, "update internalTempDisplayed: %i", internalTempDisplayed);
    //     LogHandler::debug(_TAG, "update tempSleeveEnabled: %i", tempSleeveEnabled);
    //     LogHandler::debug(_TAG, "update tempInternalEnabled: %i", tempInternalEnabled);
    //     LogHandler::debug(_TAG, "update Display_Screen_Width: %i", Display_Screen_Width);
    //     LogHandler::debug(_TAG, "update Display_Screen_Height: %i", Display_Screen_Height);
    //     LogHandler::debug(_TAG, "update TargetTemp: %i", TargetTemp);
    //     LogHandler::debug(_TAG, "update HeatPWM: %i", HeatPWM);
    //     LogHandler::debug(_TAG, "update HoldPWM: %i", HoldPWM);
    //     LogHandler::debug(_TAG, "update Display_I2C_Address: %i", Display_I2C_Address);
    //     LogHandler::debug(_TAG, "update Display_Rst_PIN: %i", Display_Rst_PIN);
    //     LogHandler::debug(_TAG, "update Sleeve_Temp_PIN: %i", Sleeve_Temp_PIN);
    //     LogHandler::debug(_TAG, "update Heater_PIN: %i", Heater_PIN);
    //     LogHandler::debug(_TAG, "update heaterThreshold: %d", heaterThreshold);
    //     LogHandler::debug(_TAG, "update heaterResolution: %i", heaterResolution);
    //     LogHandler::debug(_TAG, "update heaterFrequency: %i", heaterFrequency);
    //     LogHandler::debug(_TAG, "update logLevel: %i", (int)logLevel);
    //     LogHandler::debug(_TAG, "update bluetoothEnabled: %i", (int)bluetoothEnabled);
    // }
};

SettingsFactory* SettingsHandler::m_settingsFactory = SettingsFactory::getInstance();
SemaphoreHandle_t SettingsHandler::m_motionMutex = xSemaphoreCreateMutex();
SemaphoreHandle_t SettingsHandler::m_wifiMutex = xSemaphoreCreateMutex();
SemaphoreHandle_t SettingsHandler::m_buttonsMutex = xSemaphoreCreateMutex();
SemaphoreHandle_t SettingsHandler::m_settingsMutex = xSemaphoreCreateMutex();
bool SettingsHandler::initialized = false;
bool SettingsHandler::restartRequired = false;
bool SettingsHandler::saving = false;
bool SettingsHandler::motionPaused = false;
bool SettingsHandler::fullBuild = false;
bool SettingsHandler::apMode = false;

// BoardType SettingsHandler::boardType = BoardType::DEVKIT;
BuildFeature SettingsHandler::buildFeatures[featureCount];
const char *SettingsHandler::_TAG = TagHandler::SettingsHandler;
std::vector<int> SettingsHandler::systemI2CAddresses;
SETTING_STATE_FUNCTION_PTR_T SettingsHandler::message_callback = 0;
ChannelMap SettingsHandler::channelMap;
std::vector<Channel> SettingsHandler::currentChannels;

char SettingsHandler::currentIP[IP_ADDRESS_LEN] = LOCALIP_DEFAULT;
char SettingsHandler::currentGateway[IP_ADDRESS_LEN] = GATEWAY_DEFAULT;
char SettingsHandler::currentSubnet[IP_ADDRESS_LEN] = SUBNET_DEFAULT;
char SettingsHandler::currentDns1[IP_ADDRESS_LEN] = DNS1_DEFAULT;
char SettingsHandler::currentDns2[IP_ADDRESS_LEN] = DNS2_DEFAULT;


// MotorType SettingsHandler::motorType = MotorType::Servo;
// const char SettingsHandler::HandShakeChannel[4] = "D1\n";
// const char SettingsHandler::SettingsChannel[4] = "D2\n";
// const char *SettingsHandler::userSettingsFilePath = "/userSettings.json";
// const char *SettingsHandler::wifiPassFilePath = "/wifiInfo.json";
// const char *SettingsHandler::buttonsFilePath = "/buttons.json";
// const char *SettingsHandler::motionProfilesFilePath = "/motionProfiles.json";
// const char *SettingsHandler::logPath = "/log.json";
// const char *SettingsHandler::defaultWifiPass = "YOUR PASSWORD HERE";
// const char *SettingsHandler::decoyPass = "Too bad haxor!";
// const char SettingsHandler::defaultIP[15] = "192.168.69.1";
// const char SettingsHandler::defaultGateWay[15] = "192.168.69.254";
// const char SettingsHandler::defaultSubnet[15] = "255.255.255.0";
// bool SettingsHandler::bluetoothEnabled = true;
// LogLevel SettingsHandler::logLevel = LogLevel::INFO;
// bool SettingsHandler::isTcp = true;
// char SettingsHandler::ssid[32];
// char SettingsHandler::wifiPass[63];
// char SettingsHandler::hostname[63];
// char SettingsHandler::friendlyName[100];
// int SettingsHandler::udpServerPort;
// int SettingsHandler::webServerPort;
// int SettingsHandler::PitchRightServo_PIN;
// int SettingsHandler::RightUpperServo_PIN;
// int SettingsHandler::RightServo_PIN;
// int SettingsHandler::PitchLeftServo_PIN;
// int SettingsHandler::LeftUpperServo_PIN;
// int SettingsHandler::LeftServo_PIN;
// int SettingsHandler::ValveServo_PIN;
// int SettingsHandler::TwistServo_PIN;
// int SettingsHandler::TwistFeedBack_PIN;
// int SettingsHandler::Vibe0_PIN;
// int SettingsHandler::Vibe1_PIN;
// int SettingsHandler::LubeButton_PIN;
// int SettingsHandler::Sleeve_Temp_PIN;
// int SettingsHandler::Heater_PIN;
// int SettingsHandler::I2C_SDA_PIN_obsolete = 21;
// int SettingsHandler::I2C_SCL_PIN_obsolete = 22;

// int SettingsHandler::Internal_Temp_PIN;
// int SettingsHandler::Case_Fan_PIN;
// int SettingsHandler::Squeeze_PIN;
// int SettingsHandler::Vibe2_PIN;
// int SettingsHandler::Vibe3_PIN;

// // int SettingsHandler::HeatLED_PIN = 32;
// //  pin 25 cannot be servo. Throws error
// bool SettingsHandler::lubeEnabled = true;

// bool SettingsHandler::pitchFrequencyIsDifferent;
// int SettingsHandler::msPerRad;
// int SettingsHandler::servoFrequency;
// int SettingsHandler::pitchFrequency;
// int SettingsHandler::valveFrequency;
// int SettingsHandler::twistFrequency;
// int SettingsHandler::squeezeFrequency;
// bool SettingsHandler::feedbackTwist = false;
// bool SettingsHandler::continuousTwist;
// bool SettingsHandler::analogTwist;
// bool SettingsHandler::staticIP;
// char SettingsHandler::localIP[15];
// char SettingsHandler::gateway[15];
// char SettingsHandler::subnet[15];
// char SettingsHandler::dns1[15];
// char SettingsHandler::dns2[15];
// bool SettingsHandler::sr6Mode;
// int SettingsHandler::RightServo_ZERO = 1500;
// int SettingsHandler::LeftServo_ZERO = 1500;
// int SettingsHandler::RightUpperServo_ZERO = 1500;
// int SettingsHandler::LeftUpperServo_ZERO = 1500;
// int SettingsHandler::PitchLeftServo_ZERO = 1500;
// int SettingsHandler::PitchRightServo_ZERO = 1500;

// bool SettingsHandler::BLDC_UsePWM = false;
// bool SettingsHandler::BLDC_UseMT6701 = true;
// bool SettingsHandler::BLDC_UseHallSensor = false;
// int SettingsHandler::BLDC_Pulley_Circumference = 60;
// int SettingsHandler::BLDC_Encoder_PIN = 33;// PWM feedback pin (if used) - P pad on AS5048a
// int SettingsHandler::BLDC_Enable_PIN = 14;// Motor enable - EN on SFOCMini   
// int SettingsHandler::BLDC_HallEffect_PIN = 12;
// int SettingsHandler::BLDC_PWMchannel1_PIN = 27;
// int SettingsHandler::BLDC_PWMchannel2_PIN = 26;
// int SettingsHandler::BLDC_PWMchannel3_PIN = 25;
// float SettingsHandler::BLDC_MotorA_Voltage = 20.0; // BLDC Motor operating voltage (12-20V)
// float SettingsHandler::BLDC_MotorA_Current = 1.0;  // BLDC Maximum operating current (Amps)
// int SettingsHandler::BLDC_ChipSelect_PIN = 5;         // SPI chip select pin - CSn on AS5048a (By default on ESP32: MISO = D19, MOSI = D23, CLK = D18)
// bool SettingsHandler::BLDC_MotorA_ParametersKnown = false;     // Once you know the zero elec angle for the motor enter it below and set this flag to true.
// float SettingsHandler::BLDC_MotorA_ZeroElecAngle = 0.00; // This number is the zero angle (in radians) for the motor relative to the encoder.
// int SettingsHandler::BLDC_RailLength;
// int SettingsHandler::BLDC_StrokeLength;

// int SettingsHandler::TwistServo_ZERO = 1500;
// int SettingsHandler::ValveServo_ZERO = 1500;
// int SettingsHandler::SqueezeServo_ZERO = 1500;
// bool SettingsHandler::autoValve = false;
// bool SettingsHandler::inverseValve = false;
// bool SettingsHandler::valveServo90Degrees = false;
// bool SettingsHandler::inverseStroke = false;
// bool SettingsHandler::inversePitch = false;
// int SettingsHandler::lubeAmount = 255;

// bool SettingsHandler::displayEnabled = false;
// bool SettingsHandler::sleeveTempDisplayed = false;
// bool SettingsHandler::internalTempDisplayed = false;
// bool SettingsHandler::versionDisplayed = true;
// bool SettingsHandler::tempSleeveEnabled = false;
// bool SettingsHandler::tempInternalEnabled = false;
// bool SettingsHandler::fanControlEnabled = false;
// bool SettingsHandler::batteryLevelEnabled = true;
// //int SettingsHandler::Battery_Voltage_PIN = 32;
// bool SettingsHandler::batteryLevelNumeric = false;
// double SettingsHandler::batteryVoltageMax = 12.6;
// int SettingsHandler::batteryCapacityMax;
// int SettingsHandler::Display_Screen_Width = 128;
// int SettingsHandler::Display_Screen_Height = 64;
// int SettingsHandler::caseFanMaxDuty = 255;
// double SettingsHandler::internalTempForFan = 20.0;
// double SettingsHandler::internalMaxTemp = 50.0;
// int SettingsHandler::TargetTemp = 40;
// int SettingsHandler::HeatPWM = 255;
// int SettingsHandler::HoldPWM = 110;
// int SettingsHandler::Display_I2C_Address = 0x3C;
// int SettingsHandler::Display_Rst_PIN = -1;
// // long SettingsHandler::heaterFailsafeTime = 60000;
// float SettingsHandler::heaterThreshold = 5.0;
// int SettingsHandler::heaterResolution = 8;
// int SettingsHandler::heaterFrequency = 5000;
// int SettingsHandler::caseFanFrequency = 25;
// int SettingsHandler::caseFanResolution = 10;
const char *SettingsHandler::lastRebootReason;

// bool SettingsHandler::voiceEnabled = false;
// bool SettingsHandler::voiceMuted = false;
// int SettingsHandler::voiceWakeTime = 10;
// int SettingsHandler::voiceVolume = 10;

// bool SettingsHandler::bootButtonEnabled;
// bool SettingsHandler::buttonSetsEnabled;
// char SettingsHandler::bootButtonCommand[MAX_COMMAND];

bool SettingsHandler::motionEnabled = false;
//char SettingsHandler::motionSelectedProfileName[maxMotionProfileNameLength];
int SettingsHandler::motionSelectedProfileIndex = 0;
int SettingsHandler::motionDefaultProfileIndex = 0;
MotionProfile SettingsHandler::motionProfiles[maxMotionProfileCount];
ButtonSet SettingsHandler::buttonSets[MAX_BUTTON_SETS];
// uint8_t SettingsHandler::defaultButtonSetPin;
// uint16_t SettingsHandler::buttonAnalogDebounce;
//std::map<const char*, MotionProfile*, StrCompare> SettingsHandler::motionProfiles;
// int SettingsHandler::motionUpdateGlobal;
// int SettingsHandler::motionPeriodGlobal;
// int SettingsHandler::motionAmplitudeGlobal;
// int SettingsHandler::motionOffsetGlobal;
// float SettingsHandler::motionPhaseGlobal;
// bool SettingsHandler::motionReversedGlobal = false;
// bool SettingsHandler::motionPeriodGlobalRandom = false;
// bool SettingsHandler::motionAmplitudeGlobalRandom = false;
// bool SettingsHandler::motionOffsetGlobalRandom = false;
// int SettingsHandler::motionPeriodGlobalRandomMin;
// int SettingsHandler::motionPeriodGlobalRandomMax;
// int SettingsHandler::motionAmplitudeGlobalRandomMin;
// int SettingsHandler::motionAmplitudeGlobalRandomMax;
// int SettingsHandler::motionOffsetGlobalRandomMin;
// int SettingsHandler::motionOffsetGlobalRandomMax;
// int SettingsHandler::motionRandomChangeMin;
// int SettingsHandler:: motionRandomChangeMax;