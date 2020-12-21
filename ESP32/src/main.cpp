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
#include "ServoHandler.h"
#include "WifiHandler.h"
#include "UdpHandler.h"
//#include "BluetoothHandler.h"
#include "WebHandler.h"
#include "SettingsHandler.h"
//#include "OTAHandler.h"

ServoHandler servoHandler;
WifiHandler wifi;
Udphandler udpHandler;
//BluetoothHandler bluetooth;
WebHandler webHandler;
// This has issues running with the webserver.
//OTAHandler otaHandler;
boolean apMode = false;
char udpData[255];
void setup() {

	Serial.begin(115200);
	if(!SPIFFS.begin(true))
	{
		Serial.println("An Error has occurred while mounting SPIFFS");
		return;
	}
	SettingsHandler::load();
	if (strcmp(SettingsHandler::ssid, "YOUR SSID HERE") != 0 && SettingsHandler::ssid != nullptr) {
		if (wifi.connect(SettingsHandler::ssid, SettingsHandler::wifiPass)) 
		{ 
			udpHandler.setup(SettingsHandler::udpServerPort);
			webHandler.setup(SettingsHandler::webServerPort, SettingsHandler::hostname, SettingsHandler::friendlyName);
		} 
		else 
		{
			apMode = true;
			if (wifi.startAp()) 
			{
				webHandler.setup(SettingsHandler::webServerPort, SettingsHandler::hostname, SettingsHandler::friendlyName, true);
			}
		}
	} 
	else 
	{
		apMode = true;
		if (wifi.startAp()) 
		{
			webHandler.setup(SettingsHandler::webServerPort, SettingsHandler::hostname, SettingsHandler::friendlyName, true);
		}
	}
  if (!apMode) 
  {
    //bluetooth.setup();
    //otaHandler.setup();
    servoHandler.setup(SettingsHandler::servoFrequency);
  }
}

void loop() {
	if (!apMode) 
	{
		//otaHandler.handle();
		udpHandler.read(udpData);
		if (strlen(udpData) > 0) 
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
	}
}