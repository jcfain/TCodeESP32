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

#include <OneWire.h>
#include <DallasTemperature.h>
#include "TCode/Global.h"
#include "SettingsHandler.h"

class TemperatureHandler
{
	private: 

	static OneWire oneWireInternal;
	static DallasTemperature sensorsInternal;
	static float _currentInternalTemp;

	static OneWire oneWireSleeve;
	static DallasTemperature sensorsSleeve;
	static float _currentSleeveTemp;
	static String _currentInternalStatus;
	static float _lastSleeveTemp;
	static String _currentSleeveStatus;
	static bool _isRunning;
	static bool bootTime;
	static bool targetSleeveTempReached;
	// static int failsafeTimer;
	static bool failsafeTriggerSleeve;
	static bool failsafeTriggerInternal;
	// static bool failsafeTriggerLogged;
	static long bootTimer;
	static xSemaphoreHandle tempMutexBus;
	static xSemaphoreHandle statusMutexBus;
	static long failSafeFrequency;
	static int failSafeFrequencyLimiter;
	static int errorCountSleeve;
	static int errorCountInternal;
	static bool sleeveTempInitialized;
	static bool internalTempInitialized;


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
	static xQueueHandle sleeveTempQueue;
	static xQueueHandle internalTempQueue;

	static void setup()
	{
		LogHandler::info("temprature", "Starting sleeve temp on pin: %u", SettingsHandler::Sleeve_Temp_PIN);
		sleeveTempQueue = xQueueCreate(1, sizeof(std::string *));
		oneWireSleeve.begin(SettingsHandler::Sleeve_Temp_PIN);
		sensorsSleeve.setOneWire(&oneWireSleeve);
		bootTime = true;
		bootTimer = millis() + SettingsHandler::WarmUpTime;
		// failsafeTimer = millis() + SettingsHandler::heaterFailsafeTime;
		// failSafeFrequency = millis() + failSafeFrequencyLimiter; 
		Serial.print("Starting heat on pin: ");
		Serial.println(SettingsHandler::Heater_PIN);
  		ledcSetup(Heater_PWM, SettingsHandler::heaterFrequency, SettingsHandler::heaterResolution);
  		tempMutexBus = xSemaphoreCreateMutex();
  		statusMutexBus = xSemaphoreCreateMutex();
		ledcAttachPin(SettingsHandler::Heater_PIN, Heater_PWM);
		//sensors.setWaitForConversion(false);
		sleeveTempInitialized = true;
	}

	static void setupInternalTemp()
	{
		LogHandler::info("temprature", "Starting internal temp on pin: %u", SettingsHandler::Internal_Temp_PIN);
		internalTempQueue = xQueueCreate(1, sizeof(std::string *));
		oneWireInternal.begin(SettingsHandler::Internal_Temp_PIN);
		sensorsInternal.setOneWire(&oneWireInternal);
		if(SettingsHandler::fanControlEnabled)
			ledcAttachPin(SettingsHandler::Case_Fan_PIN, CaseFan_PWM);
		internalTempInitialized = true;
	}

	static void startLoop(void * parameter)
	{
		_isRunning = true;
		Serial.print("Temp Core: ");
		Serial.println(xPortGetCoreID());
		while(_isRunning)
		{
			if(SettingsHandler::tempControlSleeveEnabled && sleeveTempInitialized) {
				sensorsSleeve.requestTemperatures();
				if (xSemaphoreTake(tempMutexBus, 1000 / portTICK_PERIOD_MS))
				{
					_currentSleeveTemp = sensorsSleeve.getTempCByIndex(0);
					xSemaphoreGive(tempMutexBus);
				}
				else
					Serial.println("writing temperature timed out \n");

				// Serial.print("Temp task: "); // stack size used
				// Serial.println(uxTaskGetStackHighWaterMark( NULL )); // stack size used
				// if(failsafeTrigger && !failsafeTriggerLogged)
				// {
				// 	failsafeTriggerLogged = true;
				// 	Serial.print("Temp: "); 
				// 	Serial.println(_currentTemp); 
				// 	Serial.print("lastTemp: "); 
				// 	Serial.println(_lastTemp);
				// 	Serial.print("getTemp: "); 
				// 	Serial.println(getTemp()); 
				// 	Serial.print("Status: "); 
				// 	Serial.println(getControlStatus());
				// 	Serial.print("failsafeTimer: "); 
				// 	Serial.println(failsafeTimer);
				// 	Serial.print("_currentStatus.startsWith(Heatin: "); 
				// 	Serial.println(_currentStatus.startsWith("Heating"));
				// 	Serial.print("_currentStatus.startsWith(Error) "); 
				// 	Serial.println(_currentStatus.startsWith("Error"));
				// 	Serial.print("millis() > failsafeTimer: "); 
				// 	Serial.println(millis() > failsafeTimer);
				// 	Serial.print("_currentTemp <= _lastTemp "); 
				// 	Serial.println(_currentTemp <= _lastTemp);
				// 	Serial.print("definitelyLessThanORApproximatelyEqual(_currentTemp, _lastTemp) "); 
				// 	Serial.println(definitelyLessThanORApproximatelyEqual(_currentTemp, _lastTemp));
				// 	Serial.print("definitelyLessThanOREssentiallyEqual(_currentTemp, (SettingsHandler::TargetTemp + SettingsHandler::heaterThreshold)) "); 
				// 	Serial.println(definitelyLessThanOREssentiallyEqual(_currentTemp, (SettingsHandler::TargetTemp + SettingsHandler::heaterThreshold)));
				// 	Serial.print("SettingsHandler::TargetTemp + SettingsHandler::heaterThreshold "); 
				// 	Serial.println((SettingsHandler::TargetTemp + SettingsHandler::heaterThreshold));
				// 	Serial.print("errorCount "); 
				// 	Serial.println(errorCount);
				// }

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

				String* statusJson = new String("{\"temp\":" + String(_currentSleeveTemp) + ", \"status\":\""+getSleeveControlStatus()+"\"}");
				// Serial.print("Adding to queue: "); 
				// Serial.println(statusJson->c_str());
				xQueueSend(sleeveTempQueue, &statusJson, 0);
			}

			if(SettingsHandler::tempControlInternalEnabled && internalTempInitialized) {
				sensorsInternal.requestTemperatures();
				_currentInternalTemp = sensorsInternal.getTempCByIndex(0);
	 			String* statusJson = new String("{\"temp\":" + String(_currentInternalTemp) + ", \"status\":\""+getInternalControlStatus()+"\"}");
				xQueueSend(internalTempQueue, &statusJson, 0);
			}

			Serial.flush();
			chackFailSafe();
        	vTaskDelay(1000/portTICK_PERIOD_MS);
		}
		
		Serial.print("Temp task exit");
  		vTaskDelete( NULL );
	}

	static String setInternalControlStatus()
	{		
		if (_isRunning) 
		{
			if(SettingsHandler::tempControlInternalEnabled)
			{
				if(failsafeTriggerInternal)
				{
					ledcWrite(CaseFan_PWM, SettingsHandler::caseFanPWM);
				} 
				else
				{
					float currentTemp = _currentInternalTemp;
					if (currentTemp < 0) 
					{
						_currentInternalStatus = "Error reading";
					} 
					else
					{
						if(definitelyGreaterThanOREssentiallyEqual(currentTemp, SettingsHandler::internalTempForFan)) {
							_currentInternalStatus = "Cooling";
							ledcWrite(CaseFan_PWM, SettingsHandler::caseFanPWM);
						} else {
							_currentInternalStatus = "Off";
							ledcWrite(CaseFan_PWM, 0);
						}
					}
				}
			}
		}
		return _currentInternalStatus;
	}

	static String setSleeveControlStatus()
	{		
		//Serial.println(_currentTemp);
		if (_isRunning) 
		{
			if(SettingsHandler::tempControlSleeveEnabled)
			{
				if(failsafeTriggerSleeve)
				{
					ledcWrite(Heater_PWM, 0);
				} 
				else
				{
					float currentTemp = _currentSleeveTemp;
					if (currentTemp < 0) 
					{
						_currentSleeveStatus = "Error reading";
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
								_currentSleeveStatus = "Warm up time: " + String(tminutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds);
							}
							else 
							{
								_currentSleeveStatus = "Heating";
							}
							if(targetSleeveTempReached && SettingsHandler::TargetTemp - currentTemp >= 5)
								targetSleeveTempReached = false;
						} 
						else if (definitelyLessThanOREssentiallyEqual(currentTemp, (SettingsHandler::TargetTemp + SettingsHandler::heaterThreshold))) 
						{
							if(!targetSleeveTempReached)
							{
								targetSleeveTempReached = true;
								// Serial.print("Adding to queue: "); 
								// Serial.println(_statusJson->c_str());
								String* command = new String("tempReached");
								xQueueSend(sleeveTempQueue, &command, 0);
							}
							ledcWrite(Heater_PWM, SettingsHandler::HoldPWM);
							_currentSleeveStatus = "Holding";
						} 
						else 
						{
							ledcWrite(Heater_PWM, 0);
							_currentSleeveStatus = "Cooling";
						}
					}
				}
			}
			else
			{
				_currentSleeveStatus = "Disabled";
			}
		}
		return _currentSleeveStatus;
	}

	static float getSleeveTemp()
	{
		return _currentSleeveTemp;
	}

	static String getSleeveControlStatus()
	{
		return _currentSleeveStatus;
	}
	static float getInternalTemp()
	{
		return _currentInternalTemp;
	}

	static String getInternalControlStatus()
	{
		return _currentInternalStatus;
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
			if(SettingsHandler::tempControlSleeveEnabled) {
				if(errorCountSleeve > 10) 
				{
					if(!failsafeTriggerSleeve) 
					{
						failsafeTriggerSleeve = true;
						String* command = new String("failSafeTriggered");
						xQueueSend(sleeveTempQueue, &command, 0);
						_currentSleeveStatus = "Fail safe: read";
					}
				} 
				else if(_currentSleeveStatus.startsWith("Error")) 
				{
					errorCountSleeve++;
				}
			}

			if(SettingsHandler::tempControlInternalEnabled) {
				if(errorCountInternal > 10) 
				{
					if(!failsafeTriggerInternal) 
					{
						failsafeTriggerInternal = true;
						String* command = new String("failSafeTriggered");
						xQueueSend(internalTempQueue, &command, 0);
						_currentInternalStatus = "Fail safe: read";
					}
				} 
				else if(_currentInternalStatus.startsWith("Error")) 
				{
					errorCountInternal++;
				}
			}
		}
	}
};

OneWire TemperatureHandler::oneWireInternal;
DallasTemperature TemperatureHandler::sensorsInternal;
float TemperatureHandler::_currentInternalTemp;
String TemperatureHandler::_currentInternalStatus = "Off";

float TemperatureHandler::_currentSleeveTemp = 0.0f;
String TemperatureHandler::_currentSleeveStatus = "Unknown";
bool TemperatureHandler::_isRunning = false;
OneWire TemperatureHandler::oneWireSleeve;
DallasTemperature TemperatureHandler::sensorsSleeve;
bool TemperatureHandler::bootTime;
long TemperatureHandler::bootTimer;
float TemperatureHandler::_lastSleeveTemp = -127.0;
bool TemperatureHandler::targetSleeveTempReached = false;
// int TemperatureHandler::failsafeTimer;
bool TemperatureHandler::failsafeTriggerSleeve = false;
bool TemperatureHandler::failsafeTriggerInternal = false;
// bool TemperatureHandler::failsafeTriggerLogged = false;
xSemaphoreHandle TemperatureHandler::tempMutexBus;
xSemaphoreHandle TemperatureHandler::statusMutexBus;
xQueueHandle TemperatureHandler::sleeveTempQueue;
xQueueHandle TemperatureHandler::internalTempQueue;
long TemperatureHandler::failSafeFrequency;
int TemperatureHandler::failSafeFrequencyLimiter = 10000;
int TemperatureHandler::errorCountSleeve = 0;
int TemperatureHandler::errorCountInternal = 0;

bool TemperatureHandler::internalTempInitialized = false;
bool TemperatureHandler::sleeveTempInitialized = false;

