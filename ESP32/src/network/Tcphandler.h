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
// #include <WiFiClient.h>
// #include <WiFiServer.h>
// #include <ArduinoJson.h>
// #include "SettingsHandler.h"
// #include "TagHandler.h"


// class TcpHandler 
// {
//   public:
//     void setup(int localPort) 
//     {
// 	    m_server.begin(localPort);
//         Serial.println("UDP Listening");
//         initialized = true;
//     }

// 	void CommandCallback(const char* in) { //This overwrites the callback for message return
// 		if(initialized && _lastConnectedPort > 0) {
// 			LogHandler::debug(_TAG, "Sending udp to client: %s", in);
// 			m_server.beginPacket(_lastConnectedIP, _lastConnectedPort);
// 			int i = 0;
// 			while (in[i] != 0)
// 				m_server.write((uint8_t)in[i++]);
// 			m_server.endPacket();
// 		}
// 	}

//     void read(char* buf) 
//     {
//   		WiFiClient client = m_server.available();
// 		if (!initialized || !client || !client.connected()) 
// 		{
// 			buf[0] = {0};
// 			return;
// 		}
// 		if (!initialized) 
// 		{
// 			buf[0] = {0};
// 			return;
// 		}
//         // if(xQueueReceive(m_TCodeQueue, buf, 0)) {
//         //     //LogHandler::verbose(_TAG, "Recieve tcode: %s", buf);
//         // } else {
//         //     //LogHandler::error(_TAG, "Failed to read from queue");
//         //     buf[0] = {0};
// 		// 	return;
//         // }
// // 		// if there's data available, read a packet
// 		int packetSize = m_server.parsePacket();
// 		if (!packetSize) 
// 		{
// 			buf[0] = {0};
// 			return;
// 		}
// 		_lastConnectedPort = m_server.remotePort();
// 		_lastConnectedIP = m_server.remoteIP();
// // //          Serial.print("Received packet of size ");
// // //          Serial.println(packetSize);
// // //          Serial.print("From ");
// // //          Serial.print(_lastConnectedIP);
// // //          Serial.print(", port ");
// // //          Serial.println(_lastConnectedPort);
	
// 		// read the packet into packetBufffer
// 		int len = m_server.read(packetBuffer, MAX_COMMAND);
// 		if (len > 0) 
// 		{
// 			packetBuffer[len] = 0;
// 			//LogHandler::verbose(_TAG, "Udp in: %s", packetBuffer);
// 		}
// 		if (m_tcodeVersion >= TCodeVersion::v0_3 && (strpbrk(packetBuffer, "$") != nullptr || strpbrk(packetBuffer, "#") != nullptr)) 
// 		{
// 			// strcpy(buf, packetBuffer);
// 			LogHandler::debug(_TAG, "System command received: %s", buf);
// 			CommandCallback("OK");
// 		// } else if (strpbrk(packetBuffer, jsonIdentifier) != nullptr) {
// 		// 	SettingsHandler::getProcessTCodeJson()(udpData, packetBuffer);
// 		// 	//LogHandler::verbose(_TAG, "json processed: %s", udpData);
// 		} 
// 		else 
// 		{
// 			//udpData[strlen(packetBuffer) + 1];
// 			strncpy(buf, packetBuffer, len);
// 			//LogHandler::verbose(_TAG, "Udp tcode in: %s", udpData);
// 		}
//     }
    
//   private: 
//     const char* _TAG = TagHandler::TcpHandler;
// 	WiFiServer m_server;
// 	TCodeVersion m_tcodeVersion;
// 	IPAddress _lastConnectedIP;
// 	int _lastConnectedPort = 0;
//     bool initialized = false;
//     char packetBuffer[MAX_COMMAND]; //buffer to hold incoming packet
//     char jsonIdentifier[2] = "{";
// };
