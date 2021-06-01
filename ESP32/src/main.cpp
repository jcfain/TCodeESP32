/* MIT License

Copyright (c) 2020 Jason C. Fain

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


#include <Arduino.h>
#include <SPIFFS.h>
#include "SettingsHandler.h"
#include "WifiHandler.h"
#include "TemperatureHandler.h"
#include "DisplayHandler.h"
#include "BluetoothHandler.h"
#include "ServoHandler.h"
#include "UdpHandler.h"
#include "WebHandler.h"
//#include "OTAHandler.h"
#include "BLEHandler.h"

//BluetoothHandler btHandler;
Udphandler udpHandler;
ServoHandler servoHandler;
WifiHandler wifi;
WebHandler webHandler;
BLEHandler* bleHandler = new BLEHandler();
DisplayHandler* displayHandler;
TaskHandle_t temperatureTask;
TaskHandle_t displayTask;
// This has issues running with the webserver.
//OTAHandler otaHandler;
boolean apMode = false;
boolean setupSucceeded = false;
char udpData[255];
void setup() 
{
	Serial.begin(115200);
	if(!SPIFFS.begin(true))
	{
		Serial.println("An Error has occurred while mounting SPIFFS");
		setupSucceeded = false;
		return;
	}
	SettingsHandler::load();
	if(SettingsHandler::sleeveTempEnabled)
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
	}
	
	displayHandler->println("Setting up wifi...");
	if (strcmp(SettingsHandler::ssid, "YOUR SSID HERE") != 0 && SettingsHandler::ssid != nullptr) 
	{
		displayHandler->println("Connecting to: ");
		displayHandler->println(SettingsHandler::ssid);
		if (wifi.connect(SettingsHandler::ssid, SettingsHandler::wifiPass)) 
		{ 
			displayHandler->println("Connected: ");
			displayHandler->println(wifi.ip().toString());
			displayHandler->setLocalIPAddress(wifi.ip());
			displayHandler->println("Starting UDP");
			udpHandler.setup(SettingsHandler::udpServerPort);
			displayHandler->println("Starting web server");
			//displayHandler->println(SettingsHandler::webServerPort);
			webHandler.setup(SettingsHandler::webServerPort, SettingsHandler::hostname, SettingsHandler::friendlyName);
		} 
		else 
		{
			displayHandler->clearDisplay();
			displayHandler->println("Connection failed");
			displayHandler->println("Starting in APMode");
			apMode = true;
			if (wifi.startAp(bleHandler)) 
			{
				displayHandler->println("APMode started");
				webHandler.setup(SettingsHandler::webServerPort, SettingsHandler::hostname, SettingsHandler::friendlyName, true);
			} 
			else 
			{
				displayHandler->println("APMode start failed");
			}
			// displayHandler->println("Starting BLE setup");
			// bleHandler->setup();
		}
	} 
	else 
	{
		apMode = true;
		displayHandler->println("Starting in APMode");
		if (wifi.startAp(bleHandler)) 
		{
			displayHandler->println("APMode started");
			webHandler.setup(SettingsHandler::webServerPort, SettingsHandler::hostname, SettingsHandler::friendlyName, true);
		}
		else 
		{
			displayHandler->println("APMode start failed");
		}
		displayHandler->println("Starting BLE setup");
		bleHandler->setup();
	}
	// if(SettingsHandler::bluetoothEnabled)
	// {
    // 	btHandler.setup();
	// }
    //otaHandler.setup();
	displayHandler->println("Setting up servos");
    servoHandler.setup(SettingsHandler::servoFrequency);
	setupSucceeded = true;
	displayHandler->clearDisplay();
	displayHandler->println("Starting system...");
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
	
}

void loop() 
{
	if(setupSucceeded)
	{
		//otaHandler.handle();
		udpHandler.read(udpData);
		if (!apMode && strlen(udpData) > 0) 
		{
			// Serial.print("udp writing: ");
			// Serial.println(udpData);
			for (char *c = udpData; *c; ++c) 
			{
				servoHandler.read(*c);
				servoHandler.execute();
			}
		} 
		else if (Serial.available() > 0) 
		{
			servoHandler.read(Serial.read());
		} 
		// else if (SettingsHandler::bluetoothEnabled && btHandler.isConnected() && btHandler.available() > 0) 
		// {
		// 	servoHandler.read(btHandler.read());
		// }
		if (strlen(udpData) == 0) // No wifi data
		{
			servoHandler.execute();
		} 
		if(SettingsHandler::tempControlEnabled)
			TemperatureHandler::setControlStatus();
	}
}