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
	static int Temp_PIN; // Temp Sensor Pin
	static int Heater_PIN;   // Heater PWM
	static int HeatLED_PIN; // Indicator LED
	static int TargetTemp; // Desired Temp in degC
	static int HeatPWM; // Heating PWM setting 0-255
	static int HoldPWM; // Hold heat PWM setting 0-255
	static int WarmUpTime; // Time to hold heating on first boot

	public: 
	static void setup()
	{
		oneWire.begin(Temp_PIN);
		sensors.setOneWire(&oneWire);
		bootTime = true;
		bootTimer = millis() + WarmUpTime;
		pinMode(HeatLED_PIN, OUTPUT);
		pinMode(Heater_PIN, OUTPUT);
		//sensors.setWaitForConversion(false);
	}

	static void startLoop(void * parameter)
	{
		_isRunning = true;
		//Serial.print("Temp Core: ");
		//Serial.println(xPortGetCoreID());
		while(_isRunning)
		{
			sensors.requestTemperatures();
			_currentTemp = sensors.getTempCByIndex(0);
        	vTaskDelay(1000/portTICK_PERIOD_MS);
			// Serial.print("Temp task: "); // stack size used
			// Serial.print(uxTaskGetStackHighWaterMark( NULL )); // stack size used
			// Serial.println();
			// Serial.flush();
		}
		
  		vTaskDelete( NULL );
	}

	static String setControlStatus()
	{		
		//Serial.println(_currentTemp);
		
		if(SettingsHandler::tempControlEnabled)
		{
			if (_currentTemp < 0) 
			{
				digitalWrite(HeatLED_PIN, 0);
				ledcWrite(Heater_PIN, 0);
				_currentStatus = "Error reading";
			} 
			else
			{
				if(millis() >= bootTimer)
					bootTime = false;
				if ((_currentTemp < TargetTemp && _currentTemp > 0) || (_currentTemp > 0 && bootTime)) 
				{
					digitalWrite(HeatLED_PIN, 255);
					ledcWrite(Heater_PIN, HeatPWM);
					_currentStatus = "HEATING";
				} 
				else if ((_currentTemp >= TargetTemp && _currentTemp <= TargetTemp) ) 
				{
					digitalWrite(HeatLED_PIN, HoldPWM);
					ledcWrite(Heater_PIN, HoldPWM);
					_currentStatus = "HOLDING";
				} 
				else 
				{
					digitalWrite(HeatLED_PIN, 0);
					ledcWrite(Heater_PIN, 0);
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
int TemperatureHandler::Temp_PIN = SettingsHandler::Temp_PIN; // Temp Sensor Pin
int TemperatureHandler::Heater_PIN = SettingsHandler::Heater_PIN;   // Heater PWM
int TemperatureHandler::HeatLED_PIN = SettingsHandler::HeatLED_PIN; // Indicator LED
int TemperatureHandler::TargetTemp = SettingsHandler::TargetTemp; // Desired Temp in degC
int TemperatureHandler::HeatPWM = SettingsHandler::HeatPWM; // Heating PWM setting 0-255
int TemperatureHandler::HoldPWM = SettingsHandler::HoldPWM;  // Hold heat PWM setting 0-255
int TemperatureHandler::WarmUpTime = SettingsHandler::WarmUpTime; // Time to hold heating on first boot