/* MIT License

Copyright (c) 2023 Jason C. Fain

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
#include <SPIFFS.h>
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
#include "../lib/enum.h"
#include "../lib/constants.h"

//#define FIRMWARE_VERSION 0.32f Not used currently
#define FIRMWARE_VERSION_NAME "0.34b"
#define featureCount 8

using SETTING_STATE_FUNCTION_PTR_T = void (*)(const char *group, const char *settingNameThatChanged);

class SettingsHandler
{
public:
    static bool initialized;
    static bool saving;
    static bool fullBuild;
    static LogLevel logLevel;

    static std::vector<Channel> currentChannels;
    static BoardType boardType;
    static BuildFeature buildFeatures[featureCount];
    static String TCodeVersionName;
    static TCodeVersion TCodeVersionEnum;
    static MotorType motorType;
    static std::vector<int> systemI2CAddresses;
    const static char HandShakeChannel[4];
    const static char SettingsChannel[4];
    static bool bluetoothEnabled;
    static bool isTcp;
    static char ssid[32];
    static char wifiPass[63];
    static int udpServerPort;
    static int webServerPort;
    static char hostname[63];
    static char friendlyName[100];
    static int TwistFeedBack_PIN; // Vibe 4?
    static int RightServo_PIN;
    static int LeftServo_PIN;
    static int RightUpperServo_PIN;
    static int LeftUpperServo_PIN;
    static int PitchLeftServo_PIN;
    static int PitchRightServo_PIN;
    static int ValveServo_PIN;
    static int TwistServo_PIN;

    static bool BLDC_UsePWM;
    static bool BLDC_UseMT6701;
    static bool BLDC_UseHallSensor;
    static int BLDC_Pulley_Circumference;
    static int BLDC_Encoder_PIN;
    static int BLDC_HallEffect_PIN;
    static int BLDC_ChipSelect_PIN; 
    static int BLDC_Enable_PIN;
    static int BLDC_PWMchannel1_PIN;
    static int BLDC_PWMchannel2_PIN;
    static int BLDC_PWMchannel3_PIN;
    static float BLDC_MotorA_Voltage;
    static float BLDC_MotorA_Current; 
    static bool BLDC_MotorA_ParametersKnown;
    static float BLDC_MotorA_ZeroElecAngle;
    static int BLDC_RailLength;
    static int BLDC_StrokeLength;

    static int Vibe0_PIN;
    static int Vibe1_PIN;
    static int Vibe2_PIN;
    static int Vibe3_PIN;
    static int LubeButton_PIN;
    static int Squeeze_PIN;
    static int msPerRad;
    static int servoFrequency;
    static int pitchFrequency;
    static int valveFrequency;
    static int twistFrequency;
    static int squeezeFrequency;
    static bool pitchFrequencyIsDifferent;
    static bool feedbackTwist;
    static bool continuousTwist;
    static bool analogTwist;
    static bool staticIP;
    static char localIP[15];
    static char gateway[15];
    static char subnet[15];
    static char dns1[15];
    static char dns2[15];
    static bool sr6Mode;
    static int RightServo_ZERO;
    static int LeftServo_ZERO;
    static int RightUpperServo_ZERO;
    static int LeftUpperServo_ZERO;
    static int PitchLeftServo_ZERO;
    static int PitchRightServo_ZERO;
    static int TwistServo_ZERO;
    static int ValveServo_ZERO;
    static int SqueezeServo_ZERO;
    static bool autoValve;
    static bool inverseValve;
    static bool inverseStroke;
    static bool inversePitch;
    static bool valveServo90Degrees;
    static bool lubeEnabled;
    static int lubeAmount;
    static bool displayEnabled;
    static bool sleeveTempDisplayed;
    static bool internalTempDisplayed;
    static bool versionDisplayed;
    static bool tempSleeveEnabled;
    static bool tempInternalEnabled;
    static bool fanControlEnabled;
    static bool batteryLevelEnabled;
    static int Battery_Voltage_PIN;
    static bool batteryLevelNumeric;
    static double batteryVoltageMax;
    static int batteryCapacityMax;
    static int Display_Screen_Width;
    static int Display_Screen_Height;
    static int Internal_Temp_PIN;
    static int Case_Fan_PIN;
    static int Sleeve_Temp_PIN;
    static int caseFanMaxDuty;
    static double internalTempForFan; // C
    static double internalMaxTemp;    // C
    static int Heater_PIN;
    static int HeatLED_PIN;
    static int I2C_SDA_PIN;
    static int I2C_SCL_PIN;
    static int TargetTemp; // Desired Temp in degC
    static int HeatPWM;    // Heating PWM setting 0-255
    static int HoldPWM;    // Hold heat PWM setting 0-255
    static int Display_I2C_Address;
    static int Display_Rst_PIN;
    // static long heaterFailsafeTime;
    static float heaterThreshold;
    static int heaterResolution;
    static int heaterFrequency;
    static int caseFanFrequency;
    static int caseFanResolution;

    static VoiceCommand voiceCommands[147];

    static const char *lastRebootReason;

    static const char *userSettingsFilePath;
    static const char *wifiPassFilePath;
    static const char *logPath;
    static const char *defaultWifiPass;
    static const char *decoyPass;

    static const char* getFirmwareVersion() {
        return FIRMWARE_VERSION_NAME;
    }

    static void load()
    {
        DynamicJsonDocument doc(deserializeSize);
        File file;
        bool loadingDefault = false;
        if (!SPIFFS.exists(userSettingsFilePath))
        {
            LogHandler::info(_TAG, "Failed to read settings file, using default configuration");
            LogHandler::info(_TAG, "Read Settings file: %s", userSettingsDefaultFilePath);
            file = SPIFFS.open(userSettingsDefaultFilePath, "r");
            loadingDefault = true;
        }
        else
        {
            LogHandler::info(_TAG, "Read Settings file: %s", userSettingsFilePath);
            file = SPIFFS.open(userSettingsFilePath, "r");
        }
        DeserializationError error = deserializeJson(doc, file);
        if (error)
        {
            LogHandler::error(_TAG, "Error deserializing settings json: %s", file.name());
            LogHandler::error(_TAG, "Code: ");
            switch (error.code())
            {
            case DeserializationError::Code::Ok:
                LogHandler::error(_TAG, "Ok");
                break;
            case DeserializationError::Code::EmptyInput:
                LogHandler::error(_TAG, "EmptyInput");
                break;
            case DeserializationError::Code::IncompleteInput:
                LogHandler::error(_TAG, "IncompleteInput");
                break;
            case DeserializationError::Code::InvalidInput:
                LogHandler::error(_TAG, "InvalidInput");
                break;
            case DeserializationError::Code::NoMemory:
                LogHandler::error(_TAG, "NoMemory");
                break;
            case DeserializationError::Code::TooDeep:
                LogHandler::error(_TAG, "TooDeep");
                break;
            }
            file = SPIFFS.open(userSettingsDefaultFilePath, "r");
            deserializeJson(doc, file);
            loadingDefault = true;
        }
        
        DynamicJsonDocument passwordDoc(100);
        LogHandler::info(_TAG, "Read password file");
        if (!SPIFFS.exists(wifiPassFilePath)) {
            LogHandler::error(_TAG, "Password file did not exist.");
        } else {
            File passwordFile = SPIFFS.open(wifiPassFilePath, "r");
            deserializeJson(passwordDoc, passwordFile);
            JsonObject passJsonObj = passwordDoc.as<JsonObject>();
            loadWifiInfo(passJsonObj);
        }

        JsonObject jsonObj = doc.as<JsonObject>();

        update(jsonObj);

        setBuildFeatures();
        setMotorType();
        //I2CScan();

        if (loadingDefault)
            save(loadingDefault);
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
    static bool isBoardType(BoardType value) {
        return boardType == value;
    }
    static void printFree() {
        LogHandler::debug(_TAG, "xPortGetFreeHeapSize: %u", xPortGetFreeHeapSize());
        LogHandler::debug(_TAG, "DRAM %u\nIRAM %u\nFREE_HEAP %u\nMIN_FREE_HEAP %u", heap_caps_get_free_size(MALLOC_CAP_8BIT), heap_caps_get_free_size(MALLOC_CAP_32BIT),  esp_get_free_heap_size(), esp_get_minimum_free_heap_size() );
    }

    static void printMemory()
    {
        LogHandler::debug(_TAG, "Free heap: %u", ESP.getFreeHeap());
        LogHandler::debug(_TAG, "Total heap: %u", ESP.getHeapSize());
        LogHandler::debug(_TAG, "Free psram: %u", ESP.getFreePsram());
        LogHandler::debug(_TAG, "Total Psram: %u", ESP.getPsramSize());
        LogHandler::debug(_TAG, "SPIFFS used: %i", SPIFFS.usedBytes());
        LogHandler::debug(_TAG, "SPIFFS total: %i", SPIFFS.totalBytes());
    }

    static void defaultAll()
    {
        DynamicJsonDocument doc(deserializeSize);
        File file = SPIFFS.open(userSettingsDefaultFilePath, "r");
        DeserializationError error = deserializeJson(doc, file);
        if (error)
            LogHandler::error(_TAG, "Failed to read default settings file, using default configuration");
        update(doc.as<JsonObject>());
        save();
    }
    static void loadWifiInfo(JsonObject json) {
        const char *wifiPassConst = json["wifiPass"] | "YOUR PASSWORD HERE";
            //LogHandler::error(_TAG, "wifiPassConst: %s", wifiPassConst);
        if (strcmp(wifiPassConst, SettingsHandler::decoyPass) != 0)
        {
            strcpy(wifiPass, wifiPassConst);
        }
    }
    static bool update(JsonObject json)
    {
        LogHandler::info(_TAG, "Load settings");
        LogHandler::debug(_TAG, "Load Json: Memory usage: %u bytes", json.memoryUsage());
        if (json.size() > 0)
        {
            logLevel = (LogLevel)(json["logLevel"] | 2);
            LogHandler::setLogLevel(logLevel);

            boardType = (BoardType)(json["boardType"] | (uint8_t)BoardType::DEVKIT);

            if(isBoardType(BoardType::CRIMZZON) || isBoardType(BoardType::ISAAC)) {
                TCodeVersionEnum = TCodeVersion::v0_3;
                TCodeVersionName = TCodeVersionMapper(TCodeVersionEnum);
            }
            // std::vector<const char*> tags;
            // //tags.push_back(TagHandler::DisplayHandler);
            // tags.push_back(TagHandler::BLDCHandler);
            // tags.push_back(TagHandler::ServoHandler3);
            // tags.push_back(TagHandler::TCodeHandler);
            // tags.push_back(TagHandler::Main);
            // LogHandler::setTags(tags);

            // JsonArray includes = json["log-include-tags"].as<JsonArray>();
            std::vector<String> includesVec;
            // for (int i = 0; i < includes.size(); i++)
            // {
            //     includesVec.push_back(includes[i]);
            // }
            setValue(json, includesVec, "log", "log-include-tags");
            LogHandler::setIncludes(includesVec);

            // JsonArray excludes = json["log-exclude-tags"].as<JsonArray>();
            std::vector<String> excludesVec;
            
            // for (int i = 0; i < excludes.size(); i++)
            // {
            //     excludesVec.push_back(excludes[i]);
            // }
            setValue(json, excludesVec, "log", "log-exclude-tags");
            LogHandler::setExcludes(excludesVec);

            const char *ssidConst = json["ssid"] | "YOUR SSID HERE";
            if (ssid != nullptr)
            {
                strcpy(ssid, ssidConst);
            }
            if(initialized)
                loadWifiInfo(json);
            if(!isBoardType(BoardType::CRIMZZON)) {
                TCodeVersionEnum = (TCodeVersion)(json["TCodeVersion"] | 2);
                TCodeVersionName = TCodeVersionMapper(TCodeVersionEnum);
            }
#if MOTOR_TYPE == 1
            for (auto x : ChannelMapBLDC) {
                currentChannels.push_back(x);
            }
#else
            currentChannels.clear();
            if(TCodeVersionEnum == TCodeVersion::v0_2) {
                // for (size_t i = 0; i < (sizeof(ChannelMapV2)/sizeof(Channel)); i++) {
                //     currentChannels.push_back(ChannelMapV2[i]);
                // }
                for (auto x : ChannelMapV2) {
                    currentChannels.push_back(x);
                }
            } else {
                for (auto x : ChannelMapV3) {
                    currentChannels.push_back(x);
                }
            }  
#endif

            int tcodeMax = TCodeVersionEnum == TCodeVersion::v0_2 ? 999 : 9999;
            for (size_t i = 0; i < currentChannels.size(); i++)
            {
                uint16_t min = json["channelRanges"][currentChannels[i].Name]["min"].as<uint16_t>();
                uint16_t max = json["channelRanges"][currentChannels[i].Name]["max"].as<uint16_t>();
                currentChannels[i].min = !min ? 1 : min;
                currentChannels[i].max = !max ? tcodeMax : max;
                // LogHandler::debug(_TAG, "update %s min: %i", currentChannels[i].Name, currentChannels[i].min);
                // LogHandler::debug(_TAG, "update %s max: %i", currentChannels[i].Name, currentChannels[i].max);
                // LogHandler::debug(_TAG, "update %s maxStored: %i", currentChannels[i].Name, maxStored);
                // LogHandler::debug(_TAG, "update %s [min]: %i", currentChannels[i].Name, json["channelRanges"][currentChannels[i].Name]["min"].as<uint16_t>());
                // LogHandler::debug(_TAG, "update %s [max]: %i", currentChannels[i].Name, json["channelRanges"][currentChannels[i].Name]["max"].as<uint16_t>());
            }
            
            udpServerPort = json["udpServerPort"] | 8000;
            webServerPort = json["webServerPort"] | 80;
            const char *hostnameTemp = json["hostname"];
            if (hostnameTemp != nullptr)
                strcpy(hostname, hostnameTemp);
            const char *friendlyNameTemp = json["friendlyName"];
            if (friendlyNameTemp != nullptr)
                strcpy(friendlyName, friendlyNameTemp);

            bluetoothEnabled = json["bluetoothEnabled"];

            // Servo motors//////////////////////////////////////////////////////////////////////////////////
            pitchFrequencyIsDifferent = json["pitchFrequencyIsDifferent"];
            msPerRad = json["msPerRad"] | 637;
            servoFrequency = json["servoFrequency"] | 50;
            pitchFrequency = json[pitchFrequencyIsDifferent ? "pitchFrequency" : "servoFrequency"] | servoFrequency;
            sr6Mode = json["sr6Mode"];

            RightServo_ZERO = json["RightServo_ZERO"] | 1500;
            LeftServo_ZERO = json["LeftServo_ZERO"] | 1500;
            RightUpperServo_ZERO = json["RightUpperServo_ZERO"] | 1500;
            LeftUpperServo_ZERO = json["LeftUpperServo_ZERO"] | 1500;
            PitchLeftServo_ZERO = json["PitchLeftServo_ZERO"] | 1500;
            PitchRightServo_ZERO = json["PitchRightServo_ZERO"] | 1500;

            BLDC_UsePWM = json["BLDC_UsePWM"] | false; // Must be before pinout is set
            BLDC_UseMT6701 = json["BLDC_UseMT6701"] | true;
            BLDC_UseHallSensor = json["BLDC_UseHallSensor"] | false;
            BLDC_Pulley_Circumference = json["BLDC_Pulley_Circumference"] | 60;
            BLDC_MotorA_Voltage = round2(json["BLDC_MotorA_Voltage"] | 20.0);
            BLDC_MotorA_Current = round2(json["BLDC_MotorA_Current"] | 1.0);
            BLDC_MotorA_ParametersKnown = json["BLDC_MotorA_ParametersKnown"] | false;
            BLDC_MotorA_ZeroElecAngle = round2(json["BLDC_MotorA_ZeroElecAngle"] | 0.00);
            BLDC_RailLength = json["BLDC_RailLength"] | 125;
            BLDC_StrokeLength = json["BLDC_StrokeLength"] | 120;

            setBoardPinout(json);
            
            if(isBoardType(BoardType::CRIMZZON)) {
                heaterResolution = json["heaterResolution"] | 8;
                caseFanResolution = json["caseFanResolution"] | 10;
                caseFanFrequency = json["caseFanFrequency"] | 25;
                Display_Screen_Height = json["Display_Screen_Height"] | 32;
            }

            twistFrequency = json["twistFrequency"] | 50;
            squeezeFrequency = json["squeezeFrequency"] | 50;
            valveFrequency = json["valveFrequency"] | 50;
            continuousTwist = json["continuousTwist"];
            feedbackTwist = json["feedbackTwist"];
            analogTwist = json["analogTwist"];
            TwistServo_ZERO = json["TwistServo_ZERO"] | 1500;
            ValveServo_ZERO = json["ValveServo_ZERO"] | 1500;
            SqueezeServo_ZERO = json["Squeeze_ZERO"] | 1500;

            staticIP = json["staticIP"];
            const char *localIPTemp = json["localIP"];
            if (localIPTemp != nullptr)
                strcpy(localIP, localIPTemp);
            const char *gatewayTemp = json["gateway"];
            if (gatewayTemp != nullptr)
                strcpy(gateway, gatewayTemp);
            const char *subnetTemp = json["subnet"];
            if (subnetTemp != nullptr)
                strcpy(subnet, subnetTemp);
            const char *dns1Temp = json["dns1"];
            if (dns1Temp != nullptr)
                strcpy(dns1, dns1Temp);
            const char *dns2Temp = json["dns2"];
            if (dns2Temp != nullptr)
                strcpy(dns2, dns2Temp);

            autoValve = json["autoValve"];
            inverseValve = json["inverseValve"];
            valveServo90Degrees = json["valveServo90Degrees"];
            inverseStroke = json["inverseStroke"];
            inversePitch = json["inversePitch"];
            lubeEnabled = json["lubeEnabled"];
            lubeAmount = json["lubeAmount"] | 255;
            displayEnabled = json["displayEnabled"];
            sleeveTempDisplayed = json["sleeveTempDisplayed"];
            internalTempDisplayed = json["internalTempDisplayed"];
            versionDisplayed = json["versionDisplayed"];
            Display_Screen_Width = json["Display_Screen_Width"] | 128;
            if(!isBoardType(BoardType::CRIMZZON)) {
                Display_Screen_Height = json["Display_Screen_Height"] | 64;
            }
            const char *Display_I2C_AddressTemp = json["Display_I2C_Address"];
            if (Display_I2C_AddressTemp != nullptr)
                Display_I2C_Address = (int)strtol(Display_I2C_AddressTemp, NULL, 0);
            Display_Rst_PIN = json["Display_Rst_PIN"] | -1;

            tempSleeveEnabled = json["tempSleeveEnabled"];
            heaterThreshold = json["heaterThreshold"] | 5.0;
            heaterFrequency = json["heaterFrequency"] | 50;
            if(!isBoardType(BoardType::CRIMZZON)) {
                heaterResolution = json["heaterResolution"] | 8;
            }
            TargetTemp = json["TargetTemp"] | 40.0;
            HeatPWM = json["HeatPWM"] | 255;
            HoldPWM = json["HoldPWM"] | 110;

            tempInternalEnabled = json["tempInternalEnabled"];
            fanControlEnabled = json["fanControlEnabled"];
            internalTempForFan = json["internalTempForFan"] | 30.0;
            internalMaxTemp = json["internalMaxTemp"] | 50.0;

            batteryLevelEnabled = json["batteryLevelEnabled"];
            batteryLevelNumeric = json["batteryLevelNumeric"];
            batteryVoltageMax = json["batteryVoltageMax"] | 12.6;
            batteryCapacityMax = json["batteryCapacityMax"] | 3500;

            if(!isBoardType(BoardType::CRIMZZON)) {
                caseFanFrequency = json["caseFanFrequency"] | 25;
                caseFanResolution = json["caseFanResolution"] | 10;
            }
            caseFanMaxDuty = pow(2, caseFanResolution) - 1;

            lubeEnabled = json["lubeEnabled"];

            motionDefaultProfileIndex = json["motionDefaultProfileIndex"] | 0;
            if(!initialized)
                motionSelectedProfileIndex = motionDefaultProfileIndex;
            JsonArray motionProfilesObj = json["motionProfiles"].as<JsonArray>();
            
            if(motionProfilesObj.isNull()) {
                LogHandler::info(_TAG, "No motion profiles stored, loading default");
                for(int i = 0; i < maxMotionProfileCount; i++) {
                    motionProfiles[i] = MotionProfile(i + 1);
                    motionProfiles[i].addDefaultChannel("L0");
                }
            } else {
                for(int i = 0; i<motionProfilesObj.size(); i++) {
                    if(i == maxMotionProfileCount) 
                        break;
                    auto profile = MotionProfile();
                    LogHandler::debug(_TAG, "Loading motion profile '%s' from settings", motionProfilesObj[i]["name"].as<String>());
                    profile.fromJson(motionProfilesObj[i].as<JsonObject>());
                    motionProfiles[i] = profile;
                }
            }

            // bool motionChannelsChanged = false;
            // JsonArray motionChannelsObj = json["motionChannels"].as<JsonArray>();
            // if(initialized && motionChannelsObj.size() != motionChannels.size()) {
            //     motionChannelsChanged = true;
            // }
            // motionChannels.clear();
            // if(motionChannelsObj.isNull()) {
            //     LogHandler::info(_TAG, "No motion channels stored, loading default");
            //     motionChannels.push_back(MotionChannel("L0"));
            // } else {
            //     for(int i = 0; i<motionChannelsObj.size(); i++) {
            //         const char* name = motionChannelsObj[i]["name"];
            //         LogHandler::debug(_TAG, "Loading motion channel '%s' from settings", name);
            //         auto channel = MotionChannel(name);
            //         LogHandler::debug(_TAG, "Loading motion channel '%s' from settings", channel.name);
            //         float phaseStored = round2(motionChannelsObj[i]["phase"].as<float>());
            //         if(initialized && phaseStored != channel.phase)
            //             motionChannelsChanged = true;
            //         channel.phase = phaseStored;
            //         bool reverseStored = motionChannelsObj[i]["reverse"].as<bool>();
            //         if(initialized && reverseStored != channel.reverse)
            //             motionChannelsChanged = true;
            //         channel.reverse = reverseStored;
            //         motionChannels.push_back(channel);
            //     }
            // }
            // if(initialized && motionChannelsChanged)
            //     sendMessage("motionGenerator", "motionChannels");

            setValue(json, voiceEnabled, "voiceHandler", "voiceEnabled", false);
            setValue(json, voiceMuted, "voiceHandler", "voiceMuted", false);
            setValue(json, voiceVolume, "voiceHandler", "voiceVolume", 0);
            setValue(json, voiceWakeTime, "voiceHandler", "voiceWakeTime", 10);

            

            lastRebootReason = machine_reset_cause();
            LogHandler::info(_TAG, "Last reset reason: %s", SettingsHandler::lastRebootReason);

            LogUpdateDebug();
            return true;
        }
        return false;
    }
    static std::vector<MotionChannel> getMotionChannels()
    {
        return motionProfiles[motionSelectedProfileIndex].channels;
    }
    static bool getMotionEnabled()
    {
        return motionEnabled;
    }
    static int getMotionDefaultProfileIndex() {
        return motionDefaultProfileIndex;
    }
    static void setMotionEnabled(const bool& newValue)
    {
        setValue(newValue, motionEnabled, "motionGenerator", "motionEnabled");
    }
    static void setMotionProfileName(const char newValue[maxMotionProfileNameLength])
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
        setValue(profileIndex, motionSelectedProfileIndex, "motionGenerator", "motionSelectedProfileIndex");
        // setMotionUpdateGlobal(profile.motionUpdateGlobal);
        // setMotionPeriodGlobal(profile.motionPeriodGlobal);
        // setMotionAmplitudeGlobal(profile.motionAmplitudeGlobal);
        // setMotionOffsetGlobal(profile.motionOffsetGlobal);
        // setMotionPhaseGlobal(profile.motionPhaseGlobal);
        // setMotionReversedGlobal(profile.motionReversedGlobal);
        // setMotionPeriodGlobalRandom(profile.motionPeriodGlobalRandom);
        // setMotionPeriodGlobalRandomMin(profile.motionPeriodGlobalRandomMin);
        // setMotionPeriodGlobalRandomMax(profile.motionPeriodGlobalRandomMax);
        // setMotionAmplitudeGlobalRandom(profile.motionAmplitudeGlobalRandom);
        // setMotionAmplitudeGlobalRandomMin(profile.motionAmplitudeGlobalRandomMin);
        // setMotionAmplitudeGlobalRandomMax(profile.motionAmplitudeGlobalRandomMax);
        // setMotionOffsetGlobalRandom(profile.motionOffsetGlobalRandom);
        // setMotionOffsetGlobalRandomMin(profile.motionOffsetGlobalRandomMin);
        // setMotionOffsetGlobalRandomMax(profile.motionOffsetGlobalRandomMax);
        // setMotionRandomChangeMin(profile.motionRandomChangeMin);
        // setMotionRandomChangeMax(profile.motionRandomChangeMax);
    }
    // static int getMotionSelectedProfileIndex() {
    //     return motionSelectedProfileIndex;
    // }
    static bool getVoiceEnabled() {
        return voiceEnabled;
    }
    static void setVoiceEnabled(bool value) {
        setValue(value, voiceEnabled, "voiceHandler", "voiceEnabled");
    }
    static bool getVoiceMuted() {
        return voiceMuted;
    }
    static void setVoiceMuted(bool value) {
        setValue(value, voiceMuted, "voiceHandler", "voiceMuted");
    }
    static int getVoiceVolume() {
        return voiceVolume;
    }
    static void setVoiceVolume(int value) {
        setValue(value, voiceVolume, "voiceHandler", "voiceVolume");
    }
    static int getVoiceWakeTime() {
        return voiceWakeTime;
    }
    static void setVoiceWakeTime(int value) {
        setValue(value, voiceWakeTime, "voiceHandler", "voiceWakeTime");
    }

/** If the parameter json is ommited or the pin value doesnt exist on the object then the pins are set to default. */
static void setBoardPinout(JsonObject json = JsonObject()) {
    if(isBoardType(BoardType::ISAAC)) {
        // RightServo_PIN = 2;
        // LeftServo_PIN = 13;
        // PitchLeftServo_PIN = 14;
        // ValveServo_PIN = 5;
        // TwistServo_PIN = 27;
        // TwistFeedBack_PIN = 33;
        // Vibe0_PIN = 15;
        // Vibe1_PIN = 16;
        // LubeButton_PIN = 36;
        // RightUpperServo_PIN = 4;
        // LeftUpperServo_PIN = 12;
        // PitchRightServo_PIN = 17;
        // Sleeve_Temp_PIN = 25;
        // Heater_PIN = 19;
        // Squeeze_PIN = 26;
        TwistFeedBack_PIN = json["TwistFeedBack_PIN"] | 32;
        RightServo_PIN = json["RightServo_PIN"] | 4;
        LeftServo_PIN = json["LeftServo_PIN"] | 13;
        RightUpperServo_PIN = json["RightUpperServo_PIN"] | 16;
        LeftUpperServo_PIN = json["LeftUpperServo_PIN"] | 27;
        PitchLeftServo_PIN = json["PitchLeftServo_PIN"] | 26;
        PitchRightServo_PIN = json["PitchRightServo_PIN"] | 17;
        ValveServo_PIN = json["ValveServo_PIN"] | 18;
        TwistServo_PIN = json["TwistServo_PIN"] | 25;
        // Common motor
        Squeeze_PIN = json["Squeeze_PIN"] | 19;
        LubeButton_PIN = json["LubeButton_PIN"] | 34;
        // Internal_Temp_PIN = json["Internal_Temp_PIN"] | 34;
        Sleeve_Temp_PIN = json["Temp_PIN"] | 33;
        // Case_Fan_PIN = json["Case_Fan_PIN"] | 16;
        Vibe0_PIN = json["Vibe0_PIN"] | 15;
        Vibe1_PIN = json["Vibe1_PIN"] | 2;
        // Vibe2_PIN = json["Vibe2_PIN"] | 23;
        // Vibe3_PIN = json["Vibe3_PIN"] | 32;
        Heater_PIN = json["Heater_PIN"] | 5;
        Battery_Voltage_PIN = json["Battery_Voltage_PIN"] | 39;
    } else if(isBoardType(BoardType::CRIMZZON)) {

        Vibe3_PIN = json["Vibe3_PIN"] | 26;
        Internal_Temp_PIN = json["Internal_Temp_PIN"] | 32;

        // EXT
        //  EXT_Input2_PIN = 34;
        //  EXT_Input3_PIN = 39;
        //  EXT_Input4_PIN = 36;

        heaterResolution = json["heaterResolution"] | 8;
        caseFanResolution = json["caseFanResolution"] | 10;
        caseFanFrequency = json["caseFanFrequency"] | 25;
        Display_Screen_Height = json["Display_Screen_Height"] | 32;
        TwistFeedBack_PIN = json["TwistFeedBack_PIN"] | 0;
    }
    if(!isBoardType(BoardType::ISAAC)) { // Devkit v1 pins
        if(!isBoardType(BoardType::CRIMZZON)) {
            TwistFeedBack_PIN = json["TwistFeedBack_PIN"] | 26;
        }
        // Common motor
        Squeeze_PIN = json["Squeeze_PIN"] | 17;
        LubeButton_PIN = json["LubeButton_PIN"] | 35;
        if(!isBoardType(BoardType::CRIMZZON)) {
            Internal_Temp_PIN = json["Internal_Temp_PIN"] | 34;
        }
        Case_Fan_PIN = json["Case_Fan_PIN"] | 16;
#if MOTOR_TYPE == 0
        RightServo_PIN = json["RightServo_PIN"] | 13;
        LeftServo_PIN = json["LeftServo_PIN"] | 15;
        RightUpperServo_PIN = json["RightUpperServo_PIN"] | 12;
        LeftUpperServo_PIN = json["LeftUpperServo_PIN"] | 2;
        PitchLeftServo_PIN = json["PitchLeftServo_PIN"] | 4;
        PitchRightServo_PIN = json["PitchRightServo_PIN"] | 14;

        Heater_PIN = json["Heater_PIN"] | 33;
        ValveServo_PIN = json["ValveServo_PIN"] | 25;
        TwistServo_PIN = json["TwistServo_PIN"] | 27;
        Sleeve_Temp_PIN = json["Temp_PIN"] | 5;
        Vibe0_PIN = json["Vibe0_PIN"] | 18;
        Vibe1_PIN = json["Vibe1_PIN"] | 19;
        Vibe2_PIN = json["Vibe2_PIN"] | 23;
        if(!isBoardType(BoardType::CRIMZZON)) {
            Vibe3_PIN = json["Vibe3_PIN"] | 32;
        }
#elif MOTOR_TYPE == 1 // BLDC motor
        BLDC_Encoder_PIN = json["BLDC_Encoder_PIN"] | 33;
        BLDC_ChipSelect_PIN = json["BLDC_ChipSelect_PIN"] | 5;
        BLDC_Enable_PIN = json["BLDC_Enable_PIN"] | 14;
        BLDC_HallEffect_PIN = json["BLDC_HallEffect_PIN"] | 35;
        BLDC_PWMchannel1_PIN = json["BLDC_PWMchannel1_PIN"] | 27;
        BLDC_PWMchannel2_PIN = json["BLDC_PWMchannel2_PIN"] | 26;
        BLDC_PWMchannel3_PIN = json["BLDC_PWMchannel3_PIN"] | 25;

        Heater_PIN = json["Heater_PIN"] | 15;
        ValveServo_PIN = json["ValveServo_PIN"] | 12;
        TwistServo_PIN = json["TwistServo_PIN"] | 13;
        Sleeve_Temp_PIN = json["Temp_PIN"] | 36;
        
        Vibe0_PIN = json["Vibe0_PIN"] | 2;
        Vibe1_PIN = json["Vibe1_PIN"] | 22;
        Vibe2_PIN = json["Vibe2_PIN"] | 4;
        if(!isBoardType(BoardType::CRIMZZON)) {
            Vibe3_PIN = json["Vibe3_PIN"] | 32;
        }
#endif
        Battery_Voltage_PIN = json["Battery_Voltage_PIN"] | 39;
    }
}
    // Untested
    /*    static bool parse(const String jsonInput)
       {
           DynamicJsonDocument doc(readCapacity);
           DeserializationError error = deserializeJson(doc, jsonInput);
           if (error)
           {
               Serial.println(F("Failed to deserialize json string"));
               return false;
           }
           update(doc.as<JsonObject>());
           save();
           return true;
       } */

    // static char* getJsonForBLE()
    // {
    //     //DynamicJsonDocument doc(readCapacity);
    //     //DeserializationError error = deserializeJson(doc, jsonInput, sizeof(jsonInput));
    //     File file = SPIFFS.open(filename, "r");
    //     size_t size = file.size();
    //     char* bytes = new char[size];
    //     file.readBytes(bytes, size);
    //     return bytes;
    // }
    static const bool readFile(char* &buf, const char* path) {
        if(!SPIFFS.exists(path)) {
            LogHandler::error(_TAG, "Path did not exist when reading contents: %s", path);
            return false;
        }
        File file = SPIFFS.open(path, "r");
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
    // static void serialize(char buf[40000])
    // {
    //     LogHandler::info(_TAG, "Serialize settings...");
    //     File file = SPIFFS.open(userSettingsFilePath, "r");
    //     LogHandler::info(_TAG, "Serialize settings...");
    //     DynamicJsonDocument doc(deserializeSize);
    //     LogHandler::info(_TAG, "Serialize settings...");
    //     DeserializationError error = deserializeJson(doc, file);
    //     if (error)
    //     {
    //         LogHandler::error("toJson: Error deserializing settings json: %s", file.name());
    //         buf[0] = {0};
    //         return;
    //     }
    //     file.close();
    //     if (strcmp(doc["wifiPass"], defaultWifiPass) != 0)
    //         doc["wifiPass"] = "Too bad haxor!"; // Do not send password if its not default

    //     doc["motionEnabled"] = motionEnabled;
    //     doc["motionSelectedProfileIndex"] = motionSelectedProfileIndex;
    //     String output;
    //     serializeJson(doc, output);
    //     doc.clear();
    //     // serializeJson(doc, Serial);
    //     if (LogHandler::getLogLevel() == LogLevel::VERBOSE)
    //     {
    //         Serial.printf("Settings: %s\n", output.c_str());
    //         Serial.printf("Size: %ld\n", strlen(output.c_str()));
    //     }
    //     // LogHandler::verbose(_TAG, "Output: %s", output.c_str());
    //     buf[0] = {0};
    //     LogHandler::debug(_TAG, "Serialize settings...");
    //     strcpy(buf, output.c_str());
    // }

    static void getSystemInfo(char buf[1024])
    {
        DynamicJsonDocument doc(1024);

        doc["esp32Version"] = FIRMWARE_VERSION_NAME;
        doc["TCodeVersion"] = (int)TCodeVersionEnum;
        doc["lastRebootReason"] = lastRebootReason;
        
        JsonArray boardTypes = doc.createNestedArray("boardTypes");
        JsonObject devkit = boardTypes.createNestedObject();
        devkit["name"] = "Devkit/NexusPRO";
        devkit["value"] = (uint8_t)BoardType::DEVKIT;
        JsonObject SR6MB = boardTypes.createNestedObject();
        SR6MB["name"] = "SR6MB";
        SR6MB["value"] = (uint8_t)BoardType::CRIMZZON;
        JsonObject INControl = boardTypes.createNestedObject();
        INControl["name"] = "IN-Control";
        INControl["value"] = (uint8_t)BoardType::ISAAC;
        
        doc["motorType"] = (int)motorType;
        JsonArray buildFeaturesJsonArray = doc.createNestedArray("buildFeatures");
        for (BuildFeature value : buildFeatures)
        {
            buildFeaturesJsonArray.add((int)value);
        }

        JsonArray availableTagsJsonArray = doc.createNestedArray("availableTags");
        for (const char *tag : TagHandler::AvailableTags)
        {
            availableTagsJsonArray.add(tag);
        }
        JsonArray systemI2CAddressesJsonArray = doc.createNestedArray("systemI2CAddresses");
        systemI2CAddressesJsonArray.add("0x0");
        for (int value : systemI2CAddresses) {
			char buf[10];
            hexToString(value, buf);
            systemI2CAddressesJsonArray.add(buf);
        }
        doc["motionEnabled"] = motionEnabled;
        doc["motionSelectedProfileIndex"] = motionSelectedProfileIndex;
        
        doc["localIP"] = localIP;
        doc["gateway"] = gateway;
        doc["subnet"] = subnet;
        doc["dns1"] = dns1;
        doc["dns2"] = dns2;

        doc["chipModel"] = ESP.getChipModel();
        doc["chipRevision"] = ESP.getChipRevision();
        doc["chipCores"] = ESP.getChipCores();
        uint32_t chipId = 0;
        for (int i = 0; i < 17; i = i + 8)
        {
            chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
        }
        doc["chipID"] = chipId;

        String output;
        serializeJson(doc, output);
        doc.clear();
        if (LogHandler::getLogLevel() == LogLevel::VERBOSE)
            Serial.printf("SystemInfo: %s\n", output.c_str());
        buf[0] = {0};
        strcpy(buf, output.c_str());
    }
//   static void getMotionSettings(char buf[24576])
//     {
//         DynamicJsonDocument doc(24576);

//         int len = sizeof(motionProfiles)/sizeof(motionProfiles[0]);
//         for (int i=0; i < len; i++)
//         {
//             doc["motionProfiles"][i]["name"] = motionProfiles[i].motionProfileName;
//             for (size_t j = 0; j < motionProfiles[i].channels.size(); j++)
//             {
//                 doc["motionProfiles"][i]["channels"][j]["name"] = motionProfiles[i].channels[j].name;
//                 doc["motionProfiles"][i]["channels"][j]["update"] = motionProfiles[i].channels[j].motionUpdateGlobal;
//                 doc["motionProfiles"][i]["channels"][j]["period"] = motionProfiles[i].channels[j].motionPeriodGlobal;
//                 doc["motionProfiles"][i]["channels"][j]["amp"] = motionProfiles[i].channels[j].motionAmplitudeGlobal;
//                 doc["motionProfiles"][i]["channels"][j]["offset"] = motionProfiles[i].channels[j].motionOffsetGlobal;
//                 doc["motionProfiles"][i]["channels"][j]["phase"] = motionProfiles[i].channels[j].motionPhaseGlobal;
//                 doc["motionProfiles"][i]["channels"][j]["reverse"] = motionProfiles[i].channels[j].motionReversedGlobal;
//                 doc["motionProfiles"][i]["channels"][j]["periodRan"] = motionProfiles[i].channels[j].motionPeriodGlobalRandom;
//                 doc["motionProfiles"][i]["channels"][j]["ampRan"] = motionProfiles[i].channels[j].motionAmplitudeGlobalRandom;
//                 doc["motionProfiles"][i]["channels"][j]["offsetRan"] = motionProfiles[i].channels[j].motionOffsetGlobalRandom;
//                 doc["motionProfiles"][i]["channels"][j]["periodMin"] = motionProfiles[i].channels[j].motionPeriodGlobalRandomMin;
//                 doc["motionProfiles"][i]["channels"][j]["periodMax"] = motionProfiles[i].channels[j].motionPeriodGlobalRandomMax;
//                 doc["motionProfiles"][i]["channels"][j]["ampMin"] = motionProfiles[i].channels[j].motionAmplitudeGlobalRandomMin;
//                 doc["motionProfiles"][i]["channels"][j]["ampMax"] = motionProfiles[i].channels[j].motionAmplitudeGlobalRandomMax;
//                 doc["motionProfiles"][i]["channels"][j]["offsetMin"] = motionProfiles[i].channels[j].motionOffsetGlobalRandomMin;
//                 doc["motionProfiles"][i]["channels"][j]["offsetMax"] = motionProfiles[i].channels[j].motionOffsetGlobalRandomMax;
//                 doc["motionProfiles"][i]["channels"][j]["ranMin"] = motionProfiles[i].channels[j].motionRandomChangeMin;
//                 doc["motionProfiles"][i]["channels"][j]["ranMax"] = motionProfiles[i].channels[j].motionRandomChangeMax;
//             }
//         }

//         String output;
//         serializeJson(doc, output);
//         doc.clear();
//         if (LogHandler::getLogLevel() == LogLevel::VERBOSE)
//             Serial.printf("MotionInfo: %s\n", output.c_str());
//         buf[0] = {0};
//         strcpy(buf, output.c_str());
//     }

    static bool save(String data)
    {
        LogHandler::debug(_TAG, "Save frome string");
        printMemory();
        DynamicJsonDocument doc(deserializeSize);

        DeserializationError error = deserializeJson(doc, data);
        if (error)
        {
            LogHandler::error(_TAG, "Settings save: Deserialize error: %s", error.c_str());
            return false;
        }
        // const char* ssidConst = doc["ssid"];
        // if( ssid != nullptr)
        // {
        //     strcpy(ssid, ssidConst);
        // }
        // const char* wifiPassConst = doc["wifiPass"];
        // if(wifiPassConst != nullptr && strcmp(wifiPassConst, SettingsHandler::decoyPass) != 0)
        // {
        //     strcpy(wifiPass, wifiPassConst);
        // }
        // staticIP = doc["staticIP"];
        // servoFrequency = doc["servoFrequency"] | 50;
        // const char* localIPTemp = doc["localIP"];
        // if (localIPTemp != nullptr)
        //     strcpy(localIP, localIPTemp);
        // const char* gatewayTemp = doc["gateway"];
        // if (gatewayTemp != nullptr)
        //     strcpy(gateway, gatewayTemp);
        // const char* subnetTemp = doc["subnet"];
        // if (subnetTemp != nullptr)
        //     strcpy(subnet, subnetTemp);
        // const char* dns1Temp = doc["dns1"];
        // if (dns1Temp != nullptr)
        //     strcpy(dns1, dns1Temp);
        // const char* dns2Temp = doc["dns2"];
        // if (dns2Temp != nullptr)
        //     strcpy(dns2, dns2Temp);
        // sr6Mode = doc["sr6Mode"];
        // bluetoothEnabled =  doc["bluetoothEnabled"];
        // return save();
        if (!update(doc.as<JsonObject>()))
        {
            LogHandler::error(_TAG, "Settings save: update error");
            return false;
        }
        if (!save())
        {
            LogHandler::error(_TAG, "Settings save: save error");
            return false;
        }
        return true;
    }

    static bool save(bool loadingDefault = false)
    {
        LogHandler::info(_TAG, "Save settings");
        saving = true;
        // Delete existing file, otherwise the configuration is appended to the file
        // Serial.print("SPIFFS used: ");
        // Serial.println(SPIFFS.usedBytes() + "/" + SPIFFS.totalBytes());
        if (!SPIFFS.remove(userSettingsFilePath))
        {
            LogHandler::error(_TAG, "Failed to remove settings file: %s", userSettingsFilePath);
        }
        File file = SPIFFS.open(userSettingsFilePath, FILE_WRITE);
        if (!file)
        {
            LogHandler::error(_TAG, "Failed to create settings file: %s", userSettingsFilePath);
            return false;
        }

        // Allocate a temporary JsonDocument
        DynamicJsonDocument doc(serializeSize);

        doc["boardType"] = (uint8_t)boardType;
        doc["logLevel"] = (int)logLevel;
        LogHandler::setLogLevel(logLevel);
        // Serial.println("logLevel: ");
        // Serial.println((int)logLevel);
        // Serial.println( doc["logLevel"] .as<int>());
        // Serial.println((int)doc["logLevel"]);

        // std::vector<const char*> tags;
        //  tags.push_back(TagHandler::DisplayHandler);
        //  tags.push_back(TagHandler::BLDCHandler);
        //  tags.push_back(TagHandler::ServoHandler3);
        // LogHandler::setTags(tags);
        
        for (size_t i = 0; i < currentChannels.size(); i++)
        {
            doc["channelRanges"][currentChannels[i].Name]["min"] = currentChannels[i].min;
            doc["channelRanges"][currentChannels[i].Name]["max"] = currentChannels[i].max;
            // LogHandler::debug(_TAG, "save %s min: %i", currentChannels[i].Name, doc["channelRanges"][currentChannels[i].Name]["min"].as<int>());
            // LogHandler::debug(_TAG, "save %s max: %i", currentChannels[i].Name, doc["channelRanges"][currentChannels[i].Name]["max"].as<int>());
        }

        if (!tempInternalEnabled)
        {
            internalTempDisplayed = false;
            fanControlEnabled = false;
        }
        if (!tempSleeveEnabled)
        {
            sleeveTempDisplayed = false;
        }
        doc["fullBuild"] = fullBuild;
        doc["esp32Version"] = FIRMWARE_VERSION_NAME;
        doc["TCodeVersion"] = (int)TCodeVersionEnum;
        doc["ssid"] = ssid;
        
        if (!SPIFFS.exists(wifiPassFilePath)) {
            LogHandler::error(_TAG, "Password file did not exist.");
            saving = false;
            return false;
        } else {
            DynamicJsonDocument passwordDoc(100);
            LogHandler::info(_TAG, "Save password file");
            File passwordFile = SPIFFS.open(wifiPassFilePath, FILE_WRITE);
            passwordDoc["wifiPass"] = wifiPass;
            if (serializeJson(passwordDoc, passwordFile) == 0)
            {
                LogHandler::error(_TAG, "Failed to write to password file");
                passwordDoc["wifiPass"] = defaultWifiPass;
                file.close();
                saving = false;
                return false;
            }
        }
        if(strcmp(wifiPass, "YOUR PASSWORD HERE") != 0) {
            doc["wifiPass"] = decoyPass;
        } else {
            doc["wifiPass"] = "YOUR PASSWORD HERE";
        }
        // doc["wifiPass"] = wifiPass;
        doc["udpServerPort"] = udpServerPort;
        doc["webServerPort"] = webServerPort;
        doc["hostname"] = hostname;
        doc["friendlyName"] = friendlyName;
        doc["bluetoothEnabled"] = bluetoothEnabled;
        doc["pitchFrequencyIsDifferent"] = pitchFrequencyIsDifferent;
        doc["msPerRad"] = msPerRad;
        doc["servoFrequency"] = servoFrequency;
        doc["pitchFrequency"] = pitchFrequency;
        doc["valveFrequency"] = valveFrequency;
        doc["twistFrequency"] = twistFrequency;
        doc["squeezeFrequency"] = squeezeFrequency;
        doc["continuousTwist"] = continuousTwist;
        doc["feedbackTwist"] = feedbackTwist;
        doc["analogTwist"] = analogTwist;
        doc["TwistFeedBack_PIN"] = TwistFeedBack_PIN;
        doc["RightServo_PIN"] = RightServo_PIN;
        doc["LeftServo_PIN"] = LeftServo_PIN;
        doc["RightUpperServo_PIN"] = RightUpperServo_PIN;
        doc["LeftUpperServo_PIN"] = LeftUpperServo_PIN;
        doc["PitchLeftServo_PIN"] = PitchLeftServo_PIN;
        doc["PitchRightServo_PIN"] = PitchRightServo_PIN;
        doc["ValveServo_PIN"] = ValveServo_PIN;
        doc["TwistServo_PIN"] = TwistServo_PIN;
        doc["Squeeze_PIN"] = Squeeze_PIN;
        doc["Vibe0_PIN"] = Vibe0_PIN;
        doc["Vibe1_PIN"] = Vibe1_PIN;
        doc["Vibe2_PIN"] = Vibe2_PIN;
        doc["Vibe3_PIN"] = Vibe3_PIN;
        doc["Case_Fan_PIN"] = Case_Fan_PIN;
        doc["LubeButton_PIN"] = LubeButton_PIN;
        doc["Internal_Temp_PIN"] = Internal_Temp_PIN;

        doc["BLDC_UsePWM"] = BLDC_UsePWM;
        doc["BLDC_UseMT6701"] = BLDC_UseMT6701;
        doc["BLDC_UseHallSensor"] = BLDC_UseHallSensor;
        doc["BLDC_Pulley_Circumference"] = BLDC_Pulley_Circumference;
        doc["BLDC_Encoder_PIN"] = BLDC_Encoder_PIN;
        doc["BLDC_ChipSelect_PIN"] = BLDC_ChipSelect_PIN;
        doc["BLDC_Enable_PIN"] = BLDC_Enable_PIN;
        doc["BLDC_HallEffect_PIN"] = BLDC_HallEffect_PIN;
        doc["BLDC_PWMchannel1_PIN"] = BLDC_PWMchannel1_PIN;
        doc["BLDC_PWMchannel2_PIN"] = BLDC_PWMchannel2_PIN;
        doc["BLDC_PWMchannel3_PIN"] = BLDC_PWMchannel3_PIN;
        doc["BLDC_MotorA_Voltage"] = round2(BLDC_MotorA_Voltage);
        doc["BLDC_MotorA_Current"] = round2(BLDC_MotorA_Current);
        doc["BLDC_MotorA_ZeroElecAngle"] = round2(BLDC_MotorA_ZeroElecAngle);
        doc["BLDC_RailLength"] = BLDC_RailLength;
        doc["BLDC_StrokeLength"] = BLDC_StrokeLength;
        
        LogHandler::debug(_TAG, "save %s max: %f", "BLDC_MotorA_Voltage", doc["BLDC_MotorA_Current"].as<float>());

        doc["staticIP"] = staticIP;
        doc["localIP"] = localIP;
        doc["gateway"] = gateway;
        doc["subnet"] = subnet;
        doc["dns1"] = dns1;
        doc["dns2"] = dns2;
        doc["sr6Mode"] = sr6Mode;
        doc["RightServo_ZERO"] = RightServo_ZERO;
        doc["LeftServo_ZERO"] = LeftServo_ZERO;
        doc["RightUpperServo_ZERO"] = RightUpperServo_ZERO;
        doc["LeftUpperServo_ZERO"] = LeftUpperServo_ZERO;
        doc["PitchLeftServo_ZERO"] = PitchLeftServo_ZERO;
        doc["PitchRightServo_ZERO"] = PitchRightServo_ZERO;
        doc["TwistServo_ZERO"] = TwistServo_ZERO;
        doc["ValveServo_ZERO"] = ValveServo_ZERO;
        doc["Squeeze_ZERO"] = SqueezeServo_ZERO;
        doc["autoValve"] = autoValve;
        doc["inverseValve"] = inverseValve;
        doc["valveServo90Degrees"] = valveServo90Degrees;
        doc["inverseStroke"] = inverseStroke;
        doc["inversePitch"] = inversePitch;
        doc["lubeAmount"] = lubeAmount;
        doc["lubeEnabled"] = lubeEnabled;
        doc["displayEnabled"] = displayEnabled;
        doc["sleeveTempDisplayed"] = sleeveTempDisplayed;
        doc["versionDisplayed"] = versionDisplayed;
        doc["internalTempDisplayed"] = internalTempDisplayed;
        doc["tempSleeveEnabled"] = tempSleeveEnabled;
        doc["Display_Screen_Width"] = Display_Screen_Width;
        doc["Display_Screen_Height"] = Display_Screen_Height;
        doc["TargetTemp"] = TargetTemp;
        doc["HeatPWM"] = HeatPWM;
        doc["HoldPWM"] = HoldPWM;
        std::stringstream Display_I2C_Address_String;
        Display_I2C_Address_String << "0x" << std::hex << Display_I2C_Address;
        doc["Display_I2C_Address"] = Display_I2C_Address_String.str();
        doc["Display_Rst_PIN"] = Display_Rst_PIN;
        doc["Temp_PIN"] = Sleeve_Temp_PIN;
        doc["Heater_PIN"] = Heater_PIN;
        // doc["heaterFailsafeTime"] = String(heaterFailsafeTime);
        doc["heaterThreshold"] = heaterThreshold;
        doc["heaterResolution"] = heaterResolution;
        doc["heaterFrequency"] = heaterFrequency;
        doc["fanControlEnabled"] = fanControlEnabled;
        doc["caseFanFrequency"] = caseFanFrequency;
        doc["caseFanResolution"] = caseFanResolution;
        doc["internalTempForFan"] = internalTempForFan;
        doc["internalMaxTemp"] = internalMaxTemp;
        doc["tempInternalEnabled"] = tempInternalEnabled;

        doc["batteryLevelEnabled"] = batteryLevelEnabled;
        doc["Battery_Voltage_PIN"] = Battery_Voltage_PIN;
        doc["batteryLevelNumeric"] = batteryLevelNumeric;
        doc["batteryVoltageMax"] = round2(batteryVoltageMax);
        doc["batteryCapacityMax"] = batteryCapacityMax;

        doc["motionDefaultProfileIndex"] = motionDefaultProfileIndex;
        int len = sizeof(motionProfiles)/sizeof(motionProfiles[0]);
        for (int i=0; i < len; i++)
        {
            doc["motionProfiles"][i]["name"] = motionProfiles[i].motionProfileName;
            for (size_t j = 0; j < motionProfiles[i].channels.size(); j++)
            {
                doc["motionProfiles"][i]["channels"][j]["name"] = motionProfiles[i].channels[j].name;
                doc["motionProfiles"][i]["channels"][j]["update"] = motionProfiles[i].channels[j].motionUpdateGlobal;
                doc["motionProfiles"][i]["channels"][j]["period"] = motionProfiles[i].channels[j].motionPeriodGlobal;
                doc["motionProfiles"][i]["channels"][j]["amp"] = motionProfiles[i].channels[j].motionAmplitudeGlobal;
                doc["motionProfiles"][i]["channels"][j]["offset"] = motionProfiles[i].channels[j].motionOffsetGlobal;
                doc["motionProfiles"][i]["channels"][j]["phase"] = motionProfiles[i].channels[j].motionPhaseGlobal;
                doc["motionProfiles"][i]["channels"][j]["reverse"] = motionProfiles[i].channels[j].motionReversedGlobal;
                doc["motionProfiles"][i]["channels"][j]["periodRan"] = motionProfiles[i].channels[j].motionPeriodGlobalRandom;
                doc["motionProfiles"][i]["channels"][j]["periodMin"] = motionProfiles[i].channels[j].motionPeriodGlobalRandomMin;
                doc["motionProfiles"][i]["channels"][j]["periodMax"] = motionProfiles[i].channels[j].motionPeriodGlobalRandomMax;
                doc["motionProfiles"][i]["channels"][j]["ampRan"] = motionProfiles[i].channels[j].motionAmplitudeGlobalRandom;
                doc["motionProfiles"][i]["channels"][j]["ampMin"] = motionProfiles[i].channels[j].motionAmplitudeGlobalRandomMin;
                doc["motionProfiles"][i]["channels"][j]["ampMax"] = motionProfiles[i].channels[j].motionAmplitudeGlobalRandomMax;
                doc["motionProfiles"][i]["channels"][j]["offsetRan"] = motionProfiles[i].channels[j].motionOffsetGlobalRandom;
                doc["motionProfiles"][i]["channels"][j]["offsetMin"] = motionProfiles[i].channels[j].motionOffsetGlobalRandomMin;
                doc["motionProfiles"][i]["channels"][j]["offsetMax"] = motionProfiles[i].channels[j].motionOffsetGlobalRandomMax;
                doc["motionProfiles"][i]["channels"][j]["phaseRan"] = motionProfiles[i].channels[j].motionPhaseRandom;
                doc["motionProfiles"][i]["channels"][j]["phaseMin"] = motionProfiles[i].channels[j].motionPhaseRandomMin;
                doc["motionProfiles"][i]["channels"][j]["phaseMax"] = motionProfiles[i].channels[j].motionPhaseRandomMax;
                doc["motionProfiles"][i]["channels"][j]["ranMin"] = motionProfiles[i].channels[j].motionRandomChangeMin;
                doc["motionProfiles"][i]["channels"][j]["ranMax"] = motionProfiles[i].channels[j].motionRandomChangeMax;
            }
        }

        doc["voiceEnabled"] = voiceEnabled;
        doc["voiceMuted"] = voiceMuted;
        doc["voiceWakeTime"] = voiceWakeTime;
        doc["voiceVolume"] = voiceVolume;

        JsonArray includes = doc.createNestedArray("log-include-tags");
        std::vector<String> includesVec = LogHandler::getIncludes();
        for (int i = 0; i < includesVec.size(); i++)
        {
            includes.add(includesVec[i]);
        }

        JsonArray excludes = doc.createNestedArray("log-exclude-tags");
        std::vector<String> excludesVec = LogHandler::getExcludes();
        for (int i = 0; i < excludesVec.size(); i++)
        {
            excludes.add(excludesVec[i]);
        }

        LogHandler::debug(_TAG, "Is overflowed: %u", doc.overflowed());
        if (doc.overflowed())
        {
            LogHandler::error(_TAG, "Document is overflowed! Increase serialize size: %u", doc.memoryUsage());
            file.close();
            saving = false;
            return false;
        }

        LogHandler::debug(_TAG, "isNull: %u", doc.isNull());
        if (doc.isNull())
        {
            LogHandler::error(_TAG, "Document is null!");
            file.close();
            saving = false;
            return false;
        }

        LogHandler::debug(_TAG, "Memory usage: %u bytes", doc.memoryUsage());
        if (doc.memoryUsage() == 0)
        {
            LogHandler::error(_TAG, "Document is empty!");
            file.close();
            saving = false;
            return false;
        }
        // Stopped working for some reason...
        // Printing all defaults
        LogSaveDebug(doc);

        if (serializeJson(doc, file) == 0)
        {
            LogHandler::error(_TAG, "Failed to write to file");
            doc["wifiPass"] = "";
            file.close();
            saving = false;
            return false;
        }
        LogHandler::debug(_TAG, "File contents: %s", file.readString().c_str());
        printMemory();
        file.close(); // Task exception here could mean not enough space on SPIFFS.

        doc["wifiPass"] = "";
        saving = false;
        return true;
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
#if TEMP_ENABLED
        LogHandler::debug("setBuildFeatures", "TEMP_ENABLED");
        buildFeatures[index] = BuildFeature::TEMP;
        index++;
#endif
#if DISPLAY_ENABLED
        LogHandler::debug("setBuildFeatures", "DISPLAY_ENABLED");
        buildFeatures[index] = BuildFeature::DISPLAY_;
        index++;
#endif
#if TCODE_V2
        LogHandler::debug("setBuildFeatures", "TCODE_V2");
        buildFeatures[index] = BuildFeature::HAS_TCODE_V2;
        index++;
#endif
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
        motorType = MotorType::Servo;
#elif MOTOR_TYPE == 1
        motorType = MotorType::BLDC;
#endif
    }

    static void processTCodeJson(char *outbuf, const char *tcodeJson)
    {
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, tcodeJson);
        if (error)
        {
            LogHandler::error(_TAG, "Failed to read udp jsonobject, using default configuration");
            outbuf[0] = {0};
            return;
        }
        JsonArray arr = doc.as<JsonArray>();
        char buffer[255] = "";
        for (JsonObject repo : arr)
        {
            const char *channel = repo["c"];
            int value = repo["v"];
            if (channel != nullptr && value > 0)
            {
                if (buffer[0] == '\0')
                {
                    // Serial.println("tcode empty");
                    strcpy(buffer, channel);
                }
                else
                {
                    strcat(buffer, channel);
                }
                // Serial.print("channel: ");
                // Serial.print(channel);
                // Serial.print(" value: ");
                // Serial.println(value);
                char integer_string[4];
                sprintf(integer_string, SettingsHandler::TCodeVersionEnum == TCodeVersion::v0_2 ? "%03d" : "%04d", SettingsHandler::calculateRange(channel, value));
                // pad(integer_string);
                // sprintf(integer_string, "%d", SettingsHandler::calculateRange(channel, value));
                // Serial.print("integer_string");
                // Serial.println(integer_string);
                strcat(buffer, integer_string);
                int speed = repo["s"];
                int interval = repo["i"];
                if (interval > 0)
                {
                    char interval_string[5];
                    sprintf(interval_string, "%d", interval);
                    strcat(buffer, "I");
                    strcat(buffer, interval_string);
                }
                else if (speed > 0)
                {
                    char speed_string[5];
                    sprintf(speed_string, "%d", speed);
                    strcat(buffer, "S");
                    strcat(buffer, speed_string);
                }
                strcat(buffer, " ");
                // Serial.print("buffer ");
                // Serial.println(buffer);
            }
        }
        strcpy(outbuf, buffer);
        strcat(outbuf, "\n");
        // Serial.print("outbuf ");
        // Serial.println(outbuf);
    }
    static bool waitForI2CDevices(const int& i2cAddress = 0) {
        int tries = 0;
        while((systemI2CAddresses.size() == 0 || i2cAddress) && tries <= 3) {
            tries++;
            I2CScan();
            if(i2cAddress && std::find(systemI2CAddresses.begin(), systemI2CAddresses.end(), i2cAddress) != systemI2CAddresses.end()) {
                return true;
            } else if(i2cAddress) {
                LogHandler::info(_TAG, "I2c address: %ld not found. trying again...", i2cAddress);
            } else {
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
		Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
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

				systemI2CAddresses.push_back(address);
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
        return TCodeVersionEnum == TCodeVersion::v0_2 ? 999 : 9999;
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
    static const char *userSettingsDefaultFilePath;
    static SETTING_STATE_FUNCTION_PTR_T message_callback;
    // Use http://arduinojson.org/assistant to compute the capacity.
    // static const size_t readCapacity = JSON_OBJECT_SIZE(100) + 2000;
    // static const size_t saveCapacity = JSON_OBJECT_SIZE(100);
    static const int deserializeSize = 32768;
    static const int serializeSize = 24576;
    // 3072

    static bool motionEnabled;
    //static char motionSelectedProfileName[maxMotionProfileNameLength];
    static int motionSelectedProfileIndex;
    static int motionDefaultProfileIndex;
    static MotionProfile motionProfiles[maxMotionProfileCount];
    //static std::map<const char*, MotionProfile*, StrCompare>  motionProfiles;
    // static int motionUpdateGlobal;
    // static int motionPeriodGlobal;
    // static int motionAmplitudeGlobal;
    // static int motionOffsetGlobal;
    // static bool motionPeriodGlobalRandom;
    // static int motionPeriodGlobalRandomMin;
    // static int motionPeriodGlobalRandomMax;
    // static bool motionAmplitudeGlobalRandom;
    // static int motionAmplitudeGlobalRandomMin;
    // static int motionAmplitudeGlobalRandomMax;
    // static bool motionOffsetGlobalRandom;
    // static int motionOffsetGlobalRandomMin;
    // static int motionOffsetGlobalRandomMax;
    // static float motionPhaseGlobal;
    // static bool motionReversedGlobal;
    // static int motionRandomChangeMin;
    // static int motionRandomChangeMax;
    
    static bool voiceEnabled;
    static bool voiceMuted;
    static int voiceWakeTime ;
    static int voiceVolume;

    static void sendMessage(const char *group, const char *message)
    {
        if (message_callback)
        {
            LogHandler::debug(_TAG, "sendMessage: message_callback group %s: %s", group, message);
            message_callback(group, message);
        }
        else
        {
            LogHandler::debug(_TAG, "sendMessage: message_callback 0");
        }
    }

    static void setValue(JsonObject json, bool &variable, const char *propertyGroup, const char *propertyName, bool defaultValue)
    {
        bool newValue = json[propertyName] | defaultValue;
        setValue(newValue, variable, propertyGroup, propertyName);
    }
    static void setValue(JsonObject json, char *&variable, const char *propertyGroup, const char *propertyName, const char *defaultValue)
    {
        const char *newValue = json[propertyName] | defaultValue;
        setValue(newValue, variable, propertyGroup, propertyName);
    }
    static void setValue(JsonObject json, int &variable, const char *propertyGroup, const char *propertyName, int defaultValue)
    {
        int newValue = json[propertyName] | defaultValue;
        setValue(newValue, variable, propertyGroup, propertyName);
    }
    static void setValue(JsonObject json, float &variable, const char *propertyGroup, const char *propertyName, float defaultValue)
    {
        float newValue = json[propertyName] | defaultValue;
        setValue(newValue, variable, propertyGroup, propertyName);
    }

    static void setValue(JsonObject json, std::vector<const char*> &variable, const char *propertyGroup, const char *propertyName)
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
            sendMessage(propertyGroup, propertyName);
    }
    static void setValue(JsonObject json, std::vector<String> &variable, const char *propertyGroup, const char *propertyName)
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
            sendMessage(propertyGroup, propertyName);
    }

    static void setValue(bool newValue, bool &variable, const char *propertyGroup, const char *propertyName)
    {
        bool valueChanged = initialized && variable != newValue;
        LogHandler::debug(TagHandler::SettingsHandler, "Set bool '%s' oldValue '%ld' newValue '%ld' changed: '%ld'", propertyName, variable, newValue, valueChanged);
        variable = newValue;
        if (valueChanged)
            sendMessage(propertyGroup, propertyName);
    }
    static void setValue(const char *newValue, char *&variable, const char *propertyGroup, const char *propertyName)
    {
        bool valueChanged = initialized && strcmp(variable, newValue) != -1;
        LogHandler::debug(TagHandler::SettingsHandler, "Set char* '%s' oldValue '%s' newValue '%s' changed: '%ld'", propertyName, variable, newValue, valueChanged);
        strcpy(variable, newValue);
        if (valueChanged)
            sendMessage(propertyGroup, propertyName);
    }
    static void setValue(int newValue, int &variable, const char *propertyGroup, const char *propertyName)
    {
        bool valueChanged = initialized && variable != newValue;
        LogHandler::debug(TagHandler::SettingsHandler, "Set int '%s' oldValue '%ld' newValue '%ld' changed: '%ld'", propertyName, variable, newValue, valueChanged);
        variable = newValue;
        if (valueChanged)
            sendMessage(propertyGroup, propertyName);
    }
    static void setValue(float newValue, float &variable, const char *propertyGroup, const char *propertyName)
    {
        bool valueChanged = initialized && variable != newValue;
        LogHandler::debug(TagHandler::SettingsHandler, "Set float '%s' oldValue '%f' newValue '%f' changed: '%ld'", propertyName, variable, newValue, valueChanged);
        variable = newValue;
        if (valueChanged)
            sendMessage(propertyGroup, propertyName);
    }

    static u_int16_t calculateRange(const char *channel, int value)
    {
        return constrain(value, getChannelMin(channel), getChannelMax(channel));
    }

    static String TCodeVersionMapper(TCodeVersion version) 
    {
        switch (version)
        {
        case TCodeVersion::v0_2:
            return "TCode v0.2";
            break;
        case TCodeVersion::v0_3:
            return "TCode v0.3";
            break;
        case TCodeVersion::v0_5:
            return "TCode v0.5";
            break;
        default:
            return "TCode v?";
            break;
        }
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

    static void LogSaveDebug(DynamicJsonDocument doc)
    {
        LogHandler::debug(_TAG, "save TCodeVersionEnum: %i", doc["TCodeVersion"].as<int>());
        LogHandler::debug(_TAG, "save ssid: %s", doc["ssid"].as<String>());
        // LogHandler::debug(_TAG, "save pass: %s", doc["wifiPass"].as<String>());
        LogHandler::debug(_TAG, "save udpServerPort: %i", doc["udpServerPort"].as<int>());
        LogHandler::debug(_TAG, "save webServerPort: %i", doc["webServerPort"].as<int>());
        LogHandler::debug(_TAG, "save hostname: %s", doc["hostname"].as<String>());
        LogHandler::debug(_TAG, "save friendlyName: %s", doc["friendlyName"].as<String>());
        LogHandler::debug(_TAG, "save pitchFrequencyIsDifferent ", doc["pitchFrequencyIsDifferent"].as<bool>());
        LogHandler::debug(_TAG, "save msPerRad: %i", doc["msPerRad"].as<int>());
        LogHandler::debug(_TAG, "save servoFrequency: %i", doc["servoFrequency"].as<int>());
        LogHandler::debug(_TAG, "save  pitchFrequency: %i", doc["pitchFrequency"].as<int>());
        LogHandler::debug(_TAG, "save valveFrequency: %i", doc["valveFrequency"].as<int>());
        LogHandler::debug(_TAG, "save twistFrequency: %i", doc["twistFrequency"].as<int>());
        LogHandler::debug(_TAG, "save continuousTwist: %i", doc["continuousTwist"].as<bool>());
        LogHandler::debug(_TAG, "save feedbackTwist: %i", doc["feedbackTwist"].as<bool>());
        LogHandler::debug(_TAG, "save analogTwist: %i", doc["analogTwist"].as<bool>());
        LogHandler::debug(_TAG, "save TwistFeedBack_PIN: %i", doc["TwistFeedBack_PIN"].as<int>());
        LogHandler::debug(_TAG, "save RightServo_PIN: %i", doc["RightServo_PIN"].as<int>());
        LogHandler::debug(_TAG, "save LeftServo_PIN: %i", doc["LeftServo_PIN"].as<int>());
        LogHandler::debug(_TAG, "save RightUpperServo_PIN: %i", doc["RightUpperServo_PIN"].as<int>());
        LogHandler::debug(_TAG, "save LeftUpperServo_PIN: %i", doc["LeftUpperServo_PIN"].as<int>());
        LogHandler::debug(_TAG, "save PitchLeftServo_PIN: %i", doc["PitchLeftServo_PIN"].as<int>());
        LogHandler::debug(_TAG, "save PitchRightServo_PIN: %i", doc["PitchRightServo_PIN"].as<int>());
        LogHandler::debug(_TAG, "save ValveServo_PIN: %i", doc["ValveServo_PIN"].as<int>());
        LogHandler::debug(_TAG, "save TwistServo_PIN: %i", doc["TwistServo_PIN"].as<int>());
        LogHandler::debug(_TAG, "save Vibe0_PIN: %i", doc["Vibe0_PIN"].as<int>());
        LogHandler::debug(_TAG, "save Vibe1_PIN: %i", doc["Vibe1_PIN"].as<int>());
        LogHandler::debug(_TAG, "save Lube_Pin: %i", doc["Lube_Pin"].as<int>());
        LogHandler::debug(_TAG, "save LubeButton_PIN: %i", doc["LubeButton_PIN"].as<int>());
        LogHandler::debug(_TAG, "save staticIP: %i", doc["staticIP"].as<bool>());
        LogHandler::debug(_TAG, "save localIP: %s", doc["localIP"].as<String>());
        LogHandler::debug(_TAG, "save gateway: %s", doc["gateway"].as<String>());
        LogHandler::debug(_TAG, "save subnet: %s", doc["subnet"].as<String>());
        LogHandler::debug(_TAG, "save dns1: %s", doc["dns1"].as<String>());
        LogHandler::debug(_TAG, "save dns2: %s", doc["dns2"].as<String>());
        LogHandler::debug(_TAG, "save sr6Mode: %i", doc["sr6Mode"].as<bool>());
        LogHandler::debug(_TAG, "save RightServo_ZERO: %i", doc["RightServo_ZERO"].as<int>());
        LogHandler::debug(_TAG, "save LeftServo_ZERO: %i", doc["LeftServo_ZERO"].as<int>());
        LogHandler::debug(_TAG, "save RightUpperServo_ZERO: %i", doc["RightUpperServo_ZERO"].as<int>());
        LogHandler::debug(_TAG, "save LeftUpperServo_ZERO: %i", doc["LeftUpperServo_ZERO"].as<int>());
        LogHandler::debug(_TAG, "save PitchLeftServo_ZERO: %i", doc["PitchLeftServo_ZERO"].as<int>());
        LogHandler::debug(_TAG, "save PitchRightServo_ZERO: %i", doc["PitchRightServo_ZERO"].as<int>());
        LogHandler::debug(_TAG, "save TwistServo_ZERO: %i", doc["TwistServo_ZERO"].as<int>());
        LogHandler::debug(_TAG, "save ValveServo_ZERO: %i", doc["ValveServo_ZERO"].as<int>());
        LogHandler::debug(_TAG, "save autoValve: %i", doc["autoValve"].as<bool>());
        LogHandler::debug(_TAG, "save inverseValve: %i", doc["inverseValve"].as<bool>());
        LogHandler::debug(_TAG, "save valveServo90Degrees: %i", doc["valveServo90Degrees"].as<bool>());
        LogHandler::debug(_TAG, "save inverseStroke: %i", doc["inverseStroke"].as<bool>());
        LogHandler::debug(_TAG, "save inversePitch: %i", doc["inversePitch"].as<bool>());
        LogHandler::debug(_TAG, "save lubeEnabled: %i", doc["lubeEnabled"].as<bool>());
        LogHandler::debug(_TAG, "save lubeAmount: %i", doc["lubeAmount"].as<int>());
        LogHandler::debug(_TAG, "save Temp_PIN: %i", doc["Temp_PIN"].as<int>());
        LogHandler::debug(_TAG, "save Heater_PIN: %i", doc["Heater_PIN"].as<int>());
        LogHandler::debug(_TAG, "save displayEnabled: %i", doc["displayEnabled"].as<bool>());
        LogHandler::debug(_TAG, "save sleeveTempDisplayed: %i", doc["sleeveTempDisplayed"].as<bool>());
        LogHandler::debug(_TAG, "save internalTempDisplayed: %i", doc["internalTempDisplayed"].as<bool>());
        LogHandler::debug(_TAG, "save tempSleeveEnabled: %i", doc["tempSleeveEnabled"].as<bool>());
        LogHandler::debug(_TAG, "save tempInternalEnabled: %i", doc["tempInternalEnabled"].as<bool>());
        LogHandler::debug(_TAG, "save Display_Screen_Width: %i", doc["Display_Screen_Width"].as<int>());
        LogHandler::debug(_TAG, "save internalMaxTemp: %f", doc["internalMaxTemp"].as<double>());
        LogHandler::debug(_TAG, "save internalTempForFan: %f", doc["internalTempForFan"].as<double>());
        LogHandler::debug(_TAG, "save Display_Screen_Height: %i", doc["Display_Screen_Height"].as<int>());
        LogHandler::debug(_TAG, "save TargetTemp: %f", doc["TargetTemp"].as<double>());
        LogHandler::debug(_TAG, "save HeatPWM: %i", doc["HeatPWM"].as<int>());
        LogHandler::debug(_TAG, "save HoldPWM: %i", doc["HoldPWM"].as<int>());
        LogHandler::debug(_TAG, "save Display_I2C_Address: %i", doc["Display_I2C_Address"].as<int>());
        LogHandler::debug(_TAG, "save Display_Rst_PIN: %i", doc["Display_Rst_PIN"].as<int>());
        LogHandler::debug(_TAG, "save heaterFailsafeTime: %ld", doc["heaterFailsafeTime"].as<long>());
        LogHandler::debug(_TAG, "save heaterThreshold: %i", doc["heaterThreshold"].as<int>());
        LogHandler::debug(_TAG, "save heaterResolution: %i", doc["heaterResolution"].as<int>());
        LogHandler::debug(_TAG, "save heaterFrequency: %i", doc["heaterFrequency"].as<int>());
        LogHandler::debug(_TAG, "save newtoungeHatExists: %i", doc["newtoungeHatExists"].as<bool>());
        LogHandler::debug(_TAG, "save logLevel: %i", doc["logLevel"].as<int>());
        LogHandler::debug(_TAG, "save bluetoothEnabled: %i", doc["bluetoothEnabled"].as<bool>());
    }

    static void LogUpdateDebug()
    {
        // LogHandler::debug(_TAG, "update TCodeVersionEnum: %i", TCodeVersionEnum);
        // LogHandler::debug(_TAG, "update ssid: %s", ssid);
        // //LogHandler::debug(_TAG, "update wifiPass: %s", wifiPass);
        // LogHandler::debug(_TAG, "update udpServerPort: %i", udpServerPort);
        // LogHandler::debug(_TAG, "update webServerPort: %i", webServerPort);
        // LogHandler::debug(_TAG, "update hostname: %s", hostname);
        // LogHandler::debug(_TAG, "update friendlyName: %s", friendlyName);
        // LogHandler::debug(_TAG, "update pitchFrequencyIsDifferent: %i", pitchFrequencyIsDifferent);
        // LogHandler::debug(_TAG, "update msPerRad: %i", msPerRad);
        // LogHandler::debug(_TAG, "update servoFrequency: %i", servoFrequency);
        // LogHandler::debug(_TAG, "update pitchFrequency: %i", pitchFrequency);
        // LogHandler::debug(_TAG, "update valveFrequency: %i", valveFrequency);
        // LogHandler::debug(_TAG, "update twistFrequency: %i", twistFrequency);
        // LogHandler::debug(_TAG, "update continuousTwist: %i", continuousTwist);
        // LogHandler::debug(_TAG, "update feedbackTwist: %i", feedbackTwist);
        // LogHandler::debug(_TAG, "update analogTwist: %i", analogTwist);
        // LogHandler::debug(_TAG, "update TwistFeedBack_PIN: %i", TwistFeedBack_PIN);
        // LogHandler::debug(_TAG, "update RightServo_PIN: %i", RightServo_PIN);
        // LogHandler::debug(_TAG, "update LeftServo_PIN: %i", LeftServo_PIN);
        // LogHandler::debug(_TAG, "update RightUpperServo_PIN: %i", RightUpperServo_PIN);
        // LogHandler::debug(_TAG, "update LeftUpperServo_PIN: %i", LeftUpperServo_PIN);
        // LogHandler::debug(_TAG, "update PitchLeftServo_PIN: %i", PitchLeftServo_PIN);
        // LogHandler::debug(_TAG, "update PitchRightServo_PIN: %i", PitchRightServo_PIN);
        // LogHandler::debug(_TAG, "update ValveServo_PIN: %i", ValveServo_PIN);
        // LogHandler::debug(_TAG, "update TwistServo_PIN: %i", TwistServo_PIN);
        // LogHandler::debug(_TAG, "update Vibe0_PIN: %i", Vibe0_PIN);
        // LogHandler::debug(_TAG, "update Vibe1_PIN: %i", Vibe1_PIN);
        // LogHandler::debug(_TAG, "update LubeButton_PIN: %i", LubeButton_PIN);
        // LogHandler::debug(_TAG, "update staticIP: %i", staticIP);
        // LogHandler::debug(_TAG, "update localIP: %s", localIP);
        // LogHandler::debug(_TAG, "update gateway: %s", gateway);
        // LogHandler::debug(_TAG, "update subnet: %s", subnet);
        // LogHandler::debug(_TAG, "update dns1: %s", dns1);
        // LogHandler::debug(_TAG, "update dns2: %s", dns2);
        // LogHandler::debug(_TAG, "update sr6Mode: %i", sr6Mode);
        // LogHandler::debug(_TAG, "update RightServo_ZERO: %i", RightServo_ZERO);
        // LogHandler::debug(_TAG, "update LeftServo_ZERO: %i", LeftServo_ZERO);
        // LogHandler::debug(_TAG, "update RightUpperServo_ZERO: %i", RightUpperServo_ZERO);
        // LogHandler::debug(_TAG, "update LeftUpperServo_ZERO: %i", LeftUpperServo_ZERO);
        // LogHandler::debug(_TAG, "update PitchLeftServo_ZERO: %i", PitchLeftServo_ZERO);
        // LogHandler::debug(_TAG, "update PitchRightServo_ZERO: %i", PitchRightServo_ZERO);
        // LogHandler::debug(_TAG, "update TwistServo_ZERO: %i", TwistServo_ZERO);
        // LogHandler::debug(_TAG, "update ValveServo_ZERO: %i", ValveServo_ZERO);
        // LogHandler::debug(_TAG, "update autoValve: %i", autoValve);
        // LogHandler::debug(_TAG, "update inverseValve: %i", inverseValve);
        // LogHandler::debug(_TAG, "update valveServo90Degrees: %i", valveServo90Degrees);
        // LogHandler::debug(_TAG, "update inverseStroke: %i", inverseStroke);
        // LogHandler::debug(_TAG, "update inversePitch: %i", inversePitch);
        // LogHandler::debug(_TAG, "update lubeEnabled: %i", lubeEnabled);
        // LogHandler::debug(_TAG, "update lubeAmount: %i", lubeAmount);
        // LogHandler::debug(_TAG, "update displayEnabled: %i", displayEnabled);
        // LogHandler::debug(_TAG, "update sleeveTempDisplayed: %i", sleeveTempDisplayed);
        // LogHandler::debug(_TAG, "update internalTempDisplayed: %i", internalTempDisplayed);
        // LogHandler::debug(_TAG, "update tempSleeveEnabled: %i", tempSleeveEnabled);
        // LogHandler::debug(_TAG, "update tempInternalEnabled: %i", tempInternalEnabled);
        // LogHandler::debug(_TAG, "update Display_Screen_Width: %i", Display_Screen_Width);
        // LogHandler::debug(_TAG, "update Display_Screen_Height: %i", Display_Screen_Height);
        // LogHandler::debug(_TAG, "update TargetTemp: %i", TargetTemp);
        // LogHandler::debug(_TAG, "update HeatPWM: %i", HeatPWM);
        // LogHandler::debug(_TAG, "update HoldPWM: %i", HoldPWM);
        // LogHandler::debug(_TAG, "update Display_I2C_Address: %i", Display_I2C_Address);
        // LogHandler::debug(_TAG, "update Display_Rst_PIN: %i", Display_Rst_PIN);
        // LogHandler::debug(_TAG, "update Temp_PIN: %i", Temp_PIN);
        // LogHandler::debug(_TAG, "update Heater_PIN: %i", Heater_PIN);
        // LogHandler::debug(_TAG, "update heaterFailsafeTime: %ld", heaterFailsafeTime);
        // LogHandler::debug(_TAG, "update heaterThreshold: %d", heaterThreshold);
        // LogHandler::debug(_TAG, "update heaterResolution: %i", heaterResolution);
        // LogHandler::debug(_TAG, "update heaterFrequency: %i", heaterFrequency);
        // LogHandler::debug(_TAG, "update newtoungeHatExists: %i", newtoungeHatExists);
        // LogHandler::debug(_TAG, "update logLevel: %i", (int)logLevel);
        // LogHandler::debug(_TAG, "update bluetoothEnabled: %i", (int)bluetoothEnabled);
    }
};
bool SettingsHandler::initialized = false;
bool SettingsHandler::saving = false;
bool SettingsHandler::fullBuild = false;

BoardType SettingsHandler::boardType = BoardType::DEVKIT;
BuildFeature SettingsHandler::buildFeatures[featureCount];
const char *SettingsHandler::_TAG = TagHandler::SettingsHandler;
std::vector<int> SettingsHandler::systemI2CAddresses;
SETTING_STATE_FUNCTION_PTR_T SettingsHandler::message_callback = 0;
String SettingsHandler::TCodeVersionName;
TCodeVersion SettingsHandler::TCodeVersionEnum;
std::vector<Channel> SettingsHandler::currentChannels;
MotorType SettingsHandler::motorType = MotorType::Servo;
const char SettingsHandler::HandShakeChannel[4] = "D1\n";
const char SettingsHandler::SettingsChannel[4] = "D2\n";
const char *SettingsHandler::userSettingsDefaultFilePath = "/userSettingsDefault.json";
const char *SettingsHandler::userSettingsFilePath = "/userSettings.json";
const char *SettingsHandler::wifiPassFilePath = "/wifiInfo.json";
const char *SettingsHandler::logPath = "/log.json";
const char *SettingsHandler::defaultWifiPass = "YOUR PASSWORD HERE";
const char *SettingsHandler::decoyPass = "Too bad haxor!";
bool SettingsHandler::bluetoothEnabled = true;
LogLevel SettingsHandler::logLevel = LogLevel::INFO;
bool SettingsHandler::isTcp = true;
char SettingsHandler::ssid[32];
char SettingsHandler::wifiPass[63];
char SettingsHandler::hostname[63];
char SettingsHandler::friendlyName[100];
int SettingsHandler::udpServerPort;
int SettingsHandler::webServerPort;
int SettingsHandler::PitchRightServo_PIN;
int SettingsHandler::RightUpperServo_PIN;
int SettingsHandler::RightServo_PIN;
int SettingsHandler::PitchLeftServo_PIN;
int SettingsHandler::LeftUpperServo_PIN;
int SettingsHandler::LeftServo_PIN;
int SettingsHandler::ValveServo_PIN;
int SettingsHandler::TwistServo_PIN;
int SettingsHandler::TwistFeedBack_PIN;
int SettingsHandler::Vibe0_PIN;
int SettingsHandler::Vibe1_PIN;
int SettingsHandler::LubeButton_PIN;
int SettingsHandler::Sleeve_Temp_PIN;
int SettingsHandler::Heater_PIN;
int SettingsHandler::I2C_SDA_PIN = 21;
int SettingsHandler::I2C_SCL_PIN = 22;

int SettingsHandler::Internal_Temp_PIN;
int SettingsHandler::Case_Fan_PIN;
int SettingsHandler::Squeeze_PIN;
int SettingsHandler::Vibe2_PIN;
int SettingsHandler::Vibe3_PIN;

// int SettingsHandler::HeatLED_PIN = 32;
//  pin 25 cannot be servo. Throws error
bool SettingsHandler::lubeEnabled = true;

bool SettingsHandler::pitchFrequencyIsDifferent;
int SettingsHandler::msPerRad;
int SettingsHandler::servoFrequency;
int SettingsHandler::pitchFrequency;
int SettingsHandler::valveFrequency;
int SettingsHandler::twistFrequency;
int SettingsHandler::squeezeFrequency;
bool SettingsHandler::feedbackTwist = false;
bool SettingsHandler::continuousTwist;
bool SettingsHandler::analogTwist;
bool SettingsHandler::staticIP;
char SettingsHandler::localIP[15];
char SettingsHandler::gateway[15];
char SettingsHandler::subnet[15];
char SettingsHandler::dns1[15];
char SettingsHandler::dns2[15];
bool SettingsHandler::sr6Mode;
int SettingsHandler::RightServo_ZERO = 1500;
int SettingsHandler::LeftServo_ZERO = 1500;
int SettingsHandler::RightUpperServo_ZERO = 1500;
int SettingsHandler::LeftUpperServo_ZERO = 1500;
int SettingsHandler::PitchLeftServo_ZERO = 1500;
int SettingsHandler::PitchRightServo_ZERO = 1500;

bool SettingsHandler::BLDC_UsePWM = false;
bool SettingsHandler::BLDC_UseMT6701 = true;
bool SettingsHandler::BLDC_UseHallSensor = false;
int SettingsHandler::BLDC_Pulley_Circumference = 60;
int SettingsHandler::BLDC_Encoder_PIN = 33;// PWM feedback pin (if used) - P pad on AS5048a
int SettingsHandler::BLDC_Enable_PIN = 14;// Motor enable - EN on SFOCMini   
int SettingsHandler::BLDC_HallEffect_PIN = 12;
int SettingsHandler::BLDC_PWMchannel1_PIN = 27;
int SettingsHandler::BLDC_PWMchannel2_PIN = 26;
int SettingsHandler::BLDC_PWMchannel3_PIN = 25;
float SettingsHandler::BLDC_MotorA_Voltage = 20.0; // BLDC Motor operating voltage (12-20V)
float SettingsHandler::BLDC_MotorA_Current = 1.0;  // BLDC Maximum operating current (Amps)
int SettingsHandler::BLDC_ChipSelect_PIN = 5;         // SPI chip select pin - CSn on AS5048a (By default on ESP32: MISO = D19, MOSI = D23, CLK = D18)
bool SettingsHandler::BLDC_MotorA_ParametersKnown = true;     // Once you know the zero elec angle for the motor enter it below and set this flag to true.
float SettingsHandler::BLDC_MotorA_ZeroElecAngle = 0.00; // This number is the zero angle (in radians) for the motor relative to the encoder.
int SettingsHandler::BLDC_RailLength;
int SettingsHandler::BLDC_StrokeLength;

int SettingsHandler::TwistServo_ZERO = 1500;
int SettingsHandler::ValveServo_ZERO = 1500;
int SettingsHandler::SqueezeServo_ZERO = 1500;
bool SettingsHandler::autoValve = false;
bool SettingsHandler::inverseValve = false;
bool SettingsHandler::valveServo90Degrees = false;
bool SettingsHandler::inverseStroke = false;
bool SettingsHandler::inversePitch = false;
int SettingsHandler::lubeAmount = 255;

bool SettingsHandler::displayEnabled = false;
bool SettingsHandler::sleeveTempDisplayed = false;
bool SettingsHandler::internalTempDisplayed = false;
bool SettingsHandler::versionDisplayed = true;
bool SettingsHandler::tempSleeveEnabled = false;
bool SettingsHandler::tempInternalEnabled = false;
bool SettingsHandler::fanControlEnabled = false;
bool SettingsHandler::batteryLevelEnabled = true;
int SettingsHandler::Battery_Voltage_PIN = 32;
bool SettingsHandler::batteryLevelNumeric = false;
double SettingsHandler::batteryVoltageMax = 12.6;
int SettingsHandler::batteryCapacityMax;
int SettingsHandler::Display_Screen_Width = 128;
int SettingsHandler::Display_Screen_Height = 64;
int SettingsHandler::caseFanMaxDuty = 255;
double SettingsHandler::internalTempForFan = 20.0;
double SettingsHandler::internalMaxTemp = 50.0;
int SettingsHandler::TargetTemp = 40;
int SettingsHandler::HeatPWM = 255;
int SettingsHandler::HoldPWM = 110;
int SettingsHandler::Display_I2C_Address = 0; // = 0x3C;
int SettingsHandler::Display_Rst_PIN = -1;
// long SettingsHandler::heaterFailsafeTime = 60000;
float SettingsHandler::heaterThreshold = 5.0;
int SettingsHandler::heaterResolution = 8;
int SettingsHandler::heaterFrequency = 5000;
int SettingsHandler::caseFanFrequency = 25;
int SettingsHandler::caseFanResolution = 10;
const char *SettingsHandler::lastRebootReason;

bool SettingsHandler::voiceEnabled = false;
bool SettingsHandler::voiceMuted = false;
int SettingsHandler::voiceWakeTime = 10;
int SettingsHandler::voiceVolume = 10;


bool SettingsHandler::motionEnabled = false;
//char SettingsHandler::motionSelectedProfileName[maxMotionProfileNameLength];
int SettingsHandler::motionSelectedProfileIndex = 0;
int SettingsHandler::motionDefaultProfileIndex = 0;
MotionProfile SettingsHandler::motionProfiles[maxMotionProfileCount];
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