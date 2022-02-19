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
#include "TCode/v2/ServoHandler2.h"
#include "TCode/v3/ServoHandler3.h"
#include "UdpHandler.h"
#include "TcpHandler.h"
#include "WebHandler.h"
//#include "OTAHandler.h"
#include "BLEHandler.h"
#include "WebSocketHandler.h"

//BluetoothHandler btHandler;
Udphandler udpHandler;
//TcpHandler tcpHandler;
ServoHandler2 servoHandler2;
ServoHandler3 servoHandler3;
WifiHandler wifi;
WebHandler webHandler;
WebSocketHandler webSocketHandler;
BLEHandler* bleHandler = new BLEHandler();
DisplayHandler* displayHandler;
TaskHandle_t temperatureTask;
TaskHandle_t displayTask;
TaskHandle_t animationTask;
// This has issues running with the webserver.
//OTAHandler otaHandler;
boolean apMode = false;
boolean setupSucceeded = false;
char udpData[255];
void setup() 
{
	// see if we can use the onboard led for status
	//https://github.com/kriswiner/ESP32/blob/master/PWM/ledcWrite_demo_ESP32.ino
  	//digitalWrite(5, LOW);// Turn off on-board blue led

	Serial.begin(115200);
	if(!SPIFFS.begin(true))
	{
		Serial.println("An Error has occurred while mounting SPIFFS");
		setupSucceeded = false;
		return;
	}

	int sensorValue = analogRead(SettingsHandler::FIRMWARE_MODE_PIN);
	float voltage = sensorValue * (5.0 / 1023.0);
	Serial.print("AY value:");
	Serial.println(sensorValue);
	Serial.print("AY voltage:");
	Serial.println(voltage);
	SettingsHandler::load(voltage > 1.00f); // Safe value for now. Should be around 1.8v
	Serial.println(SettingsHandler::ESP32Version);
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
			if(SettingsHandler::TCodeVersionEnum == TCodeVersion::v3)
				udpHandler.setup(SettingsHandler::udpServerPort, &servoHandler3);
			else
				udpHandler.setup(SettingsHandler::udpServerPort);
			displayHandler->println("Starting web server");
			//displayHandler->println(SettingsHandler::webServerPort);
			webHandler.setup(SettingsHandler::webServerPort, SettingsHandler::hostname, SettingsHandler::friendlyName);
			webSocketHandler.setup(webHandler.getServer());
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
				webSocketHandler.setup(webHandler.getServer());
			} 
			else 
			{
				displayHandler->println("APMode start failed");
			}
			// Causes crash loop for some reason.
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
			webSocketHandler.setup(webHandler.getServer());
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
	if(SettingsHandler::TCodeVersionEnum == TCodeVersion::v2) 
	{
    	servoHandler2.setup(SettingsHandler::servoFrequency, SettingsHandler::pitchFrequency, SettingsHandler::valveFrequency, SettingsHandler::twistFrequency);
	} 
	else 
	{
		servoHandler3.setup(SettingsHandler::servoFrequency, SettingsHandler::pitchFrequency, SettingsHandler::valveFrequency, SettingsHandler::twistFrequency);
	}
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
String bufferString = "";
// float lastVoltage = 0.00f;
// int lastSensorValue = 0;
void loop() 
{
/* 	int sensorValue = analogRead(39);
	if(lastSensorValue != sensorValue) {
		lastSensorValue = sensorValue;
		float voltage = sensorValue * (5.0 / 1023.0);
		Serial.print("AY value:");
		Serial.println(sensorValue);
		if(lastVoltage != voltage) {
			lastVoltage=voltage;
			Serial.print("AY voltage:");
			Serial.println(voltage);
		}
	} */
	if(setupSucceeded)
	{
		//otaHandler.handle();
		bufferString = webSocketHandler.getTCode();
		udpHandler.read(udpData);
		if(!bufferString.isEmpty()) 
		{
			//Serial.println("!bufferString.isEmpty(): "+bufferString);
			if(SettingsHandler::TCodeVersionEnum == TCodeVersion::v2) 
			{
				for(char& c : bufferString)
				{
					//Serial.println("c: "+String(c));
					servoHandler2.read(c);
					servoHandler2.execute();
				}
			} 
			else 
			{
				for(char& c : bufferString)
				{
					servoHandler3.read(c);
					servoHandler3.execute();
				}
			}
		}
		else if (!apMode && strlen(udpData) > 0) 
		{
			//Serial.print("udp writing: ");
			//Serial.println(udpData);
			if(SettingsHandler::TCodeVersionEnum == TCodeVersion::v2) 
			{
				for (char *c = udpData; *c; ++c) 
				{
					servoHandler2.read(*c);
					servoHandler2.execute();
				}
			} 
			else 
			{
				for (char *c = udpData; *c; ++c) 
				{
					servoHandler3.read(*c);
					servoHandler3.execute();
				}
			}
		} 
		else if (Serial.available() > 0) 
		{
			if(SettingsHandler::TCodeVersionEnum == TCodeVersion::v2) 
			{
				servoHandler2.read(Serial.read());
			} 
			else
			{
				servoHandler3.read(Serial.read());
			}
		} 
		// else if (SettingsHandler::bluetoothEnabled && btHandler.isConnected() && btHandler.available() > 0) 
		// {
		// 	servoHandler.read(btHandler.read());
		// }
		if (SettingsHandler::TCodeVersionEnum == TCodeVersion::v2 && strlen(udpData) == 0) // No wifi data
		{
			servoHandler2.execute();
		}
		else if(SettingsHandler::TCodeVersionEnum == TCodeVersion::v3 && strlen(udpData) == 0)
		{
			servoHandler3.execute();
		}
		if(SettingsHandler::tempControlEnabled && TemperatureHandler::isRunning()) 
		{
			TemperatureHandler::setControlStatus();
			String* receive = 0;
			if(xQueueReceive(TemperatureHandler::tempQueue, &receive, 0)) {
				if(!receive->startsWith("{"))
					webSocketHandler.sendCommand(receive->c_str());
				else
					webSocketHandler.sendCommand("tempStatus", receive->c_str());
			}
			if(receive)
				delete receive;
		}
	}
}