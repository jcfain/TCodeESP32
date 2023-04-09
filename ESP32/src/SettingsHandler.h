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
#include "LogHandler.h"
#include "utils.h"
#include "TagHandler.h"

#define featureCount 7

enum class TCodeVersion {
    v0_2,
    v0_3,
    v0_5
};

enum class BuildFeature {
    NONE,
    DEBUG,
    WIFI,
    BLUETOOTH,
    DA,
    DISPLAY_,
    TEMP,
    HAS_TCODE_V2
};

enum class BoardType {
    DEVKIT,
    CRIMZZON,
    ISAAC
};

enum class MotorType {
    Servo,
    BLDC
};

class SettingsHandler 
{
  public:

        static bool saving;
        static bool fullBuild;
        static bool debug;
        static LogLevel logLevel;

        static BoardType boardType;
        static BuildFeature buildFeatures[featureCount];
        static String TCodeVersionName;
        static TCodeVersion TCodeVersionEnum;
        static MotorType motorType;
        const static char ESP32Version[8];
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
		static int TwistFeedBack_PIN;// Vibe 4?
		static int RightServo_PIN;
		static int LeftServo_PIN;
		static int RightUpperServo_PIN;
		static int LeftUpperServo_PIN;
		static int PitchLeftServo_PIN;
		static int PitchRightServo_PIN;
		static int ValveServo_PIN;
		static int TwistServo_PIN;
        
        static int BLDC_Encoder_PIN;
        static int BLDC_Enable_PIN;
        static int BLDC_PWMchannel1_PIN;
        static int BLDC_PWMchannel2_PIN;
        static int BLDC_PWMchannel3_PIN;
        static float BLDC_MotorA_Voltage;         // BLDC Motor operating voltage (12-20V)
        static float BLDC_MotorA_Current;       // BLDC Maximum operating current (Amps)
        
		static int Vibe0_PIN;
		static int Vibe1_PIN;
        static int Vibe2_PIN;
        static int Vibe3_PIN;
        static int LubeButton_PIN;
        static int Squeeze_PIN;
        static int StrokeMin;
        static int StrokeMax;
        static int RollMin;
        static int RollMax;
        static int PitchMin;
        static int PitchMax;
        static int SwayMin;
        static int SwayMax;
        static int SurgeMin;
        static int SurgeMax;
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
        static double internalMaxTemp; // C
		static int Heater_PIN;
		static int HeatLED_PIN;
		static int I2C_SDA_PIN;
		static int I2C_SCL_PIN;
		static int TargetTemp;// Desired Temp in degC
		static int HeatPWM;// Heating PWM setting 0-255
		static int HoldPWM;// Hold heat PWM setting 0-255
		static int Display_I2C_Address;
		static int Display_Rst_PIN;
        //static long heaterFailsafeTime;
        static float heaterThreshold;
        static int heaterResolution;
        static int heaterFrequency;
        static int caseFanFrequency;
        static int caseFanResolution;

        static int strokerSamples;
        static int strokerOffset;
        static int strokerAmplitude;

        static const char* lastRebootReason;

        static const char* userSettingsFilePath;
        static const char* logPath;
        static const char* defaultWifiPass;
        static const char* decoyPass;

        static void load() 
        {
            DynamicJsonDocument doc(deserializeSize);
			File file;
			bool loadingDefault = false;
			if(!SPIFFS.exists(userSettingsFilePath)) 
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
            if (error) {
                LogHandler::error(_TAG, "Error deserializing settings json: %s", file.name());
                LogHandler::error(_TAG, "Code: ");
                switch(error.code()) {
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
            JsonObject jsonObj = doc.as<JsonObject>();
            
            #if CRIMZZON_BUILD || ISAAC_NEWTONGUE_BUILD
                TCodeVersionEnum = TCodeVersion::v0_3;
                TCodeVersionName = TCodeVersionMapper(TCodeVersionEnum);
	        #endif

            update(jsonObj);
            
            setBoardType();
            setBuildFeatures();
            setMotorType();

			if(loadingDefault)
				save();
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

        static bool update(JsonObject json) 
        {
            LogHandler::info(_TAG, "Load settings");
            if(json.size() > 0) 
            {
                logLevel = (LogLevel)(json["logLevel"] | 2);
                LogHandler::setLogLevel(logLevel);

                // std::vector<const char*> tags;
                // //tags.push_back(TagHandler::DisplayHandler);
                // tags.push_back(TagHandler::BLDCHandler);
                // tags.push_back(TagHandler::ServoHandler3);
                // tags.push_back(TagHandler::TCodeHandler);
                // tags.push_back(TagHandler::Main);
                // LogHandler::setTags(tags);

                JsonArray includes = json["log-include-tags"].as<JsonArray>();
                std::vector<String> includesVec;
                for(int i = 0; i < includes.size(); i++) {
                    includesVec.push_back(includes[i]);
                }
                LogHandler::setIncludes(includesVec);

                JsonArray excludes = json["log-exclude-tags"].as<JsonArray>();
                std::vector<String> excludesVec;
                for(int i = 0; i < excludes.size(); i++) {
                    excludesVec.push_back(excludes[i]);
                }
                LogHandler::setExcludes(excludesVec);

                const char* ssidConst = json["ssid"];
                if( ssid != nullptr) 
                {
                    strcpy(ssid, ssidConst);
                }
                const char* wifiPassConst = json["wifiPass"];
                if( wifiPassConst != nullptr && strcmp(wifiPassConst, SettingsHandler::decoyPass) != 0) 
                {
                    strcpy(wifiPass, wifiPassConst);
                }
	            #if !CRIMZZON_BUILD
                    TCodeVersionEnum = (TCodeVersion)(json["TCodeVersion"] | 2);
                    TCodeVersionName = TCodeVersionMapper(TCodeVersionEnum);
	            #endif
                udpServerPort = json["udpServerPort"] | 8000;
                webServerPort = json["webServerPort"] | 80;
                const char* hostnameTemp = json["hostname"];
                if (hostnameTemp != nullptr)
                    strcpy(hostname, hostnameTemp);
                const char* friendlyNameTemp = json["friendlyName"];
                if (friendlyNameTemp != nullptr)
                    strcpy(friendlyName, friendlyNameTemp);
					
				bluetoothEnabled =  json["bluetoothEnabled"];
                StrokeMin = 1;
                StrokeMax = SettingsHandler::TCodeVersionEnum >= TCodeVersion::v0_3 ? 9999 : 999;
                RollMin = 1;
                RollMax = SettingsHandler::TCodeVersionEnum >= TCodeVersion::v0_3 ? 9999 : 999;
                PitchMin = 1;
                PitchMax = SettingsHandler::TCodeVersionEnum >= TCodeVersion::v0_3 ? 9999 : 999;

//Servo motors//////////////////////////////////////////////////////////////////////////////////
                pitchFrequencyIsDifferent = json["pitchFrequencyIsDifferent"];
                msPerRad =  json["msPerRad"] | 637;
                servoFrequency = json["servoFrequency"] | 50;
                pitchFrequency = json[pitchFrequencyIsDifferent ? "pitchFrequency" : "servoFrequency"] | servoFrequency;
                sr6Mode = json["sr6Mode"];

                RightServo_ZERO = json["RightServo_ZERO"] | 1500;
                LeftServo_ZERO = json["LeftServo_ZERO"] | 1500;
                RightUpperServo_ZERO = json["RightUpperServo_ZERO"] | 1500;
                LeftUpperServo_ZERO = json["LeftUpperServo_ZERO"] | 1500;
                PitchLeftServo_ZERO = json["PitchLeftServo_ZERO"] | 1500;
                PitchRightServo_ZERO = json["PitchRightServo_ZERO"] | 1500;
                #if ISAAC_NEWTONGUE_BUILD
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
                    TwistFeedBack_PIN = json["TwistFeedBack_PIN"] | 33;
                    RightServo_PIN = json["RightServo_PIN"] | 2;
                    LeftServo_PIN = json["LeftServo_PIN"] | 13;
                    RightUpperServo_PIN = json["RightUpperServo_PIN"] | 4;
                    LeftUpperServo_PIN = json["LeftUpperServo_PIN"] | 12;
                    PitchLeftServo_PIN = json["PitchLeftServo_PIN"] | 14;
                    PitchRightServo_PIN = json["PitchRightServo_PIN"] | 17;
                    ValveServo_PIN = json["ValveServo_PIN"] | 5;
                    TwistServo_PIN = json["TwistServo_PIN"] | 27;
    //Common motor
                    Squeeze_PIN = json["Squeeze_PIN"] | 26;
                    LubeButton_PIN = json["LubeButton_PIN"] | 36;
                    // Internal_Temp_PIN = json["Internal_Temp_PIN"] | 34;
                    Sleeve_Temp_PIN = json["Temp_PIN"] | 25;
                    // Case_Fan_PIN = json["Case_Fan_PIN"] | 16;
                    Vibe0_PIN = json["Vibe0_PIN"] | 15;
                    Vibe1_PIN = json["Vibe1_PIN"] | 16;
                    // Vibe2_PIN = json["Vibe2_PIN"] | 23;
                    // Vibe3_PIN = json["Vibe3_PIN"] | 32;
                    Heater_PIN = json["Heater_PIN"] | 19;
                    Battery_Voltage_PIN = json["Battery_Voltage_PIN"] | 39;
                #elif CRIMZZON_BUILD

                    Vibe3_PIN = json["Vibe3_PIN"] | 26;
                    Internal_Temp_PIN = json["Internal_Temp_PIN"] | 32;

                    //EXT
                    // EXT_Input2_PIN = 34;
                    // EXT_Input3_PIN = 39;
                    // EXT_Input4_PIN = 36;

                    heaterResolution = json["heaterResolution"] | 8;
                    caseFanResolution = json["caseFanResolution"] | 10;
                    caseFanFrequency = json["caseFanFrequency"] | 25;
                    Display_Screen_Height = json["Display_Screen_Height"] | 32;
                    TwistFeedBack_PIN = json["TwistFeedBack_PIN"] | 0;
                #endif
                #if !ISAAC_NEWTONGUE_BUILD
                    #if !CRIMZZON_BUILD
                        TwistFeedBack_PIN = json["TwistFeedBack_PIN"] | 26;
                    #endif
                    RightServo_PIN = json["RightServo_PIN"] | 13;
                    LeftServo_PIN = json["LeftServo_PIN"] | 15;
                    RightUpperServo_PIN = json["RightUpperServo_PIN"] | 12;
                    LeftUpperServo_PIN = json["LeftUpperServo_PIN"] | 2;
                    PitchLeftServo_PIN = json["PitchLeftServo_PIN"] | 4;
                    PitchRightServo_PIN = json["PitchRightServo_PIN"] | 14;
    //Common motor
                    Squeeze_PIN = json["Squeeze_PIN"] | 17;
                    LubeButton_PIN = json["LubeButton_PIN"] | 35;
                    #if !CRIMZZON_BUILD
                        Internal_Temp_PIN = json["Internal_Temp_PIN"] | 34;
                    #endif
                    Sleeve_Temp_PIN = json["Temp_PIN"] | 5;
                    Case_Fan_PIN = json["Case_Fan_PIN"] | 16;
                    Vibe0_PIN = json["Vibe0_PIN"] | 18;
                    Vibe1_PIN = json["Vibe1_PIN"] | 19;
                    Vibe2_PIN = json["Vibe2_PIN"] | 23;
                    #if !CRIMZZON_BUILD
                        Vibe3_PIN = json["Vibe3_PIN"] | 32;
                    #endif
                    #if MOTOR_TYPE == 0
                        Heater_PIN = json["Heater_PIN"] | 33;
                        ValveServo_PIN = json["ValveServo_PIN"] | 25;
                        TwistServo_PIN = json["TwistServo_PIN"] | 27;
                    #elif MOTOR_TYPE == 1
                        Heater_PIN = json["Heater_PIN"] | 15;
                        ValveServo_PIN = json["ValveServo_PIN"] | 12;
                        TwistServo_PIN = json["TwistServo_PIN"] | 13;
                    #endif
                    Battery_Voltage_PIN = json["Battery_Voltage_PIN"] | 39;
                #endif
                twistFrequency = json["twistFrequency"] | 50;
                squeezeFrequency = json["squeezeFrequency"] | 50;
                valveFrequency = json["valveFrequency"] | 50;
                continuousTwist = json["continuousTwist"];
                feedbackTwist =  json["feedbackTwist"];
                analogTwist = json["analogTwist"];
                TwistServo_ZERO = json["TwistServo_ZERO"] | 1500;
                ValveServo_ZERO = json["ValveServo_ZERO"] | 1500;
                SqueezeServo_ZERO = json["Squeeze_ZERO"] | 1500;
// BLDC motor
                BLDC_Encoder_PIN = json["BLDC_Encoder_PIN"] | 33;
                BLDC_Enable_PIN = json["BLDC_Enable_PIN"] | 14;
                BLDC_PWMchannel1_PIN = json["BLDC_PWMchannel1_PIN"] | 27;
                BLDC_PWMchannel2_PIN = json["BLDC_PWMchannel2_PIN"] | 26;
                BLDC_PWMchannel3_PIN = json["BLDC_PWMchannel3_PIN"] | 25;
                BLDC_MotorA_Voltage = round2(json["BLDC_MotorA_Voltage"] | 20.0);
                BLDC_MotorA_Current = round2(json["BLDC_MotorA_Current"] | 1.5);
/////////////////////////////////////////////////////////////////////////////////////////////////


                staticIP = json["staticIP"];
                const char* localIPTemp = json["localIP"];
                if (localIPTemp != nullptr)
                    strcpy(localIP, localIPTemp);
                const char* gatewayTemp = json["gateway"];
                if (gatewayTemp != nullptr)
                    strcpy(gateway, gatewayTemp);
                const char* subnetTemp = json["subnet"];
                if (subnetTemp != nullptr)
                    strcpy(subnet, subnetTemp);
                const char* dns1Temp = json["dns1"];
                if (dns1Temp != nullptr)
                    strcpy(dns1, dns1Temp);
                const char* dns2Temp = json["dns2"];
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
                #if !CRIMZZON_BUILD
                    Display_Screen_Height = json["Display_Screen_Height"] | 64;
                #endif
                const char* Display_I2C_AddressTemp = json["Display_I2C_Address"];
                if (Display_I2C_AddressTemp != nullptr)
                    Display_I2C_Address = (int)strtol(Display_I2C_AddressTemp, NULL, 0);
                Display_Rst_PIN = json["Display_Rst_PIN"] | -1;
                
                tempSleeveEnabled = json["tempSleeveEnabled"];
                heaterThreshold = json["heaterThreshold"] | 5.0;
                heaterFrequency = json["heaterFrequency"] | 5000;
                #if !CRIMZZON_BUILD
                    heaterResolution = json["heaterResolution"] | 8;
                #endif
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
                batteryCapacityMax = json["batteryCapacityMax"] | 0;// Let batteryHandler set the default
                
                #if !CRIMZZON_BUILD
                    caseFanFrequency = json["caseFanFrequency"] | 25;
                    caseFanResolution = json["caseFanResolution"] | 10;
                #endif
                caseFanMaxDuty = pow(2, caseFanResolution) - 1;
                
                lubeEnabled = json["lubeEnabled"];
                lastRebootReason = machine_reset_cause();
                LogHandler::info(_TAG, "Last reset reason: %s", SettingsHandler::lastRebootReason);
                LogUpdateDebug();
                return true;
            } 
            return false;
        }

        //Untested
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

        static void serialize(char buf[3072])
        {
            LogHandler::debug(_TAG, "Get settings...");
            File file = SPIFFS.open(userSettingsFilePath, "r");
            DynamicJsonDocument doc(deserializeSize);
            DeserializationError error = deserializeJson(doc, file);
            if (error) {
                LogHandler::error("toJson: Error deserializing settings json: %s", file.name());
                buf[0] = {0};
                return;
            }
            file.close();
            if(strcmp(doc["wifiPass"], defaultWifiPass) != 0 )
                doc["wifiPass"] = "Too bad haxor!";// Do not send password if its not default
                
            String output;
            serializeJson(doc, output);
            //serializeJson(doc, Serial);
            if(LogHandler::getLogLevel() == LogLevel::VERBOSE) {
                Serial.printf("Settings: %s\n", output.c_str());
                Serial.printf("Size: %ld\n", strlen(output.c_str()));
            }
            //LogHandler::verbose(_TAG, "Output: %s", output.c_str());
            buf[0]  = {0};
            strcpy(buf, output.c_str());
        }

        static void getSystemInfo(char buf[1024]) {
            DynamicJsonDocument doc(1024);
            
            doc["esp32Version"] = ESP32Version;
            doc["TCodeVersion"] = (int)TCodeVersionEnum;
            doc["lastRebootReason"] = lastRebootReason;
            doc["boardType"] = (int)boardType;
            doc["motorType"] = (int)motorType;
            JsonArray buildFeaturesJsonArray = doc.createNestedArray("buildFeatures");
            for (BuildFeature value : buildFeatures) {
                buildFeaturesJsonArray.add((int)value);
            }

            JsonArray availableTagsJsonArray = doc.createNestedArray("availableTags");
            for (const char* tag : TagHandler::AvailableTags) {
                availableTagsJsonArray.add(tag);
            }
            
            doc["localIP"] = localIP;
            doc["gateway"] = gateway;
            doc["subnet"] = subnet;
            doc["dns1"] = dns1;
            doc["dns2"] = dns2;

            doc["chipModel"] = ESP.getChipModel();
            doc["chipRevision"] = ESP.getChipRevision();
            doc["chipCores"] = ESP.getChipCores();
            uint32_t chipId = 0;
            for(int i=0; i<17; i=i+8) {
                chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
            }
            doc["chipID"] = chipId;

            String output;
            serializeJson(doc, output);
            if(LogHandler::getLogLevel() == LogLevel::VERBOSE)
                Serial.printf("SystemInfo: %s\n", output.c_str());
            buf[0]  = {0};
            strcpy(buf, output.c_str());
        }

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
            if(!update(doc.as<JsonObject>())) {
                LogHandler::error(_TAG, "Settings save: update error");
                return false;
            }
            if(!save()) {
                LogHandler::error(_TAG, "Settings save: save error");
                return false;
            }
			return true;
        }

        static bool save() 
        {
            LogHandler::info(_TAG, "Save settings");
            saving = true;
            // Delete existing file, otherwise the configuration is appended to the file
            // Serial.print("SPIFFS used: ");
            // Serial.println(SPIFFS.usedBytes() + "/" + SPIFFS.totalBytes());
            if(!SPIFFS.remove(userSettingsFilePath)) 
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

            doc["logLevel"] = (int)logLevel;
            LogHandler::setLogLevel(logLevel);
            // Serial.println("logLevel: ");
            // Serial.println((int)logLevel);
            // Serial.println( doc["logLevel"] .as<int>());
            // Serial.println((int)doc["logLevel"]);

            //std::vector<const char*> tags;
            // tags.push_back(TagHandler::DisplayHandler);
            // tags.push_back(TagHandler::BLDCHandler);
            // tags.push_back(TagHandler::ServoHandler3);
            //LogHandler::setTags(tags);

            if(!tempInternalEnabled) {
                internalTempDisplayed = false;
                fanControlEnabled = false;
            }
            if(!tempSleeveEnabled) {
                sleeveTempDisplayed = false;
            }
            doc["fullBuild"] = fullBuild;
            doc["esp32Version"] = ESP32Version;
            doc["TCodeVersion"] = (int)TCodeVersionEnum;
            doc["ssid"] = ssid;
            doc["wifiPass"] = wifiPass;
            doc["udpServerPort"] = udpServerPort;
            doc["webServerPort"] = webServerPort;
            doc["hostname"] =  hostname;
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

            doc["BLDC_Encoder_PIN"] = BLDC_Encoder_PIN;
            doc["BLDC_Enable_PIN"] = BLDC_Enable_PIN;
            doc["BLDC_PWMchannel1_PIN"] = BLDC_PWMchannel1_PIN;
            doc["BLDC_PWMchannel2_PIN"] = BLDC_PWMchannel2_PIN;
            doc["BLDC_PWMchannel3_PIN"] = BLDC_PWMchannel3_PIN;
            doc["BLDC_MotorA_Voltage"] = round2(BLDC_MotorA_Voltage);
            doc["BLDC_MotorA_Current"] = round2(BLDC_MotorA_Current);

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
            //doc["heaterFailsafeTime"] = String(heaterFailsafeTime);
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
            
            JsonArray includes = doc.createNestedArray("log-include-tags");
            std::vector<String> includesVec = LogHandler::getIncludes();
            for(int i = 0; i < includesVec.size(); i++) {
                includes.add(includesVec[i]);
            }
            
            JsonArray excludes = doc.createNestedArray("log-exclude-tags");
            std::vector<String> excludesVec = LogHandler::getExcludes();
            for(int i = 0; i < excludesVec.size(); i++) {
                excludes.add(excludesVec[i]);
            }


            LogHandler::debug(_TAG, "Is overflowed: %u", doc.overflowed());
            if(doc.overflowed()) {
                LogHandler::error(_TAG, "Document is overflowed! Increase serialize size.");
                return false;
            }
            
            LogHandler::debug(_TAG, "isNull: %u", doc.isNull());
            if(doc.isNull()) {
                LogHandler::error(_TAG, "Document is null!");
                return false;
            }

            LogHandler::debug(_TAG, "Memory usage: %u bytes", doc.memoryUsage());
            if(doc.memoryUsage() == 0) {
                LogHandler::error(_TAG, "Document is empty!");
                return false;
            }
			//Stopped working for some reason...
            //Printing all defaults
            //LogSaveDebug(doc);

            if (serializeJson(doc, file) == 0) 
            {
                LogHandler::error(_TAG, "Failed to write to file");
                doc["wifiPass"] = "";
                return false;
            }
            LogHandler::debug(_TAG, "File contents: %s", file.readString().c_str());
            printMemory();
            file.close(); // Task exception here could mean not enough space on SPIFFS.
            
            doc["wifiPass"] = "";
            saving = false;
            return true;
        }

        static void printMemory() {
            LogHandler::debug(_TAG, "Free heap: %u", ESP.getFreeHeap());
            LogHandler::debug(_TAG, "Total heap: %u", ESP.getHeapSize());
            LogHandler::debug(_TAG, "Free psram: %u", ESP.getFreePsram());
            LogHandler::debug(_TAG, "Total Psram: %u", ESP.getPsramSize());
            LogHandler::debug(_TAG, "SPIFFS used: %i", SPIFFS.usedBytes());
            LogHandler::debug(_TAG, "SPIFFS total: %i",SPIFFS.totalBytes());
        }

        static void setBuildFeatures() {
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
                LogHandler::setLogLevel(LogLevel::VERBOSE);
		        debug = true;
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
            buildFeatures[featureCount - 1] = {};
        }

        static void setBoardType() {
            #if ISAAC_NEWTONGUE_BUILD
                LogHandler::debug("setBoardType", "ISAAC_NEWTONGUE_BUILD");
                boardType = BoardType::ISAAC;
            #elif CRIMZZON_BUILD
                LogHandler::debug("setBoardType", "CRIMZZON_BUILD");
                boardType = BoardType::CRIMZZON;
            #else
                LogHandler::debug("setBoardType", "DEVKIT_BUILD");
                boardType = BoardType::DEVKIT;
            #endif
        }

        static void setMotorType() {
            #if MOTOR_TYPE == 0
                motorType = MotorType::Servo;
            #elif MOTOR_TYPE == 1
                motorType = MotorType::BLDC;
            #endif
        }

        static void processTCodeJson(char* outbuf, char* tcodeJson) 
        {
            StaticJsonDocument<512> doc;
            DeserializationError error = deserializeJson(doc, tcodeJson);
            if (error) {
                LogHandler::error(_TAG, "Failed to read udp jsonobject, using default configuration");
                outbuf[0] = {0};
                return;
            }
            JsonArray arr = doc.as<JsonArray>();
            char buffer[255] = "";
            for(JsonObject repo: arr) 
            { 
                const char* channel = repo["c"];
                int value = repo["v"];
                if(channel != nullptr && value > 0) 
                {
                    if(buffer[0] == '\0') 
                    {
                        //Serial.println("tcode empty");
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
                    //pad(integer_string);
                    //sprintf(integer_string, "%d", SettingsHandler::calculateRange(channel, value));
                    //Serial.print("integer_string");
                    //Serial.println(integer_string);
                    strcat (buffer, integer_string);
                    int speed = repo["s"];
                    int interval = repo["i"];
                    if (interval > 0) {
                        char interval_string[5];
                        sprintf(interval_string, "%d", interval);
                        strcat (buffer, "I");
                        strcat (buffer, interval_string);
                    } else if (speed > 0) {
                        char speed_string[5];
                        sprintf(speed_string, "%d", speed);
                        strcat (buffer, "S");
                        strcat (buffer, speed_string);
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
        
    private:
        static const char* _TAG;
        static const char* userSettingsDefaultFilePath;
        // Use http://arduinojson.org/assistant to compute the capacity.
        // static const size_t readCapacity = JSON_OBJECT_SIZE(100) + 2000;
        // static const size_t saveCapacity = JSON_OBJECT_SIZE(100);
		static const int deserializeSize = 6144;
		static const int serializeSize = 3072;
        //3072

        static int calculateRange(const char* channel, int value) 
        {
            return constrain(value, getchannelMin(channel), getchannelMax(channel));
        }

        //TODO: Need to think this out better.
        static int getchannelMin(const char* channel) 
        {
            if (strcmp(channel, "L0") == 0) 
            {
                return StrokeMin;
            } 
            else if (strcmp(channel, "R1") == 0) 
            {
                return RollMin;
            } 
            else if (strcmp(channel, "R2") == 0) 
            {
                return PitchMin;
            }
            return 1;
        }

        static int getchannelMax(const char* channel) 
        {
            if (strcmp(channel, "L0") == 0) 
            {
                return StrokeMax;
            } 
            else if (strcmp(channel, "R1") == 0) 
            {
                return RollMax;
            } 
            else if (strcmp(channel, "R2") == 0) 
            {
                return PitchMax;
            }
            return 9999;
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
        static unsigned long getTime() {
            time_t now;
            struct tm timeinfo;
            if (!getLocalTime(&timeinfo)) {
                //Serial.println("Failed to obtain time");
                return(0);
            }
            time(&now);
            return now;
        }

        static const char* machine_reset_cause() {
            switch (esp_reset_reason()) {
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
            LogHandler::debug(_TAG, "save udpServerPort: %i", doc["udpServerPort"].as<int>());
            LogHandler::debug(_TAG, "save webServerPort: %i", doc["webServerPort"].as<int>());
            LogHandler::debug(_TAG, "save hostname: %s", doc["hostname"].as<String>());
            LogHandler::debug(_TAG, "save friendlyName: %s", doc["friendlyName"].as<String>());
            LogHandler::debug(_TAG, "save pitchFrequencyIsDifferent ", doc["pitchFrequencyIsDifferent"].as<bool>());
            LogHandler::debug(_TAG, "save msPerRad: %i", doc["msPerRad"].as<int>());
            LogHandler::debug(_TAG, "save servoFrequency: %i", doc["servoFrequency"].as<int>());
            LogHandler::debug(_TAG, "save  pitchFrequency: %i", doc["pitchFrequency"].as<int>());
            LogHandler::debug(_TAG, "save valveFrequency: %i",doc["valveFrequency"].as<int>());
            LogHandler::debug(_TAG, "save twistFrequency: %i", doc["twistFrequency"].as<int>());
            LogHandler::debug(_TAG, "save continuousTwist: %i", doc["continuousTwist"].as<bool>());
            LogHandler::debug(_TAG, "save feedbackTwist: %i", doc["feedbackTwist"].as<bool>());
            LogHandler::debug(_TAG, "save analogTwist: %i", doc["analogTwist"].as<bool>());
            LogHandler::debug(_TAG, "save TwistFeedBack_PIN: %i", doc["TwistFeedBack_PIN"].as<int>());
            LogHandler::debug(_TAG, "save RightServo_PIN: %i", doc["RightServo_PIN"].as<int>());
            LogHandler::debug(_TAG, "save LeftServo_PIN: %i", doc["LeftServo_PIN"].as<int>());
            LogHandler::debug(_TAG, "save RightUpperServo_PIN: %i",doc["RightUpperServo_PIN"].as<int>());
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
bool SettingsHandler::saving = false;
bool SettingsHandler::fullBuild = false;

BoardType SettingsHandler::boardType = BoardType::DEVKIT;
BuildFeature SettingsHandler::buildFeatures[featureCount];
const char* SettingsHandler::_TAG = TagHandler::SettingsHandler;
String SettingsHandler::TCodeVersionName;
TCodeVersion SettingsHandler::TCodeVersionEnum;
MotorType SettingsHandler::motorType = MotorType::Servo;
const char SettingsHandler::ESP32Version[8] = "v0.276b";
const char SettingsHandler::HandShakeChannel[4] = "D1\n";
const char SettingsHandler::SettingsChannel[4] = "D2\n";
const char* SettingsHandler::userSettingsDefaultFilePath = "/userSettingsDefault.json";
const char* SettingsHandler::userSettingsFilePath = "/userSettings.json";
const char* SettingsHandler::logPath = "/log.json";
const char* SettingsHandler::defaultWifiPass = "YOUR PASSWORD HERE";
const char* SettingsHandler::decoyPass = "Too bad haxor!";
bool SettingsHandler::bluetoothEnabled = true;
bool SettingsHandler::debug = false;
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

//int SettingsHandler::HeatLED_PIN = 32;
// pin 25 cannot be servo. Throws error
bool SettingsHandler::lubeEnabled = true;

int SettingsHandler::StrokeMin = 1;
int SettingsHandler::StrokeMax = 9999;
int SettingsHandler::RollMin = 1;
int SettingsHandler::RollMax = 9999;
int SettingsHandler::PitchMin = 1;
int SettingsHandler::PitchMax = 9999;

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

int SettingsHandler::BLDC_Encoder_PIN = 33;
int SettingsHandler::BLDC_Enable_PIN = 14;
int SettingsHandler::BLDC_PWMchannel1_PIN = 27;
int SettingsHandler::BLDC_PWMchannel2_PIN = 26;
int SettingsHandler::BLDC_PWMchannel3_PIN = 25;
float SettingsHandler::BLDC_MotorA_Voltage = 20.0;         // BLDC Motor operating voltage (12-20V)
float SettingsHandler::BLDC_MotorA_Current = 1.5;     // BLDC Maximum operating current (Amps)

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
int SettingsHandler::Display_I2C_Address = 0;// = 0x3C;
int SettingsHandler::Display_Rst_PIN = -1;
//long SettingsHandler::heaterFailsafeTime = 60000;
float SettingsHandler::heaterThreshold = 5.0;
int SettingsHandler::heaterResolution = 8;
int SettingsHandler::heaterFrequency = 5000;
int SettingsHandler::caseFanFrequency = 25;
int SettingsHandler::caseFanResolution = 10;
const char* SettingsHandler::lastRebootReason;

int SettingsHandler::strokerSamples = 100;
int SettingsHandler::strokerOffset = 3276;
int SettingsHandler::strokerAmplitude = 32767;