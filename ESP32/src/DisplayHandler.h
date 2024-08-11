/* MIT License

Copyright (c) 2024 Jason C. Fain

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
#if WIFI_TCODE
#include "WifiHandler.h"
#endif
#include <vector>
#if BUILD_TEMP
#include "TemperatureHandler.h"
#endif
#include "BatteryHandler.h"
#include "TagHandler.h"
// #if ISAAC_NEWTONGUE_BUILD
// #include "../lib/animationFrames.h"
// #endif

class DisplayHandler
{
public:

	DisplayHandler() : 
		m_settingsFactory(SettingsFactory::getInstance()),
		display(
			m_settingsFactory->getDisplayScreenWidth(),  
			m_settingsFactory->getDisplayScreenHeight(), 
			//64,
			&Wire, 
			-1, 100000UL, 100000UL) { }

	void setup(int I2CAddress, bool fanControlEnabled, int8_t rstPin = -1) 
	{
		m_fanControlEnabled = fanControlEnabled;

    	LogHandler::info(_TAG, "Setting up display");
        int tries = 0;
		SettingsHandler::waitForI2CDevices(I2CAddress);
        if(SettingsHandler::systemI2CAddresses.size() == 0) {
            return;
        }

		//Wire.begin();
		//Wire.setClock(100000UL);
		if(!I2CAddress) {
    		LogHandler::info(_TAG, "No address to connect to");
			return;
		} else if(!connectDisplay(I2CAddress, rstPin)) {
    		LogHandler::info(_TAG, "Could not connect address");
			return;
		}
		delay(2000);
		if(displayConnected)
		{
			//display.setFont(Adafruit5x7);
			display.clearDisplay();
  			display.setTextColor(WHITE);
			display.setTextSize(1);
		} else
			LogHandler::error(_TAG, "Display is not connected");
    	LogHandler::info(_TAG, "Setting up display finished");
	}

	void setLocalIPAddress(IPAddress ipAddress)
	{
		_ipAddress = ipAddress;
	}

	void setSleeveTemp(float temp) {
    	LogHandler::verbose(_TAG, "setSleeveTemp: %f", temp);
		m_sleeveTemp = temp;
		memset(m_sleeveTempString,'\0',sizeof(m_sleeveTempString));
		if(temp < 0) {
			strcpy(m_sleeveTempString, "XX.XX\0");
		} else {   
			dtostrf(temp, sizeof(m_sleeveTempString) - 1, 2, m_sleeveTempString);
		}	
	}
	void setInternalTemp(float temp) {
    	LogHandler::verbose(_TAG, "setInternalTemp: %f", temp);
		m_internalTemp = temp;
		memset(m_internalTempString,'\0',sizeof(m_internalTempString));
		if(temp < 0) {
			strcpy(m_internalTempString, "XX.XX\0");
		} else {   
			dtostrf(temp, sizeof(m_internalTempString) - 1, 2, m_internalTempString);
		}	
	}
	void setHeateState(const char* state) {
    	LogHandler::verbose(_TAG, "setHeateState: %s", state);
		m_HeatState = String(state);
	}
	void setHeateStateShort(const char* state) {
    	LogHandler::verbose(_TAG, "setHeateStateShort: %s", state);
		m_HeatStateShort = String(state);
	}
	void setFanState(const char* state) {
    	LogHandler::verbose(_TAG, "setFanState: %s", state);
		m_fanState = String(state);
	}
	void setBatteryInformation(float capacityRemainingPercentage, float voltage, float temperature) {
    	LogHandler::verbose(_TAG, "setBatteryCapacityRemainingPercentage: %f", capacityRemainingPercentage);
		m_batteryCapacityRemainingPercentage = capacityRemainingPercentage;
    	LogHandler::verbose(_TAG, "setBatteryVoltage: %f", voltage);
		m_batteryVoltage = voltage;
    	LogHandler::verbose(_TAG, "setBatteryTemperature: %f", temperature);
		m_batteryTemperature = temperature;
	}

	void clearDisplay()
	{
    	LogHandler::verbose(_TAG, "clear display");
		if(displayConnected) {
			display.clearDisplay();
		}
	}

	static void startLoop(void* displayHandlerRef)
	{
		//LogHandler::debug(TagHandler::DisplayHandler, "Starting loop");
		//if(((DisplayHandler*)displayHandlerRef)->isConnected())
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
    	LogHandler::verbose(_TAG, "stopRunning");
		_isRunning = false;
	}

	void loop()
	{
		LogHandler::debug(_TAG, "Display task cpu core: %u", xPortGetCoreID());
		if(!isConnected()) {
			LogHandler::warning(_TAG, "Display not connected when starting loop");
  			vTaskDelete( NULL );
			return;
		}
        TickType_t pxPreviousWakeTime = millis();
		_isRunning = true;
		while(_isRunning) {
			if(!m_animationPlaying && displayConnected && millis() >= lastUpdate + nextUpdate) {
				LogHandler::verbose(_TAG, "Enter display loop");
				lastUpdate = millis();
				clearDisplay();
				setTextSize(1);
				int headerPadding = is32() ? 0 : 3;
				// Serial.print("Display Core: ");
				// Serial.println(xPortGetCoreID());

#if WIFI_TCODE
				if(WifiHandler::isConnected()) {
					LogHandler::verbose(_TAG, "Enter wifi connected");
					startLine(headerPadding);
					display.print(_ipAddress);

					drawBatteryLevel();
					
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
						display.fillRect((m_settingsFactory->getDisplayScreenWidth() - 17) + (b*3), barHeight - (b*2),2,b*2,WHITE); 
					}

					newLine(headerPadding);
					if(m_settingsFactory->getVersionDisplayed()) {
						LogHandler::verbose(_TAG, "Enter versionDisplayed");
						left(m_settingsFactory->getTcodeVersionString());
						right(FIRMWARE_VERSION_NAME);
						newLine();
					}
					
				} else if(WifiHandler::apMode) {
					LogHandler::verbose(_TAG, "Enter apMode");
					startLine(headerPadding);
					left("AP: 192.168.1.1");
					drawBatteryLevel();
					newLine(headerPadding);
					if(!is32()) {
						left("SSID: TCodeESP32Setup");
						newLine();
					}
					if(is32() && m_settingsFactory->getVersionDisplayed() && !m_settingsFactory->getSleeveTempDisplayed() && !m_settingsFactory->getInternalTempDisplayed()
						|| m_settingsFactory->getVersionDisplayed()) {
						left(m_settingsFactory->getTcodeVersionString());
						right(FIRMWARE_VERSION_NAME);
						newLine();
					} else if(is32()) {
						left("SSID: TCodeESP32Setup");
						newLine();
					}
				} else {
					LogHandler::verbose(_TAG, "Enter Wifi error");
					display.print("Wifi error");
					drawBatteryLevel();
				}
#endif
#if BUILD_TEMP
				if(m_settingsFactory->getSleeveTempDisplayed() || m_settingsFactory->getInternalTempDisplayed()) {
					is32() ? draw32Temp() : draw64Temp();
				}
#endif

				display.display();
			}
            xTaskDelayUntil(&pxPreviousWakeTime, 5000/portTICK_PERIOD_MS);
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
// 				display.drawBitmap(0, 0, dontPanicAnimationFrames[currentFrameIndex], m_settingsFactory->getDisplayScreenWidth(), m_settingsFactory->getDisplayScreenHeight(), 1);
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
	const char* _TAG = TagHandler::DisplayHandler;
    SettingsFactory* m_settingsFactory;
	bool m_fanControlEnabled;
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
	float m_batteryVoltage = 0.0;
	int m_batteryCapacityRemainingPercentage;
	float m_batteryTemperature;

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
		if(newLine > m_settingsFactory->getDisplayScreenHeight() - lineHeight) {
			LogHandler::warning(_TAG, "End of the display reached when newLine! Current: %i, New: %i, Max: %i", currentLine, newLine, m_settingsFactory->getDisplayScreenHeight() - lineHeight);
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
		display.setCursor(0, currentLine);
	}
	bool hasNextLine(int newLineTextSize = 1) {
		
		return currentLine + getLineHeight(newLineTextSize) <= m_settingsFactory->getDisplayScreenHeight() - getLineHeight(newLineTextSize);
	}
	void space(int count = 1) {
		display.setCursor(display.getCursorX() + (count * charWidth), currentLine);
	}
	void right(const char* text, int margin = 0) {
		display.setCursor((m_settingsFactory->getDisplayScreenWidth() - strlen(text) * charWidth) - margin * charWidth, currentLine);
		display.print(text);
	}
	void left(const char* text, int margin = 0) {
		display.setCursor(margin * charWidth, currentLine);
		display.print(text); 
	}
	void center(const char* text) {
		display.setCursor((m_settingsFactory->getDisplayScreenWidth() - (strlen(text) * charWidth)) / 2, currentLine);
		display.print(text); 
	}
	void clearCurrentLine() {
		display.fillRect(0, currentLine, m_settingsFactory->getDisplayScreenWidth(), 10, BLACK);
	}
	bool is32() {
		return m_settingsFactory->getDisplayScreenHeight() == 32;
	}

	void draw64Temp() {
		if(m_settingsFactory->getSleeveTempDisplayed() && !m_settingsFactory->getInternalTempDisplayed()) {
			LogHandler::verbose(_TAG, "Enter draw64Temp sleeveTempDisplayed");
			if(m_settingsFactory->getVersionDisplayed()) {
				LogHandler::verbose(_TAG, "versionDisplayed");
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
		} else if(!m_settingsFactory->getSleeveTempDisplayed() && m_settingsFactory->getInternalTempDisplayed()) {
			LogHandler::verbose(_TAG, "Enter draw64Temp internalTempDisplayed");
			if(m_settingsFactory->getVersionDisplayed()) {
				LogHandler::verbose(_TAG, "versionDisplayed");
				char buf[18];
				getTempString("Internal: ", m_internalTempString, buf, sizeof(buf));
				left(buf);
				if(m_fanControlEnabled) {
					newLine();
					left(m_fanState.c_str(), 3);
				}
			} else {
				setTextSize(3);
				char buf[9];
				getTempString("", m_internalTempString, buf, sizeof(buf));
				center(buf);
				newLine(0, 1);
				if(m_fanControlEnabled) {
					center(m_fanState.c_str());
				}
			}
		} else if(m_settingsFactory->getSleeveTempDisplayed() && m_settingsFactory->getInternalTempDisplayed()) {
			LogHandler::verbose(_TAG, "Enter draw64Temp sleeveTempDisplayed && internalTempDisplayed");
			left("Sleeve");
			right("Internal");

			newLine();

			char buf[9];
			getTempString("", m_sleeveTempString, buf, sizeof(buf));
			left(buf);
			if(m_settingsFactory->getVersionDisplayed()) {
				display.print("("+m_HeatStateShort+")");
			}
			char buf2[9];
			getTempString("", m_internalTempString, buf2, sizeof(buf2));
			right(buf2);

			if(!m_settingsFactory->getVersionDisplayed()) {
				LogHandler::verbose(_TAG, "versionDisplayed");
				newLine();
				
				left(m_HeatState.c_str());
				if(m_fanControlEnabled) {
					right(m_fanState.c_str());
				}
			}
		}
	}
	void draw32Temp() {
		if(m_settingsFactory->getSleeveTempDisplayed() && !m_settingsFactory->getInternalTempDisplayed()) {
			LogHandler::verbose(_TAG, "Enter draw32Temp sleeveTempDisplayed");
			char buf[19];
			if(m_settingsFactory->getVersionDisplayed() || !hasNextLine()) {
				LogHandler::verbose(_TAG, "versionDisplayed");
				getTempString("Sleeve: ", m_sleeveTempString, buf, sizeof(buf));
				left(buf);
				display.print("("+m_HeatStateShort+")");
			} else if(hasNextLine(2)) {
				LogHandler::verbose(_TAG, "hasNextLine(2)");
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
		} else if(!m_settingsFactory->getSleeveTempDisplayed() && m_settingsFactory->getInternalTempDisplayed()) {
			LogHandler::verbose(_TAG, "Enter draw32Temp internalTempDisplayed");
			char buf[21];
			if(m_settingsFactory->getVersionDisplayed() || !hasNextLine()) {
				LogHandler::verbose(_TAG, "versionDisplayed");
				getTempString("Internal: ", m_internalTempString, buf, sizeof(buf));
				left(buf);
			} else if(hasNextLine(2)) {
				LogHandler::verbose(_TAG, "hasNextLine(2)");
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
		} else if(m_settingsFactory->getSleeveTempDisplayed() && m_settingsFactory->getInternalTempDisplayed()) {
			LogHandler::verbose(_TAG, "Enter draw32Temp sleeveTempDisplayed && internalTempDisplayed");
			char buf[10];
			if(m_settingsFactory->getVersionDisplayed() || !hasNextLine()) {
				LogHandler::verbose(_TAG, "versionDisplayed");
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

	void drawBatteryLevel() {
		bool wifiConnected = false;
#if WIFI_TCODE
		wifiConnected = WifiHandler::isConnected();
#endif
		if(BatteryHandler::connected()) {
    		LogHandler::verbose(_TAG, "Enter draw battery");
			if(m_settingsFactory->getBatteryLevelNumeric()) {
				//double voltageNumber = mapf(m_batteryVoltage, 0.0, 3.3, 0.0, m_settingsFactory->getBatteryVoltageMax());
				if (false) {//Display voltage
					display.setCursor((m_settingsFactory->getDisplayScreenWidth() - (m_batteryVoltage < 10.0 ? 3 : 4) * charWidth) - (wifiConnected ? 3 : 0) * charWidth, currentLine);
					display.print(m_batteryVoltage, 1);
				} else {
					display.setCursor((m_settingsFactory->getDisplayScreenWidth() - (m_batteryCapacityRemainingPercentage < 10.0 ? 3 : 4) * charWidth) - (wifiConnected ? 3 : 0) * charWidth, currentLine);
					display.print(m_batteryCapacityRemainingPercentage, 1);
					display.print("%");
				}
			} else {
				int batteryBars;
				if(false) {//Display voltage
					if (m_batteryVoltage >= 3.17) { 
						batteryBars = 5;
					} else if (m_batteryVoltage < 3.17 && m_batteryVoltage > 3.09) {
						batteryBars = 4;
					} else if (m_batteryVoltage < 3.09 && m_batteryVoltage > 3.02) {
						batteryBars = 3;
					} else if (m_batteryVoltage < 3.02 && m_batteryVoltage > 2.92) {
						batteryBars = 2;
					} else if (m_batteryVoltage < 2.92 && m_batteryVoltage > 2.83) {
						batteryBars = 1;
					} else {
						batteryBars = 0;
					}
				} else {
					if (m_batteryCapacityRemainingPercentage >= 80) { 
						batteryBars = 5;
					} else if (m_batteryCapacityRemainingPercentage < 80 && m_batteryCapacityRemainingPercentage > 60) {
						batteryBars = 4;
					} else if (m_batteryCapacityRemainingPercentage < 60 && m_batteryCapacityRemainingPercentage > 40) {
						batteryBars = 3;
					} else if (m_batteryCapacityRemainingPercentage < 40 && m_batteryCapacityRemainingPercentage > 20) {
						batteryBars = 2;
					} else if (m_batteryCapacityRemainingPercentage < 20 && m_batteryCapacityRemainingPercentage > 1) {
						batteryBars = 1;
					} else {
						batteryBars = 0;
					}
				}
				for (int b=0; b < batteryBars; b++) {
					display.fillRect(
						(m_settingsFactory->getDisplayScreenWidth() - (!wifiConnected ? 20 : 37)) + (b*3), 
						2, 
						2, 
						lineHeight - 4, 
						WHITE); 
				}
				display.drawRect(
					m_settingsFactory->getDisplayScreenWidth() - (!wifiConnected ? 23 : 40),
					1, 
					20, 
					lineHeight-2, 
					WHITE); // draw the outline box
			}
		}
	}
	// bool tryConnect() //Connects to the wrong address
	// {
  	// 	unsigned int vecSize = m_settingsFactory->getSystemI2CAddresses.size()();
	// 	LogHandler::info(_TAG, "System I2c device count %ld", vecSize);
	// 	if(vecSize) {
	// 		for(unsigned int i = 0; i < vecSize; i++)
	// 		{
	// 			//const char* address = m_settingsFactory->getSystemI2CAddresses[i].c_str()();
	// 			char buf[10];
	// 			hexToString(m_settingsFactory->getSystemI2CAddresses[i](), buf);
	// 			LogHandler::info(_TAG, "Trying to connect to %s", buf);
	// 			if(m_settingsFactory->getSystemI2CAddresses[i]() && connectDisplay(m_settingsFactory->getSystemI2CAddresses[i]())) {
	// 				LogHandler::info(_TAG, "Sucess!");
	// 				m_settingsFactory->getDisplay_I2C_Address() = m_settingsFactory->getSystemI2CAddresses[i]();
	// 				m_settingsFactory->getSave()();
	// 				return true;
	// 			} else {
	// 				LogHandler::info(_TAG, "Failed..");
	// 			}
	// 		}
	// 		LogHandler::info(_TAG, "Could not connect to any I2C address");
	// 	}
	// 	return false;
	// }

	bool connectDisplay(int address, int pin) {
		if(LogHandler::getLogLevel() == LogLevel::DEBUG) {
			char buf[10];
			hexToString(address, buf);
			LogHandler::debug(_TAG, "Connect to display at address: %s", buf);
			LogHandler::debug(_TAG, "byte: %ld", address);
		}
		if (pin >= 0)
		{
			displayConnected = display.begin(SSD1306_SWITCHCAPVCC, address, pin);
			if (!displayConnected)
    			LogHandler::error(_TAG, "SSD1306 RST_PIN allocation failed");
		}
		else
		{
			displayConnected = display.begin(SSD1306_SWITCHCAPVCC, address);
			if (!displayConnected)
    			LogHandler::error(_TAG, "SSD1306 allocation failed");
		}
		LogHandler::debug(_TAG, "Exit connectDisplay connected: %ld", displayConnected);
		return displayConnected;
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
};
