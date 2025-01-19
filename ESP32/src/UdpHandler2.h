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
#include <AsyncUDP.h>
#include <ArduinoJson.h>
#include "SettingsHandler.h"
#include "logging/LogHandler.h"
#include "TagHandler.h"


class Udphandler
{
  public:
    bool setup(int localPort) 
    {
		LogHandler::info(_TAG, "Starting UDP on port: %ld", localPort);
		if(!m_udp.listen(localPort)) 
		{
        	LogHandler::error(_TAG, "UDP Error Listening");
			return false;
		}
        LogHandler::info(_TAG, "UDP2 Listening");
    	SettingsFactory* m_settingsFactory = SettingsFactory::getInstance();
		m_tcodeVersion = m_settingsFactory->getTcodeVersion();
		m_udp.onPacket(udpCallback, static_cast<void*>(this));
		//m_udp.onPacket(udpCallback2);
        m_TCodeQueue = xQueueCreate(25, sizeof(char[MAX_COMMAND]));
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
	
	static void udpCallback(void * arg, AsyncUDPPacket& packet) 
	{
		Udphandler* udp = static_cast<Udphandler*>(arg);
		//LogHandler::verbose(udp->_TAG, "UDP recieve: %s", packet.data());
		udp->_lastConnectedPort = packet.remotePort();
		udp->_lastConnectedIP = packet.remoteIP();
		udp->packetBuffer[0] = {0};
		
		memcpy(udp->packetBuffer, packet.data(), packet.length());
		//size_t len = packet.readBytes(udp->packetBuffer, sizeof(packetBuffer));
		udp->packetBuffer[packet.length()] = '\0';
		if(xQueueSend(udp->m_TCodeQueue, udp->packetBuffer, 0) != pdTRUE)
			LogHandler::error(udp->_TAG, "UDP queue full");
	}

	void CommandCallback(const char* in) 
	{ //This overwrites the callback for message return
		if(udpInitialized && _lastConnectedPort > 0) {
			LogHandler::debug(_TAG, "Sending udp to client: %s", in);
			int i = 0;
			AsyncUDPMessage message;
			while (in[i] != 0)
				message.write((uint8_t)in[i++]);
			m_udp.sendTo(message, _lastConnectedIP, _lastConnectedPort);
			//m_udp.endPacket();
		}
	}

    void read(char* buf) 
    {
		if (!udpInitialized) 
		{
			buf[0] = {0};
			return;
		}
        if(xQueueReceive(m_TCodeQueue, buf, 0)) {
            //LogHandler::verbose(_TAG, "Recieve tcode: %s", buf);
        } else {
            //LogHandler::error(_TAG, "Failed to read from queue");
            buf[0] = {0};
			return;
        }
    }
    
  private: 
    const char* _TAG = TagHandler::UdpHandler;
	TCodeVersion m_tcodeVersion;
    TaskHandle_t m_task;
    QueueHandle_t m_TCodeQueue;
	
    AsyncUDP m_udp;
	IPAddress _lastConnectedIP;
	int _lastConnectedPort = 0;
    bool udpInitialized = false;
    char packetBuffer[MAX_COMMAND] = {0}; //buffer to hold incoming packet
    char jsonIdentifier[2] = "{";
};
