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

#include <Wire.h>
#include <Adafruit_GFX.h>
#include "../lib/Ext/Adafruit_SSD1306_RSB.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#pragma once

class DisplayHandler
{
	private:
	bool bootTime;
	int bootTimer;
	IPAddress _ipAddress;
	bool _wifiConnected = false;
	bool _apModeConnected = false;
	int SCREEN_WIDTH = SettingsHandler::Display_Screen_Width; // OLED display width, in pixels
	int SCREEN_HEIGHT = SettingsHandler::Display_Screen_Height; // OLED display height, in pixels
	int Temp_PIN = SettingsHandler::Temp_PIN; // Temp Sensor Pin
	int Heater_PIN = SettingsHandler::Heater_PIN;   // Heater PWM
	int HeatLED_PIN = SettingsHandler::HeatLED_PIN; // Indicator LED
	int TargetTemp = SettingsHandler::TargetTemp; // Desired Temp in degC
	int HeatPWM = SettingsHandler::HeatPWM; // Heating PWM setting 0-255
	int HoldPWM = SettingsHandler::HoldPWM; // Hold heat PWM setting 0-255
	int I2C_ADDRESS = SettingsHandler::Display_I2C_Address; // Display address
	int RST_PIN = SettingsHandler::Display_Rst_PIN; // Display RST_PIN if needed
	bool displayConnected = false;
	int currentPrintLine = 0;

	OneWire oneWire;
	DallasTemperature sensors;
	Adafruit_SSD1306_RSB display;

	public:
	DisplayHandler() : 
		oneWire(Temp_PIN), 
		sensors(&oneWire),
		display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1)
	{

	}

	void setup() 
	{
		bootTime = true;
		bootTimer = millis() + 600000;
    	Serial.println(F("Setting up display"));
		pinMode(HeatLED_PIN, OUTPUT);
		pinMode(Heater_PIN, OUTPUT);

		// Serial.begin(115200);
		// delay(1000);

		// Wire.begin();
		// Wire.setClock(400000L);


		if (RST_PIN >= 0)
		{
			displayConnected = display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS, RST_PIN);
			if (!displayConnected)
    			Serial.println(F("SSD1306 RST_PIN allocation failed"));
		}
		else
		{
			displayConnected = display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS);
			if (!displayConnected)
    			Serial.println(F("SSD1306 allocation failed"));
		}
		delay(2000);
		if(displayConnected)
		{
			// oled.setFont(Adafruit5x7);
			display.clearDisplay();
  			display.setTextColor(WHITE);
			display.setTextSize(1);
		}
	}

	void setLocalIPAddress(IPAddress ipAddress)
	{
		_ipAddress = ipAddress;
	}

	void setLocalWifiConnected(bool status)
	{
		_wifiConnected = status;
	}
	void setLocalApModeConnected(bool status)
	{
		_apModeConnected = status;
	}

	void clearDisplay()
	{
		if(displayConnected)
		{
			display.clearDisplay();
		}
	}

	void loop(int8_t RSSI) 
	{
		if(displayConnected)
		{

			int bars;
			//  int bars = map(RSSI,-80,-44,1,6); // this method doesn't refelct the Bars well
			// simple if then to set the number of bars
			
			if (RSSI > -55) { 
				bars = 5;
			} else if (RSSI < -55 && RSSI > -65) {
				bars = 4;
			} else if (RSSI < -65 && RSSI > -70) {
				bars = 3;
			} else if (RSSI < -70 && RSSI > -78) {
				bars = 2;
			} else if (RSSI < -78 && RSSI > -82) {
				bars = 1;
			} else {
				bars = 0;
			}
			for (int b=0; b <= bars; b++) 
			{
				display.fillRect((SCREEN_WIDTH - 17) + (b*3), 10 - (b*2),2,b*2,WHITE); 
			}

			display.setCursor(0,3);
			if(!_wifiConnected)
			{
				display.println("AP mode");
				display.println();
				display.println("SSID: TCodeESP32Setup");
				display.println("IP: 192.168.1.1");
			}
			else
			{
				display.print("IP: ");
				display.println(_ipAddress);
			}
			display.setCursor(0,15);
			display.print(SettingsHandler::TCodeVersion);
			display.println(" Ready");
			display.setCursor(0,25);
			display.println(SettingsHandler::TCodeESP32Version);
			
			if(SettingsHandler::sleeveTempEnabled)
			{
				int tempValue = sensors.getTempCByIndex(0);
				sensors.requestTemperatures();
				//Display Temperature
				Serial.println(tempValue);
				display.setCursor(0,40);
				display.print("Sleeve temp: ");
				if (tempValue >= 0) 
				{
					display.setCursor(75,40);
					display.fillRect(75, 40, SCREEN_WIDTH - 75, 10, BLACK);
					display.print(tempValue, 1);
					display.print((char)247);
					display.print("C");
				} 
				else
				{
					display.fillRect(0, 50, SCREEN_WIDTH, 10, BLACK);
					display.setCursor(15,50);
					digitalWrite(HeatLED_PIN, 0);
					ledcWrite(Heater_PIN, 0);
					display.println("Error reading");
				}
				
				if(SettingsHandler::tempControlEnabled)
				{
					if(millis() >= bootTimer)
						bootTime = false;
					//Temperature Contro
					if (tempValue < TargetTemp && tempValue > 0 || tempValue > 0 && bootTime) 
					{
						display.fillRect(0, 50, SCREEN_WIDTH, 10, BLACK);
						display.setCursor(15,50);
						digitalWrite(HeatLED_PIN, 255);
						ledcWrite(Heater_PIN, HeatPWM);
						display.println("HEATING");
					} 
					else if (tempValue < 0) 
					{
						display.fillRect(0, 50, SCREEN_WIDTH, 10, BLACK);
						display.setCursor(15,50);
						digitalWrite(HeatLED_PIN, 0);
						ledcWrite(Heater_PIN, 0);
						display.println("Error reading");
					}
					else if ((tempValue >= TargetTemp && tempValue <= TargetTemp) ) 
					{
						display.fillRect(0, 50, SCREEN_WIDTH, 10, BLACK);
						display.setCursor(15,50);
						digitalWrite(HeatLED_PIN, HoldPWM);
						ledcWrite(Heater_PIN, HoldPWM);
						display.println("HOLDING");
					} 
					else 
					{
						display.fillRect(0, 50, SCREEN_WIDTH, 10, BLACK);
						display.setCursor(15,50);
						digitalWrite(HeatLED_PIN, 0);
						ledcWrite(Heater_PIN, 0);
						display.println("COOLING");
					}
				}
			}
			display.display();
		}
	}

	void println(String value)
	{
		if(displayConnected)
		{
			display.println(value);
			display.display();	
			//display.startvertscroll(0x04, 0x1F, true);
		}
	}

	void println(int value)
	{
		if(displayConnected)
		{
			display.println(value);
			display.display();
		}
	}

	void I2CScan() 
	{
		byte error, address;
		int nDevices;
		Serial.println("Scanning...");
		nDevices = 0;
		for(address = 1; address < 127; address++ ) 
		{
			Wire.beginTransmission(address);
			error = Wire.endTransmission();
			if (error == 0) 
			{
				Serial.print("I2C device found at address 0x");
				if (address<16) 
				{
					Serial.print("0");
				}
				Serial.println(address,HEX);
				nDevices++;
			}
			else if (error==4) 
			{
				Serial.print("Unknow error at address 0x");
				if (address<16) 
				{
					Serial.print("0");
				}
				Serial.println(address,HEX);
			}    
		}
		if (nDevices == 0) 
		{
			Serial.println("No I2C devices found\n");
		}
		else 
		{
			Serial.println("done\n");
		}
		delay(5000);  
	}
};