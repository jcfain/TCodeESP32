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

enum class TemperatureType {
	INTERNAL,
	SLEEVE
};

class TemperatureState {
	public:
	// Global state
	static const int MAX_LEN;
	static const char* UNKNOWN;
	static const char* DISABLED_STATE;
	static const char* RESTART_REQUIRED;
	// Heater state
	static const char* FAIL_SAFE;
	static const char* ERROR;
	static const char* WARM_UP;
	static const char* HOLD;
	static const char* HEAT;
	//Fan states
	static const char* COOLING;
	static const char* OFF;
};
// Global state
const int TemperatureState::MAX_LEN = 10;
const char* TemperatureState::UNKNOWN = "Unknown";
const char* TemperatureState::DISABLED_STATE = "Disabled";
const char* TemperatureState::RESTART_REQUIRED = "Restart";
// Heater state
const char* TemperatureState::FAIL_SAFE = "Fail safe";
const char* TemperatureState::ERROR = "Error";
const char* TemperatureState::WARM_UP = "Warm up";
const char* TemperatureState::HOLD = "Holding";
const char* TemperatureState::HEAT = "Heating";
//Fan states
const char* TemperatureState::COOLING = "Cooling";
const char* TemperatureState::OFF = "Off";

using TEMP_CHANGE_FUNCTION_PTR_T = void (*)(TemperatureType type, const char* message, float temp);
using STATE_CHANGE_FUNCTION_PTR_T = void (*)(TemperatureType type, const char* state);

class TemperatureHandler
{
	private: 
	static char _TAG[5];
    static TEMP_CHANGE_FUNCTION_PTR_T message_callback;
    static STATE_CHANGE_FUNCTION_PTR_T state_change_callback;

	static int resolution;
	static int delayInMillis;

	static unsigned long lastSleeveTempRequest;
	static unsigned long lastInternalTempRequest;

	static OneWire oneWireInternal;
	static DallasTemperature sensorsInternal;
	static DeviceAddress internalDeviceAddress;
	static float _currentInternalTemp;

	static OneWire oneWireSleeve;
	static DallasTemperature sensorsSleeve;
	static DeviceAddress sleeveDeviceAddress;
	static float _currentSleeveTemp;
	static String m_lastInternalStatus;
	static float _lastSleeveTemp;
	static String m_lastSleeveStatus;
	static bool _isRunning;
	// static bool bootTime;
	static bool targetSleeveTempReached;
	// static int failsafeTimer;
	static bool failsafeTriggerSleeve;
	static bool failsafeTriggerInternal;
	// static bool failsafeTriggerLogged;
	// static long bootTimer;
	// static xSemaphoreHandle sleeveTempMutexBus;
	// static xSemaphoreHandle sleeveStatusMutexBus;
	// static xSemaphoreHandle internalTempMutexBus;
	// static xSemaphoreHandle internalStatusMutexBus;
	static long failSafeFrequency;
	static int failSafeFrequencyLimiter;
	static int errorCountSleeve;
	static int errorCountInternal;
	static bool sleeveTempInitialized;
	static bool internalTempInitialized;
	static bool fanControlInitialized;


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
	static void setState(TemperatureType type, const char* state) {
		bool stateChanged = false;
		if(type == TemperatureType::INTERNAL) {
			stateChanged = strcmp(state, m_lastInternalStatus.c_str()) != 0;
			if(stateChanged)
				m_lastInternalStatus = state;
		} else {
			stateChanged = strcmp(state, m_lastSleeveStatus.c_str()) != 0;
			if(stateChanged)
				m_lastSleeveStatus = state;
		}
		if(state_change_callback && stateChanged) {		
			state_change_callback(type, state);
		}
	}
	
	public: 
	// static xQueueHandle sleeveTempQueue;
	// static xQueueHandle internalTempQueue;

	
	static void setMessageCallback(TEMP_CHANGE_FUNCTION_PTR_T f) // Sets the callback function used by TCode
	{
		if (f == nullptr) {
			message_callback = 0;
		} else {
			message_callback = f;
		}
	}
	static void setStateChangeCallback(STATE_CHANGE_FUNCTION_PTR_T f) // Sets the callback function used by TCode
	{
		if (f == nullptr) {
			state_change_callback = 0;
		} else {
			state_change_callback = f;
		}
	}

	static void setup() {
  		delayInMillis = 750 / (1 << (12 - resolution)); 
	}

	static void setupSleeveTemp()
	{
		LogHandler::info(_TAG, "Starting sleeve temp on pin: %u", SettingsHandler::Sleeve_Temp_PIN);
		oneWireSleeve.begin(SettingsHandler::Sleeve_Temp_PIN);
		sensorsSleeve.setOneWire(&oneWireSleeve);
  		sensorsSleeve.getAddress(sleeveDeviceAddress, 0);
		sensorsSleeve.begin();
  		sensorsSleeve.setResolution(sleeveDeviceAddress, resolution);
		sensorsSleeve.setWaitForConversion(false);
		requestSleeveTemp();

		// bootTime = true;
		// bootTimer = millis() + SettingsHandler::WarmUpTime;
		LogHandler::debug(_TAG, "Starting heat on pin: %u", SettingsHandler::Heater_PIN);
  		ledcSetup(Heater_PWM, SettingsHandler::heaterFrequency, SettingsHandler::heaterResolution);
		ledcAttachPin(SettingsHandler::Heater_PIN, Heater_PWM);
		sleeveTempInitialized = true;
	}

	static void setupInternalTemp()
	{
		LogHandler::info(_TAG, "Starting internal temp on pin: %u", SettingsHandler::Internal_Temp_PIN);
		oneWireInternal.begin(SettingsHandler::Internal_Temp_PIN);
		sensorsInternal.setOneWire(&oneWireInternal);
  		sensorsInternal.getAddress(internalDeviceAddress, 0);
		sensorsInternal.begin();
  		sensorsInternal.setResolution(internalDeviceAddress, resolution);
		sensorsInternal.setWaitForConversion(false);
		requestInternalTemp();

		if(SettingsHandler::fanControlEnabled) {
			LogHandler::debug(_TAG, "Setting up fan hz: %i, resolution: %i, max PWM: %i", SettingsHandler::caseFanFrequency, SettingsHandler::caseFanResolution, SettingsHandler::caseFanPWM);
  			ledcSetup(CaseFan_PWM, SettingsHandler::caseFanFrequency, SettingsHandler::caseFanResolution);
			ledcAttachPin(SettingsHandler::Case_Fan_PIN, CaseFan_PWM);
  			// internalStatusMutexBus = xSemaphoreCreateMutex();
			fanControlInitialized = true;
		}
		internalTempInitialized = true;
	}

	static void startLoop(void * parameter)
	{
		_isRunning = true;
		LogHandler::debug(_TAG, "Temp Core: %u", xPortGetCoreID());
		lastSleeveTempRequest = millis(); 
		while(_isRunning)
		{
			getInternalTemp();
			getSleeveTemp();
			chackFailSafe();

        	vTaskDelay(1000/portTICK_PERIOD_MS);
			// Serial.print("uxTaskGetStackHighWaterMark: ");
			// Serial.println(uxTaskGetStackHighWaterMark(NULL) *4);
			// Serial.print("xPortGetFreeHeapSize: ");
			// Serial.println(xPortGetFreeHeapSize());
		}
		
		LogHandler::debug(_TAG, "Temp task exit");
  		vTaskDelete( NULL );
	}

	static void requestInternalTemp() {
		sensorsInternal.requestTemperaturesByIndex(0);
		lastInternalTempRequest = millis(); 
	}

	static void getInternalTemp() {
		if(SettingsHandler::tempInternalEnabled && internalTempInitialized 
			&& millis() - lastInternalTempRequest >= delayInMillis) {

			long start = micros();

			_currentInternalTemp = sensorsInternal.getTempC(internalDeviceAddress);

			LogHandler::verbose(_TAG, "internal getTempC duration: %ld", micros() - start);

			String statusJson("{\"temp\":" + String(_currentInternalTemp) + ", \"status\":\""+m_lastInternalStatus+"\"}");
			if(message_callback) {
				message_callback(TemperatureType::INTERNAL, statusJson.c_str(), _currentInternalTemp);
			}
			requestInternalTemp();
		}
	}

	static void requestSleeveTemp() {
		sensorsSleeve.requestTemperaturesByIndex(0);
		lastSleeveTempRequest = millis(); 
	}

	static void getSleeveTemp() {
		if(SettingsHandler::tempSleeveEnabled && sleeveTempInitialized 
			&& millis() - lastSleeveTempRequest >= delayInMillis) {
			long start = micros();

			_currentSleeveTemp = sensorsSleeve.getTempC(sleeveDeviceAddress);

			long duration = micros() - start;

			LogHandler::verbose(_TAG, "sleeve getTempC duration: %ld", micros() - start);

			String statusJson("{\"temp\":" + String(_currentSleeveTemp) + ", \"status\":\""+m_lastSleeveStatus+"\"}");
			if(message_callback) {
				message_callback(TemperatureType::SLEEVE, statusJson.c_str(), _currentSleeveTemp);
			}
			requestSleeveTemp();
		}
	}

	static void setFanState()
	{		
		if (_isRunning) 
		{
			String currentState;
			if(SettingsHandler::fanControlEnabled && fanControlInitialized)
			{
				if(failsafeTriggerInternal)
				{
					ledcWrite(CaseFan_PWM, SettingsHandler::caseFanPWM);
				} 
				else
				{
					float currentTemp = _currentInternalTemp;
					if (currentTemp == -127) 
					{
						currentState = TemperatureState::ERROR;
					} 
					else
					{
						if(definitelyGreaterThanOREssentiallyEqual(currentTemp, SettingsHandler::internalTempForFan)) {

							currentState = TemperatureState::COOLING;
							// Calculate pwm based on user entered values.
							int pwm = map(round(currentTemp), 
											SettingsHandler::internalTempForFan, 
											SettingsHandler::internalTempForFan + round(SettingsHandler::internalTempForFan * 0.50), 
											50, // Min duty.
											SettingsHandler::caseFanPWM);
							//LogHandler::verbose(_TAG, "Setting fan PWM: %i, maxTemp: %f", pwm, SettingsHandler::internalTempForFan + round(SettingsHandler::internalTempForFan * 0.50));

							ledcWrite(CaseFan_PWM, pwm);
						} else {
							currentState = TemperatureState::OFF;
							ledcWrite(CaseFan_PWM, 0);
						}
					}
				}
			}
			else
			{
				if(SettingsHandler::fanControlEnabled && !fanControlInitialized)
					currentState = TemperatureState::RESTART_REQUIRED;
				else
					currentState = TemperatureState::DISABLED_STATE;
			}
			if(!currentState.isEmpty())
				setState(TemperatureType::INTERNAL, currentState.c_str());
		}
	}

	static void setHeaterState()
	{		
		//Serial.println(_currentTemp);
		if (_isRunning) 
		{
			String currentState;
			if(SettingsHandler::tempSleeveEnabled && sleeveTempInitialized)
			{
				if(failsafeTriggerSleeve)
				{
					ledcWrite(Heater_PWM, 0);
				} 
				else
				{
					float currentTemp = _currentSleeveTemp;
					if (currentTemp == -127) 
					{
						currentState = TemperatureState::ERROR;
					} 
					else
					{
						// if(currentTemp >= SettingsHandler::TargetTemp || millis() >= bootTimer)
						// 	bootTime = false;
						if (definitelyLessThan(currentTemp, SettingsHandler::TargetTemp) 
						//|| (currentTemp > 0 && bootTime)
						) 
						{
							ledcWrite(Heater_PWM, SettingsHandler::HeatPWM);

							// if(bootTime) 
							// {
							// 	long time = bootTimer - millis();
							// 	int tseconds = time / 1000;
							// 	int tminutes = tseconds / 60;
							// 	int seconds = tseconds % 60;
							// 	currentState = "Warm up time: " + String(tminutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds);
							// }
							// else 
							// {
								currentState = TemperatureState::HEAT;
							// }
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
								if(message_callback)
									message_callback(TemperatureType::SLEEVE, "tempReached", _currentSleeveTemp);
								//xQueueSend(sleeveTempQueue, &command, 0);
							}
							ledcWrite(Heater_PWM, SettingsHandler::HoldPWM);
							currentState = TemperatureState::HOLD;
						} 
						else 
						{
							ledcWrite(Heater_PWM, 0);
							currentState = TemperatureState::OFF;
						}
					}
				}
			}
			else
			{
				if(SettingsHandler::tempSleeveEnabled && !sleeveTempInitialized)
					currentState = TemperatureState::RESTART_REQUIRED;
				else
					currentState = TemperatureState::DISABLED_STATE;
			}
			if(!currentState.isEmpty())
				setState(TemperatureType::SLEEVE, currentState.c_str());
		}
	}

	static const char* getShortSleeveControlStatus(const char* state)
	{
		if(strcmp(state, TemperatureState::FAIL_SAFE) == 0) {
			return "F";
		} else if(strcmp(state, TemperatureState::ERROR) == 0) {
			return "E";
		} else if(strcmp(state, TemperatureState::WARM_UP) == 0) {
			return "W";
		} else if(strcmp(state, TemperatureState::HOLD) == 0) {
			return "S";
		} else if(strcmp(state, TemperatureState::HEAT) == 0) {
			return "H";
		} else if(strcmp(state, TemperatureState::UNKNOWN) == 0) {
			return "U";
		}
		//TemperatureState::OFF
		return "O";
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
			if(SettingsHandler::tempSleeveEnabled) {
				if(errorCountSleeve > 10) 
				{
					if(!failsafeTriggerSleeve) 
					{
						failsafeTriggerSleeve = true;
						// String* command = new String("failSafeTriggered");
						// xQueueSend(sleeveTempQueue, &command, 0);
						if(message_callback)
							message_callback(TemperatureType::SLEEVE, "failSafeTriggered", _currentSleeveTemp);
						setState(TemperatureType::SLEEVE, TemperatureState::FAIL_SAFE);
					}
				} 
				else if(m_lastSleeveStatus == TemperatureState::ERROR) 
				{
					errorCountSleeve++;
				}
			}

			if(SettingsHandler::tempInternalEnabled) {
				if(errorCountInternal > 10) 
				{
					if(!failsafeTriggerInternal) 
					{
						failsafeTriggerInternal = true;
						// String* command = new String("failSafeTriggered");
						// xQueueSend(internalTempQueue, &command, 0);
						if(message_callback)
							message_callback(TemperatureType::INTERNAL, "failSafeTriggered", _currentInternalTemp);
						setState(TemperatureType::INTERNAL, TemperatureState::FAIL_SAFE);
					}
				} 
				else if(m_lastInternalStatus == TemperatureState::ERROR) 
				{
					errorCountInternal++;
				}
			}
		}
	}
};

char TemperatureHandler::_TAG[5] = "temp";

int TemperatureHandler::resolution = 9;
int TemperatureHandler::delayInMillis = 0;
unsigned long TemperatureHandler::lastSleeveTempRequest = 0;
unsigned long TemperatureHandler::lastInternalTempRequest = 0;

OneWire TemperatureHandler::oneWireInternal;
DallasTemperature TemperatureHandler::sensorsInternal;
DeviceAddress TemperatureHandler::internalDeviceAddress;
float TemperatureHandler::_currentInternalTemp;
String TemperatureHandler::m_lastInternalStatus = TemperatureState::UNKNOWN;
TEMP_CHANGE_FUNCTION_PTR_T TemperatureHandler::message_callback = 0;
STATE_CHANGE_FUNCTION_PTR_T TemperatureHandler::state_change_callback = 0;

float TemperatureHandler::_currentSleeveTemp = 0.0f;
String TemperatureHandler::m_lastSleeveStatus = TemperatureState::UNKNOWN;
bool TemperatureHandler::_isRunning = false;
OneWire TemperatureHandler::oneWireSleeve;
DallasTemperature TemperatureHandler::sensorsSleeve;
DeviceAddress TemperatureHandler::sleeveDeviceAddress;
// bool TemperatureHandler::bootTime;
// long TemperatureHandler::bootTimer;
float TemperatureHandler::_lastSleeveTemp = -127.0;
bool TemperatureHandler::targetSleeveTempReached = false;
// int TemperatureHandler::failsafeTimer;
bool TemperatureHandler::failsafeTriggerSleeve = false;
bool TemperatureHandler::failsafeTriggerInternal = false;
// bool TemperatureHandler::failsafeTriggerLogged = false;
// xSemaphoreHandle TemperatureHandler::sleeveTempMutexBus;
// xSemaphoreHandle TemperatureHandler::sleeveStatusMutexBus;
// xSemaphoreHandle TemperatureHandler::internalTempMutexBus;
// xSemaphoreHandle TemperatureHandler::internalStatusMutexBus;
// xQueueHandle TemperatureHandler::sleeveTempQueue;
// xQueueHandle TemperatureHandler::internalTempQueue;
long TemperatureHandler::failSafeFrequency;
int TemperatureHandler::failSafeFrequencyLimiter = 10000;
int TemperatureHandler::errorCountSleeve = 0;
int TemperatureHandler::errorCountInternal = 0;

bool TemperatureHandler::internalTempInitialized = false;
bool TemperatureHandler::sleeveTempInitialized = false;
bool TemperatureHandler::fanControlInitialized = false;

