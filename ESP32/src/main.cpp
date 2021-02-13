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
#include "TemperatureHandler.h"
#include "DisplayHandler.h"
#include "ServoHandler.h"
#include "WifiHandler.h"
#include "UdpHandler.h"
//#include "BluetoothHandler.h"
#include "WebHandler.h"
//#include "OTAHandler.h"

Udphandler udpHandler;
ServoHandler servoHandler;
WifiHandler wifi;
//BluetoothHandler bluetooth;
WebHandler webHandler;
DisplayHandler* displayHandler;
TaskHandle_t temperatureTask;
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
			0,  /* Priority of the task */
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
			displayHandler->setLocalWifiConnected(true);
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
			displayHandler->setLocalWifiConnected(false);
			displayHandler->clearDisplay();
			displayHandler->println("Connection failed");
			displayHandler->println("Starting in APMode");
			apMode = true;
			if (wifi.startAp()) 
			{
				displayHandler->println("APMode started");
				displayHandler->setLocalApModeConnected(true);
				webHandler.setup(SettingsHandler::webServerPort, SettingsHandler::hostname, SettingsHandler::friendlyName, true);
			}
			else
			{
				displayHandler->setLocalApModeConnected(false);
			}
			
		}
	} 
	else 
	{
		apMode = true;
		displayHandler->setLocalWifiConnected(false);
		displayHandler->println("Starting in APMode");
		if (wifi.startAp()) 
		{
			displayHandler->println("APMode started");
			displayHandler->setLocalApModeConnected(true);
			webHandler.setup(SettingsHandler::webServerPort, SettingsHandler::hostname, SettingsHandler::friendlyName, true);
		}
		else
		{
			displayHandler->setLocalApModeConnected(false);
		}
	}
    //bluetooth.setup();
    //otaHandler.setup();
	displayHandler->println("Setting up servos");
    servoHandler.setup(SettingsHandler::servoFrequency);
	setupSucceeded = true;
	displayHandler->println("Starting system...");
	displayHandler->clearDisplay();
}

void loop() 
{
	if(setupSucceeded)
	{
		//otaHandler.handle();
		udpHandler.read(udpData);
		if (!apMode && strlen(udpData) > 0) 
		{
			// Serial.print("web writing: ");
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
		// else if (bluetooth.available() > 0) 
		// {
		// 	servoHandler.read(bluetooth.read());
		// }
		if (strlen(udpData) == 0) 
		{
			servoHandler.execute();
		} 
		if(SettingsHandler::displayEnabled)
		{
			displayHandler->loop(wifi.RSSI());
		}
	}
}