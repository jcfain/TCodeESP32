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


#include <Arduino.h>
#include <Wire.h>
#include <LTC2944.h>
#include "SettingsHandler.h"
#include "utils.h"
// #include "driver/adc.h"
// #include "esp_adc_cal.h"
#include "LogHandler.h"
#include "TagHandler.h"

using BATTERY_STATE_FUNCTION_PTR_T = void (*)(float capacityRemainingPercentage, float capacityRemaining, float voltage, float temperature);
/** This class is setup for a specific board with an LTC2944 gas guage 
 * The module used is CJMCU-294.
*/
class BatteryHandler {
public:
    static bool connected() {
        return m_battery_connected;
    }

	static void startLoop(void * parameter) {
		((BatteryHandler*)parameter)->loop();
	}

    /** Maximum value is 22000 mAh */
    static void setBatteryCapacity(int maxCapacity) {
        if(connected()) {
            gauge.setBatteryCapacity(maxCapacity);
            m_maxCapacity = maxCapacity;
            LogHandler::info(_TAG, "Battery capacity max set to %u mAh", maxCapacity);
        }
    }

    /** Sets accumulated charge registers to the maximum value */
    static void setBatteryToFull() {
        if(connected()) {        
            SettingsFactory* settingsFactory = SettingsFactory::getInstance();
            setBatteryCapacity(settingsFactory->getBatteryCapacityMax());
            gauge.setBatteryToFull();
            LogHandler::info(_TAG, "Battery capacity set to full");
        }
    }

    bool setup() {
        LogHandler::info(_TAG, "Setup battery monitor");
		if(!SettingsHandler::waitForI2CDevices(LTC2944_ADDRESS)) {
            return false;
        } 
        Wire.begin();
        long timeout = millis() + 10000;
        LogHandler::info(_TAG, "Connecting to monitor");
        while (!gauge.begin()) {
            Serial.print(".");
        	vTaskDelay(1000/portTICK_PERIOD_MS);
            if(millis() > timeout) {
                LogHandler::error(_TAG, "Detecting battery gauge (LTC2944) timed out. Exit.");
                return false;
            }
        }
        LogHandler::info(_TAG, "Monitor connected!");
        m_battery_connected = true;
        gauge.setADCMode(ADC_MODE_SLEEP); // In sleep mode, voltage and temperature measurements will only take place when requested
        SettingsFactory* settingsFactory = SettingsFactory::getInstance();
        setBatteryCapacity(settingsFactory->getBatteryCapacityMax());
        gauge.startMeasurement();
        LogHandler::debug(_TAG, "Complete");
        return true;
    }

	void setMessageCallback(BATTERY_STATE_FUNCTION_PTR_T f) {
		if (f == nullptr) {
			message_callback = 0;
		} else {
			message_callback = f;
		}
	}

    void loop() {
		_isRunning = true;
		LogHandler::debug(_TAG, "Battery task cpu core: %u", xPortGetCoreID());
        TickType_t pxPreviousWakeTime = millis();
		while(_isRunning) {
            if(m_battery_connected && millis() >= lastTick) {
                LogHandler::verbose(_TAG, "Enter getBatteryLevel");
                lastTick = millis() + tick;
                m_batteryCapacity = gauge.getRemainingCapacity();
                m_batteryVoltage = gauge.getVoltage();
                m_batteryTemp = gauge.getTemperature();

		        LogHandler::verbose(_TAG, "Battery remaining capacity: %f", m_batteryCapacity);
		        LogHandler::verbose(_TAG, "Battery voltage: %f", m_batteryVoltage);
		        LogHandler::verbose(_TAG, "Battery temp: %f", m_batteryTemp);
                float capacityRemainingPercentage;
                if(m_batteryCapacity > 0)
                    capacityRemainingPercentage = m_batteryCapacity / m_maxCapacity * 100;

                if(message_callback)
                    message_callback(capacityRemainingPercentage, m_batteryCapacity, m_batteryVoltage, m_batteryTemp);
            }
            xTaskDelayUntil(&pxPreviousWakeTime, 5000/portTICK_PERIOD_MS);
        }
        
  		vTaskDelete( NULL );
    }

    //void setup() {
        // Method to get via ADC I had issues with.
        // if(SettingsHandler::getBatteryLevelEnabled()) {
        // 	LogHandler::info("batteryHandler", "Setting up voltage on pin: %ld", SettingsHandler::getBattery_Voltage_PIN());
        // 	adc1_config_width(ADC_WIDTH_12Bit);
        // 	m_adc1Channel = gpioToADC1(SettingsHandler::getBattery_Voltage_PIN());
        // 	if(m_adc1Channel == adc1_channel_t::ADC1_CHANNEL_MAX) {
        // 		LogHandler::error(_TAG, "Invalid Battery voltage pin: %ld", SettingsHandler::getBattery_Voltage_PIN());
        // 	}
        // 	if(m_adc1Channel != adc1_channel_t::ADC1_CHANNEL_MAX) {
        // 		LogHandler::info("batteryHandler", "ADC channel: %ld", (int)m_adc1Channel);
        // 		adc1_config_channel_atten(m_adc1Channel, ADC_ATTEN_DB_11);

        // 		LogHandler::debug("batteryHandler", "Calibrating battery voltage");
        // 		m_val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 0, &m_adc1_chars);
        // 		//Check type of calibration value used to characterize ADC
        // 		LogHandler::debug("batteryHandler", "Complete: ");
        // 		if (m_val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        // 			LogHandler::debug("batteryHandler", "eFuse Vref");
        // 		} else if (m_val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        // 			LogHandler::debug("batteryHandler", "Two Point");
        // 		} else {
        // 			LogHandler::debug("batteryHandler", "Default");
        // 		}
        // 	}
        // m_battery_connected = true;
        // }
    //}
    // Method to get via ADC I had issues with.
    //void getBatteryLevel() {
        // if(m_adc1Channel == adc1_channel_t::ADC1_CHANNEL_MAX) {
        //     return;
        // }
        // https://esp32tutorials.com/esp32-adc-esp-idf/
        //int adc_value = adc1_get_raw(m_adc1Channel);
        // uint16_t raw = analogRead(SettingsHandler::getBattery_Voltage_PIN()); // analogRead is less accurate than adc1_get_raw
        // m_batteryVoltage = (raw * 3.3 ) / 4095;
        // uint32_t mV;
        // if (m_val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        // 	//LogHandler::debug("batteryHandler", "eFuse Vref");
        // 	mV = esp_adc_cal_raw_to_voltage(adc_value, &m_adc1_chars);
        // } else if (m_val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        // 	//LogHandler::debug("batteryHandler", "Two Point");
        // 	mV = esp_adc_cal_raw_to_voltage(adc_value, &m_adc1_chars);
        // } else {
        // 	//LogHandler::debug("batteryHandler", "Default");
        // 	mV = ((adc_value * 3.3 ) / 4095) * 1000;
        // }
        // if(mV > 0) {
        // 	m_batteryVoltage = mV / 1000.0;
        // } else {
        // 	m_batteryVoltage = 0.0;
        // }

    //https://www.youtube.com/watch?v=qKUrXwkr3cc
    //https://github.com/G6EJD/LiPo_Battery_Capacity_Estimator/blob/master/ReadBatteryCapacity_LIPO.ino
    //author David Bird
        // uint8_t percentage = 2808.3808 * pow(m_batteryVoltage, 4) - 43560.9157 * pow(m_batteryVoltage, 3) + 252848.5888 * pow(m_batteryVoltage, 2) - 650767.4615 * m_batteryVoltage + 626532.5703;

        // Serial.print("mV: ");
        // Serial.println(mV);
        // Serial.print("areadValue: ");
        // Serial.println(areadValue);
        // Serial.print("m_batteryVoltage calc: ");
        // Serial.println((adc_value * 3.3 ) / 4095);
    //}
private:
    static const char* _TAG;
	BATTERY_STATE_FUNCTION_PTR_T message_callback = 0;
    static LTC2944 gauge;
    unsigned long lastTick = 0;
    int tick = 5000;
    bool _isRunning;
	float m_batteryVoltage = 0.0;
	float m_batteryCapacity = 0.0;
	float m_batteryTemp = 0.0;

    static int m_maxCapacity;
    static bool m_battery_connected;
	// esp_adc_cal_characteristics_t m_adc1_chars;
	// esp_adc_cal_value_t m_val_type;
	// adc1_channel_t m_adc1Channel;
};

const char* BatteryHandler::_TAG = TagHandler::BatteryHandler;
LTC2944 BatteryHandler::gauge;
bool BatteryHandler::m_battery_connected = false;
int BatteryHandler::m_maxCapacity;