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


#include <Arduino.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>


class Udphandler 
{
  public:
    void setup(int localPort) 
    {
		LogHandler::info("UDP-setup", "Starting UDP");
		wifiUdp.begin(localPort);
        LogHandler::info(_TAG, "UDP Listening");
		udpInitialized = true;
    }

	void CommandCallback(const char* in){ //This overwrites the callback for message return
		if(udpInitialized) {
			LogHandler::info(_TAG, "Sending udp: %s", in);
			wifiUdp.beginPacket(wifiUdp.remoteIP(), wifiUdp.remotePort());
			int i = 0;
			while (in[i] != 0)
				wifiUdp.write((uint8_t)in[i++]);
			wifiUdp.endPacket();
		}
	}


    void read(char* udpData) 
    {
		if (!udpInitialized) 
		{
			udpData[0] = {0};
			return;
		}
		// if there's data available, read a packet
		int packetSize = wifiUdp.parsePacket();
		if (packetSize) 
		{
//          Serial.print("Received packet of size ");
//          Serial.println(packetSize);
//          Serial.print("From ");
//          IPAddress remoteIp = Udp.remoteIP();
//          Serial.print(remoteIp);
//          Serial.print(", port ");
//          Serial.println(Udp.remotePort());
      
			// read the packet into packetBufffer
			int len = wifiUdp.read(packetBuffer, 255);
			if (len > 0) 
			{
				packetBuffer[len] = 0;
			}
			if (SettingsHandler::TCodeVersionEnum >= TCodeVersion::v0_3 && strpbrk(packetBuffer, "$") != nullptr) 
			{
				CommandCallback("OK");
				strcpy(udpData, packetBuffer);
				LogHandler::info(_TAG, "Settings save received: %s", udpData);
				return;
			} 
			else if (strpbrk(packetBuffer, jsonIdentifier) != nullptr) 
			{
				LogHandler::verbose(_TAG, "json recieved: %s", udpData);
				SettingsHandler::processTCodeJson(udpData, packetBuffer);
				return;
			} 
			//udpData[strlen(packetBuffer) + 1];
			strcpy(udpData, packetBuffer);
            LogHandler::verbose(_TAG, "Udp tcode in: %s", udpData);
			// Serial.print("tcode: ");
			// Serial.println(udpData);
			return;
		}
      	udpData[0] = {0};
    }
    
  private: 
    const char* _TAG = "UDP";
    WiFiUDP wifiUdp;
    bool udpInitialized = false;
    char packetBuffer[255];; //buffer to hold incoming packet
    char jsonIdentifier[2] = "{";
};
