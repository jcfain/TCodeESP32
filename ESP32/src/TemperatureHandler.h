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
	static bool targetTempReached;
	static int failsafeTimer;
	static bool failsafeTrigger;
	static bool failsafeTriggerLogged;
	static long bootTimer;
	static xSemaphoreHandle tempMutexBus;
	static xSemaphoreHandle statusMutexBus;
	static long failSafeFrequency;
	static int failSafeFrequencyLimiter;
	static int errorCount;


	static bool definitelyGreaterThan(float a, float b)
	{
		return (a - b) > ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * __FLT_EPSILON__);
	}

	static bool definitelyLessThan(float a, float b)
	{
		return (b - a) > ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) *  __FLT_EPSILON__);
	}
	static bool essentiallyEqual(float a, float b)
	{
		return fabs(a - b) <= ( (fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * __FLT_EPSILON__);
	}
	static bool approximatelyEqual(float a, float b)
	{
		return fabs(a - b) <= ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * __FLT_EPSILON__);
	}
	static bool definitelyLessThanOREssentiallyEqual(float a, float b)
	{
		return definitelyLessThan(a, b) || essentiallyEqual(a, b);
	}
	static bool definitelyLessThanORApproximatelyEqual(float a, float b)
	{
		return definitelyLessThan(a, b) || approximatelyEqual(a, b);
	}
	static bool definitelyGreaterThanOREssentiallyEqual(float a, float b)
	{
		return definitelyGreaterThan(a, b) || essentiallyEqual(a, b);
	}
	
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
		failSafeFrequency = millis() + failSafeFrequencyLimiter; 
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
			if(failsafeTrigger && !failsafeTriggerLogged)
			{
				failsafeTriggerLogged = true;
				Serial.print("Temp: "); 
				Serial.println(_currentTemp); 
				Serial.print("lastTemp: "); 
				Serial.println(_lastTemp);
				Serial.print("getTemp: "); 
				Serial.println(getTemp()); 
				Serial.print("Status: "); 
				Serial.println(getControlStatus());
				Serial.print("failsafeTimer: "); 
				Serial.println(failsafeTimer);
				Serial.print("_currentStatus.startsWith(Heatin: "); 
				Serial.println(_currentStatus.startsWith("Heating"));
				Serial.print("_currentStatus.startsWith(Error) "); 
				Serial.println(_currentStatus.startsWith("Error"));
				Serial.print("millis() > failsafeTimer: "); 
				Serial.println(millis() > failsafeTimer);
				Serial.print("_currentTemp <= _lastTemp "); 
				Serial.println(_currentTemp <= _lastTemp);
				Serial.print("definitelyLessThanORApproximatelyEqual(_currentTemp, _lastTemp) "); 
				Serial.println(definitelyLessThanORApproximatelyEqual(_currentTemp, _lastTemp));
				Serial.print("definitelyLessThanOREssentiallyEqual(_currentTemp, (SettingsHandler::TargetTemp + SettingsHandler::heaterThreshold)) "); 
				Serial.println(definitelyLessThanOREssentiallyEqual(_currentTemp, (SettingsHandler::TargetTemp + SettingsHandler::heaterThreshold)));
				Serial.print("SettingsHandler::TargetTemp + SettingsHandler::heaterThreshold "); 
				Serial.println((SettingsHandler::TargetTemp + SettingsHandler::heaterThreshold));
				Serial.print("errorCount "); 
				Serial.println(errorCount);
			}

			// if(!failsafeTrigger)
			// {
			// 	Serial.print("Temp: "); 
			// 	Serial.println(_currentTemp); 
			// 	Serial.print("lastTemp: "); 
			// 	Serial.println(_lastTemp);
			// 	long time = failsafeTimer - millis();
			// 	int tseconds = time / 1000;
			// 	int tminutes = tseconds / 60;
			// 	int seconds = tseconds % 60;
			// 	Serial.println("failsafeTimer: " + String(tminutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds));
			// }

	 		String* statusJson = new String("{\"temp\":" + String(_currentTemp) + ", \"status\":\""+getControlStatus()+"\"}");
			Serial.print("Adding to queue: "); 
			Serial.println(statusJson->c_str());
			
			xQueueSend(tempQueue, &statusJson, 0);
			Serial.flush();
			chackFailSafe();
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
				if(failsafeTrigger)
				{
					ledcWrite(Heater_PWM, 0);
				} 
				else
				{
					float currentTemp = _currentTemp;
					if (currentTemp < 0) 
					{
						_currentStatus = "Error reading";
					} 
					else
					{
						if(currentTemp >= SettingsHandler::TargetTemp || millis() >= bootTimer)
							bootTime = false;
						if (definitelyLessThan(currentTemp, SettingsHandler::TargetTemp) || (currentTemp > 0 && bootTime)) 
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
							else 
							{
								_currentStatus = "Heating";
							}
							if(targetTempReached && SettingsHandler::TargetTemp - currentTemp >= 5)
								targetTempReached = false;
						} 
						else if (definitelyLessThanOREssentiallyEqual(currentTemp, (SettingsHandler::TargetTemp + SettingsHandler::heaterThreshold))) 
						{
							if(!targetTempReached)
							{
								targetTempReached = true;
								// Serial.print("Adding to queue: "); 
								// Serial.println(_statusJson->c_str());
								String* command = new String("tempReached");
								xQueueSend(tempQueue, &command, 0);
							}
							ledcWrite(Heater_PWM, SettingsHandler::HoldPWM);
							_currentStatus = "Holding";
						} 
						else 
						{
							ledcWrite(Heater_PWM, 0);
							_currentStatus = "Cooling";
						}
					}
				}
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
	static void chackFailSafe() 
	{
		if(millis() >= failSafeFrequency)
		{
			failSafeFrequency = millis() + failSafeFrequencyLimiter;
			//Prevent runaway heating.
			if(errorCount > 10 || (_currentTemp > 0 && _lastTemp > 0 && _currentStatus.startsWith("Heating") && millis() > failsafeTimer && definitelyLessThanORApproximatelyEqual(_currentTemp, _lastTemp))) 
			{
				String* command = new String("failSafeTriggered");
				xQueueSend(tempQueue, &command, 0);
				failsafeTrigger = true;
				if(errorCount > 10)
					_currentStatus = "Fail safe: read";
				else 
					_currentStatus = "Fail safe: heat";
			} 
			else if((_currentTemp > 0 && _lastTemp > 0 && _currentStatus.startsWith("Heating") && definitelyGreaterThan(_currentTemp, _lastTemp)) 
					|| _currentStatus.startsWith("Cooling") 
					|| _currentStatus.startsWith("Holding")
					|| _currentStatus.startsWith("Warm up time")
					) 
			{
				errorCount = 0;
				failsafeTimer = millis() + SettingsHandler::heaterFailsafeTime;
			}
			else if(_currentStatus.startsWith("Error")) 
			{
				errorCount++;
			}

			_lastTemp = _currentTemp;
		}
	}
};

float TemperatureHandler::_currentTemp = 0.0f;
String TemperatureHandler::_currentStatus = "Unknown";
bool TemperatureHandler::_isRunning = false;
OneWire TemperatureHandler::oneWire;
DallasTemperature TemperatureHandler::sensors;
bool TemperatureHandler::bootTime;
long TemperatureHandler::bootTimer;
float TemperatureHandler::_lastTemp = -127.0;
bool TemperatureHandler::targetTempReached = false;
int TemperatureHandler::failsafeTimer;
bool TemperatureHandler::failsafeTrigger = false;
bool TemperatureHandler::failsafeTriggerLogged = false;
xSemaphoreHandle TemperatureHandler::tempMutexBus;
xSemaphoreHandle TemperatureHandler::statusMutexBus;
xQueueHandle TemperatureHandler::tempQueue;
long TemperatureHandler::failSafeFrequency;
int TemperatureHandler::failSafeFrequencyLimiter = 3000;
int TemperatureHandler::errorCount = 0;

