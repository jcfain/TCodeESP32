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

enum TCodeVersion {
    v0_2,
    v0_3,
    v1_0
};

class SettingsHandler 
{
  public:

        static bool saving;
        static bool fullBuild;
        static bool debug;
        static LogLevel logLevel;

        static String TCodeVersionName;
        static TCodeVersion TCodeVersionEnum;
        const static char ESP32Version[14];
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
		static int TwistFeedBack_PIN;
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
        static int LubeManual_PIN;
        static int FIRMWARE_MODE_PIN;
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
        static int servoFrequency;
        static int pitchFrequency;
        static int valveFrequency;
        static int twistFrequency;
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
		static bool autoValve;
		static bool inverseValve;
		static bool inverseStroke;
		static bool inversePitch;
		static bool valveServo90Degrees;
        static bool lubeEnabled;
		static int lubeAmount;
		static bool displayEnabled;
		static bool sleeveTempDisplayed;
		static bool tempControlEnabled;
		static int Display_Screen_Width; 
		static int Display_Screen_Height; 
		static int Temp_PIN; 
		static int Heater_PIN;
		static int HeatLED_PIN;
		static int TargetTemp;// Desired Temp in degC
		static int HeatPWM;// Heating PWM setting 0-255
		static int HoldPWM;// Hold heat PWM setting 0-255
		static int Display_I2C_Address;
		static int Display_Rst_PIN;
        static long WarmUpTime;// Time to hold heating on first boot
        static long heaterFailsafeTime;
        static float heaterThreshold;
        static int heaterResolution;
        static int heaterFrequency;

        static int strokerSamples;
        static int strokerOffset;
        static int strokerAmplitude;

        static bool newtoungeHatExists;
        static bool restartRequired;
        static const char* lastRebootReason;

        static const char* userSettingsFilePath;
        static const char* logPath;
        static const char* defaultWifiPass;
        static const char* decoyPass;
		static const int deserialize = 3072;
		static const int serialize = 1536;

        static void load() 
        {
            DynamicJsonDocument doc(deserialize);
			File file;
			bool loadingDefault = false;
			if(!SPIFFS.exists(userSettingsFilePath)) 
			{
                Serial.println(F("Failed to read settings file, using default configuration"));
                Serial.println(F("Read Settings: /userSettingsDefault.json"));
            	file = SPIFFS.open("/userSettingsDefault.json", "r");
				loadingDefault = true;
			} 
			else 
			{
                Serial.print("Read Settings: ");
                Serial.println(userSettingsFilePath);
            	file = SPIFFS.open(userSettingsFilePath, "r");
			}
            DeserializationError error = deserializeJson(doc, file);
            if (error) {
                Serial.print(F("Error deserializing settings json: "));
                Serial.println(F(file.name()));
                Serial.print("Error: ");
                switch(error.code()) {
                    case DeserializationError::Code::Ok:
                        Serial.println("Ok");
                    break;
                    case DeserializationError::Code::EmptyInput:
                        Serial.println("EmptyInput");
                    break;
                    case DeserializationError::Code::IncompleteInput:
                        Serial.println("IncompleteInput");
                    break;
                    case DeserializationError::Code::InvalidInput:
                        Serial.println("InvalidInput");
                    break;
                    case DeserializationError::Code::NoMemory:
                        Serial.println("NoMemory");
                    break;
                    case DeserializationError::Code::TooDeep:
                        Serial.println("TooDeep");
                    break;
                }
            	file = SPIFFS.open("/userSettingsDefault.json", "r");
                deserializeJson(doc, file);
                loadingDefault = true;
            }
            JsonObject jsonObj = doc.as<JsonObject>();
            const char* storedVersion = jsonObj["esp32Version"];

            update(jsonObj);
            if(newtoungeHatExists) 
            {
                RightServo_PIN = 13;
                LeftServo_PIN = 2;
                PitchLeftServo_PIN = 16;
                ValveServo_PIN = 5;
                TwistServo_PIN = 27;
                TwistFeedBack_PIN = 26;
                Vibe0_PIN = 25;
                Vibe1_PIN = 19;
                LubeManual_PIN = 15;
                RightUpperServo_PIN = 12;
                LeftUpperServo_PIN = 4;
                PitchRightServo_PIN = 14;
                Temp_PIN = 18; 
                Heater_PIN = 23;
            }
            else
            {
                PitchRightServo_PIN = 14;
                RightUpperServo_PIN = 12;
                RightServo_PIN = 13;
                PitchLeftServo_PIN = 4;
                LeftUpperServo_PIN = 2;
                LeftServo_PIN = 15;
                ValveServo_PIN = 28;
                TwistServo_PIN = 27;
                TwistFeedBack_PIN = 26;
                Vibe0_PIN = 18;
                Vibe1_PIN = 19;
                LubeManual_PIN = 23;
                Temp_PIN = 5; 
                Heater_PIN = 33;
            }

			if(loadingDefault || strcmp(storedVersion, ESP32Version) != 0)
				    save();
                    
        }

        static void reset() 
        {
            const char* filename = "/userSettingsDefault.json";
            DynamicJsonDocument doc(deserialize);
            File file = SPIFFS.open(filename, "r");
            DeserializationError error = deserializeJson(doc, file);
            if (error)
                Serial.println(F("Failed to read default settings file, using default configuration"));
            update(doc.as<JsonObject>());
			save();
        }

        static bool update(JsonObject json) 
        {
            if(json.size() > 0) 
            {
                logLevel = (LogLevel)(json["logLevel"] | 0);
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
                TCodeVersionEnum = (TCodeVersion)(json["TCodeVersion"] | 1);
                TCodeVersionName = TCodeVersionMapper(TCodeVersionEnum);
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
                servoFrequency = json["servoFrequency"] | 50;
                pitchFrequency = json[pitchFrequencyIsDifferent ? "pitchFrequency" : "servoFrequency"];
                valveFrequency = json["valveFrequency"] | 50;
                twistFrequency = json["twistFrequency"] | 50;
				continuousTwist = json["continuousTwist"];
                feedbackTwist =  json["feedbackTwist"];
                analogTwist = json["analogTwist"];
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
				LubeManual_PIN = json["LubeManual_PIN"];
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
				autoValve = json["autoValve"];
				inverseValve = json["inverseValve"];
				valveServo90Degrees = json["valveServo90Degrees"];
				inverseStroke = json["inverseStroke"];
				inversePitch = json["inversePitch"];
                lubeEnabled = json["lubeEnabled"];
				lubeAmount = json["lubeAmount"] | 255;
				displayEnabled = json["displayEnabled"];
				sleeveTempDisplayed = json["sleeveTempDisplayed"];
				tempControlEnabled = json["tempControlEnabled"];
				Display_Screen_Width = json["Display_Screen_Width"] | 128;
				Display_Screen_Height = json["Display_Screen_Height"] | 64;
				Temp_PIN = json["Temp_PIN"] | 5;
				Heater_PIN = json["Heater_PIN"] | 33;
				TargetTemp = json["TargetTemp"] | 40;
				HeatPWM = json["HeatPWM"] | 255;
				HoldPWM = json["HoldPWM"] | 110;
                const char* Display_I2C_AddressTemp = json["Display_I2C_Address"];
                if (Display_I2C_AddressTemp != nullptr)
					Display_I2C_Address = (int)strtol(Display_I2C_AddressTemp, NULL, 0);
				Display_Rst_PIN = json["Display_Rst_PIN"] | -1;
                WarmUpTime = json["WarmUpTime"] | 600000;
                heaterFailsafeTime = json["heaterFailsafeTime"] | 60000;
                heaterThreshold = json["heaterThreshold"] | 5.0;
                heaterResolution = json["heaterResolution"] | 8;
                heaterFrequency = json["heaterFrequency"] | 5000;
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
        
        static char* getJsonForBLE() 
        {
            //DynamicJsonDocument doc(readCapacity);
            //DeserializationError error = deserializeJson(doc, jsonInput, sizeof(jsonInput));
            const char* filename = "/userSettings.json";
            File file = SPIFFS.open(filename, "r");
            size_t size = file.size();
            char* bytes = new char[size];
            file.readBytes(bytes, size);
            return bytes;
        }

        static String serializeWifiSettings()
        {
            DynamicJsonDocument doc(serialize);
            String output;
            doc["ssid"] = ssid;
            doc["wifiPass"] = SettingsHandler::decoyPass;
			doc["staticIP"] = staticIP;
			doc["localIP"] = localIP;
			doc["gateway"] = gateway;
			doc["subnet"] = subnet;
			doc["dns1"] = dns1;
			doc["dns2"] = dns2;
            doc["servoFrequency"] = servoFrequency;
			doc["sr6Mode"] = sr6Mode;
			doc["bluetoothEnabled"] = bluetoothEnabled;
            if (serializeJson(doc, output) == 0) 
            {
                LogHandler::error(_TAG, "Failed to create settings json string");
                return "{}";
            }
            return output;
        }

        static bool derializeWifiSettings(String data)
        {
            String output;
            DynamicJsonDocument doc(deserialize);
            DeserializationError error = deserializeJson(doc, data);
            if (error) 
            {
                LogHandler::error(_TAG, "Failed to read settings file, using default configuration");
                return false;
            }
            const char* ssidConst = doc["ssid"];
            if( ssid != nullptr) 
            {
                strcpy(ssid, ssidConst);
            }
            const char* wifiPassConst = doc["wifiPass"];
            if(wifiPassConst != nullptr && strcmp(wifiPassConst, SettingsHandler::decoyPass) != 0) 
            {
                strcpy(wifiPass, wifiPassConst);
            }
            staticIP = doc["staticIP"];
            servoFrequency = doc["servoFrequency"] | 50;
            const char* localIPTemp = doc["localIP"];
            if (localIPTemp != nullptr)
                strcpy(localIP, localIPTemp);
            const char* gatewayTemp = doc["gateway"];
            if (gatewayTemp != nullptr)
                strcpy(gateway, gatewayTemp);
            const char* subnetTemp = doc["subnet"];
            if (subnetTemp != nullptr)
                strcpy(subnet, subnetTemp);
            const char* dns1Temp = doc["dns1"];
            if (dns1Temp != nullptr)
                strcpy(dns1, dns1Temp);
            const char* dns2Temp = doc["dns2"];
            if (dns2Temp != nullptr)
                strcpy(dns2, dns2Temp);
            sr6Mode = doc["sr6Mode"];
			bluetoothEnabled =  doc["bluetoothEnabled"];
            return save();
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
            DynamicJsonDocument doc(serialize);

            doc["logLevel"] = (int)logLevel;
            LogHandler::setLogLevel(logLevel);

            LogHandler::debug(_TAG, "Save settings");
            doc["fullBuild"] = fullBuild;
            doc["esp32Version"] = ESP32Version;
            doc["TCodeVersion"] = TCodeVersionEnum;
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
            doc["servoFrequency"] = servoFrequency;
            doc["pitchFrequency"] = pitchFrequency;
            doc["valveFrequency"] = valveFrequency;
            doc["twistFrequency"] = twistFrequency;
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
			doc["Vibe0_PIN"] = Vibe0_PIN;
			doc["Vibe1_PIN"] = Vibe1_PIN;
			doc["LubeManual_PIN"] = LubeManual_PIN;
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
			doc["autoValve"] = autoValve;
			doc["inverseValve"] = inverseValve;
			doc["valveServo90Degrees"] = valveServo90Degrees;
			doc["inverseStroke"] = inverseStroke;
			doc["inversePitch"] = inversePitch;
			doc["lubeAmount"] = lubeAmount;
            doc["lubeEnabled"] = lubeEnabled;
			doc["displayEnabled"] = displayEnabled;
			doc["sleeveTempDisplayed"] = sleeveTempDisplayed;
			doc["tempControlEnabled"] = tempControlEnabled;
			doc["Display_Screen_Width"] = Display_Screen_Width;
			doc["Display_Screen_Height"] = Display_Screen_Height;
			doc["TargetTemp"] = TargetTemp;
			doc["HeatPWM"] = HeatPWM;
			doc["HoldPWM"] = HoldPWM;
			std::stringstream Display_I2C_Address_String;
			Display_I2C_Address_String << "0x" << std::hex << Display_I2C_Address;
			doc["Display_I2C_Address"] = Display_I2C_Address_String.str();
			doc["Display_Rst_PIN"] = Display_Rst_PIN;
			doc["Temp_PIN"] = Temp_PIN;
			doc["Heater_PIN"] = Heater_PIN;
            doc["WarmUpTime"] = WarmUpTime;
            doc["heaterFailsafeTime"] = String(heaterFailsafeTime);
            doc["heaterThreshold"] = heaterThreshold;
            doc["heaterResolution"] = heaterResolution;
            doc["heaterFrequency"] = heaterFrequency;
			doc["newtoungeHatExists"] = newtoungeHatExists;
			
            LogSaveDebug(doc);

            if (serializeJson(doc, file) == 0) 
            {
                Serial.println(F("Failed to write to file"));
                doc["wifiPass"] = "";
                return false;
            }
            LogHandler::debug(_TAG, "File contents: %s", file.readString());
            LogHandler::debug(_TAG, "Free heap: %u", ESP.getFreeHeap());
            LogHandler::debug(_TAG, "Total heap: %u", ESP.getHeapSize());
            LogHandler::debug(_TAG, "Free psram: %u", ESP.getFreePsram());
            LogHandler::debug(_TAG, "Total Psram: %u", ESP.getPsramSize());
            LogHandler::debug(_TAG, "SPIFFS used: %i", SPIFFS.usedBytes());
            LogHandler::debug(_TAG, "SPIFFS total: %i",SPIFFS.totalBytes());
            file.close(); // Task exception here could mean not enough space on SPIFFS.
            
            doc["wifiPass"] = "";
            saving = false;
            return true;
        }

        static void processTCodeJson(char* outbuf, char* tcodeJson) 
        {
            const size_t readCapacity = JSON_ARRAY_SIZE(5) + 5*JSON_OBJECT_SIZE(2) + 100;

            StaticJsonDocument<readCapacity> doc;
            //DynamicJsonDocument doc(readCapacity);
            DeserializationError error = deserializeJson(doc, tcodeJson);
            if (error) {
                Serial.println("Failed to read udp jsonobject, using default configuration");
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
        // Use http://arduinojson.org/assistant to compute the capacity.
        // static const size_t readCapacity = JSON_OBJECT_SIZE(100) + 2000;
        // static const size_t saveCapacity = JSON_OBJECT_SIZE(100);

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
            case 0:
                return "TCode v0.2";
                break;
            case 1:
                return "TCode v0.3";
                break;
            case 2:
                return "TCode v0.4";
                break;
            default:
                return "TCode v?";
                break;
            }
        }

        static void log_last_reset_reason() {
            // //if(debug) {
            //     //Serial.println("enter log_last_reset_reason");
            //     double spiffs90Percent = SPIFFS.totalBytes()/0.90;
            //     LogHandler::debug(_TAG, "SPIFFS used: %.1f%%", SPIFFS.usedBytes());
            //     LogHandler::debug(_TAG, "SPIFFS 90 of total: %.1f%%", spiffs90Percent);
            //     if(SPIFFS.usedBytes() > spiffs90Percent) {
            //         LogHandler::warning(_TAG, "Disk usage is over 90%, replacing log.");
            //         if(!SPIFFS.remove(logPath)) 
            //         {
            //             LogHandler::error(_TAG, "Failed to remove file log.json");
            //             return;
            //         }
            //     }
            //     File file = SPIFFS.open(logPath, FILE_WRITE);
            //     if (!file) 
            //     {
            //         LogHandler::error(_TAG, "Failed to create file");
            //         return;
            //     }
            //     DynamicJsonDocument docDeserialize(deserialize);
            //     deserializeJson(docDeserialize, file);
            //     // if(error) {
            //     //     Serial.println(F("Deserialization Error: deleting log file"));
            //     //     if(!SPIFFS.remove(logPath)) 
            //     //     {
            //     //         Serial.println(F("Failed to remove file log.json"));
            //     //     }
            //     //     file = SPIFFS.open(logPath, FILE_WRITE);
            //     //     DeserializationError error = deserializeJson(docDeserialize, file);
            //     //     if (!file) 
            //     //     {
            //     //         Serial.println(F("Failed to create file"));
            //     //         return;
            //     //     }
            //     // }

            //      //JsonObject jsonObj = docDeserialize.as<JsonObject>();
            //      //JsonArray resetReasons = docDeserialize["resetReasons"].as<JsonArray>();
            //      JsonArray resetReasons = docDeserialize.createNestedArray("resetReasons");
                //JsonObject resonObj;
                //resonObj["time"] = getTime();
                //resonObj["reason"] = resetCause;
                // //Serial.println(resetCause);
                // resetReasons.add(resetCause);
                // // DynamicJsonDocument docSerialize(JSON_ARRAY_SIZE(resetReasons.size()));
                // // docSerialize["resetReasons"] = resetReasons;
                // if (serializeJson(docDeserialize, file) == 0) 
                // {
                //     LogHandler::error(_TAG, "Failed to write to log file");
                //     return;
                // }
                // file.close(); 
            //}
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
            LogHandler::verbose(_TAG, "save TCodeVersionEnum: %i", (int)doc["TCodeVersion"]);
            LogHandler::verbose(_TAG, "save ssid: %s", (const char*) doc["ssid"]);
            // LogHandler::verbose(_TAG, "save wifiPass: %s", (const char*) doc["wifiPass"]);
            LogHandler::verbose(_TAG, "save udpServerPort: %i", (int)doc["udpServerPort"]);
            LogHandler::verbose(_TAG, "save webServerPort: %i", (int)doc["webServerPort"]);
            LogHandler::verbose(_TAG, "save hostname: %s", (const char*) doc["hostname"]);
            LogHandler::verbose(_TAG, "save friendlyName: %s", (const char*) doc["friendlyName"]);
            // LogHandler::verbose(_TAG, "save xMin: %i", (int)doc["xMin"]);
            // LogHandler::verbose(_TAG, "save xMax: %i", (int)doc["xMax"]);
            // LogHandler::verbose(_TAG, "save yRollMin: %i", (int)doc["yRollMin"]);
            // LogHandler::verbose(_TAG, "save yRollMax: %i", (int)doc["yRollMax"]);
            // LogHandler::verbose(_TAG, "save xRollMin: %i", (int)doc["xRollMin"]);
            // LogHandler::verbose(_TAG, "save xRollMax: %i", (int)doc["xRollMax"]);
            // LogHandler::verbose(_TAG, "save speed: %i", (int)doc["speed"]);
            LogHandler::verbose(_TAG, "save pitchFrequencyIsDifferent ", (bool)doc["pitchFrequencyIsDifferent"]);
            LogHandler::verbose(_TAG, "save servoFrequency: %i", (int)doc["servoFrequency"]);
            LogHandler::verbose(_TAG, "save  pitchFrequency: %i", (int)doc["pitchFrequency"]);
            LogHandler::verbose(_TAG, "save valveFrequency: %i",(int)doc["valveFrequency"]);
            LogHandler::verbose(_TAG, "save twistFrequency: %i", (int)doc["twistFrequency"]);
            LogHandler::verbose(_TAG, "save continuousTwist: %i", (bool)doc["continuousTwist"]);
            LogHandler::verbose(_TAG, "save feedbackTwist: %i", (bool)doc["feedbackTwist"]);
            LogHandler::verbose(_TAG, "save analogTwist: %i", (bool)doc["analogTwist"]);
            LogHandler::verbose(_TAG, "save TwistFeedBack_PIN: %i", (int)doc["TwistFeedBack_PIN"]);
            LogHandler::verbose(_TAG, "save RightServo_PIN: %i", (int)doc["RightServo_PIN"]);
            LogHandler::verbose(_TAG, "save LeftServo_PIN: %i", (int)doc["LeftServo_PIN"]);
            LogHandler::verbose(_TAG, "save RightUpperServo_PIN: %i",(int)doc["RightUpperServo_PIN"]);
            LogHandler::verbose(_TAG, "save LeftUpperServo_PIN: %i", (int)doc["LeftUpperServo_PIN"]);
            LogHandler::verbose(_TAG, "save PitchLeftServo_PIN: %i", (int)doc["PitchLeftServo_PIN"]);
            LogHandler::verbose(_TAG, "save PitchRightServo_PIN: %i", (int)doc["PitchRightServo_PIN"]);
            LogHandler::verbose(_TAG, "save ValveServo_PIN: %i", (int)doc["ValveServo_PIN"]);
            LogHandler::verbose(_TAG, "save TwistServo_PIN: %i", (int)doc["TwistServo_PIN"]);
            LogHandler::verbose(_TAG, "save Vibe0_PIN: %i", (int)doc["Vibe0_PIN"]);
            LogHandler::verbose(_TAG, "save Vibe1_PIN: %i", (int)doc["Vibe1_PIN"]);
            LogHandler::verbose(_TAG, "save Lube_Pin: %i", (int)doc["Lube_Pin"]);
            LogHandler::verbose(_TAG, "save LubeManual_Pin: %i", (int)doc["LubeManual_Pin"]);
            LogHandler::verbose(_TAG, "save staticIP: %i", (bool)doc["staticIP"]);
            LogHandler::verbose(_TAG, "save localIP: %s", (const char*) doc["localIP"]);
            LogHandler::verbose(_TAG, "save gateway: %s", (const char*) doc["gateway"]);
            LogHandler::verbose(_TAG, "save subnet: %s", (const char*) doc["subnet"]);
            LogHandler::verbose(_TAG, "save dns1: %s", (const char*) doc["dns1"]);
            LogHandler::verbose(_TAG, "save dns2: %s", (const char*) doc["dns2"]);
            LogHandler::verbose(_TAG, "save sr6Mode: %i", (bool)doc["sr6Mode"]);
            LogHandler::verbose(_TAG, "save RightServo_ZERO: %i", (int)doc["RightServo_ZERO"]);
            LogHandler::verbose(_TAG, "save LeftServo_ZERO: %i", (int)doc["LeftServo_ZERO"]);
            LogHandler::verbose(_TAG, "save RightUpperServo_ZERO: %i", (int)doc["RightUpperServo_ZERO"]);
            LogHandler::verbose(_TAG, "save LeftUpperServo_ZERO: %i", (int)doc["LeftUpperServo_ZERO"]);
            LogHandler::verbose(_TAG, "save PitchLeftServo_ZERO: %i", (int)doc["PitchLeftServo_ZERO"]);
            LogHandler::verbose(_TAG, "save PitchRightServo_ZERO: %i", (int)doc["PitchRightServo_ZERO"]);
            LogHandler::verbose(_TAG, "save TwistServo_ZERO: %i", (int)doc["TwistServo_ZERO"]);
            LogHandler::verbose(_TAG, "save ValveServo_ZERO: %i", (int)doc["ValveServo_ZERO"]);
            LogHandler::verbose(_TAG, "save autoValve: %i", (bool)doc["autoValve"]);
            LogHandler::verbose(_TAG, "save inverseValve: %i", (bool)doc["inverseValve"]);
            LogHandler::verbose(_TAG, "save valveServo90Degrees: %i", (bool)doc["valveServo90Degrees"]);
            LogHandler::verbose(_TAG, "save inverseStroke: %i", (bool)doc["inverseStroke"]);
            LogHandler::verbose(_TAG, "save inversePitch: %i", (bool)doc["inversePitch"]);
            LogHandler::verbose(_TAG, "save lubeEnabled: %i", (bool)doc["lubeEnabled"]);
            LogHandler::verbose(_TAG, "save lubeAmount: %i", (int)doc["lubeAmount"]);
            LogHandler::verbose(_TAG, "save Temp_PIN: %i", (int)doc["Temp_PIN"]);
            LogHandler::verbose(_TAG, "save Heater_PIN: %i", (int)doc["Heater_PIN"]);
            LogHandler::verbose(_TAG, "save displayEnabled: %i", (bool)doc["displayEnabled"]);
            LogHandler::verbose(_TAG, "save sleeveTempDisplayed: %i", (bool)doc["sleeveTempDisplayed"]);
            LogHandler::verbose(_TAG, "save tempControlEnabled: %i", (bool)doc["tempControlEnabled"]);
            LogHandler::verbose(_TAG, "save Display_Screen_Width: %i", (int)doc["Display_Screen_Width"]);
            LogHandler::verbose(_TAG, "save Display_Screen_Height: %i", (int)doc["Display_Screen_Height"]);
            LogHandler::verbose(_TAG, "save TargetTemp: %i", (int)doc["TargetTemp"]);
            LogHandler::verbose(_TAG, "save HeatPWM: %i", (int)doc["HeatPWM"]);
            LogHandler::verbose(_TAG, "save HoldPWM: %i", (int)doc["HoldPWM"]);
            LogHandler::verbose(_TAG, "save Display_I2C_Address: %i", (int)doc["Display_I2C_Address"]);
            LogHandler::verbose(_TAG, "save Display_Rst_PIN: %i", (int)doc["Display_Rst_PIN"]);
            LogHandler::verbose(_TAG, "save WarmUpTime: %ld", (long)doc["WarmUpTime"]);
            LogHandler::verbose(_TAG, "save heaterFailsafeTime: %ld", (long)doc["heaterFailsafeTime"]);
            LogHandler::verbose(_TAG, "save heaterFailsafeThreshold: %i", (int)doc["heaterFailsafeThreshold"]);
            LogHandler::verbose(_TAG, "save heaterResolution: %i", (int)doc["heaterResolution"]);
            LogHandler::verbose(_TAG, "save heaterFrequency: %i", (int)doc["heaterFrequency"]);
            LogHandler::verbose(_TAG, "save newtoungeHatExists: %i", (bool)doc["newtoungeHatExists"]);
            LogHandler::verbose(_TAG, "save logLevel: %i", (int)doc["logLevel"]);
            
        }

        static void LogUpdateDebug() 
        {
            LogHandler::verbose(_TAG, "update TCodeVersionEnum: %i", TCodeVersionEnum);
            LogHandler::verbose(_TAG, "update ssid: %s", ssid);
            //LogHandler::verbose(_TAG, "update wifiPass: %s", wifiPass);
            LogHandler::verbose(_TAG, "update udpServerPort: %i", udpServerPort);
            LogHandler::verbose(_TAG, "update webServerPort: %i", webServerPort);
            LogHandler::verbose(_TAG, "update hostname: %s", hostname);
            LogHandler::verbose(_TAG, "update friendlyName: %s", friendlyName);
            // LogHandler::verbose(_TAG, "update xMax: %i", StrokeMax);
            // LogHandler::verbose(_TAG, "update yRollMin: %i", RollMin);
            // LogHandler::verbose(_TAG, "update yRollMax: %i", RollMax);
            // LogHandler::verbose(_TAG, "update xRollMin: %i", PitchMin);
            // LogHandler::verbose(_TAG, "update xRollMax: %i", PitchMax);
            // LogHandler::verbose(_TAG, "update speed: %i", speed);
            LogHandler::verbose(_TAG, "update pitchFrequencyIsDifferent: %i", pitchFrequencyIsDifferent);
            LogHandler::verbose(_TAG, "update servoFrequency: %i", servoFrequency);
            LogHandler::verbose(_TAG, "update pitchFrequency: %i", pitchFrequency);
            LogHandler::verbose(_TAG, "update valveFrequency: %i", valveFrequency);
            LogHandler::verbose(_TAG, "update twistFrequency: %i", twistFrequency);
            LogHandler::verbose(_TAG, "update continuousTwist: %i", continuousTwist);
            LogHandler::verbose(_TAG, "update feedbackTwist: %i", feedbackTwist);
            LogHandler::verbose(_TAG, "update analogTwist: %i", analogTwist);
            LogHandler::verbose(_TAG, "update TwistFeedBack_PIN: %i", TwistFeedBack_PIN);
            LogHandler::verbose(_TAG, "update RightServo_PIN: %i", RightServo_PIN);
            LogHandler::verbose(_TAG, "update LeftServo_PIN: %i", LeftServo_PIN);
            LogHandler::verbose(_TAG, "update RightUpperServo_PIN: %i", RightUpperServo_PIN);
            LogHandler::verbose(_TAG, "update LeftUpperServo_PIN: %i", LeftUpperServo_PIN);
            LogHandler::verbose(_TAG, "update PitchLeftServo_PIN: %i", PitchLeftServo_PIN);
            LogHandler::verbose(_TAG, "update PitchRightServo_PIN: %i", PitchRightServo_PIN);
            LogHandler::verbose(_TAG, "update ValveServo_PIN: %i", ValveServo_PIN);
            LogHandler::verbose(_TAG, "update TwistServo_PIN: %i", TwistServo_PIN);
            LogHandler::verbose(_TAG, "update Vibe0_PIN: %i", Vibe0_PIN);
            LogHandler::verbose(_TAG, "update Vibe1_PIN: %i", Vibe1_PIN);
            LogHandler::verbose(_TAG, "update LubeManual_PIN: %i", LubeManual_PIN);
            LogHandler::verbose(_TAG, "update staticIP: %i", staticIP);
            LogHandler::verbose(_TAG, "update localIP: %s", localIP);
            LogHandler::verbose(_TAG, "update gateway: %s", gateway);
            LogHandler::verbose(_TAG, "update subnet: %s", subnet);
            LogHandler::verbose(_TAG, "update dns1: %s", dns1);
            LogHandler::verbose(_TAG, "update dns2: %s", dns2);
            LogHandler::verbose(_TAG, "update sr6Mode: %i", sr6Mode);
            LogHandler::verbose(_TAG, "update RightServo_ZERO: %i", RightServo_ZERO);
            LogHandler::verbose(_TAG, "update LeftServo_ZERO: %i", LeftServo_ZERO);
            LogHandler::verbose(_TAG, "update RightUpperServo_ZERO: %i", RightUpperServo_ZERO);
            LogHandler::verbose(_TAG, "update LeftUpperServo_ZERO: %i", LeftUpperServo_ZERO);
            LogHandler::verbose(_TAG, "update PitchLeftServo_ZERO: %i", PitchLeftServo_ZERO);
            LogHandler::verbose(_TAG, "update PitchRightServo_ZERO: %i", PitchRightServo_ZERO);
            LogHandler::verbose(_TAG, "update TwistServo_ZERO: %i", TwistServo_ZERO);
            LogHandler::verbose(_TAG, "update ValveServo_ZERO: %i", ValveServo_ZERO);
            LogHandler::verbose(_TAG, "update autoValve: %i", autoValve);
            LogHandler::verbose(_TAG, "update inverseValve: %i", inverseValve);
            LogHandler::verbose(_TAG, "update valveServo90Degrees: %i", valveServo90Degrees);
            LogHandler::verbose(_TAG, "update inverseStroke: %i", inverseStroke);
            LogHandler::verbose(_TAG, "update inversePitch: %i", inversePitch);
            LogHandler::verbose(_TAG, "update lubeEnabled: %i", lubeEnabled);
            LogHandler::verbose(_TAG, "update lubeAmount: %i", lubeAmount);
            LogHandler::verbose(_TAG, "update displayEnabled: %i", displayEnabled);
            LogHandler::verbose(_TAG, "update sleeveTempDisplayed: %i", sleeveTempDisplayed);
            LogHandler::verbose(_TAG, "update tempControlEnabled: %i", tempControlEnabled);
            LogHandler::verbose(_TAG, "update Display_Screen_Width: %i", Display_Screen_Width);
            LogHandler::verbose(_TAG, "update Display_Screen_Height: %i", Display_Screen_Height);
            LogHandler::verbose(_TAG, "update TargetTemp: %i", TargetTemp);
            LogHandler::verbose(_TAG, "update HeatPWM: %i", HeatPWM);
            LogHandler::verbose(_TAG, "update HoldPWM: %i", HoldPWM);
            LogHandler::verbose(_TAG, "update Display_I2C_Address: %i", Display_I2C_Address);
            LogHandler::verbose(_TAG, "update Display_Rst_PIN: %i", Display_Rst_PIN);
            LogHandler::verbose(_TAG, "update Temp_PIN: %i", Temp_PIN);
            LogHandler::verbose(_TAG, "update Heater_PIN: %i", Heater_PIN); 
            LogHandler::verbose(_TAG, "update WarmUpTime: %ld", WarmUpTime);
            LogHandler::verbose(_TAG, "update heaterFailsafeTime: %ld", heaterFailsafeTime);
            LogHandler::verbose(_TAG, "update heaterThreshold: %d", heaterThreshold);
            LogHandler::verbose(_TAG, "update heaterResolution: %i", heaterResolution);
            LogHandler::verbose(_TAG, "update heaterFrequency: %i", heaterFrequency);
            LogHandler::verbose(_TAG, "update newtoungeHatExists: %i", newtoungeHatExists);
            LogHandler::verbose(_TAG, "update logLevel: %i", (int)logLevel);
            
        }
};
bool SettingsHandler::saving = false;
bool SettingsHandler::fullBuild = false;

const char* SettingsHandler::_TAG = "_SETTINGS_HANDLER";
String SettingsHandler::TCodeVersionName;
TCodeVersion SettingsHandler::TCodeVersionEnum;
const char SettingsHandler::ESP32Version[14] = "ESP32 v0.246b";
const char SettingsHandler::HandShakeChannel[4] = "D1\n";
const char SettingsHandler::SettingsChannel[4] = "D2\n";
const char* SettingsHandler::userSettingsFilePath = "/userSettings.json";
const char* SettingsHandler::logPath = "/log.json";
const char* SettingsHandler::defaultWifiPass = "YOUR PASSWORD HERE";
const char* SettingsHandler::decoyPass = "Too bad haxor!";
bool SettingsHandler::bluetoothEnabled = true;
bool SettingsHandler::restartRequired = false;
bool SettingsHandler::debug = false;
LogLevel SettingsHandler::logLevel = LogLevel::ERROR;
bool SettingsHandler::isTcp = true;
char SettingsHandler::ssid[32];
char SettingsHandler::wifiPass[63];
char SettingsHandler::hostname[63];
char SettingsHandler::friendlyName[100];
int SettingsHandler::udpServerPort;
int SettingsHandler::webServerPort;
int SettingsHandler::PitchRightServo_PIN = 14;
int SettingsHandler::RightUpperServo_PIN = 12;
int SettingsHandler::RightServo_PIN = 13;
int SettingsHandler::PitchLeftServo_PIN = 4;
int SettingsHandler::LeftUpperServo_PIN = 2;
int SettingsHandler::LeftServo_PIN = 15;
int SettingsHandler::ValveServo_PIN = 28;
int SettingsHandler::TwistServo_PIN = 27;
int SettingsHandler::TwistFeedBack_PIN = 26;
int SettingsHandler::Vibe0_PIN = 18;
int SettingsHandler::Vibe1_PIN = 19;
int SettingsHandler::LubeManual_PIN = 23;
int SettingsHandler::Temp_PIN = 5; 
int SettingsHandler::Heater_PIN = 33;
int SettingsHandler::FIRMWARE_MODE_PIN = 39;

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
int SettingsHandler::servoFrequency;
int SettingsHandler::pitchFrequency;
int SettingsHandler::valveFrequency;
int SettingsHandler::twistFrequency;
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
bool SettingsHandler::autoValve = false;
bool SettingsHandler::inverseValve = false;
bool SettingsHandler::valveServo90Degrees = false;
bool SettingsHandler::inverseStroke = false;
bool SettingsHandler::inversePitch = false;
int SettingsHandler::lubeAmount = 255;

bool SettingsHandler::displayEnabled = false;
bool SettingsHandler::sleeveTempDisplayed = false;
bool SettingsHandler::tempControlEnabled = false;
int SettingsHandler::Display_Screen_Width = 128; 
int SettingsHandler::Display_Screen_Height = 64; 
int SettingsHandler::TargetTemp = 40;
int SettingsHandler::HeatPWM = 255;
int SettingsHandler::HoldPWM = 110;
int SettingsHandler::Display_I2C_Address = 0x3C;
int SettingsHandler::Display_Rst_PIN = -1;
long SettingsHandler::WarmUpTime = 600000;
long SettingsHandler::heaterFailsafeTime = 60000;
float SettingsHandler::heaterThreshold = 5.0;
int SettingsHandler::heaterResolution = 8;
int SettingsHandler::heaterFrequency = 5000;
bool SettingsHandler::newtoungeHatExists = false;
const char* SettingsHandler::lastRebootReason;

int SettingsHandler::strokerSamples = 100;
int SettingsHandler::strokerOffset = 3276;
int SettingsHandler::strokerAmplitude = 32767;