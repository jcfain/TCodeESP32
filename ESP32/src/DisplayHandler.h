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

	DisplayHandler() : 
		display(
			SettingsHandler::Display_Screen_Width,  
			//SettingsHandler::Display_Screen_Height, 
			64,
			&Wire, 
			SettingsHandler::Display_Rst_PIN, 100000UL, 100000UL) { }

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
		if(is32()) {
			currentLine = 0;
		}
		delay(2000);
		if(displayConnected)
		{
			//display.setFont(Adafruit5x7);
			display.clearDisplay();
  			display.setTextColor(WHITE);
			display.setTextSize(1);
		}
	}

	void setLocalIPAddress(IPAddress ipAddress)
	{
		_ipAddress = ipAddress;
	}

	void setSleeveTemp(float temp) {
		m_sleeveTemp = temp;
	}
	void setInternalTemp(float temp) {
		m_internalTemp = temp;
	}
	void setHeateState(const char* state) {
		m_HeatState = String(state);
	}
	void setHeateStateShort(const char* state) {
		m_HeatStateShort = String(state);
	}
	void setFanState(const char* state) {
		m_fanState = String(state);
	}
	

	void clearDisplay()
	{
		if(displayConnected)
			display.clearDisplay();
	}

	static void startLoop(void* displayHandlerRef)
	{
		if(((DisplayHandler*)displayHandlerRef)->isConnected())
			((DisplayHandler*)displayHandlerRef)->loop();
	}

	bool isConnected() {
		return displayConnected;
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
						display.fillRect((SettingsHandler::Display_Screen_Width - 17) + (b*3), 8 - (b*2),2,b*2,WHITE); 
					}

					left("IP: "); display.print(_ipAddress);
					nextLine(is32() ? 0 : 3);
					left(SettingsHandler::TCodeVersionName.c_str());
					right(SettingsHandler::ESP32Version);
					
				} 
				else if(WifiHandler::apMode)
				{
					left("AP mode: 192.168.1.1");
					nextLine(is32() ? 0 : 3);
					left("SSID: TCodeESP32Setup");
					left(SettingsHandler::TCodeVersionName.c_str());
					right(SettingsHandler::ESP32Version);
				}
				else
				{
					display.print("Wifi error");
				}

				
#if TEMP_ENABLED == 1
				if(SettingsHandler::sleeveTempDisplayed || SettingsHandler::internalTempDisplayed)
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
	const char* _TAG = "displayHandler";
	IPAddress _ipAddress;
	bool displayConnected = false;
	int lastUpdate = 0;
	const int nextUpdate = 1000;
	bool _isRunning = false;

	int currentLine = 3;
	int lineHeight = 10;
	int charWidth = 6;

	float m_internalTemp = -127.0f;
	float m_sleeveTemp = -127.0f;
	String m_fanState = "Unknown";
	String m_HeatState = "Unknown";
	String m_HeatStateShort = "U";

	Adafruit_SSD1306_RSB display;
	bool m_animationPlaying = false;
	int m_animationMilliSeconds = 10000;
	
	int nextLine(int additionalPixels = 0) {
		int newHeight = currentLine + (lineHeight + additionalPixels);
		if(newHeight <= SettingsHandler::Display_Screen_Height - lineHeight) {
			currentLine = newHeight;
		} else {
			LogHandler::error(_TAG, "End of the display reached when nextLine!");
		}
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
	void space(int count = 1) {
		display.setCursor(display.getCursorX() + (count * charWidth), currentLine);
	}
	void right(const char* text, int margin = 0) {
		display.setCursor((SettingsHandler::Display_Screen_Width - strlen(text) * charWidth) - margin * charWidth, currentLine);
		display.print(text);
	}
	void left(const char* text, int margin = 0) {
		display.setCursor(margin * charWidth, currentLine);
		display.print(text); 
	}
	int getCurrentLineCurserPos() {
		return display.getCursorX() * charWidth;	
	}
	void clearCurrentLine() {
		display.fillRect(0, currentLine, SettingsHandler::Display_Screen_Width, 10, BLACK);
	}
	bool is32() {
		return SettingsHandler::Display_Screen_Height == 32;
	}

	void draw64Temp() {
		if(SettingsHandler::sleeveTempDisplayed && !SettingsHandler::internalTempDisplayed)
		{
			display.setCursor(0,nextLine());
			char sleeveTemp[9] = "Sleeve: ";
			display.print(sleeveTemp);
			int width = sizeof(sleeveTemp) * charWidth;
			display.setCursor(width, currentLine);
			display.fillRect(width, currentLine, SettingsHandler::Display_Screen_Width - width, 10, BLACK);
			display.print(m_sleeveTemp, 1);
			display.print((char)247);
			display.print("C");
			if(SettingsHandler::tempSleeveEnabled) {
				display.fillRect(0, nextLine(), SettingsHandler::Display_Screen_Width, 10, BLACK);
				display.setCursor(15,currentLine);
				display.println(m_HeatState);
			}
		}
		else if(!SettingsHandler::sleeveTempDisplayed && SettingsHandler::internalTempDisplayed)
		{
			display.setCursor(0,nextLine());
			char sleeveTemp[11] = "Internal: ";
			display.print(sleeveTemp);
			int width = sizeof(sleeveTemp) * charWidth;
			display.setCursor(width, currentLine);
			display.fillRect(width, currentLine, SettingsHandler::Display_Screen_Width - width, 10, BLACK);
			display.print(m_internalTemp, 1);
			display.print((char)247);
			display.print("C");
			if(SettingsHandler::fanControlEnabled) {
				display.fillRect(0, nextLine(), SettingsHandler::Display_Screen_Width, 10, BLACK);
				display.setCursor(15, currentLine);
				display.println(m_fanState);
			}
		}
		else if(SettingsHandler::sleeveTempDisplayed && SettingsHandler::internalTempDisplayed)
		{
			int cursurPos = 0;
			display.setCursor(cursurPos,nextLine());
			char sleeveTempText[7] = "Sleeve";
			display.print(sleeveTempText);
			
			char internalTempText[9] = "Internal";
			cursurPos = SettingsHandler::Display_Screen_Width - ((sizeof(internalTempText) - 1) * charWidth);
			display.setCursor(cursurPos, currentLine);
			display.print(internalTempText);

			newLine();
			clearCurrentLine();
			display.print(m_sleeveTemp, 1);
			display.print((char)247);
			display.print("C");//Max Length 7

			cursurPos = SettingsHandler::Display_Screen_Width - 7 * charWidth;
			display.setCursor(cursurPos, currentLine);
			display.print(m_internalTemp, 1);
			display.print((char)247);
			display.print("C");//Max Length 7

			if(SettingsHandler::tempSleeveEnabled) {
				newLine();
				clearCurrentLine();
				display.print(m_HeatState);
				display.print(" ");
			}
		}
	}
	void draw32Temp() {
		if(SettingsHandler::sleeveTempDisplayed && !SettingsHandler::internalTempDisplayed)
		{
			int cursurPos = 0;
			display.setCursor(cursurPos, nextLine());
			clearCurrentLine();
			char sleeveTempText[9] = "Sleeve: ";
			display.print(sleeveTempText);
			cursurPos = (sizeof(sleeveTempText) - 1) * charWidth;
			display.setCursor(cursurPos, currentLine);

			display.print(m_sleeveTemp, 1);
			display.print((char)247);
			display.print("C");//Max Length 6
			cursurPos += 8 * charWidth;
			
			if(SettingsHandler::tempSleeveEnabled) {
				display.setCursor(cursurPos, currentLine);
				display.print(m_HeatStateShort);
			}
		}
		else if(!SettingsHandler::sleeveTempDisplayed && SettingsHandler::internalTempDisplayed)
		{
			//Display Temperature
			//Serial.println(tempValue);
			int cursurPos = 0;
			display.setCursor(cursurPos, nextLine());
			clearCurrentLine();
			char sleeveTempText[11] = "Internal: ";
			display.print(sleeveTempText);
			cursurPos = (sizeof(sleeveTempText) - 1) * charWidth;
			display.setCursor(cursurPos, currentLine);
			display.print(m_internalTemp, 1);
			display.print((char)247);
			display.print("C");
		}
		else if(SettingsHandler::sleeveTempDisplayed && SettingsHandler::internalTempDisplayed)
		{
			//Display Temperature
			//Serial.println(tempValue);
			newLine();
			//char sleeveTempText[2] = "S";
			char sleeveTempText[10];
			char array[10];
			sprintf(array, "%f", m_sleeveTemp);
			int sleeveLen = strlen(array);
			snprintf(sleeveTempText, 3 + sleeveLen, "S%f%cC", m_sleeveTemp, (char)247);
			sleeveTempText[4 + sleeveLen] = {0};
			left(sleeveTempText);

			// cursurPos = (sizeof(sleeveTempText) - 1) * charWidth;
			// display.setCursor(cursurPos, currentLine);
			// clearCurrentLine();
			// display.print(sleeveTemp, 1);
			// display.print((char)247);
			// display.print("C");//Max Length 6
			// cursurPos += 8 * charWidth;
			
			if(SettingsHandler::tempSleeveEnabled) {
				//space();
				display.print("("+m_HeatStateShort+")");
			}
			space();

			char internalTempText[10];
			snprintf(internalTempText, 10, "I%f%cC", m_internalTemp, (char)247);
			internalTempText[strlen(internalTempText)] = {0};
			display.print(internalTempText);

			// cursurPos += ((sizeof(internalTempText) - 1) * charWidth);
			// display.setCursor(cursurPos, currentLine);
			// display.print(m_internalTemp, 1);
			// display.print((char)247);
			// display.print("C");//Max Length 6
		}
	}
};
