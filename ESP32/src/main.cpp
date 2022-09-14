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

// #define ST(A) #A
// #define STR(A) ST(A)
// #ifdef DEBUG
// #pragma message STR(DEBUG)
// #endif
// #ifdef ISAAC_NEWTONGUE_BUILD
// #pragma message STR(ISAAC_NEWTONGUE_BUILD)
// #endif
// #ifdef FULL_BUILD
// #pragma message STR(FULL_BUILD)
// #endif
// #ifdef FW_VERSION
// #pragma message STR(FW_VERSION)
// #endif

#include <Arduino.h>
#include <SPIFFS.h>
#include "LogHandler.h"
#include "SettingsHandler.h"
#include "WifiHandler.h"

#if FULL_BUILD == 1
	#include "TemperatureHandler.h"
	#include "DisplayHandler.h"
#endif

#include "BluetoothHandler.h"
#include "TCode/ServoHandler.h"

#if DEBUG_BUILD == 0//Too much memory needed with debug
	#include "TCode/v0.2/ServoHandler0_2.h"
#endif
#include "TCode/v0.3/ServoHandler0_3.h"
#include "TCode/v1.0/ServoHandler1_0.h"
#include "UdpHandler.h"
//#include "TcpHandler.h"
#include "WebHandler.h"
//#include "OTAHandler.h"
#include "BLEHandler.h"
#include "WebSocketHandler.h"

Udphandler* udpHandler = 0;
BluetoothHandler* btHandler = 0;
//TcpHandler tcpHandler;
ServoHandler* servoHandler;
WifiHandler wifi;
WebHandler webHandler;
WebSocketHandler* webSocketHandler = 0;
BLEHandler* bleHandler = new BLEHandler();

#if FULL_BUILD == 1
	DisplayHandler* displayHandler;
	TaskHandle_t temperatureTask;
	TaskHandle_t displayTask;
	TaskHandle_t animationTask;
#endif
// This has issues running with the webserver.
//OTAHandler otaHandler;
bool apMode = false;
bool setupSucceeded = false;
bool restarting = false;

char udpData[255];
char webSocketData[255];

void displayPrint(String text) {
	#if FULL_BUILD == 1
		displayHandler->println(text);
	#endif
}

void CommandCallback(const String& in) {
	log_d("main", "Enter TCode Command callback");
	if (SettingsHandler::bluetoothEnabled && btHandler->isConnected())
		btHandler->CommandCallback(in);
	if(webSocketHandler)
		webSocketHandler->CommandCallback(in);
	if(udpHandler)
		udpHandler->CommandCallback(in);
	if(Serial)
		Serial.println(in);
}

void logCallBack(const char* in, LogLevel level) {
	if(webSocketHandler)
		webSocketHandler->sendDebug(in, level);
}

void setup() 
{
	// see if we can use the onboard led for status
	//https://github.com/kriswiner/ESP32/blob/master/PWM/ledcWrite_demo_ESP32.ino
  	//digitalWrite(5, LOW);// Turn off on-board blue led

	Serial.begin(115200);
	
    LogHandler::setLogLevel(LogLevel::INFO);
	LogHandler::setMessageCallback(logCallBack);

	#if ISAAC_NEWTONGUE_BUILD == 1
		LogHandler::debug("main", "ISAAC_NEWTONGUE_BUILD");
		SettingsHandler::newtoungeHatExists = true;
	#endif
	#if FULL_BUILD == 1
		LogHandler::debug("main", "FULL_BUILD");
		SettingsHandler::fullBuild = true;
	#endif
	#if DEBUG_BUILD == 1
		LogHandler::debug("main", "DEBUG_BUILD");
        LogHandler::setLogLevel(LogLevel::DEBUG);
		SettingsHandler::debug = true;
	#endif


    // esp_log_level_set("*", ESP_LOG_VERBOSE);
	// LogHandler::debug("main", "this is verbose");
	// LogHandler::debug("main", "this is debug");
	// LogHandler::info("main", "this is info");
	// LogHandler::warning("main", "this is warning");
	// LogHandler::error("main", "this is error");

	if(!SPIFFS.begin(true))
	{
		LogHandler::error("main-setup", "An Error has occurred while mounting SPIFFS");
		setupSucceeded = false;
		return;
	}

	SettingsHandler::load();
	LogHandler::info("main-setup", "Version: %s", SettingsHandler::ESP32Version);

	if(SettingsHandler::TCodeVersionEnum == TCodeVersion::v0_3)
		servoHandler = new ServoHandler0_3();
#if DEBUG_BUILD == 0
	else if(SettingsHandler::TCodeVersionEnum == TCodeVersion::v0_2)
		servoHandler = new ServoHandler0_2();
#endif
	else {
		servoHandler = new ServoHandler1_0();
	}

	servoHandler->setMessageCallback(CommandCallback);
	
#if FULL_BUILD == 1
	if(SettingsHandler::tempControlEnabled)
	{
		TemperatureHandler::setup();
		xTaskCreatePinnedToCore(
			TemperatureHandler::startLoop,/* Function to implement the task */
			"TempTask", /* Name of the task */
			10000,  /* Stack size in words */
			NULL,  /* Task input parameter */
			1,  /* Priority of the task */
			&temperatureTask,  /* Task handle. */
			0); /* Core where the task should run */
	}
	displayHandler = new DisplayHandler();
	if(SettingsHandler::displayEnabled)
	{
		displayHandler->setup();
		if(SettingsHandler::newtoungeHatExists)
		{
			xTaskCreatePinnedToCore(
				DisplayHandler::startAnimationDontPanic,/* Function to implement the task */
				"DisplayTask", /* Name of the task */
				10000,  /* Stack size in words */
				displayHandler,  /* Task input parameter */
				1,  /* Priority of the task */
				&animationTask,  /* Task handle. */
				1); /* Core where the task should run */
		}
	}
#endif
	
	if (!SettingsHandler::bluetoothEnabled && strcmp(SettingsHandler::wifiPass, SettingsHandler::defaultWifiPass) != 0 && SettingsHandler::ssid != nullptr) 
	{
		displayPrint("Setting up wifi...");
		displayPrint("Connecting to: ");
		displayPrint(SettingsHandler::ssid);
		if (wifi.connect(SettingsHandler::ssid, SettingsHandler::wifiPass)) 
		{ 
			displayPrint("Connected IP: " + wifi.ip().toString());
#if FULL_BUILD == 1
			displayHandler->setLocalIPAddress(wifi.ip());
#endif
			displayPrint("Starting UDP");
			udpHandler = new Udphandler();
			udpHandler->setup(SettingsHandler::udpServerPort);
			displayPrint("Starting web server");
			//displayPrint(SettingsHandler::webServerPort);
			webSocketHandler = new WebSocketHandler();
			webHandler.setup(SettingsHandler::webServerPort, SettingsHandler::hostname, SettingsHandler::friendlyName, webSocketHandler);
		} 
		else 
		{
#if FULL_BUILD == 1
			displayHandler->clearDisplay();
#endif
			displayPrint("Connection failed");
			displayPrint("Starting in APMode");
			apMode = true;
			if (wifi.startAp(bleHandler)) 
			{
				displayPrint("APMode started");
				webSocketHandler = new WebSocketHandler();
				webHandler.setup(SettingsHandler::webServerPort, SettingsHandler::hostname, SettingsHandler::friendlyName, webSocketHandler, true);
			} 
			else 
			{
				displayPrint("APMode start failed");
			}
			// Causes crash loop for some reason.
			// displayPrint("Starting BLE setup");
			// bleHandler->setup();
		}
	} 
	else 
	{
		apMode = true;
		if(!SettingsHandler::bluetoothEnabled) {
			displayPrint("Starting in APMode");
			if (wifi.startAp(bleHandler)) 
			{
				displayPrint("APMode started");
				webSocketHandler = new WebSocketHandler();
				webHandler.setup(SettingsHandler::webServerPort, SettingsHandler::hostname, SettingsHandler::friendlyName, webSocketHandler, true);
			}
			else 
			{
				displayPrint("APMode start failed");
			}
		} else {
			displayPrint("Starting Bluetooth serial setup");
			btHandler = new BluetoothHandler();
			btHandler->setup();
		}
		displayPrint("Starting BLE setup");
		bleHandler->setup();
	}

    //otaHandler.setup();
	displayPrint("Setting up servos");
    servoHandler->setup(SettingsHandler::servoFrequency, SettingsHandler::pitchFrequency, SettingsHandler::valveFrequency, SettingsHandler::twistFrequency);
	setupSucceeded = true;
#if FULL_BUILD == 1
	displayHandler->clearDisplay();
	if(SettingsHandler::displayEnabled)
	{
		xTaskCreatePinnedToCore(
			DisplayHandler::startLoop,/* Function to implement the task */
			"DisplayTask", /* Name of the task */
			10000,  /* Stack size in words */
			displayHandler,  /* Task input parameter */
			1,  /* Priority of the task */
			&displayTask,  /* Task handle. */
			1); /* Core where the task should run */
	}
#endif
}

void loop() 
{
	
	if (SettingsHandler::restartRequired || restarting){  // check the flag here to determine if a restart is required
		if(!restarting) {
			Serial.printf("Restarting ESP\n\r");
			ESP.restart();
        	restarting = true;
		}
		return;
	}
	if(setupSucceeded && !SettingsHandler::saving)
	{
		//otaHandler.handle();
		if(webSocketHandler)
			webSocketHandler->getTCode(webSocketData);
		if(udpHandler)
			udpHandler->read(udpData);
		String serialData;
		if (strlen(webSocketData) > 0) 
		{
			//LogHandler::debug("main-loop", "webSocket writing: %s", webSocketData);
			servoHandler->read(webSocketData);
		}
		else if (!apMode && strlen(udpData) > 0) 
		{
			//LogHandler::debug("main-loop", "udp writing: %s", udpData);
			servoHandler->read(udpData);
		} 
		else if (Serial.available() > 0) 
		{
			serialData = Serial.readStringUntil('\n');
			servoHandler->read(serialData);
		} 
		else if (SettingsHandler::bluetoothEnabled && btHandler->isConnected() && btHandler->available() > 0) 
		{
			serialData = btHandler->readStringUntil('\n');
			servoHandler->read(serialData);
		}

		if (strlen(udpData) == 0 && strlen(webSocketData) == 0 && serialData.length() == 0) // No wifi or websocket data
		{
			servoHandler->execute();
		}
#if FULL_BUILD == 1
		if(SettingsHandler::tempControlEnabled && TemperatureHandler::isRunning()) 
		{
			if(TemperatureHandler::tempQueue != NULL) {
				TemperatureHandler::setControlStatus();
				String* receive = 0;
				if(xQueueReceive(TemperatureHandler::tempQueue, &receive, 0)) {
					if(webSocketHandler && !receive->startsWith("{"))
						webSocketHandler->sendCommand(receive->c_str());
					else if(webSocketHandler)
						webSocketHandler->sendCommand("tempStatus", receive->c_str());
				}
				if(receive)
					delete receive;
			}
		}
#endif
	}
}