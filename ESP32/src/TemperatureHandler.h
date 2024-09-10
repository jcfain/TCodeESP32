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

#include <soc/gpio_struct.h>
#include <OneWire.h>
#include <DallasTemperature.h>
//#include <AutoPID.h>
#include "TCode/Global.h"
#include "SettingsHandler.h"
// #include "LogHandler.h"
#include "TagHandler.h"

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
	
public: 
	void setup(bool internalTempEnabled,
				bool sleeveTempEnabled,
				int8_t sleeveTempPin, 
				int8_t internalTempPin, 
				int8_t heaterPin, 
				int8_t heaterChannel, 
				int8_t caseFanPin,
				int8_t caseFanChannel,
				int heaterFrequency,
				int heaterResolution,
				bool fanControlEnabled,
				int fanFrequency,
				int fanResolution) {
		m_settingsFactory = SettingsFactory::getInstance();
		m_internalTempEnabled = internalTempEnabled;
		m_sleeveTempEnabled = sleeveTempEnabled;
		m_sleeveTempPin = sleeveTempPin;
		m_internalTempPin = internalTempPin; 
		m_heaterPin = heaterPin;
		m_heatChannel = heaterChannel;
		m_caseFanPin = caseFanPin;
		m_caseFanChannel = caseFanChannel;
		m_heaterFrequency = heaterFrequency;
		m_heaterResolution = heaterResolution;
		m_fanControlEnabled = fanControlEnabled;
		m_fanFrequency = fanFrequency;
		m_fanResolution = fanResolution;
		if(m_internalTempEnabled) {
			setupInternalTemp();
		}
		if(m_sleeveTempEnabled) {
			setupSleeveTemp();
		}
	}

	void setupSleeveTemp() {
		if(m_sleeveTempPin < 0 ) {
			return;
		}
		LogHandler::info(_TAG, "Starting sleeve temp on pin: %d", m_sleeveTempPin);
		oneWireSleeve.begin(m_sleeveTempPin);
		sensorsSleeve.setOneWire(&oneWireSleeve);
		sensorsSleeve.getAddress(sleeveDeviceAddress, 0);
		sensorsSleeve.begin();
		sensorsSleeve.setResolution(sleeveDeviceAddress, resolution);
		sensorsSleeve.setWaitForConversion(false);
		requestSleeveTemp();

		// bootTime = true;
		// bootTimer = millis() + SettingsHandler::getWarmUpTime();
		if(m_heaterPin > -1 && m_heatChannel > -1) {
			LogHandler::debug(_TAG, "Starting heat on pin: %d", m_heaterPin);
			#ifdef ESP_ARDUINO3
			ledcAttachChannel(m_heaterPin, m_heaterFrequency, m_heaterResolution, m_heatChannel);
			#else
			ledcSetup(m_heatChannel, m_heaterFrequency, m_heaterResolution);
			ledcAttachPin(m_heaterPin, m_heatChannel);
			#endif
		}
		sleeveTempInitialized = true;
	}

	void setupInternalTemp() {
		if(m_internalTempPin < 0) {
			return;
		}
		LogHandler::info(_TAG, "Starting internal temp on pin: %d", m_internalTempPin);
		oneWireInternal.begin(m_internalTempPin);
		sensorsInternal.setOneWire(&oneWireInternal);
		sensorsInternal.getAddress(internalDeviceAddress, 0);
		sensorsInternal.begin();
		sensorsInternal.setResolution(internalDeviceAddress, resolution);
		sensorsInternal.setWaitForConversion(false);
		//internalPID.setGains(0.12f, 0.0003f, 0);
		//internalPID.setOutputRange();
		requestInternalTemp();

		if(m_fanControlEnabled && m_caseFanPin > -1 && m_caseFanChannel > -1) {
			LogHandler::debug(_TAG, "Setting up fan, PIN: %i, hz: %i, resolution: %i, MAX PWM: %i", m_caseFanPin, m_fanFrequency, m_fanResolution, m_fanMaxDuty);
			
			#ifdef ESP_ARDUINO3
			ledcAttachChannel(m_caseFanPin, m_fanFrequency, m_fanResolution, m_caseFanChannel);
			#else
			ledcSetup(m_caseFanChannel, m_fanFrequency, m_fanResolution);
			ledcAttachPin(m_caseFanPin, m_caseFanChannel);
			#endif
			//LogHandler::debug(_TAG, "Setting up PID: Output max: %i", m_fanMaxDuty);
			//internalPID = new AutoPID(&_currentInternalTemp, &SettingsHandler::getInternalTempForFan(), &m_currentInternalTempDuty, m_fanMaxDuty, 0, 0.12, 0.0003, 0.0); 
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

	void loop() {
		_isRunning = true;
		LogHandler::debug(_TAG, "Temp task cpu core: %u", xPortGetCoreID());
		lastSleeveTempRequest = millis(); 
        TickType_t pxPreviousWakeTime = millis();
		while(_isRunning)
		{
			getInternalTemp();
			getSleeveTemp();
			chackFailSafe();

            xTaskDelayUntil(&pxPreviousWakeTime, 5000/portTICK_PERIOD_MS);
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
	bool isMaxTempTriggered() {
		return maxTempTriggerInternal;
	}

	void getInternalTemp() {
		if(m_internalTempEnabled && internalTempInitialized 
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

			LogHandler::verbose(_TAG, "internal getTempC duration: %d", micros() - start);

			String statusJson("{\"temp\":\"" + String(_currentInternalTemp) + "\", \"status\":\""+m_lastInternalStatus+"\"}");
			if(tempChanged && message_callback) {
				message_callback(TemperatureType::INTERNAL, statusJson.c_str(), _currentInternalTemp);
			}
			requestInternalTemp();
		}
	}

	void getSleeveTemp() {
		if(m_sleeveTempEnabled && sleeveTempInitialized 
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

			LogHandler::verbose(_TAG, "sleeve getTempC duration: %d", micros() - start);

			String statusJson("{\"temp\":\"" + String(_currentSleeveTemp) + "\", \"status\":\""+m_lastSleeveStatus+"\"}");
			if(tempChanged && message_callback) {
				message_callback(TemperatureType::SLEEVE, statusJson.c_str(), _currentSleeveTemp);
			}
			requestSleeveTemp();
		}
	}

	void setFanState() {		
		if (m_caseFanPin < 0 || m_caseFanChannel < 0 || !m_fanControlEnabled || !_isRunning || !fanControlInitialized) {
			return;
		}
		String currentState;
		if(fanControlInitialized) {
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
					// if(definitelyLessThanOREssentiallyEqual(currentTemp, SettingsHandler::getInternalTempForFan() + 3) &&
					// 	definitelyGreaterThanOREssentiallyEqual(currentTemp, SettingsHandler::getInternalTempForFan() - 3)) {
					// 	currentState = TemperatureState::COOLING;
					// 	currentDuty = m_fanMaxDuty * 0.8;
					// } else 
					if(definitelyGreaterThanOREssentiallyEqual(currentTemp, m_settingsFactory->getInternalTempForFanOn()) || 
						(definitelyGreaterThanOREssentiallyEqual(currentTemp, m_settingsFactory->getInternalMaxTemp() - 5) && m_lastInternalTempDuty > 0)) {
						//LogHandler::debug(_TAG, "definitelyGreaterThanOREssentiallyEqual: %f >= %f", currentTemp, SettingsHandler::getInternalTempForFan());
						currentState = TemperatureState::COOLING;
							// Calculate pwm based on user entered values.
							// double maxTemp = SettingsHandler::getInternalTempForFan() + SettingsHandler::getInternalTempForFan() * 0.50;
							// if(definitelyGreaterThanOREssentiallyEqual(currentTemp, maxTemp))
							// 	maxTemp = currentTemp;
							//  m_currentInternalTempDuty = map(currentTemp, 
							// 				SettingsHandler::getInternalTempForFan(), 
							// 				maxTemp, 
							// 				50, // Min duty.
							// 				m_fanMaxDuty);
							// 				// https://sciencing.com/calculate-pulse-width-8618299.html
							// 				// Period = 1/Frequency
							// 				// Frequency = 1/Period
							// 				// duty cycle = PulseWidth/Period
							// 				// duty percentage = (duty/SettingsHandler::getCaseFanPWM()) * 100

// if (currentTemp < SettingsHandler::getInternalTempForFan()) {  // Fan at 10% over 27 degree
//   	m_lastInternalTempDuty = 0;
//   } else if (currentTemp > SettingsHandler::getInternalTempForFan() + round(SettingsHandler::getInternalTempForFan() * 0.50)){
//   	m_lastInternalTempDuty = 800;
//   } else if (currentTemp > SettingsHandler::getInternalTempForFan() + round(SettingsHandler::getInternalTempForFan() * 0.40)){
//   	m_lastInternalTempDuty = 500;
//   } else if (currentTemp > SettingsHandler::getInternalTempForFan() + round(SettingsHandler::getInternalTempForFan() * 0.30)){
//   	m_lastInternalTempDuty = 200;
//   } else if (currentTemp > SettingsHandler::getInternalTempForFan()){
//   	m_lastInternalTempDuty = 50;
//   } 
						//  if(m_lastInternalTempDuty != m_currentInternalTempDuty) {
						// 	m_lastInternalTempDuty = m_currentInternalTempDuty;
						// // 	m_lastInternalTempDuty = constrain(duty, 50, SettingsHandler::getCaseFanPWM());
						// 	LogHandler::debug(_TAG, "Setting fan duty: %f, Max duty: %i", m_lastInternalTempDuty, m_fanMaxDuty);
						// 	// LogHandler::debug(_TAG, "Current temp: %f, maxTemp: %f, fan on temp: %f", currentTemp, maxTemp, SettingsHandler::getInternalTempForFan());
						// 	LogHandler::debug(_TAG, "Current temp: %f,  fan on temp: %f", _currentInternalTemp, SettingsHandler::getInternalTempForFan());
						//  }
						
					} else {
						currentState = TemperatureState::OFF;
						m_fanMaxDuty = 0;
					}
				}
			}
			if (m_lastInternalTempDuty != m_fanMaxDuty) {
				m_lastInternalTempDuty = m_fanMaxDuty;
				LogHandler::debug(_TAG, "Setting fan duty: %f", m_lastInternalTempDuty);
			}
			#if ARDUINO_V3
			ledcWrite(m_caseFanPin, m_fanMaxDuty);
			#else
			ledcWrite(m_caseFanChannel, m_fanMaxDuty);
			#endif
		} else {
			if(m_fanControlEnabled && !fanControlInitialized)
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

	void setHeaterState() {		
		//Serial.println(_currentTemp);
		if (m_heaterPin < 0 || m_heatChannel < 0 || !m_sleeveTempEnabled || !_isRunning || !sleeveTempInitialized) {
			return;
		}
		String currentState;
		if(m_sleeveTempEnabled && sleeveTempInitialized) {
			if(failsafeTriggerSleeve) {
				#if ARDUINO_V3
				ledcWrite(m_heatPin, 0);
				#else
				ledcWrite(m_heatChannel, 0);
				#endif
			} 
			else {
				double currentTemp = _currentSleeveTemp;
				if (currentTemp == -127.0) {
					currentState = TemperatureState::ERROR;
				} else {
					// if(currentTemp >= SettingsHandler::getTargetTemp() || millis() >= bootTimer)
					// 	bootTime = false;
					if (definitelyLessThan(currentTemp, m_settingsFactory->getTargetTemp()) 
					//|| (currentTemp > 0 && bootTime)
					) {
						#if ARDUINO_V3
						ledcWrite(m_heatPin, m_settingsFactory->getHeatPWM()));
						#else
						ledcWrite(m_heatChannel, m_settingsFactory->getHeatPWM());
						#endif

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
						if(targetSleeveTempReached && m_settingsFactory->getTargetTemp() - currentTemp >= 5)
							targetSleeveTempReached = false;
					} else if (definitelyLessThanOREssentiallyEqual(currentTemp, (m_settingsFactory->getTargetTemp() + m_settingsFactory->getHeaterThreshold()))) {
						if(!targetSleeveTempReached) {
							targetSleeveTempReached = true;
							String* command = new String("tempReached");
							if(message_callback)
								message_callback(TemperatureType::SLEEVE, "tempReached", _currentSleeveTemp);
						}
						#if ARDUINO_V3
						ledcWrite(m_heatPin, m_settingsFactory->getHoldPWM()));
						#else
						ledcWrite(m_heatChannel, m_settingsFactory->getHoldPWM());
						#endif
						currentState = TemperatureState::HOLD;
					} else {
						#if ARDUINO_V3
						ledcWrite(m_heatPin, 0);
						#else
						ledcWrite(m_heatChannel, 0);
						#endif
						currentState = TemperatureState::OFF;
					}
				}
			}
		} else {
			if(m_sleeveTempEnabled && !sleeveTempInitialized)
				currentState = TemperatureState::RESTART_REQUIRED;
			else
				currentState = TemperatureState::DISABLED_STATE;
		}
		if(!currentState.isEmpty())
			setState(TemperatureType::SLEEVE, currentState.c_str());
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
			if(m_sleeveTempEnabled) {
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

			if(m_internalTempEnabled) {
				if(definitelyGreaterThanOREssentiallyEqual(_currentInternalTemp, m_settingsFactory->getInternalMaxTemp())) {
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
private: 
		const char* _TAG = TagHandler::TemperatureHandler;
    	SettingsFactory* m_settingsFactory;

		bool _isRunning = false;

		// Boot value
		bool m_internalTempEnabled = TEMP_INTERNAL_ENABLED_DEFAULT;
		bool m_sleeveTempEnabled = TEMP_SLEEVE_ENABLED_DEFAULT;
		int8_t m_sleeveTempPin = SLEEVE_TEMP_DISPLAYED_DEFAULT;
		int8_t m_internalTempPin = INTERNAL_TEMP_PIN_DEFAULT; 
		int8_t m_heaterPin = HEATER_PIN_DEFAULT;
		int8_t m_caseFanPin = CASE_FAN_PIN_DEFAULT;
		int m_heaterFrequency = ESP_TIMER_FREQUENCY_DEFAULT;
		int m_heaterResolution = HEATER_RESOLUTION_DEFAULT;
		bool m_fanControlEnabled = FAN_CONTROL_ENABLED_DEFAULT;
		int m_fanFrequency = ESP_TIMER_FREQUENCY_DEFAULT;
		int m_fanResolution = CASE_FAN_RESOLUTION_DEFAULT;
		int m_fanMaxDuty = pow(2, m_fanResolution) - 1;
		float m_internalTempForFanOn = INTERNAL_TEMP_FOR_FAN_DEFAULT;

		int8_t m_caseFanChannel = -1;
		int8_t m_heatChannel = -1;

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
};
