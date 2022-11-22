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

#pragma once

#include <Wire.h>
#include <Adafruit_GFX.h>
#include "../lib/Ext/Adafruit_SSD1306_RSB.h"
#include "SettingsHandler.h"
#include "WifiHandler.h"
#if TEMP_ENABLED == 1
#include "TemperatureHandler.h"
#endif

// #if ISAAC_NEWTONGUE_BUILD == 1
// #include "../lib/animationFrames.h"
// #endif

class DisplayHandler
{
public:

	DisplayHandler() : display(SettingsHandler::Display_Screen_Width,  SettingsHandler::Display_Screen_Height, &Wire, SettingsHandler::Display_Rst_PIN, 100000UL, 100000UL) { }

	void setup() 
	{
    	LogHandler::info("displayHandler", "Setting up display");

		// Serial.begin(115200);
		// delay(1000);

		//Wire.begin();
		//Wire.setClock(100000UL);

		if (SettingsHandler::Display_Rst_PIN >= 0)
		{
			displayConnected = display.begin(SSD1306_SWITCHCAPVCC, SettingsHandler::Display_I2C_Address, SettingsHandler::Display_Rst_PIN);
			if (!displayConnected)
    			LogHandler::error("displayHandler", "SSD1306 RST_PIN allocation failed");
		}
		else
		{
			displayConnected = display.begin(SSD1306_SWITCHCAPVCC, SettingsHandler::Display_I2C_Address);
			if (!displayConnected)
    			LogHandler::error("displayHandler", "SSD1306 allocation failed");
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

	void clearDisplay()
	{
		if(displayConnected)
		{
			display.clearDisplay();
		}
	}

	static void startLoop(void* displayHandlerRef)
	{
		((DisplayHandler*)displayHandlerRef)->loop();
	}

	bool isRunning()
	{
		return _isRunning;
	}
	void stopRunning()
	{
		_isRunning = false;
	}

	void loop()
	{
		_isRunning = true;
		clearDisplay();
		while(_isRunning) 
		{
			if(!m_animationPlaying && displayConnected && millis() >= lastUpdate + nextUpdate)
			{
				lastUpdate = millis();
				startLine();
				// Serial.print("Display Core: ");
				// Serial.println(xPortGetCoreID());
				display.setCursor(0,currentLine);
				if(WifiHandler::isConnected())
				{
					int bars;
					//  int bars = map(RSSI,-80,-44,1,6); // this method doesn't refelct the Bars well
					// simple if then to set the number of bars
					int8_t RSSI = WifiHandler::getRSSI();
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
						display.fillRect((SettingsHandler::Display_Screen_Width - 17) + (b*3), 10 - (b*2),2,b*2,WHITE); 
					}

					display.print("IP: ");
					display.println(_ipAddress);
					display.setCursor(0,nextLine(is32() ? 0 : 3));
					display.print(SettingsHandler::TCodeVersionName);
					display.setCursor(SettingsHandler::Display_Screen_Width - (sizeof(SettingsHandler::ESP32Version) - 1) *6, currentLine);
					display.println(SettingsHandler::ESP32Version);
					
				} 
				else if(WifiHandler::apMode)
				{
					display.println("AP mode: 192.168.1.1");
					display.setCursor(0,nextLine(is32() ? 0 : 3));
					display.println("SSID: TCodeESP32Setup");
					display.setCursor(0,nextLine());
					display.print(SettingsHandler::TCodeVersionName);
					display.setCursor(SettingsHandler::Display_Screen_Width - (sizeof(SettingsHandler::ESP32Version) - 1) *6, currentLine);
					display.println(SettingsHandler::ESP32Version);
				}
				else
				{
					display.println("Wifi error");
				}

				
#if TEMP_ENABLED == 1
				if(SettingsHandler::sleeveTempDisplayed || SettingsHandler::tempInternalEnabled)
				{
					is32() ? draw32Temp() : draw64Temp();
				}
#endif
				display.display();
			}
        	vTaskDelay(1000/portTICK_PERIOD_MS);
			// Serial.print("Display task: "); // stack size used
			// Serial.print(uxTaskGetStackHighWaterMark( NULL )); // stack size used
			// Serial.println();
			// Serial.flush();
		}
		
  		vTaskDelete( NULL );
	}

	void println(String value)
	{
		if(displayConnected && !m_animationPlaying)
		{
			display.println(value);
			display.display();	
			//Serial.println(value);
			//display.startvertscroll(0x04, 0x1F, true);
		}
	}

	void println(int value)
	{
		if(displayConnected && !m_animationPlaying)
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

	// static void startAnimationDontPanic(void* displayHandlerRef) 
	// {
	// 	((DisplayHandler*)displayHandlerRef)->playBootAnimationDontPanic();
	// }

	// void playBootAnimationDontPanic() 
	// {
// #if ISAAC_NEWTONGUE_BUILD == 1
// 		if(displayConnected)
// 		{
// 			display.clearDisplay();
// 			m_animationPlaying = true;
// 			int endTime = millis() + m_animationMilliSeconds;
// 			int currentFrameIndex = 0;
// 			while(millis() < endTime) 
// 			{
// 				display.drawBitmap(0, 0, dontPanicAnimationFrames[currentFrameIndex], SettingsHandler::Display_Screen_Width, SettingsHandler::Display_Screen_Height, 1);
// 				display.display();
// 				currentFrameIndex++;
// 				if(currentFrameIndex == dontPanicAnimationFramesCount)
// 					currentFrameIndex = 0;
// 				vTaskDelay(30);
// 				display.clearDisplay();
// 			}
// 			m_animationPlaying = false;
// 		}
// #endif
  	// 	vTaskDelete( NULL );
	// }

private:
	IPAddress _ipAddress;
	bool displayConnected = false;
	int lastUpdate = 0;
	const int nextUpdate = 1000;
	bool _isRunning = false;

	int currentLine = 3;
	int lineHeight = 10;
	int charWidth = 6;

	Adafruit_SSD1306_RSB display;
	bool m_animationPlaying = false;
	int m_animationMilliSeconds = 10000;
	
	int nextLine(int additional = 0) {
		currentLine += (lineHeight + additional);
		return currentLine;
	}
	void newLine() {
		display.setCursor(0, nextLine());
	}
	void startLine() {
		if(is32()) {
			currentLine = 0;
		} else {
			currentLine = 3;
		}
	}
	void clearCurrentLine() {
		display.fillRect(0, currentLine, SettingsHandler::Display_Screen_Width, 10, BLACK);
	}
	bool is32() {
		return SettingsHandler::Display_Screen_Height == 32;
	}

	void draw64Temp() {
		if(SettingsHandler::sleeveTempDisplayed && !SettingsHandler::tempInternalEnabled)
		{
			float tempValue = TemperatureHandler::getSleeveTemp();
			display.setCursor(0,nextLine());
			char sleeveTemp[14] = "Sleeve temp: ";
			display.print(sleeveTemp);
			int width = sizeof(sleeveTemp) * charWidth;
			display.setCursor(width, currentLine);
			display.fillRect(width, currentLine, SettingsHandler::Display_Screen_Width - width, 10, BLACK);
			display.print(tempValue, 1);
			display.print((char)247);
			display.print("C");
			if(SettingsHandler::tempSleeveEnabled) {
				display.fillRect(0, nextLine(), SettingsHandler::Display_Screen_Width, 10, BLACK);
				display.setCursor(15,currentLine);
				String tempStatus = TemperatureHandler::getSleeveControlStatus();
				display.println(tempStatus);
			}
		}
		else if(!SettingsHandler::sleeveTempDisplayed && SettingsHandler::tempInternalEnabled)
		{
			float tempValue = TemperatureHandler::getInternalTemp();
			display.setCursor(0,nextLine());
			char sleeveTemp[16] = "Internal temp: ";
			display.print(sleeveTemp);
			int width = sizeof(sleeveTemp) * charWidth;
			display.setCursor(width, currentLine);
			display.fillRect(width, currentLine, SettingsHandler::Display_Screen_Width - width, 10, BLACK);
			display.print(tempValue, 1);
			display.print((char)247);
			display.print("C");
		}
		else if(SettingsHandler::sleeveTempDisplayed && SettingsHandler::tempInternalEnabled)
		{
			float sleeveTemp = TemperatureHandler::getSleeveTemp();
			int cursurPos = 0;
			display.setCursor(cursurPos,nextLine());
			char sleeveTempText[7] = "Sleeve";
			display.print(sleeveTempText);
			
			float internalTemp = TemperatureHandler::getInternalTemp();
			char internalTempText[9] = "Internal";
			cursurPos = SettingsHandler::Display_Screen_Width - ((sizeof(internalTempText) - 1) * charWidth);
			display.setCursor(cursurPos, currentLine);
			display.print(internalTempText);

			newLine();
			clearCurrentLine();
			display.print(sleeveTemp, 1);
			display.print((char)247);
			display.print("C");//Max Length 7

			cursurPos = SettingsHandler::Display_Screen_Width - 7 * charWidth;
			display.setCursor(cursurPos, currentLine);
			display.print(internalTemp, 1);
			display.print((char)247);
			display.print("C");//Max Length 7

			if(SettingsHandler::tempSleeveEnabled) {
				newLine();
				clearCurrentLine();
				String tempStatus = TemperatureHandler::getSleeveControlStatus() + " ";
				display.print(tempStatus);
			}
		}
	}
	void draw32Temp() {
		if(SettingsHandler::sleeveTempDisplayed && !SettingsHandler::tempInternalEnabled)
		{
			float sleeveTemp = TemperatureHandler::getSleeveTemp();
			int cursurPos = 0;
			display.setCursor(cursurPos, nextLine());
			clearCurrentLine();
			char sleeveTempText[9] = "Sleeve: ";
			display.print(sleeveTempText);
			cursurPos = (sizeof(sleeveTempText) - 1) * charWidth;
			display.setCursor(cursurPos, currentLine);

			display.print(sleeveTemp, 1);
			display.print((char)247);
			display.print("C");//Max Length 6
			cursurPos += 8 * charWidth;
			
			if(SettingsHandler::tempSleeveEnabled) {
				display.setCursor(cursurPos, currentLine);
				String tempStatus = TemperatureHandler::getShortSleeveControlStatus() + " ";
				display.print(tempStatus);
			}
		}
		else if(!SettingsHandler::sleeveTempDisplayed && SettingsHandler::tempInternalEnabled)
		{
			float internalTemp = TemperatureHandler::getInternalTemp();
			//Display Temperature
			//Serial.println(tempValue);
			int cursurPos = 0;
			display.setCursor(cursurPos, nextLine());
			clearCurrentLine();
			char sleeveTempText[11] = "Internal: ";
			display.print(sleeveTempText);
			cursurPos = (sizeof(sleeveTempText) - 1) * charWidth;
			display.setCursor(cursurPos, currentLine);
			display.print(internalTemp, 1);
			display.print((char)247);
			display.print("C");
		}
		else if(SettingsHandler::sleeveTempDisplayed && SettingsHandler::tempInternalEnabled)
		{
			float sleeveTemp = TemperatureHandler::getSleeveTemp();
			//Display Temperature
			//Serial.println(tempValue);
			int cursurPos = 0;
			display.setCursor(cursurPos, nextLine());
			char sleeveTempText[2] = "S";
			display.print(sleeveTempText);
			cursurPos = (sizeof(sleeveTempText) - 1) * charWidth;
			display.setCursor(cursurPos, currentLine);
			clearCurrentLine();
			display.print(sleeveTemp, 1);
			display.print((char)247);
			display.print("C");//Max Length 6
			cursurPos += 8 * charWidth;
			
			if(SettingsHandler::tempSleeveEnabled) {
				display.setCursor(cursurPos, currentLine);
				String tempStatus = TemperatureHandler::getShortSleeveControlStatus() + " ";
				display.print(tempStatus);
			}
			
			cursurPos += 3 * charWidth;
			float internalTemp = TemperatureHandler::getInternalTemp();
			char internalTempText[2] = "I";
			display.setCursor(cursurPos, currentLine);
			display.print(internalTempText);

			cursurPos += ((sizeof(internalTempText) - 1) * charWidth);
			display.setCursor(cursurPos, currentLine);
			display.print(internalTemp, 1);
			display.print((char)247);
			display.print("C");//Max Length 6
		}
	}
};
