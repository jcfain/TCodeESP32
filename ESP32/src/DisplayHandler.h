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
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#pragma once

class DisplayHandler
{
	IPAddress _ipAddress;
	bool _wifiConnected = false;
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
	Adafruit_SSD1306 display;

	public:
	DisplayHandler() : 
		oneWire(Temp_PIN), 
		sensors(&oneWire),
		display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1)
	{

	}

	void setup() 
	{
    	Serial.println(F("Setting up display"));
		pinMode(HeatLED_PIN, OUTPUT);
		pinMode(Heater_PIN,OUTPUT);

		// Serial.begin(115200);
		// delay(1000);

		//Wire.begin();
		//Wire.setClock(400000L);


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

	void setLocalWifiStatus(bool status)
	{
		_wifiConnected = status;
	}

	void clearDisplay()
	{
		if(displayConnected)
		{
			display.clearDisplay();
		}
	}

	void loop() 
	{
		if(displayConnected)
		{
			int tempValue = sensors.getTempCByIndex(0);
			sensors.requestTemperatures();

			display.setCursor(0,0);
			if(!_wifiConnected)
			{
				display.println("AP mode");
				display.println();
				display.println("SSID: TCodeESP32Setup");
				display.println("IP: 192.168.1.1");
			}
			else
			{
				display.print("IP:");
				display.println(_ipAddress);
			}
			
			//Display Temperature
			display.setCursor(0,40);
			display.println("Sleeve temp:");
			display.setCursor(3,50);
			if (tempValue > 0) 
			{
				display.print(tempValue,1);
				display.print("C ");
			}
			// Serial.print("Sleeve temp: ");
			// Serial.println(tempValue);

			// Serial.print("OLED write error: ");
			// Serial.println(oled.getWriteError());
			//Temperature Controls
			display.setCursor(13,50);
			if (tempValue < TargetTemp && tempValue > 0) 
			{
				//digitalWrite(HeatLED_PIN, 255);
				ledcWrite(Heater_PIN, HeatPWM);
				display.println("HEAT");
			} 
			else if (tempValue >= TargetTemp && tempValue <= TargetTemp) 
			{
				//digitalWrite(HeatLED_PIN, HoldPWM);
				ledcWrite(Heater_PIN, HoldPWM);
				display.println("HOLD");
			} 
			else if (tempValue < 0) 
			{
				display.setCursor(3,50);
				//digitalWrite(HeatLED_PIN, HoldPWM);
				ledcWrite(Heater_PIN, 0);
				display.println("Error reading temp");
			} 
			else 
			{
				//digitalWrite(HeatLED_PIN, 0);
				ledcWrite(Heater_PIN, 0);
				display.println("COOL");
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