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

#include <OneWire.h>
#include <DallasTemperature.h>
#include <mutex>

#pragma once

class TemperatureHandler
{
	private: 
	static OneWire oneWire;
	static DallasTemperature sensors;
	static float _currentTemp;
	static String _currentStatus;
	static bool _isRunning;
	static bool bootTime;
	static int bootTimer;

	public: 
	static void setup()
	{
		Serial.print("Starting temp on pin: ");
		Serial.println(SettingsHandler::Temp_PIN);
		oneWire.begin(SettingsHandler::Temp_PIN);
		sensors.setOneWire(&oneWire);
		bootTime = true;
		bootTimer = millis() + SettingsHandler::WarmUpTime;
		pinMode(SettingsHandler::Heater_PIN, OUTPUT);
		//sensors.setWaitForConversion(false);
	}

	static void startLoop(void * parameter)
	{
		_isRunning = true;
		Serial.print("Temp Core: ");
		Serial.println(xPortGetCoreID());
		while(_isRunning)
		{
			sensors.requestTemperatures();
			_currentTemp = sensors.getTempCByIndex(0);
        	vTaskDelay(1000/portTICK_PERIOD_MS);
/* 			Serial.print("Temp task: "); // stack size used
			Serial.println(uxTaskGetStackHighWaterMark( NULL )); // stack size used
			Serial.print("Temp: "); // stack size used
			Serial.println(_currentTemp); // stack size used
			Serial.flush(); */
		}
		
		Serial.print("Temp task exit");
  		vTaskDelete( NULL );
	}

	static String setControlStatus()
	{		
		//Serial.println(_currentTemp);
		
		if(SettingsHandler::tempControlEnabled)
		{
			if (_currentTemp < 0) 
			{
				ledcWrite(SettingsHandler::Heater_PIN, 0);
				_currentStatus = "Error reading";
			} 
			else
			{
				if(millis() >= bootTimer)
					bootTime = false;
				if ((_currentTemp < SettingsHandler::TargetTemp && _currentTemp > 0) || (_currentTemp > 0 && bootTime)) 
				{
					ledcWrite(SettingsHandler::Heater_PIN, SettingsHandler::HeatPWM);
					_currentStatus = "HEATING";
				} 
				else if ((_currentTemp >= SettingsHandler::TargetTemp && _currentTemp <= SettingsHandler::TargetTemp) ) 
				{
					ledcWrite(SettingsHandler::Heater_PIN, SettingsHandler::HoldPWM);
					_currentStatus = "HOLDING";
				} 
				else 
				{
					ledcWrite(SettingsHandler::Heater_PIN, 0);
					_currentStatus = "COOLING";
				}
			}
		}
		else
		{
				_currentStatus = "";
		}
		return _currentStatus;
	}

	static float getTemp()
	{
		return _currentTemp;
	}

	static String getControlStatus()
	{
		return _currentStatus;
	}

	static void stop()
	{
		_isRunning = false;
	}
};

float TemperatureHandler::_currentTemp = 0.0f;
String TemperatureHandler::_currentStatus = "Unknown";
bool TemperatureHandler::_isRunning = false;
OneWire TemperatureHandler::oneWire;
DallasTemperature TemperatureHandler::sensors;
bool TemperatureHandler::bootTime;
int TemperatureHandler::bootTimer;