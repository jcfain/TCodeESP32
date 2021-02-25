/* MIT License

Copyright (c) 2020 Jason C. Fain

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


class SettingsHandler 
{
  public:
        const static char TCodeVersion[11];
        const static char TCodeESP32Version[14];
        const static char HandShakeChannel[4];
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
		static int Lube_Pin;
        static int xMin;
        static int xMax;
        static int yRollMin;
        static int yRollMax;
        static int xRollMin;
        static int xRollMax;
        static int speed;
        static int servoFrequency;
		static bool continousTwist;
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
		static int lubeAmount;
		static bool displayEnabled;
		static bool sleeveTempEnabled;
		static bool tempControlEnabled;
		static int Display_Screen_Width; 
		static int Display_Screen_Height; 
		static int Temp_PIN; 
		static int Heater_PIN;
		static int HeatLED_PIN;
		static int TargetTemp;
		static int HeatPWM;
		static int HoldPWM;
		static int Display_I2C_Address;
		static int Display_Rst_PIN;
		static int WarmUpTime;

        static void load() 
        {
            const char* filename = "/userSettings.json";
            DynamicJsonDocument doc(deserialize);
            File file = SPIFFS.open(filename, "r");
            DeserializationError error = deserializeJson(doc, file);
            if (error)
                Serial.println(F("Failed to read settings file, using default configuration"));
            update(doc.as<JsonObject>());
        }

        static bool update(JsonObject json) 
        {
            if(json.size() > 0) 
            {
                const char* ssidConst = json["ssid"];
                if( ssid != nullptr) 
                {
                    strcpy(ssid, ssidConst);
                }
                const char* wifiPassConst = json["wifiPass"];
                if( wifiPass != nullptr) 
                {
                    strcpy(wifiPass, wifiPassConst);
                }
                udpServerPort = json["udpServerPort"] | 8000;
                webServerPort = json["webServerPort"] | 80;
                const char* hostnameTemp = json["hostname"];
                if (hostnameTemp != nullptr)
                    strcpy(hostname, hostnameTemp);
                const char* friendlyNameTemp = json["friendlyName"];
                if (friendlyNameTemp != nullptr)
                    strcpy(friendlyName, friendlyNameTemp);

                xMin = json["xMin"] | 1;
                xMax = json["xMax"] | 1000;
                yRollMin = json["yRollMin"] | 1;
                yRollMax = json["yRollMax"] | 1000;
                xRollMin = json["xRollMin"] | 1;
                xRollMax = json["xRollMax"] | 1000;
                speed = json["speed"] | 1000;
                servoFrequency = json["servoFrequency"] | 50;
				continousTwist = json["continousTwist"];
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
				Lube_Pin = json["Lube_Pin"];
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
				lubeAmount = json["lubeAmount"] | 255;
				displayEnabled = json["displayEnabled"];
				sleeveTempEnabled = json["sleeveTempEnabled"];
				tempControlEnabled = json["tempControlEnabled"];
				Display_Screen_Width = json["Display_Screen_Width"] | 128;
				Display_Screen_Height = json["Display_Screen_Height"] | 64;
				Temp_PIN = json["Temp_PIN"] | 5;
				Heater_PIN = json["Heater_PIN"] | 33;
				HeatLED_PIN = json["HeatLED_PIN"] | 32;
				TargetTemp = json["TargetTemp"] | 40;
				HeatPWM = json["HeatPWM"] | 255;
				HoldPWM = json["HoldPWM"] | 110;
                const char* Display_I2C_AddressTemp = json["Display_I2C_Address"];
                if (Display_I2C_AddressTemp != nullptr)
					Display_I2C_Address = (int)strtol(Display_I2C_AddressTemp, NULL, 0);
				Display_Rst_PIN = json["Display_Rst_PIN"] | -1;
				WarmUpTime = json["WarmUpTime"] | 600000;

                //LogUpdateDebug();

                return true;
            } 
            return false;
        }

        //Untested
        static bool parse(const char* jsonInput) 
        {
            DynamicJsonDocument doc(readCapacity);
            DeserializationError error = deserializeJson(doc, jsonInput, sizeof(jsonInput));
            if (error) 
            {
                Serial.println(F("Failed to read settings file, using default configuration"));
                return false;
            }
            update(doc.as<JsonObject>());
            return true;
        }

        static bool save() 
        {
            const char* filename = "/userSettings.json";
            // Delete existing file, otherwise the configuration is appended to the file
            SPIFFS.remove(filename);

            File file = SPIFFS.open(filename, FILE_WRITE);
            if (!file) 
            {
                Serial.println(F("Failed to create file"));
                return false;
            }

            // Allocate a temporary JsonDocument
            DynamicJsonDocument doc(serialize);


            doc["ssid"] = ssid;
            doc["wifiPass"] = wifiPass;
            doc["udpServerPort"] = udpServerPort;
            doc["webServerPort"] = webServerPort;
            doc["hostname"] =  hostname;
            doc["friendlyName"] = friendlyName;
            doc["xMin"] = xMin;
            doc["xMax"] = xMax;
            doc["yRollMin"] = yRollMin;
            doc["yRollMax"] = yRollMax;
            doc["xRollMin"] = xRollMin;
            doc["xRollMax"] = xRollMax;
            doc["speed"] = speed;
            doc["servoFrequency"] = servoFrequency;
			doc["continousTwist"] = continousTwist;
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
			doc["Lube_Pin"] = Lube_Pin;
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
			doc["lubeAmount"] = lubeAmount;
			doc["displayEnabled"] = displayEnabled;
			doc["sleeveTempEnabled"] = sleeveTempEnabled;
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
			doc["HeatLED_PIN"] = Display_Rst_PIN;
			doc["WarmUpTime"] = WarmUpTime;
			

            //LogSaveDebug(doc);

            if (serializeJson(doc, file) == 0) 
            {
                Serial.println(F("Failed to write to file"));
                return false;
            }

            file.close();
            return true;
        }

        static int calculateRange(const char* channel, int value) 
        {
            return constrain(value, getchannelMin(channel), getchannelMax(channel));
        }
        
    private:
        //static char* filename = "/userSettings.json";
        // Use http://arduinojson.org/assistant to compute the capacity.
        static const size_t readCapacity = JSON_OBJECT_SIZE(100) + 2000;
        static const size_t saveCapacity = JSON_OBJECT_SIZE(100);
		static const int deserialize = 2048;
		static const int serialize = 1536;


        //TODO: Need to think this out better.
        static int getchannelMin(const char* channel) 
        {
            if (strcmp(channel, "L0") == 0) 
            {
                return xMin;
            } 
            else if (strcmp(channel, "R1") == 0) 
            {
                return yRollMin;
            } 
            else if (strcmp(channel, "R2") == 0) 
            {
                return xRollMin;
            }
            return 1;
        }

        static int getchannelMax(const char* channel) 
        {
            if (strcmp(channel, "L0") == 0) 
            {
                return xMax;
            } 
            else if (strcmp(channel, "R1") == 0) 
            {
                return yRollMax;
            } 
            else if (strcmp(channel, "R2") == 0) 
            {
                return xRollMax;
            }
            return 1000;
        }

        static void LogSaveDebug(DynamicJsonDocument doc) 
        {
            Serial.print("save ssid ");
            Serial.println((const char*) doc["ssid"]);
            Serial.print("save wifiPass ");
            Serial.println((const char*) doc["wifiPass"]);
            Serial.print("save udpServerPort ");
            Serial.println((int)doc["udpServerPort"]);
            Serial.print("save webServerPort ");
            Serial.println((int)doc["webServerPort"]);
            Serial.print("save hostname ");
            Serial.println((const char*) doc["hostname"]);
            Serial.print("save friendlyName ");
            Serial.println((const char*) doc["friendlyName"]);
            Serial.print("save xMin ");
            Serial.println((int)doc["xMin"]);
            Serial.print("save xMax ");
            Serial.println((int)doc["xMax"]);
            Serial.print("save yRollMin ");
            Serial.println((int)doc["yRollMin"]);
            Serial.print("save yRollMax ");
            Serial.println((int)doc["yRollMax"]);
            Serial.print("save xRollMin ");
            Serial.println((int)doc["xRollMin"]);
            Serial.print("save xRollMax ");
            Serial.println((int)doc["xRollMax"]);
            Serial.print("save speed ");
            Serial.println((int)doc["speed"]);
            Serial.print("save servoFrequency ");
            Serial.println((int)doc["servoFrequency"]);
            Serial.print("save continousTwist ");
            Serial.println((bool)doc["continousTwist"]);
            Serial.print("save TwistFeedBack_PIN ");
            Serial.println((int)doc["TwistFeedBack_PIN"]);
            Serial.print("save RightServo_PIN ");
            Serial.println((int)doc["RightServo_PIN"]);
            Serial.print("save LeftServo_PIN ");
            Serial.println((int)doc["LeftServo_PIN"]);
            Serial.print("save RightUpperServo_PIN ");
            Serial.println((int)doc["RightUpperServo_PIN"]);
            Serial.print("save LeftUpperServo_PIN ");
            Serial.println((int)doc["LeftUpperServo_PIN"]);
            Serial.print("save PitchLeftServo_PIN ");
            Serial.println((int)doc["PitchLeftServo_PIN"]);
            Serial.print("save PitchRightServo_PIN ");
            Serial.println((int)doc["PitchRightServo_PIN"]);
            Serial.print("save ValveServo_PIN ");
            Serial.println((int)doc["ValveServo_PIN"]);
            Serial.print("save TwistServo_PIN ");
            Serial.println((int)doc["TwistServo_PIN"]);
            Serial.print("save Vibe0_PIN ");
            Serial.println((int)doc["Vibe0_PIN"]);
            Serial.print("save Vibe1_PIN ");
            Serial.println((int)doc["Vibe1_PIN"]);
            Serial.print("save Lube_Pin ");
            Serial.println((int)doc["Lube_Pin"]);
            Serial.print("save staticIP ");
            Serial.println((bool)doc["staticIP"]);
            Serial.print("save localIP ");
            Serial.println((const char*) doc["localIP"]);
            Serial.print("save gateway ");
            Serial.println((const char*) doc["gateway"]);
            Serial.print("save subnet ");
            Serial.println((const char*) doc["subnet"]);
            Serial.print("save dns1 ");
            Serial.println((const char*) doc["dns1"]);
            Serial.print("save dns2 ");
            Serial.println((const char*) doc["dns2"]);
            Serial.print("save sr6Mode ");
            Serial.println((bool)doc["sr6Mode"]);
            Serial.print("save RightServo_ZERO ");
            Serial.println((int)doc["RightServo_ZERO"]);
            Serial.print("save LeftServo_ZERO ");
            Serial.println((int)doc["LeftServo_ZERO"]);
            Serial.print("save RightUpperServo_ZERO ");
            Serial.println((int)doc["RightUpperServo_ZERO"]);
            Serial.print("save LeftUpperServo_ZERO ");
            Serial.println((int)doc["LeftUpperServo_ZERO"]);
            Serial.print("save PitchLeftServo_ZERO ");
            Serial.println((int)doc["PitchLeftServo_ZERO"]);
            Serial.print("save PitchRightServo_ZERO ");
            Serial.println((int)doc["PitchRightServo_ZERO"]);
            Serial.print("save TwistServo_ZERO ");
            Serial.println((int)doc["TwistServo_ZERO"]);
            Serial.print("save ValveServo_ZERO ");
            Serial.println((int)doc["ValveServo_ZERO"]);
            Serial.print("save autoValve ");
            Serial.println((bool)doc["autoValve"]);
            Serial.print("save inverseValve ");
            Serial.println((bool)doc["inverseValve"]);
            Serial.print("save lubeAmount ");
            Serial.println((int)doc["lubeAmount"]);
            Serial.print("save Temp_PIN ");
            Serial.println((int)doc["Temp_PIN"]);
            Serial.print("save Heater_PIN ");
            Serial.println((int)doc["Heater_PIN"]);
            Serial.print("save HeatLED_PIN ");
            Serial.println((int)doc["HeatLED_PIN"]);
            Serial.print("save displayEnabled ");
            Serial.println((bool)doc["displayEnabled"]);
            Serial.print("save tempControlEnabled ");
            Serial.println((bool)doc["tempControlEnabled"]);
            Serial.print("save tempControlEnabled ");
            Serial.println((bool)doc["tempControlEnabled"]);
            Serial.print("save Display_Screen_Width ");
            Serial.println((int)doc["Display_Screen_Width"]);
            Serial.print("save Display_Screen_Height ");
            Serial.println((int)doc["Display_Screen_Height"]);
            Serial.print("save TargetTemp ");
            Serial.println((int)doc["TargetTemp"]);
            Serial.print("save HeatPWM ");
            Serial.println((int)doc["HeatPWM"]);
            Serial.print("save HoldPWM ");
            Serial.println((int)doc["HoldPWM"]);
            Serial.print("save Display_I2C_Address ");
            Serial.println((int)doc["Display_I2C_Address"]);
            Serial.print("save Display_Rst_PIN ");
            Serial.println((int)doc["Display_Rst_PIN"]);
            Serial.print("save WarmUpTime ");
            Serial.println((int)doc["WarmUpTime"]);
        }

        static void LogUpdateDebug() 
        {
            Serial.print("update ssid ");
            Serial.println(ssid);
            Serial.print("update wifiPass ");
            Serial.println(wifiPass);
            Serial.print("update udpServerPort ");
            Serial.println(udpServerPort);
            Serial.print("update webServerPort ");
            Serial.println(webServerPort);
            Serial.print("update hostname ");
            Serial.println(hostname);
            Serial.print("update friendlyName ");
            Serial.println(friendlyName);
            Serial.print("update xMax ");
            Serial.println(xMax);
            Serial.print("update yRollMin ");
            Serial.println(yRollMin);
            Serial.print("update yRollMax ");
            Serial.println(yRollMax);
            Serial.print("update xRollMin ");
            Serial.println(xRollMin);
            Serial.print("update xRollMax ");
            Serial.println(xRollMax);
            Serial.print("update speed ");
            Serial.println(speed);
            Serial.print("update servoFrequency ");
            Serial.println(servoFrequency);
            Serial.print("update continousTwist ");
            Serial.println(continousTwist);
            Serial.print("update TwistFeedBack_PIN ");
            Serial.println(TwistFeedBack_PIN);
            Serial.print("update RightServo_PIN ");
            Serial.println(RightServo_PIN);
            Serial.print("update LeftServo_PIN ");
            Serial.println(LeftServo_PIN);
            Serial.print("update RightUpperServo_PIN ");
            Serial.println(RightUpperServo_PIN);
            Serial.print("update LeftUpperServo_PIN ");
            Serial.println(LeftUpperServo_PIN);
            Serial.print("update PitchLeftServo_PIN ");
            Serial.println(PitchLeftServo_PIN);
            Serial.print("update PitchRightServo_PIN ");
            Serial.println(PitchRightServo_PIN);
            Serial.print("update ValveServo_PIN ");
            Serial.println(ValveServo_PIN);
            Serial.print("update TwistServo_PIN ");
            Serial.println(TwistServo_PIN);
            Serial.print("update Vibe0_PIN ");
            Serial.println(Vibe0_PIN);
            Serial.print("update Vibe1_PIN ");
            Serial.println(Vibe1_PIN);
            Serial.print("update Lube_Pin ");
            Serial.println(Lube_Pin);
            Serial.print("update staticIP ");
            Serial.println(staticIP);
            Serial.print("update localIP ");
            Serial.println(localIP);
            Serial.print("update gateway ");
            Serial.println(gateway);
            Serial.print("update subnet ");
            Serial.println(subnet);
            Serial.print("update dns1 ");
            Serial.println(dns1);
            Serial.print("update dns2 ");
            Serial.println(dns2);
            Serial.print("update sr6Mode ");
            Serial.println(sr6Mode);
            Serial.print("update RightServo_ZERO ");
            Serial.println(RightServo_ZERO);
            Serial.print("update LeftServo_ZERO ");
            Serial.println(LeftServo_ZERO);
            Serial.print("update RightUpperServo_ZERO ");
            Serial.println(RightUpperServo_ZERO);
            Serial.print("update LeftUpperServo_ZERO ");
            Serial.println(LeftUpperServo_ZERO);
            Serial.print("update PitchLeftServo_ZERO ");
            Serial.println(PitchLeftServo_ZERO);
            Serial.print("update PitchRightServo_ZERO ");
            Serial.println(PitchRightServo_ZERO);
            Serial.print("update TwistServo_ZERO ");
            Serial.println(TwistServo_ZERO);
            Serial.print("update ValveServo_ZERO ");
            Serial.println(ValveServo_ZERO);
            Serial.print("update autoValve ");
            Serial.println(autoValve);
            Serial.print("update inverseValve ");
            Serial.println(inverseValve);
            Serial.print("update lubeAmount ");
            Serial.println(lubeAmount);

            Serial.print("update displayEnabled ");
            Serial.println(displayEnabled);
            Serial.print("update sleeveTempEnabled ");
            Serial.println(sleeveTempEnabled);
            Serial.print("update Display_Screen_Width ");
            Serial.println(Display_Screen_Width);
            Serial.print("update Display_Screen_Height ");
            Serial.println(Display_Screen_Height);
            Serial.print("update TargetTemp ");
            Serial.println(TargetTemp);
            Serial.print("update HeatPWM ");
            Serial.println(HeatPWM);
            Serial.print("update HoldPWM ");
            Serial.println(HoldPWM);
            Serial.print("update Display_I2C_Address ");
            Serial.println(Display_I2C_Address);
            Serial.print("update Display_Rst_PIN ");
            Serial.println(Display_Rst_PIN);

            Serial.print("update Temp_PIN ");
            Serial.println(Temp_PIN);
            Serial.print("update Heater_PIN ");
            Serial.println(Heater_PIN);
            Serial.print("update HeatLED_PIN ");
            Serial.println(HeatLED_PIN);
            Serial.print("update WarmUpTime ");
            Serial.println(WarmUpTime);
        }
};


const char SettingsHandler::TCodeVersion[11] = "TCode v0.2";
const char SettingsHandler::TCodeESP32Version[14] = "ESP32 v0.161b";
const char SettingsHandler::HandShakeChannel[4] = "D1\n";
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
int SettingsHandler::Lube_Pin = 23;
int SettingsHandler::Temp_PIN = 5; 
int SettingsHandler::Heater_PIN = 33;
int SettingsHandler::HeatLED_PIN = 32;
// pin 25 cannot be servo. Throws error
int SettingsHandler::xMin;
int SettingsHandler::xMax;
int SettingsHandler::yRollMin;
int SettingsHandler::yRollMax;
int SettingsHandler::xRollMin;
int SettingsHandler::xRollMax;
int SettingsHandler::speed;
int SettingsHandler::servoFrequency;
bool SettingsHandler::continousTwist;
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
int SettingsHandler::lubeAmount = 255;

bool SettingsHandler::displayEnabled = false;
bool SettingsHandler::sleeveTempEnabled = false;
bool SettingsHandler::tempControlEnabled = false;
int SettingsHandler::Display_Screen_Width = 128; 
int SettingsHandler::Display_Screen_Height = 64; 
int SettingsHandler::TargetTemp = 40;
int SettingsHandler::HeatPWM = 255;
int SettingsHandler::HoldPWM = 110;
int SettingsHandler::Display_I2C_Address = 0x3C;
int SettingsHandler::Display_Rst_PIN = -1;
int SettingsHandler::WarmUpTime = 600000;