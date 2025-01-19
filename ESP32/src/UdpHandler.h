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

#pragma once


#include <Arduino.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include "SettingsHandler.h"
// #include "LogHandler.h"
#include "TagHandler.h"


class Udphandler 
{
  public:
    bool setup(int localPort) 
    {
		LogHandler::info(_TAG, "Starting UDP");
		if(!m_udp.begin(localPort)) {
			return false;
		}
        LogHandler::info(_TAG, "UDP Listening");
    	SettingsFactory* m_settingsFactory = SettingsFactory::getInstance();
		m_tcodeVersion = m_settingsFactory->getTcodeVersion();
        // m_TCodeQueue = xQueueCreate(25, sizeof(char[MAX_COMMAND]));
		// if(xTaskCreatePinnedToCore(
		// 	handlerTask,/* Function to implement the task */
		// 	"UDPTask", /* Name of the task */
		// 	configMINIMAL_STACK_SIZE*4,  /* Stack size in words */
		// 	static_cast<void*>(this),  /* Task input parameter */
		// 	tskIDLE_PRIORITY,  /* Priority of the task */
		// 	&m_task,  /* Task handle. */
		// 	WIFI_TASK_CORE_ID) == pdFALSE) /* Core where the task should run */
		// 	return; 
		udpInitialized = true;
		return true;
    }

    // static void handlerTask(void* arg) {
    //     Udphandler* handler = static_cast<Udphandler*>(arg);
    // 	char packetBuffer[MAX_COMMAND];; //buffer to hold incoming packet
    //     TickType_t pxPreviousWakeTime = millis();
    //     while(1) {
	// 		int packetSize = handler->m_udp.parsePacket();
    //         if(packetSize) {
	// 			int len = handler->m_udp.read(packetBuffer, MAX_COMMAND);
	// 			if (len > 0) {
	// 				packetBuffer[len] = 0;
	// 				//LogHandler::verbose(_TAG, "Udp in: %s", packetBuffer);
	// 			}
    //             LogHandler::verbose(handler->_TAG, "Recieve: %s", packetBuffer);
    //             if(xQueueSend(handler->m_TCodeQueue, packetBuffer, 0) != pdTRUE) {
    //                 //LogHandler::error(_TAG, "Failed to write to queue");
    //             }
    //         }
    //         xTaskDelayUntil(&pxPreviousWakeTime, 10/portTICK_PERIOD_MS);
    //     }
    // }

	void CommandCallback(const char* in) { //This overwrites the callback for message return
		if(udpInitialized && _lastConnectedPort > 0) {
			LogHandler::debug(_TAG, "Sending udp to client: %s", in);
			m_udp.beginPacket(_lastConnectedIP, _lastConnectedPort);
			int i = 0;
			while (in[i] != 0)
				m_udp.write((uint8_t)in[i++]);
			m_udp.endPacket();
		}
	}

    void read(char* buf) 
    {
		if (!udpInitialized) 
		{
			buf[0] = {0};
			return;
		}
        // if(xQueueReceive(m_TCodeQueue, buf, 0)) {
        //     //LogHandler::verbose(_TAG, "Recieve tcode: %s", buf);
        // } else {
        //     //LogHandler::error(_TAG, "Failed to read from queue");
        //     buf[0] = {0};
		// 	return;
        // }
// 		// if there's data available, read a packet
		int packetSize = m_udp.parsePacket();
		if (!packetSize) 
		{
			buf[0] = {0};
			return;
		}
		_lastConnectedPort = m_udp.remotePort();
		_lastConnectedIP = m_udp.remoteIP();
// //          Serial.print("Received packet of size ");
// //          Serial.println(packetSize);
// //          Serial.print("From ");
// //          Serial.print(_lastConnectedIP);
// //          Serial.print(", port ");
// //          Serial.println(_lastConnectedPort);
	
		// read the packet into packetBufffer
		int len = m_udp.read(packetBuffer, MAX_COMMAND);
		if (len > 0) 
		{
			packetBuffer[len] = 0;
			//LogHandler::verbose(_TAG, "Udp in: %s", packetBuffer);
		}
		if (m_tcodeVersion >= TCodeVersion::v0_3 && (strpbrk(packetBuffer, "$") != nullptr || strpbrk(packetBuffer, "#") != nullptr)) 
		{
			// strcpy(buf, packetBuffer);
			LogHandler::debug(_TAG, "System command received: %s", buf);
			CommandCallback("OK");
		// } else if (strpbrk(packetBuffer, jsonIdentifier) != nullptr) {
		// 	SettingsHandler::getProcessTCodeJson()(udpData, packetBuffer);
		// 	//LogHandler::verbose(_TAG, "json processed: %s", udpData);
		} 
		else 
		{
			//udpData[strlen(packetBuffer) + 1];
			strncpy(buf, packetBuffer, len);
			//LogHandler::verbose(_TAG, "Udp tcode in: %s", udpData);
		}
    }
    
  private: 
    const char* _TAG = TagHandler::UdpHandler;
	TCodeVersion m_tcodeVersion;
    //TaskHandle_t m_task;
    //QueueHandle_t m_TCodeQueue;
	
    WiFiUDP m_udp;
	IPAddress _lastConnectedIP;
	int _lastConnectedPort = 0;
    bool udpInitialized = false;
    char packetBuffer[MAX_COMMAND];; //buffer to hold incoming packet
    char jsonIdentifier[2] = "{";
};
