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
#include "TCode/Global.h"

#pragma once

class TemperatureHandler
{
	private: 

	static OneWire oneWire;
	static DallasTemperature sensors;
	static float _currentTemp;
	static float _lastTemp;
	static String _currentStatus;
	static bool _isRunning;
	static bool bootTime;
	static int failsafeTimer;
	static bool failsafeTrigger;
	static String _warmUpTimeTaken;

	public: 

	static void setup()
	{
		Serial.print("Starting temp on pin: ");
		Serial.println(SettingsHandler::Temp_PIN);
		oneWire.begin(SettingsHandler::Temp_PIN);
		sensors.setOneWire(&oneWire);
		bootTime = true;
		failsafeTimer = millis() + SettingsHandler::heaterFailsafeTime;
		Serial.print("Starting heat on pin: ");
		Serial.println(SettingsHandler::Heater_PIN);
  		ledcSetup(Heater_PWM, SettingsHandler::heaterFrequency, SettingsHandler::heaterResolution);
		ledcAttachPin(SettingsHandler::Heater_PIN, Heater_PWM);
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
			// Serial.print("Temp task: "); // stack size used
			// Serial.println(uxTaskGetStackHighWaterMark( NULL )); // stack size used
			Serial.print("Temp: "); 
			Serial.println(_currentTemp); 
			Serial.print("lastTemp: "); 
			Serial.println(_lastTemp);
			Serial.print("Status: "); 
			Serial.println(_currentStatus);
			Serial.print("failsafeTimer: "); 
			Serial.println(failsafeTimer);
			
			if(failsafeTrigger) {
				Serial.print("Fail safe triggered"); 
			}
			Serial.flush();
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
				_currentStatus = "Error reading";
			} 
			else if(failsafeTrigger)
			{
				ledcWrite(Heater_PWM, 0);
				_currentStatus = "Fail safe triggered";
			}
			else
			{
				if(_currentTemp >= SettingsHandler::TargetTemp)
					bootTime = false;
				if (_currentTemp < SettingsHandler::TargetTemp) 
				{
					ledcWrite(Heater_PWM, SettingsHandler::HeatPWM);
					_currentStatus = "HEAT";
					if(bootTime) 
					{
						long time = millis();
						int tseconds = time / 1000;
						int tminutes = tseconds / 60;
						int seconds = tseconds % 60;
						_warmUpTimeTaken = String(tminutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds);
					}
				} 
				else if (_currentTemp <= (SettingsHandler::TargetTemp + SettingsHandler::heaterFailsafeThreshold)) 
				{
					ledcWrite(Heater_PWM, SettingsHandler::HoldPWM);
					_currentStatus = "HOLD";
				} 
				else 
				{
					ledcWrite(Heater_PWM, 0);
					_currentStatus = "COOL";
				}
				_currentStatus += ": " + _warmUpTimeTaken;

				//Prevent runaway heating.
				if(millis() > failsafeTimer && (_currentTemp <= (_lastTemp + SettingsHandler::heaterFailsafeThreshold) || _currentTemp == _lastTemp) && _currentStatus.startsWith("HEAT")) 
				{
					failsafeTrigger = true;
				} 
				else if((_currentTemp > _lastTemp && _currentStatus.startsWith("HEAT")) 
						|| _currentStatus.startsWith("COOL") 
						|| _currentStatus.startsWith("HOLD")
						) 
				{
					failsafeTimer = millis() + SettingsHandler::heaterFailsafeTime;
				}
				
				_lastTemp = _currentTemp;
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
float TemperatureHandler::_lastTemp = -130;
int TemperatureHandler::failsafeTimer;
bool TemperatureHandler::failsafeTrigger = false;
String TemperatureHandler::_warmUpTimeTaken = "0:00";

