/* MIT License

Copyright (c) 2022 Jason C. Fain

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
		static int Vibe0_PIN;
		static int Vibe1_PIN;
        static int Vibe3_PIN;
        static int Vibe4_PIN;
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
        static int speed;
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
		static int Display_Screen_Width; 
		static int Display_Screen_Height; 
        static int Internal_Temp_PIN;
        static int Case_Fan_PIN;
		static int Sleeve_Temp_PIN; 
		static int caseFanPWM;// setting 0-255
        static int internalTempForFan; // C
		static int Heater_PIN;
		static int HeatLED_PIN;
		static int TargetTemp;// Desired Temp in degC
		static int HeatPWM;// Heating PWM setting 0-255
		static int HoldPWM;// Hold heat PWM setting 0-255
		static int Display_I2C_Address;
		static int Display_Rst_PIN;
        static long WarmUpTime;// Time to hold heating on first boot
        //static long heaterFailsafeTime;
        static float heaterThreshold;
        static int heaterResolution;
        static int heaterFrequency;
        static int caseFanFrequency;
        static int caseFanResolution;

        static int strokerSamples;
        static int strokerOffset;
        static int strokerAmplitude;

        static bool restartRequired;
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
                LogHandler::info(_TAG, "Read Settings: %s", userSettingsDefaultFilePath);
            	file = SPIFFS.open(userSettingsDefaultFilePath, "r");
				loadingDefault = true;
			} 
			else 
			{
                LogHandler::info(_TAG, "Read Settings: %s", userSettingsFilePath);
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

            update(jsonObj);
            
            setBoardType();
            setBuildFeatures();

	        #if ISAAC_NEWTONGUE_BUILD == 1
                RightServo_PIN = 13;
                LeftServo_PIN = 2;
                PitchLeftServo_PIN = 16;
                ValveServo_PIN = 5;
                TwistServo_PIN = 27;
                TwistFeedBack_PIN = 26;
                Vibe0_PIN = 25;
                Vibe1_PIN = 19;
                LubeButton_PIN = 15;
                RightUpperServo_PIN = 12;
                LeftUpperServo_PIN = 4;
                PitchRightServo_PIN = 14;
                Sleeve_Temp_PIN = 18; 
                Heater_PIN = 23;
            #elif CRIMZZON_BUILD == 1
                TCodeVersionEnum = TCodeVersion::v0_3;
                TCodeVersionName = TCodeVersionMapper(TCodeVersionEnum);
                tempInternalEnabled = true;
                RightServo_PIN = 13;
                LeftServo_PIN = 15;
                PitchLeftServo_PIN = 4;
                ValveServo_PIN = 25;
                TwistServo_PIN = 27;
                Vibe0_PIN = 18;
                Vibe1_PIN = 19;
                RightUpperServo_PIN = 12;
                LeftUpperServo_PIN = 2;
                PitchRightServo_PIN = 14;
                Sleeve_Temp_PIN = 5; 
                Heater_PIN = 33;

                Internal_Temp_PIN = 32;
                Case_Fan_PIN = 16;
                Squeeze_PIN = 17;
                Vibe3_PIN = 23;
                Vibe4_PIN = 26;

                TwistFeedBack_PIN = 0;// Boot pin

                //EXT
                LubeButton_PIN = 35;// EXT_Input1_PIN
                // EXT_Input2_PIN = 34;
		        // EXT_Input3_PIN = 39;
		        // EXT_Input4_PIN = 36;

                if(loadingDefault) {
                    heaterResolution = 8;
                    caseFanResolution = 10;
                    caseFanFrequency = 25;
                    Display_Screen_Height = 32;
                }
	        #endif
			if(loadingDefault)
				save();
        }

        static void reset() 
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
            if(json.size() > 0) 
            {
                logLevel = (LogLevel)(json["logLevel"] | 1);
                LogHandler::setLogLevel(logLevel);

                LogHandler::debug(_TAG, "Update settings");
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
	            #if CRIMZZON_BUILD == 0
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
                StrokeMin = json["xMin"] | 1;
                StrokeMax = json["xMax"] | 1000;
                RollMin = json["yRollMin"] | 1;
                RollMax = json["yRollMax"] | 1000;
                PitchMin = json["xRollMin"] | 1;
                PitchMax = json["xRollMax"] | 1000;
                speed = json["speed"] | 1000;
                pitchFrequencyIsDifferent = json["pitchFrequencyIsDifferent"];
                msPerRad =  json["msPerRad"] | 637;
                servoFrequency = json["servoFrequency"] | 50;
                pitchFrequency = json[pitchFrequencyIsDifferent ? "pitchFrequency" : "servoFrequency"];
                valveFrequency = json["valveFrequency"] | 50;
                twistFrequency = json["twistFrequency"] | 50;
                squeezeFrequency = json["squeezeFrequency"] | 50;
				continuousTwist = json["continuousTwist"];
                feedbackTwist =  json["feedbackTwist"];
                analogTwist = json["analogTwist"];

	        #if ISAAC_NEWTONGUE_BUILD == 0 && CRIMZZON_BUILD == 0
				TwistFeedBack_PIN = json["TwistFeedBack_PIN"];
				RightServo_PIN = json["RightServo_PIN"];
				LeftServo_PIN = json["LeftServo_PIN"];
				RightUpperServo_PIN = json["RightUpperServo_PIN"];
				LeftUpperServo_PIN = json["LeftUpperServo_PIN"];
				PitchLeftServo_PIN = json["PitchLeftServo_PIN"];
				PitchRightServo_PIN = json["PitchRightServo_PIN"];
				ValveServo_PIN = json["ValveServo_PIN"];
				TwistServo_PIN = json["TwistServo_PIN"];
				Vibe0_PIN = json["Vibe0_PIN"];
				Vibe1_PIN = json["Vibe1_PIN"];
				LubeButton_PIN = json["LubeButton_PIN"];
                Internal_Temp_PIN = json["Internal_Temp_PIN"];
                Case_Fan_PIN = json["Case_Fan_PIN"];
                Squeeze_PIN  = json["Squeeze_PIN"];
                Vibe3_PIN = json["Vibe3_PIN"];
                Vibe4_PIN = json["Vibe4_PIN"];
                Squeeze_PIN = json["Squeeze_PIN"];
				Sleeve_Temp_PIN = json["Temp_PIN"] | 5;
				Heater_PIN = json["Heater_PIN"] | 33;
	        #endif

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

				sr6Mode = json["sr6Mode"];
    			RightServo_ZERO = json["RightServo_ZERO"];
    			LeftServo_ZERO = json["LeftServo_ZERO"];
    			RightUpperServo_ZERO = json["RightUpperServo_ZERO"];
    			LeftUpperServo_ZERO = json["LeftUpperServo_ZERO"];
    			PitchLeftServo_ZERO = json["PitchLeftServo_ZERO"];
    			PitchRightServo_ZERO = json["PitchRightServo_ZERO"];
    			TwistServo_ZERO = json["TwistServo_ZERO"];
    			ValveServo_ZERO = json["ValveServo_ZERO"];
    			SqueezeServo_ZERO = json["Squeeze_ZERO"];
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
				Display_Screen_Height = json["Display_Screen_Height"] | 64;
                const char* Display_I2C_AddressTemp = json["Display_I2C_Address"];
                if (Display_I2C_AddressTemp != nullptr)
					Display_I2C_Address = (int)strtol(Display_I2C_AddressTemp, NULL, 0);
				Display_Rst_PIN = json["Display_Rst_PIN"] | -1;
                
				tempSleeveEnabled = json["tempSleeveEnabled"];
                heaterThreshold = json["heaterThreshold"] | 5.0;
                heaterFrequency = json["heaterFrequency"] | 5000;
                heaterResolution = json["heaterResolution"] | 8;
				TargetTemp = json["TargetTemp"] | 40;
				HeatPWM = json["HeatPWM"] | 255;
				HoldPWM = json["HoldPWM"] | 110;
                WarmUpTime = json["WarmUpTime"] | 600000;

                tempInternalEnabled = json["tempInternalEnabled"];
                fanControlEnabled = json["fanControlEnabled"];
                internalTempForFan = json["internalTempForFan"] | 30;
                caseFanFrequency = json["caseFanFrequency"] | 25;
                caseFanResolution = json["caseFanResolution"] | 10;
                caseFanPWM = pow(2, caseFanResolution) - 1;
                
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

        static void serialize(char buf[2048])
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
            if(LogHandler::getLogLevel() == LogLevel::VERBOSE)
                Serial.printf("Settings: %s\n", output.c_str());
            //LogHandler::verbose(_TAG, "Output: %s", output.c_str());
            buf[0]  = {0};
            strcpy(buf, output.c_str());
        }

        static void getSystemInfo(char buf[512]) {
            DynamicJsonDocument doc(512);
            
            doc["esp32Version"] = ESP32Version;
            doc["TCodeVersion"] = (int)TCodeVersionEnum;
            doc["lastRebootReason"] = lastRebootReason;
            doc["boardType"] = (int)boardType;
            JsonArray buildFeaturesJsonArray = doc.createNestedArray("buildFeatures");
            for (BuildFeature value : buildFeatures) {
                buildFeaturesJsonArray.add((int)value);
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

            if(!tempInternalEnabled) {
                internalTempDisplayed = false;
                fanControlEnabled = false;
            }
            if(!tempSleeveEnabled) {
                sleeveTempDisplayed = false;
            }
            LogHandler::debug(_TAG, "Save settings");
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
            doc["xMin"] = StrokeMin;
            doc["xMax"] = StrokeMax;
            doc["yRollMin"] = RollMin;
            doc["yRollMax"] = RollMax;
            doc["xRollMin"] = PitchMin;
            doc["xRollMax"] = PitchMax;
            doc["speed"] = speed;
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
			doc["Case_Fan_PIN"] = Case_Fan_PIN;
			doc["LubeButton_PIN"] = LubeButton_PIN;
            doc["Internal_Temp_PIN"] = Internal_Temp_PIN;
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
            doc["WarmUpTime"] = WarmUpTime;
            //doc["heaterFailsafeTime"] = String(heaterFailsafeTime);
            doc["heaterThreshold"] = heaterThreshold;
            doc["heaterResolution"] = heaterResolution;
            doc["heaterFrequency"] = heaterFrequency;
            doc["fanControlEnabled"] = fanControlEnabled;
            doc["caseFanFrequency"] = caseFanFrequency;
            doc["caseFanResolution"] = caseFanResolution;
            doc["internalTempForFan"] = internalTempForFan;
            doc["tempInternalEnabled"] = tempInternalEnabled;
			
            LogSaveDebug(doc);

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
            #if WIFI_TCODE == 1
                LogHandler::debug("setBuildFeatures", "WIFI_TCODE");
                buildFeatures[index] = BuildFeature::WIFI;
                index++;
            #endif
            #if BLUETOOTH_TCODE == 1
                LogHandler::debug("setBuildFeatures", "BLUETOOTH_TCODE");
                buildFeatures[index] = BuildFeature::BLUETOOTH;
                index++;
            #endif
            #if DEBUG_BUILD == 1
                LogHandler::debug("setBuildFeatures", "DEBUG_BUILD");
                buildFeatures[index] = BuildFeature::DEBUG;
                LogHandler::setLogLevel(LogLevel::VERBOSE);
		        debug = true;
                index++;
            #endif
            #if ESP32_DA == 1
                LogHandler::debug("setBuildFeatures", "ESP32_DA");
                buildFeatures[index] = BuildFeature::DA;
                index++;
            #endif
            #if TEMP_ENABLED == 1
                LogHandler::debug("setBuildFeatures", "TEMP_ENABLED");
                buildFeatures[index] = BuildFeature::TEMP;
                index++;
            #endif
            #if DISPLAY_ENABLED == 1
                LogHandler::debug("setBuildFeatures", "DISPLAY_ENABLED");
                buildFeatures[index] = BuildFeature::DISPLAY_;
                index++;
            #endif
            #if TCODE_V2 == 1
                LogHandler::debug("setBuildFeatures", "TCODE_V2");
                buildFeatures[index] = BuildFeature::HAS_TCODE_V2;
                index++;
            #endif
            buildFeatures[featureCount - 1] = {};
        }

        static void setBoardType() {
            #if ISAAC_NEWTONGUE_BUILD == 1
                LogHandler::debug("setBoardType", "ISAAC_NEWTONGUE_BUILD");
                boardType = BoardType::ISAAC;
            #elif CRIMZZON_BUILD == 1
                LogHandler::debug("setBoardType", "CRIMZZON_BUILD");
                boardType = BoardType::CRIMZZON;
            #else
                LogHandler::debug("setBoardType", "DEVKIT_BUILD");
                boardType = BoardType::DEVKIT;
            #endif
        }

        static void processTCodeJson(char* outbuf, char* tcodeJson) 
        {
            const size_t readCapacity = JSON_ARRAY_SIZE(5) + 5*JSON_OBJECT_SIZE(2) + 100;

            StaticJsonDocument<readCapacity> doc;
            //DynamicJsonDocument doc(readCapacity);
            DeserializationError error = deserializeJson(doc, tcodeJson);
            if (error) {
                LogHandler::error(_TAG, "Failed to read udp jsonobject, using default configuration");
                outbuf[0] = {0};
                return;
            }
            JsonArray arr = doc.as<JsonArray>();
            char buffer[100] = "";
            for(JsonObject repo: arr) 
            { 
                const char* channel = repo["Channel"];
                int value = repo["Value"];
                if(channel != nullptr && value != 0) 
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
                char integer_string[4];
                sprintf(integer_string, "%03d", SettingsHandler::calculateRange(channel, value));
                //pad(integer_string);
                //sprintf(integer_string, "%d", SettingsHandler::calculateRange(channel, value));
                //Serial.print("integer_string");
                //Serial.println(integer_string);
                strcat (buffer, integer_string);
                if (SettingsHandler::speed > 0) {
                    char speed_string[5];
                    sprintf(speed_string, "%04d", SettingsHandler::speed);
                    strcat (buffer, "S");
                    strcat (buffer, speed_string);
                }
                strcat(buffer, " ");
                // Serial.print("buffer");
                // Serial.println(buffer);
                }
            }
            strcat(buffer, "\n");
            strcpy(outbuf, buffer);
        }
        
    private:
        static const char* _TAG;
        static const char* userSettingsDefaultFilePath;
        // Use http://arduinojson.org/assistant to compute the capacity.
        // static const size_t readCapacity = JSON_OBJECT_SIZE(100) + 2000;
        // static const size_t saveCapacity = JSON_OBJECT_SIZE(100);
		static const int deserializeSize = 3072;
		static const int serializeSize = 2048;

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
            return 1000;
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
            // LogHandler::verbose(_TAG, "save TCodeVersionEnum: %i", (int)doc["TCodeVersion"]);
            // LogHandler::verbose(_TAG, "save ssid: %s", (const char*) doc["ssid"]);
            // // LogHandler::verbose(_TAG, "save wifiPass: %s", (const char*) doc["wifiPass"]);
            // LogHandler::verbose(_TAG, "save udpServerPort: %i", (int)doc["udpServerPort"]);
            // LogHandler::verbose(_TAG, "save webServerPort: %i", (int)doc["webServerPort"]);
            // LogHandler::verbose(_TAG, "save hostname: %s", (const char*) doc["hostname"]);
            // LogHandler::verbose(_TAG, "save friendlyName: %s", (const char*) doc["friendlyName"]);
            // LogHandler::verbose(_TAG, "save pitchFrequencyIsDifferent ", (bool)doc["pitchFrequencyIsDifferent"]);
            // LogHandler::verbose(_TAG, "save msPerRad: %i", (int)doc["msPerRad"]);
            // LogHandler::verbose(_TAG, "save servoFrequency: %i", (int)doc["servoFrequency"]);
            // LogHandler::verbose(_TAG, "save  pitchFrequency: %i", (int)doc["pitchFrequency"]);
            // LogHandler::verbose(_TAG, "save valveFrequency: %i",(int)doc["valveFrequency"]);
            // LogHandler::verbose(_TAG, "save twistFrequency: %i", (int)doc["twistFrequency"]);
            // LogHandler::verbose(_TAG, "save continuousTwist: %i", (bool)doc["continuousTwist"]);
            // LogHandler::verbose(_TAG, "save feedbackTwist: %i", (bool)doc["feedbackTwist"]);
            // LogHandler::verbose(_TAG, "save analogTwist: %i", (bool)doc["analogTwist"]);
            // LogHandler::verbose(_TAG, "save TwistFeedBack_PIN: %i", (int)doc["TwistFeedBack_PIN"]);
            // LogHandler::verbose(_TAG, "save RightServo_PIN: %i", (int)doc["RightServo_PIN"]);
            // LogHandler::verbose(_TAG, "save LeftServo_PIN: %i", (int)doc["LeftServo_PIN"]);
            // LogHandler::verbose(_TAG, "save RightUpperServo_PIN: %i",(int)doc["RightUpperServo_PIN"]);
            // LogHandler::verbose(_TAG, "save LeftUpperServo_PIN: %i", (int)doc["LeftUpperServo_PIN"]);
            // LogHandler::verbose(_TAG, "save PitchLeftServo_PIN: %i", (int)doc["PitchLeftServo_PIN"]);
            // LogHandler::verbose(_TAG, "save PitchRightServo_PIN: %i", (int)doc["PitchRightServo_PIN"]);
            // LogHandler::verbose(_TAG, "save ValveServo_PIN: %i", (int)doc["ValveServo_PIN"]);
            // LogHandler::verbose(_TAG, "save TwistServo_PIN: %i", (int)doc["TwistServo_PIN"]);
            // LogHandler::verbose(_TAG, "save Vibe0_PIN: %i", (int)doc["Vibe0_PIN"]);
            // LogHandler::verbose(_TAG, "save Vibe1_PIN: %i", (int)doc["Vibe1_PIN"]);
            // LogHandler::verbose(_TAG, "save Lube_Pin: %i", (int)doc["Lube_Pin"]);
            // LogHandler::verbose(_TAG, "save LubeButton_PIN: %i", (int)doc["LubeButton_PIN"]);
            // LogHandler::verbose(_TAG, "save staticIP: %i", (bool)doc["staticIP"]);
            // LogHandler::verbose(_TAG, "save localIP: %s", (const char*) doc["localIP"]);
            // LogHandler::verbose(_TAG, "save gateway: %s", (const char*) doc["gateway"]);
            // LogHandler::verbose(_TAG, "save subnet: %s", (const char*) doc["subnet"]);
            // LogHandler::verbose(_TAG, "save dns1: %s", (const char*) doc["dns1"]);
            // LogHandler::verbose(_TAG, "save dns2: %s", (const char*) doc["dns2"]);
            // LogHandler::verbose(_TAG, "save sr6Mode: %i", (bool)doc["sr6Mode"]);
            // LogHandler::verbose(_TAG, "save RightServo_ZERO: %i", (int)doc["RightServo_ZERO"]);
            // LogHandler::verbose(_TAG, "save LeftServo_ZERO: %i", (int)doc["LeftServo_ZERO"]);
            // LogHandler::verbose(_TAG, "save RightUpperServo_ZERO: %i", (int)doc["RightUpperServo_ZERO"]);
            // LogHandler::verbose(_TAG, "save LeftUpperServo_ZERO: %i", (int)doc["LeftUpperServo_ZERO"]);
            // LogHandler::verbose(_TAG, "save PitchLeftServo_ZERO: %i", (int)doc["PitchLeftServo_ZERO"]);
            // LogHandler::verbose(_TAG, "save PitchRightServo_ZERO: %i", (int)doc["PitchRightServo_ZERO"]);
            // LogHandler::verbose(_TAG, "save TwistServo_ZERO: %i", (int)doc["TwistServo_ZERO"]);
            // LogHandler::verbose(_TAG, "save ValveServo_ZERO: %i", (int)doc["ValveServo_ZERO"]);
            // LogHandler::verbose(_TAG, "save autoValve: %i", (bool)doc["autoValve"]);
            // LogHandler::verbose(_TAG, "save inverseValve: %i", (bool)doc["inverseValve"]);
            // LogHandler::verbose(_TAG, "save valveServo90Degrees: %i", (bool)doc["valveServo90Degrees"]);
            // LogHandler::verbose(_TAG, "save inverseStroke: %i", (bool)doc["inverseStroke"]);
            // LogHandler::verbose(_TAG, "save inversePitch: %i", (bool)doc["inversePitch"]);
            // LogHandler::verbose(_TAG, "save lubeEnabled: %i", (bool)doc["lubeEnabled"]);
            // LogHandler::verbose(_TAG, "save lubeAmount: %i", (int)doc["lubeAmount"]);
            // LogHandler::verbose(_TAG, "save Temp_PIN: %i", (int)doc["Temp_PIN"]);
            // LogHandler::verbose(_TAG, "save Heater_PIN: %i", (int)doc["Heater_PIN"]);
            // LogHandler::verbose(_TAG, "save displayEnabled: %i", (bool)doc["displayEnabled"]);
            // LogHandler::verbose(_TAG, "save sleeveTempDisplayed: %i", (bool)doc["sleeveTempDisplayed"]);
            // LogHandler::verbose(_TAG, "save internalTempDisplayed: %i", (bool)doc["internalTempDisplayed"]);
            // LogHandler::verbose(_TAG, "save tempSleeveEnabled: %i", (bool)doc["tempSleeveEnabled"]);
            LogHandler::debug(_TAG, "save tempInternalEnabled: %i", (bool)doc["tempInternalEnabled"]);
            // LogHandler::verbose(_TAG, "save Display_Screen_Width: %i", (int)doc["Display_Screen_Width"]);
            // LogHandler::verbose(_TAG, "save Display_Screen_Height: %i", (int)doc["Display_Screen_Height"]);
            // LogHandler::verbose(_TAG, "save TargetTemp: %i", (int)doc["TargetTemp"]);
            // LogHandler::verbose(_TAG, "save HeatPWM: %i", (int)doc["HeatPWM"]);
            // LogHandler::verbose(_TAG, "save HoldPWM: %i", (int)doc["HoldPWM"]);
            // LogHandler::verbose(_TAG, "save Display_I2C_Address: %i", (int)doc["Display_I2C_Address"]);
            // LogHandler::verbose(_TAG, "save Display_Rst_PIN: %i", (int)doc["Display_Rst_PIN"]);
            // LogHandler::verbose(_TAG, "save WarmUpTime: %ld", (long)doc["WarmUpTime"]);
            // LogHandler::verbose(_TAG, "save heaterFailsafeTime: %ld", (long)doc["heaterFailsafeTime"]);
            // LogHandler::verbose(_TAG, "save heaterThreshold: %i", (int)doc["heaterThreshold"]);
            // LogHandler::verbose(_TAG, "save heaterResolution: %i", (int)doc["heaterResolution"]);
            // LogHandler::verbose(_TAG, "save heaterFrequency: %i", (int)doc["heaterFrequency"]);
            // LogHandler::verbose(_TAG, "save newtoungeHatExists: %i", (bool)doc["newtoungeHatExists"]);
            // LogHandler::verbose(_TAG, "save logLevel: %i", (int)doc["logLevel"]);
            // LogHandler::verbose(_TAG, "save bluetoothEnabled: %i", (int)doc["bluetoothEnabled"]);
            
        }

        static void LogUpdateDebug() 
        {
            // LogHandler::verbose(_TAG, "update TCodeVersionEnum: %i", TCodeVersionEnum);
            // LogHandler::verbose(_TAG, "update ssid: %s", ssid);
            // //LogHandler::verbose(_TAG, "update wifiPass: %s", wifiPass);
            // LogHandler::verbose(_TAG, "update udpServerPort: %i", udpServerPort);
            // LogHandler::verbose(_TAG, "update webServerPort: %i", webServerPort);
            // LogHandler::verbose(_TAG, "update hostname: %s", hostname);
            // LogHandler::verbose(_TAG, "update friendlyName: %s", friendlyName);
            // LogHandler::verbose(_TAG, "update pitchFrequencyIsDifferent: %i", pitchFrequencyIsDifferent);
            // LogHandler::verbose(_TAG, "update msPerRad: %i", msPerRad);
            // LogHandler::verbose(_TAG, "update servoFrequency: %i", servoFrequency);
            // LogHandler::verbose(_TAG, "update pitchFrequency: %i", pitchFrequency);
            // LogHandler::verbose(_TAG, "update valveFrequency: %i", valveFrequency);
            // LogHandler::verbose(_TAG, "update twistFrequency: %i", twistFrequency);
            // LogHandler::verbose(_TAG, "update continuousTwist: %i", continuousTwist);
            // LogHandler::verbose(_TAG, "update feedbackTwist: %i", feedbackTwist);
            // LogHandler::verbose(_TAG, "update analogTwist: %i", analogTwist);
            // LogHandler::verbose(_TAG, "update TwistFeedBack_PIN: %i", TwistFeedBack_PIN);
            // LogHandler::verbose(_TAG, "update RightServo_PIN: %i", RightServo_PIN);
            // LogHandler::verbose(_TAG, "update LeftServo_PIN: %i", LeftServo_PIN);
            // LogHandler::verbose(_TAG, "update RightUpperServo_PIN: %i", RightUpperServo_PIN);
            // LogHandler::verbose(_TAG, "update LeftUpperServo_PIN: %i", LeftUpperServo_PIN);
            // LogHandler::verbose(_TAG, "update PitchLeftServo_PIN: %i", PitchLeftServo_PIN);
            // LogHandler::verbose(_TAG, "update PitchRightServo_PIN: %i", PitchRightServo_PIN);
            // LogHandler::verbose(_TAG, "update ValveServo_PIN: %i", ValveServo_PIN);
            // LogHandler::verbose(_TAG, "update TwistServo_PIN: %i", TwistServo_PIN);
            // LogHandler::verbose(_TAG, "update Vibe0_PIN: %i", Vibe0_PIN);
            // LogHandler::verbose(_TAG, "update Vibe1_PIN: %i", Vibe1_PIN);
            // LogHandler::verbose(_TAG, "update LubeButton_PIN: %i", LubeButton_PIN);
            // LogHandler::verbose(_TAG, "update staticIP: %i", staticIP);
            // LogHandler::verbose(_TAG, "update localIP: %s", localIP);
            // LogHandler::verbose(_TAG, "update gateway: %s", gateway);
            // LogHandler::verbose(_TAG, "update subnet: %s", subnet);
            // LogHandler::verbose(_TAG, "update dns1: %s", dns1);
            // LogHandler::verbose(_TAG, "update dns2: %s", dns2);
            // LogHandler::verbose(_TAG, "update sr6Mode: %i", sr6Mode);
            // LogHandler::verbose(_TAG, "update RightServo_ZERO: %i", RightServo_ZERO);
            // LogHandler::verbose(_TAG, "update LeftServo_ZERO: %i", LeftServo_ZERO);
            // LogHandler::verbose(_TAG, "update RightUpperServo_ZERO: %i", RightUpperServo_ZERO);
            // LogHandler::verbose(_TAG, "update LeftUpperServo_ZERO: %i", LeftUpperServo_ZERO);
            // LogHandler::verbose(_TAG, "update PitchLeftServo_ZERO: %i", PitchLeftServo_ZERO);
            // LogHandler::verbose(_TAG, "update PitchRightServo_ZERO: %i", PitchRightServo_ZERO);
            // LogHandler::verbose(_TAG, "update TwistServo_ZERO: %i", TwistServo_ZERO);
            // LogHandler::verbose(_TAG, "update ValveServo_ZERO: %i", ValveServo_ZERO);
            // LogHandler::verbose(_TAG, "update autoValve: %i", autoValve);
            // LogHandler::verbose(_TAG, "update inverseValve: %i", inverseValve);
            // LogHandler::verbose(_TAG, "update valveServo90Degrees: %i", valveServo90Degrees);
            // LogHandler::verbose(_TAG, "update inverseStroke: %i", inverseStroke);
            // LogHandler::verbose(_TAG, "update inversePitch: %i", inversePitch);
            // LogHandler::verbose(_TAG, "update lubeEnabled: %i", lubeEnabled);
            // LogHandler::verbose(_TAG, "update lubeAmount: %i", lubeAmount);
            // LogHandler::verbose(_TAG, "update displayEnabled: %i", displayEnabled);
            // LogHandler::verbose(_TAG, "update sleeveTempDisplayed: %i", sleeveTempDisplayed);
            // LogHandler::verbose(_TAG, "update internalTempDisplayed: %i", internalTempDisplayed);
            // LogHandler::verbose(_TAG, "update tempSleeveEnabled: %i", tempSleeveEnabled);
            LogHandler::debug(_TAG, "update tempInternalEnabled: %i", tempInternalEnabled);
            // LogHandler::verbose(_TAG, "update Display_Screen_Width: %i", Display_Screen_Width);
            // LogHandler::verbose(_TAG, "update Display_Screen_Height: %i", Display_Screen_Height);
            // LogHandler::verbose(_TAG, "update TargetTemp: %i", TargetTemp);
            // LogHandler::verbose(_TAG, "update HeatPWM: %i", HeatPWM);
            // LogHandler::verbose(_TAG, "update HoldPWM: %i", HoldPWM);
            // LogHandler::verbose(_TAG, "update Display_I2C_Address: %i", Display_I2C_Address);
            // LogHandler::verbose(_TAG, "update Display_Rst_PIN: %i", Display_Rst_PIN);
            // LogHandler::verbose(_TAG, "update Temp_PIN: %i", Temp_PIN);
            // LogHandler::verbose(_TAG, "update Heater_PIN: %i", Heater_PIN); 
            // LogHandler::verbose(_TAG, "update WarmUpTime: %ld", WarmUpTime);
            // LogHandler::verbose(_TAG, "update heaterFailsafeTime: %ld", heaterFailsafeTime);
            // LogHandler::verbose(_TAG, "update heaterThreshold: %d", heaterThreshold);
            // LogHandler::verbose(_TAG, "update heaterResolution: %i", heaterResolution);
            // LogHandler::verbose(_TAG, "update heaterFrequency: %i", heaterFrequency);
            // LogHandler::verbose(_TAG, "update newtoungeHatExists: %i", newtoungeHatExists);
            // LogHandler::verbose(_TAG, "update logLevel: %i", (int)logLevel);
            // LogHandler::verbose(_TAG, "update bluetoothEnabled: %i", (int)bluetoothEnabled);
            
        }
};
bool SettingsHandler::saving = false;
bool SettingsHandler::fullBuild = false;

BoardType SettingsHandler::boardType = BoardType::DEVKIT;
BuildFeature SettingsHandler::buildFeatures[featureCount];
const char* SettingsHandler::_TAG = "_SETTINGS_HANDLER";
String SettingsHandler::TCodeVersionName;
TCodeVersion SettingsHandler::TCodeVersionEnum;
const char SettingsHandler::ESP32Version[8] = "v0.256a";
const char SettingsHandler::HandShakeChannel[4] = "D1\n";
const char SettingsHandler::SettingsChannel[4] = "D2\n";
const char* SettingsHandler::userSettingsDefaultFilePath = "/userSettingsDefault.json";
const char* SettingsHandler::userSettingsFilePath = "/userSettings.json";
const char* SettingsHandler::logPath = "/log.json";
const char* SettingsHandler::defaultWifiPass = "YOUR PASSWORD HERE";
const char* SettingsHandler::decoyPass = "Too bad haxor!";
bool SettingsHandler::bluetoothEnabled = true;
bool SettingsHandler::restartRequired = false;
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

int SettingsHandler::Internal_Temp_PIN;
int SettingsHandler::Case_Fan_PIN;
int SettingsHandler::Squeeze_PIN;
int SettingsHandler::Vibe3_PIN;
int SettingsHandler::Vibe4_PIN;

//int SettingsHandler::HeatLED_PIN = 32;
// pin 25 cannot be servo. Throws error
bool SettingsHandler::lubeEnabled = true;

int SettingsHandler::StrokeMin;
int SettingsHandler::StrokeMax;
int SettingsHandler::RollMin;
int SettingsHandler::RollMax;
int SettingsHandler::PitchMin;
int SettingsHandler::PitchMax;

int SettingsHandler::speed;

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
int SettingsHandler::Display_Screen_Width = 128; 
int SettingsHandler::Display_Screen_Height = 64; 
int SettingsHandler::caseFanPWM = 255;
int SettingsHandler::internalTempForFan = 20;
int SettingsHandler::TargetTemp = 40;
int SettingsHandler::HeatPWM = 255;
int SettingsHandler::HoldPWM = 110;
int SettingsHandler::Display_I2C_Address = 0x3C;
int SettingsHandler::Display_Rst_PIN = -1;
long SettingsHandler::WarmUpTime = 600000;
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