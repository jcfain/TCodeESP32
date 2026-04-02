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

#include "esp_idf_version.h"
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#define ESP_ARDUINO3
#endif
#include <Arduino.h>
#include <EEPROM.h>

#if PICO_BUILD
// #include <FreeRTOS.h>
#endif

#if ESP8266 == 1
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#endif
#include "constants.h"
#include "enum.h"
#include "utils.h"
#include <LittleFS.h>
#include "InitHandler.hpp"
#include "BenchHandler.hpp"

InitHandler* initHandler;
BenchHandler* benchHandler;
TickType_t pxPreviousWakeTime = millis();
SettingsFactory *settingsFactory;
// This has issues running with the webserver.
// OTAHandler otaHandler;
bool setupSucceeded = false;
bool restarting = false;

char tcodeData[MAX_COMMAND];
size_t tcodeData_len;
// char serialData[MAX_COMMAND];
size_t serialData_len;
// String serialData;
// char commandTCodeData[MAX_COMMAND];
size_t commandTCodeData_len;
// char udpData[MAX_COMMAND];
size_t udpData_len;
// char webSocketData[MAX_COMMAND];
size_t webSocketData_len;
#if BLE_TCODE
// char bleData[MAX_COMMAND];
size_t bleData_len;
#endif
#if BLUETOOTH_TCODE
// char bluetoothData[MAX_COMMAND];
size_t bluetoothData_len;
#endif
char movement[MAX_COMMAND];
size_t movement_len;
ButtonModel *buttonCommand = 0;
bool dStopped = false;
bool bluetoothEnabled = BLUETOOTH_ENABLED_DEFAULT;
bool bleEnabled = BLE_ENABLED_DEFAULT;

void setup()
{
	benchHandler = benchHandler->getInstance();
	benchHandler->init();
	// benchHandler->enable();
	initHandler = InitHandler::getInstance();
	setupSucceeded = initHandler->init();
}

// Main loop functions/////////////////////////////////////////////////
// void readTCode(String &tcode)
// {
// 	if (motorHandler)
// 	{
// 		motorHandler->read(tcode);
// 		tcode.clear();
// 	}
// }

void readTCode(char *tcode, size_t& len)
{
	if (motorHandler)
	{
		motorHandler->read(tcode, len);
		tcode[0] = {0};
		//memset(tcode, 0, len);
		len = 0;
	}
}

void processButton()
{
	if (buttonHandler)
	{
		buttonHandler->read(buttonCommand);
		if (buttonCommand)
		{
			char command[MAX_COMMAND];
			systemCommandHandler->process(buttonCommand, command);
			size_t commandLen = strnlen(command, MAX_COMMAND);
			if (commandLen > 0)
			{
				readTCode(command, commandLen);
			}
		}
	}
}

void getTCodeInput()
{
// 	serialData_len = 0;
// 	movement_len = 0;
// 	commandTCodeData_len = 0;
// 	udpData_len = 0;
// 	webSocketData_len = 0;
// 	bleData_len = 0;
// #if BLUETOOTH_TCODE
// 	bluetoothData_len = 0;
// #endif
	if (serialHandler && serialHandler->available())
	{
		serialData_len = serialHandler->read(tcodeData);
	}
	else if (systemCommandHandler && systemCommandHandler->available())
	{
		commandTCodeData_len = systemCommandHandler->read(tcodeData);
	}
#if BLUETOOTH_TCODE
	// else if (initHandler->bluetoothHandler && initHandler->bluetoothHandler->isConnected() && initHandler->bluetoothHandler->available() > 0)
	// {
	// 	size_t len = initHandler->bluetoothHandler.readBytesUntil('\n', tcodeData, MAX_COMMAND);
	// 	if(len < MAX_COMMAND) {
	// 		bluetoothData_len = len;
	// 		tcodeData[bluetoothData_len] = '\n';
	// 	}
	// }
	else if (initHandler->bluetoothHandler && initHandler->bluetoothHandler->available())
	{
		bluetoothData_len = initHandler->bluetoothHandler->read(tcodeData);
	}
#endif
#if WIFI_TCODE
	else if (webSocketHandler && webSocketHandler->available())
	{
		benchHandler->benchStart(1);
		webSocketData_len = webSocketHandler->read(tcodeData);
		benchHandler->benchFinish("Websocket get", 1);
	}
	else if (udpHandler && udpHandler->available())
	{
		benchHandler->benchStart(2);
		udpData_len = udpHandler->read(tcodeData);
		benchHandler->benchFinish("Udp get", 2);
	}
#endif
#if BLE_TCODE
	else if (bleHandler && bleHandler->available())
	{
		bleData_len = bleHandler->read(tcodeData);
	}
#endif
}

void processCommand()
{
	if(systemCommandHandler && systemCommandHandler->isCommand(tcodeData))
	{
		size_t * len = 0;
		// Read and process tcode $ and # commands
		if (serialData_len > 0)
		{
			len = &serialData_len;
		}
#if BLUETOOTH_TCODE
		else if (bluetoothData_len > 0)
		{
			len = &bluetoothData_len;
		}
#endif
#if WIFI_TCODE
		else if (udpData_len > 0)
		{
			len = &udpData_len;
		}
		else if (webSocketData_len > 0)
		{
			len = &webSocketData_len;
		}
#endif
		if(len)
		{
			readTCode(tcodeData, *len);
		}
	}
}

void processMotionHandlerMovement()
{
	motionHandler->getMovement(movement, MAX_COMMAND);
	size_t len = strlen(movement);
	if (len > 0)
	{
		LogHandler::verbose(TagHandler::MainLoop, "motion handler writing: %s", movement);
		readTCode(movement, len);
	}
}

void stop()
{
	size_t len = 7;
	char stop[len] = "DSTOP\n";
	readTCode(stop, len);
	dStopped = true;
}

void loop()
{
	// if(setupSucceeded && SettingsHandler::getSaving()) {
	// 	motorHandler->execute();
	// 	vTaskDelay(250/portTICK_PERIOD_MS);
	// 	return;
	// }
	// LogHandler::verbose(TagHandler::MainLoop, "Enter loop ############################################");
	benchHandler->benchStart(0);
	if (SettingsHandler::restartInSecs > -1 || restarting)
	{ // check the flag here to determine if a restart is required
		if (SettingsHandler::restartInSecs <= 0 && !restarting)
		{
			LogHandler::info(TagHandler::Main, "Restarting ESP");
			ESP.restart();
			restarting = true;
		}
		else
		{
			LogHandler::info(TagHandler::Main, "Restarting ESP in: %ld", SettingsHandler::restartInSecs);
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		SettingsHandler::restartInSecs--;
	}
#if BUILD_TEMP
	else if (temperatureHandler && temperatureHandler->isMaxTempTriggered())
	{
		stop();
		LogHandler::error(TagHandler::Main, "Internal temp has reached maximum user set. Main loop disabled! Restart system to enable the loop.");
		temperatureHandler->setFanState();
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
#endif
	else
	{
		if (setupSucceeded)
		{
			// otaHandler.handle();

			getTCodeInput(); // Must be executed first!

			processButton();

			processCommand();

			if (!SettingsHandler::getMotionPaused())
			{
				dStopped = false;
				benchHandler->benchStart(3);
				if (SettingsHandler::getMotionEnabled())
				{ // Motion overrides all other input
					processMotionHandlerMovement();
				}
				else if (commandTCodeData_len > 0)
				{
					LogHandler::verbose(TagHandler::MainLoop, "System command tcode received. Writing: %s, len: %u", tcodeData, commandTCodeData_len);
					readTCode(tcodeData, commandTCodeData_len);
				}
				else if (serialData_len > 0)
				{
					LogHandler::verbose(TagHandler::MainLoop, "Serial tcode received. Writing: %s, len: %u", tcodeData, serialData_len);
					readTCode(tcodeData, serialData_len);
#if WIFI_TCODE == 1
				}
				else if (webSocketData_len > 0)
				{
					LogHandler::verbose(TagHandler::MainLoop, "WebSocket tcode received. Writing: %s, len: %u", tcodeData, webSocketData_len);
					readTCode(tcodeData, webSocketData_len);
				}
				else if (!SettingsHandler::apMode && udpData_len > 0)
				{
					benchHandler->benchStart(6);
					LogHandler::verbose(TagHandler::MainLoop, "Udp tcode received. Writing: %s, len: %u", tcodeData, udpData_len);
					readTCode(tcodeData, udpData_len);
					benchHandler->benchFinish("Udp write", 6);
#endif
#if BLE_TCODE
				}
				else if (bleData_len > 0)
				{
					LogHandler::verbose(TagHandler::MainLoop, "BLE tcode received. Writing: %s, len: %u", tcodeData, bleData_len);
					readTCode(tcodeData, bleData_len);
#endif
#if BLUETOOTH_TCODE
				}
				else if (bluetoothData_len > 0)
				{
					LogHandler::verbose(TagHandler::MainLoop, "Bluetooth tcode received. Writing: %s, len: %u", tcodeData, bluetoothData_len);
					readTCode(bluetoothData);
#endif
				}
				benchHandler->benchFinish("Input check", 3);
			}
			else if (!dStopped)
			{ // All motion is paused execute stop.
				// movement[0] = {0};
				// udpData[0] = {0};
				// webSocketData[0] = {0};
				// serialData[0] = {0};
				stop();
#if BLE_TCODE
				// bleData = {0};
#endif
#if BLUETOOTH_TCODE
				// bluetoothData = {0};
#endif
			}

			benchHandler->benchStart(4);
			if (motorHandler)
				motorHandler->execute();
			benchHandler->benchFinish("Motor handler Execute", 4);

#if BUILD_TEMP
			benchHandler->benchStart(5);
			if (temperatureHandler && temperatureHandler->isRunning())
			{
				temperatureHandler->setHeaterState();
				temperatureHandler->setFanState();
			}
			benchHandler->benchFinish("Temp check", 5);
#endif
		}
	}
	if (!setupSucceeded && SettingsHandler::restartInSecs == -1 && !restarting)
	{
		LogHandler::error(TagHandler::Main, "There was an issue in setup");
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	} else {
        //xTaskDelayUntil(&pxPreviousWakeTime, 10/portTICK_PERIOD_MS);
	}

	benchHandler->benchFinish("Main loop", 0);
}
