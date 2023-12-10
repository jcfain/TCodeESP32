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


#include <Arduino.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include "SettingsHandler.h"
#include "LogHandler.h"
#include "TagHandler.h"


class Udphandler 
{
  public:
    void setup(int localPort) 
    {
		LogHandler::info(_TAG, "Starting UDP");
		wifiUdp.begin(localPort);
        LogHandler::info(_TAG, "UDP Listening");
		udpInitialized = true;
    }

	void CommandCallback(const char* in){ //This overwrites the callback for message return
		if(udpInitialized && _lastConnectedPort > 0) {
			LogHandler::info(_TAG, "Sending udp to client: %s", in);
			wifiUdp.beginPacket(_lastConnectedIP, _lastConnectedPort);
			int i = 0;
			while (in[i] != 0)
				wifiUdp.write((uint8_t)in[i++]);
			wifiUdp.endPacket();
		}
	}

    void read(char* udpData) 
    {
		if (!udpInitialized) {
			udpData[0] = {0};
			return;
		}
		// if there's data available, read a packet
		int packetSize = wifiUdp.parsePacket();
		if (!packetSize) {
			udpData[0] = {0};
			return;
		}
		_lastConnectedPort = wifiUdp.remotePort();
		_lastConnectedIP = wifiUdp.remoteIP();
//          Serial.print("Received packet of size ");
//          Serial.println(packetSize);
//          Serial.print("From ");
//          IPAddress remoteIp = Udp.remoteIP();
//          Serial.print(remoteIp);
//          Serial.print(", port ");
//          Serial.println(Udp.remotePort());
	
		// read the packet into packetBufffer
		int len = wifiUdp.read(packetBuffer, packetSize);
		if (len > 0) {
			packetBuffer[len] = 0;
			//LogHandler::verbose(_TAG, "Udp in: %s", packetBuffer);
		}
		if (SettingsHandler::TCodeVersionEnum >= TCodeVersion::v0_3 && (strpbrk(packetBuffer, "$") != nullptr || strpbrk(packetBuffer, "#") != nullptr)) {
			strcpy(udpData, packetBuffer);
			LogHandler::info(_TAG, "System command received: %s", udpData);
			CommandCallback("OK");
		} else if (strpbrk(packetBuffer, jsonIdentifier) != nullptr) {
			SettingsHandler::processTCodeJson(udpData, packetBuffer);
			//LogHandler::verbose(_TAG, "json processed: %s", udpData);
		} else {
			//udpData[strlen(packetBuffer) + 1];
			strcpy(udpData, packetBuffer);
			//LogHandler::verbose(_TAG, "Udp tcode in: %s", udpData);
		}
    }
    
  private: 
    const char* _TAG = TagHandler::UdpHandler;
    WiFiUDP wifiUdp;
	IPAddress _lastConnectedIP;
	int _lastConnectedPort = 0;
    bool udpInitialized = false;
    char packetBuffer[255];; //buffer to hold incoming packet
    char jsonIdentifier[2] = "{";
};
