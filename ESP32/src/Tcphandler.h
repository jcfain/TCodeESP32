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

// #pragma once


// #include <string>
// #include <ArduinoJson.h>
// #include "TagHandler.h"


// class TcpHandler 
// {
//   public:
//     void setup(int localPort) 
//     {
// 	    wifiTcp.begin(localPort);
//         Serial.println("UDP Listening");
//         udpInitialized = true;
//     }

//     void read(char* udpData) 
//     {
//   		WiFiClient client = wifiTcp.available();
// 		if (!udpInitialized || !client) 
// 		{
// 			udpData[0] = {0};
// 			return;
// 		}

// 		// if there's data available, read a packet
// 		int packetSize = wifiTcp.parsePacket();
// 		if (packetSize) 
// 		{
// 	//          Serial.print("Received packet of size ");
// 	//          Serial.println(packetSize);
// 	//          Serial.print("From ");
// 	//          IPAddress remoteIp = Udp.remoteIP();
// 	//          Serial.print(remoteIp);
// 	//          Serial.print(", port ");
// 	//          Serial.println(Udp.remotePort());
		
// 		// read the packet into packetBufffer
// 		packetBuffer = wifiTcp.read();
// 		if (packetBuffer.length > 0) 
// 		{
// 			packetBuffer[len] = 0;
// 		}
// 		//Serial.println("packetBuffer");
// 		//Serial.println(packetBuffer);
// 		//send a reply, to the IP address and port that sent us the packet we received
// 		if (strcmp(packetBuffer, SettingsHandler::HandShakeChannel) == 0) 
// 		{
// 			Serial.println("Handshake received");
// 			wifiUdp.beginPacket(wifiTcp.remoteIP(), wifiTcp.remotePort());
// 			int i = 0;
// 			while (SettingsHandler::TCodeVersionName[i] != 0)
// 				wifiTcp.write((uint8_t)SettingsHandler::TCodeVersionName[i++]);
// 			wifiTcp.endPacket();
// 			udpData = nullptr;
// 			return;
// 		} 
// 		else if (strpbrk(packetBuffer, jsonIdentifier) != nullptr) 
// 		{
// 			//Serial.println("json");

// 			const size_t readCapacity = JSON_ARRAY_SIZE(5) + 5*JSON_OBJECT_SIZE(2) + 100;

// 			StaticJsonDocument<readCapacity> doc;
// 			//DynamicJsonDocument doc(readCapacity);
// 			DeserializationError error = deserializeJson(doc, packetBuffer);
// 			if (error) {
// 				Serial.println("Failed to read udp jsonobject, using default configuration");
// 				udpData = nullptr;
// 				return;
// 			}
// 			JsonArray arr = doc.as<JsonArray>();
// 			char buffer[100] = "";
// 			for(JsonObject repo: arr) 
// 			{ 
// 				const char* channel = repo["Channel"];
// 				int value = repo["Value"];
// 				if(channel != nullptr && value != 0) 
// 				{
// 				if(buffer[0] == '\0') 
// 				{
// 					//Serial.println("tcode empty");
// 					strcpy(buffer, channel);
// 				} 
// 				else 
// 				{
// 					strcat(buffer, channel);
// 				}
// 				char integer_string[4];
// 				sprintf(integer_string, "%03d", SettingsHandler::calculateRange(channel, value));
// 				//pad(integer_string);
// 				//sprintf(integer_string, "%d", SettingsHandler::calculateRange(channel, value));
// 				//Serial.print("integer_string");
// 				//Serial.println(integer_string);
// 				strcat (buffer, integer_string);
// 				if (SettingsHandler::speed > 0) {
// 					char speed_string[5];
// 					sprintf(speed_string, "%04d", SettingsHandler::speed);
// 					strcat (buffer, "S");
// 					strcat (buffer, speed_string);
// 				}
// 				strcat(buffer, " ");
// 				// Serial.print("buffer");
// 				// Serial.println(buffer);
// 				}
// 			}
// 			strcat(buffer, "\n");
// 			strcpy(udpData, buffer);
// 			// Serial.print("tcode: ");
// 			// Serial.println(udpData);
// 			return;
// 			} 
// 			//udpData[strlen(packetBuffer) + 1];
// 			strcpy(udpData, packetBuffer);
// 			// Serial.print("tcode: ");
// 			// Serial.println(udpData);
// 			return;
// 		}
// 		udpData[0] = {0};
//     }
    
//   private: 
// 	WiFiServer wifiTcp;
//     bool udpInitialized = false;
//     char packetBuffer[255];; //buffer to hold incoming packet
//     char jsonIdentifier[2] = "{";
// };
