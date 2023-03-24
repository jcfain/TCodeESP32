/* MIT License

Copyright (c) 2023 Jason C. Fain

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
//#include <AutoPID.h>
#include "TCode/Global.h"
#include "SettingsHandler.h"
#include "LogHandler.h"

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
	static const char* MAX_TEMP_ERROR;
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
const char* TemperatureState::MAX_TEMP_ERROR = "Max temp";
const char* TemperatureState::HOLD = "Holding";
const char* TemperatureState::HEAT = "Heating";
//Fan states
const char* TemperatureState::COOLING = "Cooling";
const char* TemperatureState::OFF = "Off";

using TEMP_CHANGE_FUNCTION_PTR_T = void (*)(TemperatureType type, const char* message, float temp);
using STATE_CHANGE_FUNCTION_PTR_T = void (*)(TemperatureType type, const char* state);

class TemperatureHandler {
	private: 
		char _TAG[5] = "temp";

		bool _isRunning = false;

		const int resolution = 9;
		const int delayInMillis = 750 / (1 << (12 - resolution));

		TEMP_CHANGE_FUNCTION_PTR_T message_callback = 0;
		STATE_CHANGE_FUNCTION_PTR_T state_change_callback = 0;

		long failSafeFrequency;
		int failSafeFrequencyLimiter = 10000;

		//Internal temp/Fan control
		OneWire oneWireInternal;
		DallasTemperature sensorsInternal;
		DeviceAddress internalDeviceAddress;
		//AutoPID* internalPID;
		double _currentInternalTemp = 0.0f;
		String m_lastInternalStatus = TemperatureState::UNKNOWN;
		double m_lastInternalTemp = -127.0;
		double m_currentInternalTempDuty = 0.0;
		double m_lastInternalTempDuty = 0.0;
		unsigned long lastInternalTempRequest = 0;

		int errorCountInternal = 0;
		bool internalTempInitialized = false;

		bool failsafeTriggerInternal = false;
		bool maxTempTriggerInternal = false;
		bool fanControlInitialized = false;
		/////////////////////////////////////////

		// Sleeve temp/Heater
		OneWire oneWireSleeve;
		DallasTemperature sensorsSleeve;
		DeviceAddress sleeveDeviceAddress;
		double _currentSleeveTemp = 0.0f;
		String m_lastSleeveStatus = TemperatureState::UNKNOWN;
		double m_lastSleeveTemp = -127.0;
		unsigned long lastSleeveTempRequest = 0;

		int errorCountSleeve = 0;
		bool sleeveTempInitialized = false;

		bool targetSleeveTempReached = false;
		bool failsafeTriggerSleeve = false;
		///////////////////////////////////////////


	bool definitelyGreaterThan(float a, float b)
	{
		return (a - b) > ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * __FLT_EPSILON__);
	}

	bool definitelyLessThan(float a, float b)
	{
		return (b - a) > ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) *  __FLT_EPSILON__);
	}
	bool essentiallyEqual(float a, float b)
	{
		return fabs(a - b) <= ( (fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * __FLT_EPSILON__);
	}
	bool approximatelyEqual(float a, float b)
	{
		return fabs(a - b) <= ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * __FLT_EPSILON__);
	}
	bool definitelyLessThanOREssentiallyEqual(float a, float b)
	{
		return definitelyLessThan(a, b) || essentiallyEqual(a, b);
	}
	bool definitelyLessThanORApproximatelyEqual(float a, float b)
	{
		return definitelyLessThan(a, b) || approximatelyEqual(a, b);
	}
	bool definitelyGreaterThanOREssentiallyEqual(float a, float b)
	{
		return definitelyGreaterThan(a, b) || essentiallyEqual(a, b);
	}
	void setState(TemperatureType type, const char* state) {
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
	void requestInternalTemp() {
		sensorsInternal.requestTemperaturesByIndex(0);
		lastInternalTempRequest = millis(); 
	}

	void requestSleeveTemp() {
		sensorsSleeve.requestTemperaturesByIndex(0);
		lastSleeveTempRequest = millis(); 
	}
	
	public: 
	// xQueueHandle sleeveTempQueue;
	// xQueueHandle internalTempQueue;

	void loop() {
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
	
	void setMessageCallback(TEMP_CHANGE_FUNCTION_PTR_T f) // Sets the callback function used by TCode
	{
		if (f == nullptr) {
			message_callback = 0;
		} else {
			message_callback = f;
		}
	}
	void setStateChangeCallback(STATE_CHANGE_FUNCTION_PTR_T f) // Sets the callback function used by TCode
	{
		if (f == nullptr) {
			state_change_callback = 0;
		} else {
			state_change_callback = f;
		}
	}

	void setup() {
		if(SettingsHandler::tempInternalEnabled) {
			setupInternalTemp();
		}
		if(SettingsHandler::tempSleeveEnabled) {
			setupSleeveTemp();
		}
	}

	void setupSleeveTemp() {
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

	void setupInternalTemp() {
		LogHandler::info(_TAG, "Starting internal temp on pin: %u", SettingsHandler::Internal_Temp_PIN);
		oneWireInternal.begin(SettingsHandler::Internal_Temp_PIN);
		sensorsInternal.setOneWire(&oneWireInternal);
  		sensorsInternal.getAddress(internalDeviceAddress, 0);
		sensorsInternal.begin();
  		sensorsInternal.setResolution(internalDeviceAddress, resolution);
		sensorsInternal.setWaitForConversion(false);
		//internalPID.setGains(0.12f, 0.0003f, 0);
		//internalPID.setOutputRange();
		requestInternalTemp();

		if(SettingsHandler::fanControlEnabled) {
			LogHandler::debug(_TAG, "Setting up fan, PIN: %i, hz: %i, resolution: %i, MAX PWM: %i", SettingsHandler::Case_Fan_PIN, SettingsHandler::caseFanFrequency, SettingsHandler::caseFanResolution, SettingsHandler::caseFanMaxDuty);
  			ledcSetup(CaseFan_PWM, SettingsHandler::caseFanFrequency, SettingsHandler::caseFanResolution);
			ledcAttachPin(SettingsHandler::Case_Fan_PIN, CaseFan_PWM);
			//LogHandler::debug(_TAG, "Setting up PID: Output max: %i", SettingsHandler::caseFanMaxDuty);
			//internalPID = new AutoPID(&_currentInternalTemp, &SettingsHandler::internalTempForFan, &m_currentInternalTempDuty, SettingsHandler::caseFanMaxDuty, 0, 0.12, 0.0003, 0.0); 
			// //if temperature is more than 4 degrees below or above setpoint, OUTPUT will be set to min or max respectively
			//internalPID->setBangBang(2, 0);
			// //set PID update interval to 4000ms
			// internalPID->setTimeStep(4000);
			fanControlInitialized = true;
		}
		internalTempInitialized = true;
	}

	static void startLoop(void * parameter) {
		((TemperatureHandler*)parameter)->loop();
	}

	bool isMaxTempTriggered() {
		return maxTempTriggerInternal;
	}

	void getInternalTemp() {
		if(SettingsHandler::tempInternalEnabled && internalTempInitialized 
			&& millis() - lastInternalTempRequest >= delayInMillis) {

			long start = micros();

			float currentInternalTemp = sensorsInternal.getTempC(internalDeviceAddress);
			bool tempChanged = false;

			if(!essentiallyEqual(_currentInternalTemp, m_lastInternalTemp)) {
				tempChanged = true;
				m_lastInternalTemp = _currentInternalTemp;
				LogHandler::debug(_TAG, "Last internal temp: %f", m_lastInternalTemp);
				//LogHandler::debug(_TAG, "Current internal temp: %f", _currentInternalTemp);
			}
			_currentInternalTemp = currentInternalTemp;
			//LogHandler::debug(_TAG, "Current internal temp: %f", _currentInternalTemp);

			LogHandler::verbose(_TAG, "internal getTempC duration: %ld", micros() - start);

			String statusJson("{\"temp\":" + String(_currentInternalTemp) + ", \"status\":\""+m_lastInternalStatus+"\"}");
			if(tempChanged && message_callback) {
				message_callback(TemperatureType::INTERNAL, statusJson.c_str(), _currentInternalTemp);
			}
			requestInternalTemp();
		}
	}

	void getSleeveTemp() {
		if(SettingsHandler::tempSleeveEnabled && sleeveTempInitialized 
			&& millis() - lastSleeveTempRequest >= delayInMillis) {
			long start = micros();

			float currentSleeveTemp = sensorsSleeve.getTempC(sleeveDeviceAddress);
			bool tempChanged = false;

			if(!essentiallyEqual(_currentSleeveTemp, m_lastSleeveTemp)) {
				tempChanged = true;
				m_lastSleeveTemp = _currentSleeveTemp;
			}
			_currentSleeveTemp = currentSleeveTemp;

			long duration = micros() - start;

			LogHandler::verbose(_TAG, "sleeve getTempC duration: %ld", micros() - start);

			String statusJson("{\"temp\":" + String(_currentSleeveTemp) + ", \"status\":\""+m_lastSleeveStatus+"\"}");
			if(tempChanged && message_callback) {
				message_callback(TemperatureType::SLEEVE, statusJson.c_str(), _currentSleeveTemp);
			}
			requestSleeveTemp();
		}
	}

	void setFanState() {		
		if (_isRunning) {
			String currentState;
			double currentDuty = SettingsHandler::caseFanMaxDuty;
			if(SettingsHandler::fanControlEnabled && fanControlInitialized) {
				if(failsafeTriggerInternal) {
					currentState = TemperatureState::FAIL_SAFE;
				} else if (maxTempTriggerInternal) {
					currentState = TemperatureState::MAX_TEMP_ERROR;
				} else {
					double currentTemp = _currentInternalTemp;
					//LogHandler::debug(_TAG, "Current global temp: %f", _currentInternalTemp);
					if (currentTemp == -127.0) {
						currentState = TemperatureState::ERROR;
					} else {
						// if(definitelyLessThanOREssentiallyEqual(currentTemp, SettingsHandler::internalTempForFan + 3) &&
						// 	definitelyGreaterThanOREssentiallyEqual(currentTemp, SettingsHandler::internalTempForFan - 3)) {
						// 	currentState = TemperatureState::COOLING;
						// 	currentDuty = SettingsHandler::caseFanMaxDuty * 0.8;
						// } else 
						if(definitelyGreaterThanOREssentiallyEqual(currentTemp, SettingsHandler::internalTempForFan) || 
							(definitelyGreaterThanOREssentiallyEqual(currentTemp, SettingsHandler::internalTempForFan - 5) && m_lastInternalTempDuty > 0)) {
							//LogHandler::debug(_TAG, "definitelyGreaterThanOREssentiallyEqual: %f >= %f", currentTemp, SettingsHandler::internalTempForFan);
							currentState = TemperatureState::COOLING;
								// Calculate pwm based on user entered values.
								// double maxTemp = SettingsHandler::internalTempForFan + SettingsHandler::internalTempForFan * 0.50;
								// if(definitelyGreaterThanOREssentiallyEqual(currentTemp, maxTemp))
								// 	maxTemp = currentTemp;
								//  m_currentInternalTempDuty = map(currentTemp, 
								// 				SettingsHandler::internalTempForFan, 
								// 				maxTemp, 
								// 				50, // Min duty.
								// 				SettingsHandler::caseFanMaxDuty);
								// 				// https://sciencing.com/calculate-pulse-width-8618299.html
								// 				// Period = 1/Frequency
								// 				// Frequency = 1/Period
								// 				// duty cycle = PulseWidth/Period
								// 				// duty percentage = (duty/SettingsHandler::caseFanPWM) * 100

// if (currentTemp < SettingsHandler::internalTempForFan) {  // Fan at 10% over 27 degree
//   	m_lastInternalTempDuty = 0;
//   } else if (currentTemp > SettingsHandler::internalTempForFan + round(SettingsHandler::internalTempForFan * 0.50)){
//   	m_lastInternalTempDuty = 800;
//   } else if (currentTemp > SettingsHandler::internalTempForFan + round(SettingsHandler::internalTempForFan * 0.40)){
//   	m_lastInternalTempDuty = 500;
//   } else if (currentTemp > SettingsHandler::internalTempForFan + round(SettingsHandler::internalTempForFan * 0.30)){
//   	m_lastInternalTempDuty = 200;
//   } else if (currentTemp > SettingsHandler::internalTempForFan){
//   	m_lastInternalTempDuty = 50;
//   } 
							//  if(m_lastInternalTempDuty != m_currentInternalTempDuty) {
							// 	m_lastInternalTempDuty = m_currentInternalTempDuty;
							// // 	m_lastInternalTempDuty = constrain(duty, 50, SettingsHandler::caseFanPWM);
							// 	LogHandler::debug(_TAG, "Setting fan duty: %f, Max duty: %i", m_lastInternalTempDuty, SettingsHandler::caseFanMaxDuty);
							// 	// LogHandler::debug(_TAG, "Current temp: %f, maxTemp: %f, fan on temp: %f", currentTemp, maxTemp, SettingsHandler::internalTempForFan);
							// 	LogHandler::debug(_TAG, "Current temp: %f,  fan on temp: %f", _currentInternalTemp, SettingsHandler::internalTempForFan);
							//  }
							
						} else {
							currentState = TemperatureState::OFF;
							currentDuty = 0;
						}
					}
				}
				if (m_lastInternalTempDuty != currentDuty) {
					m_lastInternalTempDuty = currentDuty;
					LogHandler::debug(_TAG, "Setting fan duty: %f", m_lastInternalTempDuty);
				}
				ledcWrite(CaseFan_PWM, currentDuty);
			} else {
				if(SettingsHandler::fanControlEnabled && !fanControlInitialized)
					currentState = TemperatureState::RESTART_REQUIRED;
				else
					currentState = TemperatureState::DISABLED_STATE;
			}
			if(!currentState.isEmpty()) {
				if(m_lastInternalStatus != currentState) {
					LogHandler::debug(_TAG, "Setting fan state: %s", currentState);
				}
				setState(TemperatureType::INTERNAL, currentState.c_str());
			}
		}
	}

	void setHeaterState() {		
		//Serial.println(_currentTemp);
		if (_isRunning) {
			String currentState;
			if(SettingsHandler::tempSleeveEnabled && sleeveTempInitialized) {
				if(failsafeTriggerSleeve) {
					ledcWrite(Heater_PWM, 0);
				} 
				else {
					double currentTemp = _currentSleeveTemp;
					if (currentTemp == -127.0) {
						currentState = TemperatureState::ERROR;
					} else {
						// if(currentTemp >= SettingsHandler::TargetTemp || millis() >= bootTimer)
						// 	bootTime = false;
						if (definitelyLessThan(currentTemp, SettingsHandler::TargetTemp) 
						//|| (currentTemp > 0 && bootTime)
						) {
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
						} else if (definitelyLessThanOREssentiallyEqual(currentTemp, (SettingsHandler::TargetTemp + SettingsHandler::heaterThreshold))) {
							if(!targetSleeveTempReached) {
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
						} else {
							ledcWrite(Heater_PWM, 0);
							currentState = TemperatureState::OFF;
						}
					}
				}
			} else {
				if(SettingsHandler::tempSleeveEnabled && !sleeveTempInitialized)
					currentState = TemperatureState::RESTART_REQUIRED;
				else
					currentState = TemperatureState::DISABLED_STATE;
			}
			if(!currentState.isEmpty())
				setState(TemperatureType::SLEEVE, currentState.c_str());
		}
	}

	const char* getShortSleeveControlStatus(const char* state) {
		if(strcmp(state, TemperatureState::FAIL_SAFE) == 0) {
			return "F";
		} else if(strcmp(state, TemperatureState::ERROR) == 0) {
			return "E";
		} else if(strcmp(state, TemperatureState::MAX_TEMP_ERROR) == 0) {
			return "M";
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

	void stop() {
		_isRunning = false;
	}

	bool isRunning() {
		return _isRunning ;
	}

	void chackFailSafe() {
		if(millis() >= failSafeFrequency) {
			failSafeFrequency = millis() + failSafeFrequencyLimiter;
			if(SettingsHandler::tempSleeveEnabled) {
				if(errorCountSleeve > 10) {
					if(!failsafeTriggerSleeve) {
						failsafeTriggerSleeve = true;
						if(message_callback)
							message_callback(TemperatureType::SLEEVE, "failSafeTriggered", _currentSleeveTemp);
						setState(TemperatureType::SLEEVE, TemperatureState::FAIL_SAFE);
					}
				} else if(m_lastSleeveStatus == TemperatureState::ERROR) {
					errorCountSleeve++;
				} else {
					errorCountSleeve = 0;
				}
			}

			if(SettingsHandler::tempInternalEnabled) {
				if(definitelyGreaterThanOREssentiallyEqual(_currentInternalTemp, SettingsHandler::internalMaxTemp)) {
					if(!failsafeTriggerInternal && !maxTempTriggerInternal) {
						maxTempTriggerInternal = true;
						if(message_callback)
							message_callback(TemperatureType::INTERNAL, "failSafeTriggered", _currentInternalTemp);
						setState(TemperatureType::INTERNAL, TemperatureState::MAX_TEMP_ERROR);
					}
				} 
				if(errorCountInternal > 10) {
					if(!failsafeTriggerInternal && !maxTempTriggerInternal) {
						failsafeTriggerInternal = true;
						if(message_callback)
							message_callback(TemperatureType::INTERNAL, "failSafeTriggered", _currentInternalTemp);
						setState(TemperatureType::INTERNAL, TemperatureState::FAIL_SAFE);
					}
				} else if(m_lastInternalStatus == TemperatureState::ERROR) {
					errorCountInternal++;
				} else {
					errorCountInternal = 0;
				}
			}
		}
	}
};
