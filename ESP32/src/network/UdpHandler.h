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
#include <WiFi.h>
#include "settings/SettingsHandler.h"
#include "logging/LogHandler.h"
#include "logging/TagHandler.h"
#include "tasks/TaskHandler.h"

class UdpHandler : public TaskHandler::Task
{
public:
	UdpHandler() : Task(TaskHandler::Rates::SLOW) {}
	void setup() override
	{
		// Defer UDP initialization until WiFi/lwip is ready
		// Don't initialize here - let loop() handle it on first real call
		udpInitialized = false;
		m_TCodeQueue = xQueueCreate(25, sizeof(char[MAX_COMMAND]));
		SettingsFactory* m_settingsFactory = SettingsFactory::getInstance();
		m_tcodeVersion = m_settingsFactory->getTcodeVersion();
	}

	bool initializeUdp()
	{
		if (udpInitialized)
			return true;

		int localPort = SettingsFactory::getInstance()->getUdpServerPort();
		LogHandler::info(Tags::Udp, "Starting UDP on port: %ld", localPort);
		if (!m_udp.listen(localPort))
		{
			LogHandler::error(Tags::Udp, "UDP Error Listening");
			return false;
		}
		LogHandler::info(Tags::Udp, "UDP Listening");
		m_udp.onPacket(udpCallback, static_cast<void*>(this));
		udpInitialized = true;
		return true;
	}

	void loop() override
	{
		// Lazy-initialize UDP once WiFi/lwip is ready
		if (!udpInitialized)
		{
			// Do not call AsyncUDP until WiFi stack has been enabled.
			if (WiFi.getMode() == WIFI_OFF)
			{
				return;
			}

			// Try initialize; if it fails, retry on a later tick.
			initializeUdp();
		}

		// Drain queued UDP TCode commands and forward to motor task
		char buf[MAX_COMMAND];
		while (xQueueReceive(m_TCodeQueue, buf, 0) == pdTRUE)
		{
			extern void feedMotorCommand(const char* cmd, size_t len);
			feedMotorCommand(buf, strlen(buf));
		}
	}

	static void udpCallback(void *arg, AsyncUDPPacket &packet)
	{
		UdpHandler *udp = static_cast<UdpHandler *>(arg);
		// LogHandler::verbose(udp->Tags::Udp, "UDP recieve: %s", packet.data());
		udp->_lastConnectedPort = packet.remotePort();
		udp->_lastConnectedIP = packet.remoteIP();
		udp->packetBuffer[0] = {0};

		memcpy(udp->packetBuffer, packet.data(), packet.length());
		// size_t len = packet.readBytes(udp->packetBuffer, sizeof(packetBuffer));
		udp->packetBuffer[packet.length()] = '\0';
		if (xQueueSend(udp->m_TCodeQueue, udp->packetBuffer, 0) != pdTRUE)
			LogHandler::error(Tags::Udp, "UDP queue full");
	}

	void CommandCallback(const char *in)
	{ // This overwrites the callback for message return
		if (udpInitialized && _lastConnectedPort > 0)
		{
			LogHandler::debug(Tags::Udp, "Sending udp to client: %s", in);
			int i = 0;
			AsyncUDPMessage message;
			while (in[i] != 0)
				message.write((uint8_t)in[i++]);
			m_udp.sendTo(message, _lastConnectedIP, _lastConnectedPort);
			// m_udp.endPacket();
		}
	}

	void read(char *buf)
	{
		if (!udpInitialized)
		{
			buf[0] = {0};
			return;
		}
		if (xQueueReceive(m_TCodeQueue, buf, 0))
		{
			// LogHandler::verbose(Tags::Udp, "Recieve tcode: %s", buf);
		}
		else
		{
			// LogHandler::error(Tags::Udp, "Failed to read from queue");
			buf[0] = {0};
			return;
		}
	}

private:
	TCodeVersion m_tcodeVersion;
	QueueHandle_t m_TCodeQueue;

	AsyncUDP m_udp;
	IPAddress _lastConnectedIP;
	int _lastConnectedPort = 0;
	bool udpInitialized = false;
	char packetBuffer[MAX_COMMAND] = {0}; // buffer to hold incoming packet
	char jsonIdentifier[2] = "{";
};
