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
#include "LogHandler.h"
#include "WifiHandler.h"
#if TEMP_ENABLED
#include "TemperatureHandler.h"
#endif

// #if ISAAC_NEWTONGUE_BUILD
// #include "../lib/animationFrames.h"
// #endif

class DisplayHandler
{
public:

	DisplayHandler() : 
		display(
			SettingsHandler::Display_Screen_Width,  
			SettingsHandler::Display_Screen_Height, 
			//64,
			&Wire, 
			SettingsHandler::Display_Rst_PIN, 100000UL, 100000UL) { }

	void setup() 
	{
    	LogHandler::info("displayHandler", "Setting up display");

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
		memset(m_sleeveTempString,'\0',sizeof(m_sleeveTempString));
		if(temp < 0) {
			strcpy(m_sleeveTempString, "XX.XX\0");
		} else {   
			dtostrf(temp, sizeof(m_sleeveTempString) - 1, 2, m_sleeveTempString);
		}	
	}
	void setInternalTemp(float temp) {
		m_internalTemp = temp;
		memset(m_internalTempString,'\0',sizeof(m_internalTempString));
		if(temp < 0) {
			strcpy(m_internalTempString, "XX.XX\0");
		} else {   
			dtostrf(temp, sizeof(m_internalTempString) - 1, 2, m_internalTempString);
		}	
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
		if(displayConnected) {
			display.clearDisplay();
		}
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
		while(_isRunning) {
			if(!m_animationPlaying && displayConnected && millis() >= lastUpdate + nextUpdate) {
				lastUpdate = millis();
				clearDisplay();
				setTextSize(1);
				int headerPadding = is32() ? 0 : 3;
				// Serial.print("Display Core: ");
				// Serial.println(xPortGetCoreID());
				if(WifiHandler::isConnected()) {
					startLine(headerPadding);
					left("IP: "); display.print(_ipAddress);

					// Draw Wifi signal bars
					int barHeight = is32() ? 8 : 10;
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
					for (int b=0; b <= bars; b++) {
						display.fillRect((SettingsHandler::Display_Screen_Width - 17) + (b*3), barHeight - (b*2),2,b*2,WHITE); 
					}
					newLine(headerPadding);
					if(SettingsHandler::versionDisplayed) {
						left(SettingsHandler::TCodeVersionName.c_str());
						right(SettingsHandler::ESP32Version);
						newLine();
					}
					
				} else if(WifiHandler::apMode) {
					startLine(headerPadding);
					left("AP mode: 192.168.1.1");
					newLine(headerPadding);
					left("SSID: TCodeESP32Setup");
					newLine();
					if(is32() && SettingsHandler::versionDisplayed && !SettingsHandler::sleeveTempDisplayed && !SettingsHandler::internalTempDisplayed
						|| SettingsHandler::versionDisplayed) {
						left(SettingsHandler::TCodeVersionName.c_str());
						right(SettingsHandler::ESP32Version);
						newLine();
					}
				} else {
					display.print("Wifi error");
				}
#if TEMP_ENABLED
				if(SettingsHandler::sleeveTempDisplayed || SettingsHandler::internalTempDisplayed) {
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
// #if ISAAC_NEWTONGUE_BUILD
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

	int currentLine = 0;
	int lineCount = 0;
	int lineHeight = 10;
	int charWidth = 6;

	float m_internalTemp = -127.0f;
	float m_sleeveTemp = -127.0f;
	char m_internalTempString[7] = "XX.XX";
	char m_sleeveTempString[7] = "XX.XX";
	String m_fanState = "Unknown";
	String m_HeatState = "Unknown";
	String m_HeatStateShort = "U";

	Adafruit_SSD1306_RSB display;
	bool m_animationPlaying = false;
	int m_animationMilliSeconds = 10000;

	// Text size 1 is 6x8, 2 is 12x16, 3 is 18x24, etc
	void setTextSize(uint8_t size) {
		if(size >= 1) {
			charWidth = 6 * size;
			lineHeight = getLineHeight(size);
			display.setTextSize(size);
		}
	}
	void newLine(int additionalPixels = 0, int newLineTextSize = 0) {
		int newLine = currentLine + (lineHeight + additionalPixels);
		if(newLineTextSize > 0)
			setTextSize(newLineTextSize);
		if(newLine > SettingsHandler::Display_Screen_Height - lineHeight) {
			LogHandler::warning(_TAG, "End of the display reached when newLine! Current: %i, New: %i, Max: %i", currentLine, newLine, SettingsHandler::Display_Screen_Height - lineHeight);
		}
		currentLine = newLine;
		display.setCursor(0, currentLine);
		//clearCurrentLine();
	}
	int getLineHeight(uint8_t size) {
		int margin = 2;
		return 8 * size + margin;
	}
	void startLine(int additionalPixels = 0) {
		currentLine = (0 + additionalPixels);
	}
	bool hasNextLine(int newLineTextSize = 1) {
		
		return currentLine + getLineHeight(newLineTextSize) <= SettingsHandler::Display_Screen_Height - getLineHeight(newLineTextSize);
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
	void center(const char* text) {
		display.setCursor((SettingsHandler::Display_Screen_Width - (strlen(text) * charWidth)) / 2, currentLine);
		display.print(text); 
	}
	void clearCurrentLine() {
		display.fillRect(0, currentLine, SettingsHandler::Display_Screen_Width, 10, BLACK);
	}
	bool is32() {
		return SettingsHandler::Display_Screen_Height == 32;
	}

	void draw64Temp() {
		if(SettingsHandler::sleeveTempDisplayed && !SettingsHandler::internalTempDisplayed) {
			if(SettingsHandler::versionDisplayed) {
				char buf[16];
				getTempString("Sleeve: ", m_sleeveTempString, buf, sizeof(buf));
				left(buf);
				newLine();
				left(m_HeatState.c_str(), 3);
			} else {
				setTextSize(3);
				char buf[9];
				getTempString("", m_sleeveTempString, buf, sizeof(buf));
				left(buf);
				newLine(0, 1);
				center(m_HeatState.c_str());
			}
		} else if(!SettingsHandler::sleeveTempDisplayed && SettingsHandler::internalTempDisplayed) {
			if(SettingsHandler::versionDisplayed) {
				char buf[18];
				getTempString("Internal: ", m_internalTempString, buf, sizeof(buf));
				left(buf);
				if(SettingsHandler::fanControlEnabled) {
					newLine();
					left(m_fanState.c_str(), 3);
				}
			} else {
				setTextSize(3);
				char buf[9];
				getTempString("", m_internalTempString, buf, sizeof(buf));
				center(buf);
				newLine(0, 1);
				if(SettingsHandler::fanControlEnabled) {
					center(m_fanState.c_str());
				}
			}
		} else if(SettingsHandler::sleeveTempDisplayed && SettingsHandler::internalTempDisplayed) {
			left("Sleeve");
			right("Internal");

			newLine();

			char buf[9];
			getTempString("", m_sleeveTempString, buf, sizeof(buf));
			left(buf);
			if(SettingsHandler::versionDisplayed) {
				display.print("("+m_HeatStateShort+")");
			}
			char buf2[9];
			getTempString("", m_internalTempString, buf2, sizeof(buf2));
			right(buf2);

			if(!SettingsHandler::versionDisplayed) {
				newLine();
				
				left(m_HeatState.c_str());
				if(SettingsHandler::fanControlEnabled) {
					right(m_fanState.c_str());
				}
			}
		}
	}
	void draw32Temp() {
		if(SettingsHandler::sleeveTempDisplayed && !SettingsHandler::internalTempDisplayed) {
			char buf[19];
			if(SettingsHandler::versionDisplayed || !hasNextLine()) {
				getTempString("Sleeve: ", m_sleeveTempString, buf, sizeof(buf));
				left(buf);
				display.print("("+m_HeatStateShort+")");
			} else if(hasNextLine(2)) {
				setTextSize(2);
				getTempString("SLT: ", m_sleeveTempString, buf, sizeof(buf));
				left(buf);
				setTextSize(1);
				display.print("("+m_HeatStateShort+")");
			} else {
				getTempString("Sleeve: ", m_sleeveTempString, buf, sizeof(buf));
				left(buf);
				newLine();
				left(m_HeatState.c_str(), 3);
			} 
		} else if(!SettingsHandler::sleeveTempDisplayed && SettingsHandler::internalTempDisplayed) {
			char buf[21];
			if(SettingsHandler::versionDisplayed || !hasNextLine()) {
				getTempString("Internal: ", m_internalTempString, buf, sizeof(buf));
				left(buf);
			} else if(hasNextLine(2)) {
				setTextSize(2);
				getTempString("INT:", m_internalTempString, buf, sizeof(buf));
				left(buf);
				// setTextSize(1);
				// display.print("("+m_HeatStateShort+")");
			} else {
				getTempString("Internal: ", m_internalTempString, buf, sizeof(buf));
				left(buf);
				newLine();
				left(m_fanState.c_str(), 3);
			}
		} else if(SettingsHandler::sleeveTempDisplayed && SettingsHandler::internalTempDisplayed) {
			char buf[10];
			if(SettingsHandler::versionDisplayed || !hasNextLine()) {
				getTempString("S", m_sleeveTempString, buf, sizeof(buf));
				left(buf);
				display.print("("+m_HeatStateShort+")");
				getTempString("I", m_internalTempString, buf, sizeof(buf));
				right(buf);
			} else {
				left("Sleeve", 1);
				right("Internal", 1);
				newLine();
				getTempString("", m_sleeveTempString, buf, sizeof(buf));
				left(buf, 1);
				char buf2[10];
				getTempString("", m_internalTempString, buf2, sizeof(buf2));
				right(buf2, 2);
			}
		}
	}
	void getTempString(const char* displayText, char* temp, char* buf, int size) {
		strtrim(temp);
		snprintf(buf, size, "%s%s%cC", displayText, temp, (char)247);
		//strtrim(buf);
		// //tempText[strlen(tempText)] = '\n';
		
		// strtrim(temp);
		// String tempText = displayText + String(temp) + (char)247 + "C";// TODO: remove String
		// strcpy(buf, tempText.c_str());
	}

	void strtrim(char* str) {
		int start = 0; // number of leading spaces
		char* buffer = str;
		while (*str && *str++ == ' ') ++start;
		while (*str++); // move to end of string
		int end = str - buffer - 1; 
		while (end > 0 && buffer[end - 1] == ' ') --end; // backup over trailing spaces
		buffer[end] = 0; // remove trailing spaces
		if (end <= start || start == 0) return; // exit if no leading spaces or string is now empty
		str = buffer + start;
		while ((*buffer++ = *str++));  // remove leading spaces: K&R
	}
};
