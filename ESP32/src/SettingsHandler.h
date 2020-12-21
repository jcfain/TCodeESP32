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

#include <SPIFFS.h>
#include <ArduinoJson.h>


class SettingsHandler 
{
  public:
        const static char TCodeVersion[11];
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
		static int PitchServo_PIN;
		static int ValveServo_PIN;
		static int TwistServo_PIN;
		static int Vibe0_PIN;
		static int Vibe1_PIN;
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

        static void load() 
        {
            const char* filename = "/userSettings.json";
            DynamicJsonDocument doc(readCapacity);
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
				PitchServo_PIN = json["PitchServo_PIN"];
				ValveServo_PIN = json["ValveServo_PIN"];
				TwistServo_PIN = json["TwistServo_PIN"];
				Vibe0_PIN = json["Vibe0_PIN"];
				Vibe1_PIN = json["Vibe1_PIN"];
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
            DynamicJsonDocument doc(saveCapacity);


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
			doc["PitchServo_PIN"] = PitchServo_PIN;
			doc["ValveServo_PIN"] = ValveServo_PIN;
			doc["TwistServo_PIN"] = TwistServo_PIN;
			doc["Vibe0_PIN"] = Vibe0_PIN;
			doc["Vibe1_PIN"] = Vibe1_PIN;
			doc["staticIP"] = staticIP;
			doc["localIP"] = localIP;
			doc["gateway"] = gateway;
			doc["subnet"] = subnet;
			doc["dns1"] = dns1;
			doc["dns2"] = dns2;

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
        static const size_t readCapacity = JSON_OBJECT_SIZE(40) + 666;
        static const size_t saveCapacity = JSON_OBJECT_SIZE(40);

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
            Serial.print("save PitchServo_PIN ");
            Serial.println((int)doc["PitchServo_PIN"]);
            Serial.print("save ValveServo_PIN ");
            Serial.println((int)doc["ValveServo_PIN"]);
            Serial.print("save TwistServo_PIN ");
            Serial.println((int)doc["TwistServo_PIN"]);
            Serial.print("save Vibe0_PIN ");
            Serial.println((int)doc["Vibe0_PIN"]);
            Serial.print("save Vibe1_PIN ");
            Serial.println((int)doc["Vibe1_PIN"]);
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
            Serial.print("update PitchServo_PIN ");
            Serial.println(PitchServo_PIN);
            Serial.print("update ValveServo_PIN ");
            Serial.println(ValveServo_PIN);
            Serial.print("update TwistServo_PIN ");
            Serial.println(TwistServo_PIN);
            Serial.print("update Vibe0_PIN ");
            Serial.println(Vibe0_PIN);
            Serial.print("update Vibe1_PIN ");
            Serial.println(Vibe1_PIN);
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
        }
};


const char SettingsHandler::TCodeVersion[11] = "TCode v0.2";
const char SettingsHandler::HandShakeChannel[4] = "D1\n";
char SettingsHandler::ssid[32];
char SettingsHandler::wifiPass[63];
char SettingsHandler::hostname[63];
char SettingsHandler::friendlyName[100];
int SettingsHandler::udpServerPort;
int SettingsHandler::webServerPort;
int SettingsHandler::TwistFeedBack_PIN = 27;
int SettingsHandler::RightServo_PIN = 12;
int SettingsHandler::LeftServo_PIN = 13;
int SettingsHandler::PitchServo_PIN = 14;
int SettingsHandler::ValveServo_PIN = 15;
int SettingsHandler::TwistServo_PIN = 2;
int SettingsHandler::Vibe0_PIN = 22;
int SettingsHandler::Vibe1_PIN = 23;
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
