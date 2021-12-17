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
	static long bootTimer;
	static xSemaphoreHandle tempMutexBus;
	static xSemaphoreHandle statusMutexBus;
	static String* _statusJson;

	public: 
	static xQueueHandle tempQueue;

	static void setup()
	{
		tempQueue = xQueueCreate(1, sizeof(std::string *));
		Serial.print("Starting temp on pin: ");
		Serial.println(SettingsHandler::Temp_PIN);
		oneWire.begin(SettingsHandler::Temp_PIN);
		sensors.setOneWire(&oneWire);
		bootTime = true;
		bootTimer = millis() + SettingsHandler::WarmUpTime;
		failsafeTimer = millis() + SettingsHandler::heaterFailsafeTime;
		Serial.print("Starting heat on pin: ");
		Serial.println(SettingsHandler::Heater_PIN);
  		ledcSetup(Heater_PWM, SettingsHandler::heaterFrequency, SettingsHandler::heaterResolution);
  		tempMutexBus = xSemaphoreCreateMutex();
  		statusMutexBus = xSemaphoreCreateMutex();
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
			if (xSemaphoreTake(tempMutexBus, 1000 / portTICK_PERIOD_MS))
			{
				_currentTemp = sensors.getTempCByIndex(0);
				xSemaphoreGive(tempMutexBus);
			}
			else
				Serial.println("writing temperature timed out \n");

			// Serial.print("Temp task: "); // stack size used
			// Serial.println(uxTaskGetStackHighWaterMark( NULL )); // stack size used
			Serial.print("Temp: "); 
			Serial.println(_currentTemp); 
			// Serial.print("getTemp: "); 
			// Serial.println(getTemp()); 
			Serial.print("Status: "); 
			Serial.println(getControlStatus());
			// Serial.print("failsafeTimer: "); 
			// Serial.println(failsafeTimer);
			// Serial.print("lastTemp: "); 
			// Serial.println(_lastTemp);
			_statusJson = new String("{\"temp\":" + String(_currentTemp) + ", \"status\":\""+getControlStatus()+"\"}");
			Serial.print("Adding to queue: "); 
			Serial.println(_statusJson->c_str());
			xQueueSend(tempQueue, &_statusJson, 0);
			Serial.flush();

        	vTaskDelay(1000/portTICK_PERIOD_MS);
		}
		
		Serial.print("Temp task exit");
  		vTaskDelete( NULL );
	}

	static String setControlStatus()
	{		
		//Serial.println(_currentTemp);
		if (_isRunning) 
		{
			if(SettingsHandler::tempControlEnabled)
			{
				int currentTemp = _currentTemp;
				if (currentTemp < 0) 
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
					if(currentTemp >= SettingsHandler::TargetTemp || millis() >= bootTimer)
						bootTime = false;
					if (currentTemp < SettingsHandler::TargetTemp || (currentTemp > 0 && bootTime)) 
					{
						ledcWrite(Heater_PWM, SettingsHandler::HeatPWM);

						if(bootTime) 
						{
							long time = bootTimer - millis();
							int tseconds = time / 1000;
							int tminutes = tseconds / 60;
							int seconds = tseconds % 60;
							_currentStatus = "Warm up time: " + String(tminutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds);
						}
						else {
						_currentStatus = "Heating";
						}
					} 
					else if (currentTemp <= (SettingsHandler::TargetTemp + SettingsHandler::heaterThreshold)) 
					{
						ledcWrite(Heater_PWM, SettingsHandler::HoldPWM);
						_currentStatus = "Holding";
					} 
					else 
					{
						ledcWrite(Heater_PWM, 0);
						_currentStatus = "Cooling";
					}

					//Prevent runaway heating.
					if( _currentStatus.startsWith("HEAT") && millis() > failsafeTimer && currentTemp <= _lastTemp) 
					{
						failsafeTrigger = true;
					} 
					else if((currentTemp > _lastTemp && _currentStatus.startsWith("Heating")) 
							|| _currentStatus.startsWith("Cooling") 
							|| _currentStatus.startsWith("Holding")
							|| _currentStatus.startsWith("Warm up time")
							) 
					{
						failsafeTimer = millis() + SettingsHandler::heaterFailsafeTime;
					}
					
					_lastTemp = currentTemp;
				}
				xSemaphoreGive(statusMutexBus);
			}
			else
			{
				_currentStatus = "Disabled";
			}
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
	static bool isRunning()
	{
		return _isRunning ;
	}
};

float TemperatureHandler::_currentTemp = 0.0f;
String TemperatureHandler::_currentStatus = "Unknown";
bool TemperatureHandler::_isRunning = false;
OneWire TemperatureHandler::oneWire;
DallasTemperature TemperatureHandler::sensors;
bool TemperatureHandler::bootTime;
long TemperatureHandler::bootTimer;
float TemperatureHandler::_lastTemp = -130;
int TemperatureHandler::failsafeTimer;
bool TemperatureHandler::failsafeTrigger = false;
xSemaphoreHandle TemperatureHandler::tempMutexBus;
xSemaphoreHandle TemperatureHandler::statusMutexBus;
String* TemperatureHandler::_statusJson;// = "{\"temp\":00.00, \"status\":\"Fail safe triggered\"}";
xQueueHandle TemperatureHandler::tempQueue;

