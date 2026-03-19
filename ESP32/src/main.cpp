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
#if DEBUG_BUILD
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
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

String serialData;
char commandTCodeData[MAX_COMMAND];
char udpData[MAX_COMMAND];
char webSocketData[MAX_COMMAND];
#if BLE_TCODE
char bleData[MAX_COMMAND];
#endif
#if BLUETOOTH_TCODE
String bluetoothData;
#endif
char movement[MAX_COMMAND];
ButtonModel *buttonCommand = 0;
bool dStopped = false;
bool tcodeV2Recieved = false;
bool bluetoothEnabled = BLUETOOTH_ENABLED_DEFAULT;
bool bleEnabled = BLE_ENABLED_DEFAULT;

void setup()
{
	benchHandler = benchHandler->getInstance();
	benchHandler->init();
	initHandler = InitHandler::getInstance();
	setupSucceeded = initHandler->init();
}

// Main loop functions/////////////////////////////////////////////////
void readTCode(String &tcode)
{
	if (initHandler->motorHandler)
	{
		initHandler->motorHandler->read(tcode);
		tcode.clear();
	}
}

void readTCode(char *tcode, int len)
{
	if (initHandler->motorHandler)
	{
		initHandler->motorHandler->read(tcode, len);
		tcode[0] = {0};
	}
}

void processButton()
{
	if (initHandler->buttonHandler)
	{
		initHandler->buttonHandler->read(buttonCommand);
		if (buttonCommand)
		{
			char command[MAX_COMMAND];
			initHandler->systemCommandHandler->process(buttonCommand, command);
			if (strlen(command) > 0)
			{
				readTCode(command, strlen(command));
			}
		}
	}
}

void getTCodeInput()
{
	if (Serial.available() > 0)
	{
		serialData = Serial.readStringUntil('\n');
		if(!serialData.isEmpty())
			serialData += '\n';
	}
	else if (serialData.length())
	{
		serialData.clear();
	}
	if (initHandler->systemCommandHandler)
	{
		initHandler->systemCommandHandler->getTCode(commandTCodeData);
	}
#if BLUETOOTH_TCODE
	if (btHandler && btHandler->isConnected() && btHandler->available() > 0)
	{
		bluetoothData = btHandler->readStringUntil('\n');
	}
#endif
#if WIFI_TCODE
	if (initHandler->webSocketHandler)
	{
		benchHandler->benchStart(1);
		initHandler->webSocketHandler->getTCode(webSocketData);
		benchHandler->benchFinish("Websocket get", 1);
	}
	if (initHandler->udpHandler)
	{
		benchHandler->benchStart(2);
		initHandler->udpHandler->read(udpData);
		benchHandler->benchFinish("Udp get", 2);
	}
#endif
#if BLE_TCODE
	if (initHandler->bleHandler)
	{
		initHandler->bleHandler->read(bleData);
	}
#endif
}

void processCommand()
{
	// Read and process tcode $ and # commands
	if (serialData.length() > 0)
	{
		if (initHandler->systemCommandHandler && initHandler->systemCommandHandler->isCommand(serialData.c_str()))
		{
			// initHandler->systemCommandHandler->process(serialData.c_str());
			readTCode(serialData);
		}
	}
#if BLUETOOTH_TCODE
	if (bluetoothData.length() > 0)
	{
		if (initHandler->systemCommandHandler && initHandler->systemCommandHandler->isCommand(bluetoothData.c_str()))
		{
			// initHandler->systemCommandHandler->process(bluetoothData.c_str());
			readTCode(bluetoothData);
		}
	}
#endif
#if WIFI_TCODE
	else if (strlen(udpData) > 0 && initHandler->systemCommandHandler && initHandler->systemCommandHandler->isCommand(udpData))
	{
		// initHandler->systemCommandHandler->process(udpData);
		readTCode(udpData, strlen(udpData));
	}
	else if (strlen(webSocketData) > 0 && initHandler->systemCommandHandler && initHandler->systemCommandHandler->isCommand(webSocketData))
	{
		// initHandler->systemCommandHandler->process(webSocketData);
		readTCode(webSocketData, strlen(webSocketData));
	}
#endif
}

void processMotionHandlerMovement()
{
	initHandler->motionHandler->getMovement(movement, MAX_COMMAND);
	if (strlen(movement) > 0)
	{
		LogHandler::verbose(TagHandler::MainLoop, "motion handler writing: %s", movement);
		readTCode(movement, strlen(movement));
	}
}

void loop()
{
	// if(setupSucceeded && SettingsHandler::getSaving()) {
	// 	initHandler->motorHandler->execute();
	// 	vTaskDelay(250/portTICK_PERIOD_MS);
	// 	return;
	// }
	// LogHandler::verbose(TagHandler::MainLoop, "Enter loop ############################################");
	tcodeV2Recieved = false;
	benchHandler->benchStart(0);
	if (SettingsHandler::restartRequired > -1 || restarting)
	{ // check the flag here to determine if a restart is required
		if (SettingsHandler::restartRequired <= 0 && !restarting)
		{
			LogHandler::info(TagHandler::Main, "Restarting ESP");
			ESP.restart();
			restarting = true;
		}
		else
		{
			LogHandler::info(TagHandler::Main, "Restarting ESP in: %ld", SettingsHandler::restartRequired);
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		SettingsHandler::restartRequired--;
	}
#if BUILD_TEMP
	else if (initHandler->temperatureHandler && initHandler->temperatureHandler->isMaxTempTriggered())
	{
		char stop[7] = "DSTOP\n";
		readTCode(stop, 7);
		LogHandler::error(TagHandler::Main, "Internal temp has reached maximum user set. Main loop disabled! Restart system to enable the loop.");
		initHandler->temperatureHandler->setFanState();
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
				else if (strlen(commandTCodeData) > 0)
				{
					LogHandler::verbose(TagHandler::MainLoop, "system command tcode writing: %s", commandTCodeData);
					readTCode(commandTCodeData, strlen(commandTCodeData));
				}
				else if (serialData.length() > 0)
				{
					LogHandler::verbose(TagHandler::MainLoop, "serial writing: %s", serialData.c_str());
					readTCode(serialData);
#if WIFI_TCODE == 1
				}
				else if (strlen(webSocketData) > 0)
				{
					LogHandler::verbose(TagHandler::MainLoop, "webSocket writing: %s", webSocketData);
					readTCode(webSocketData, strlen(webSocketData));
				}
				else if (!SettingsHandler::apMode && strlen(udpData) > 0)
				{
					benchHandler->benchStart(6);
					LogHandler::verbose(TagHandler::MainLoop, "udp writing: %s", udpData);
					readTCode(udpData, strlen(udpData));
					benchHandler->benchFinish("Udp write", 6);
#endif
#if BLE_TCODE
				}
				else if (strlen(bleData) > 0)
				{
					LogHandler::verbose(TagHandler::MainLoop, "BLE writing: %s", bleData);
					readTCode(bleData, strlen(bleData));
#endif
#if BLUETOOTH_TCODE
				}
				else if (bluetoothData.length() > 0)
				{
					LogHandler::verbose(TagHandler::MainLoop, "bluetooth writing: %s", bluetoothData.c_str());
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
				// serialData.clear();
				char stop[7] = "DSTOP\n";
				readTCode(stop, 7);
				dStopped = true;
				tcodeV2Recieved = false;
#if BLE_TCODE
				// bleData = {0};
#endif
#if BLUETOOTH_TCODE
				// bluetoothData.clear();
#endif
			}

			benchHandler->benchStart(4);
			if (initHandler->motorHandler)
				initHandler->motorHandler->execute();
			benchHandler->benchFinish("Execute", 4);

#if BUILD_TEMP
			benchHandler->benchStart(5);
			if (initHandler->temperatureHandler && initHandler->temperatureHandler->isRunning())
			{
				initHandler->temperatureHandler->setHeaterState();
				initHandler->temperatureHandler->setFanState();
			}
			benchHandler->benchFinish("Temp check", 5);
#endif
		}
	}
	if (!setupSucceeded)
	{
		LogHandler::error(TagHandler::Main, "There was an issue in setup");
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	} else {
        //xTaskDelayUntil(&pxPreviousWakeTime, 10/portTICK_PERIOD_MS);
	}

	benchHandler->benchFinish("Main loop", 0);
}
