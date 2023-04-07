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


#include <Arduino.h>
#include "SettingsHandler.h"
#include "utils.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "TagHandler.h"

class BatteryHandler {
public:
    static bool connected() {
        return m_battery_connected;
    }
    void setup() {
        // if(SettingsHandler::batteryLevelEnabled) {
        // 	LogHandler::info("batteryHandler", "Setting up voltage on pin: %ld", SettingsHandler::Battery_Voltage_PIN);
        // 	adc1_config_width(ADC_WIDTH_12Bit);
        // 	m_adc1Channel = gpioToADC1(SettingsHandler::Battery_Voltage_PIN);
        // 	if(m_adc1Channel == adc1_channel_t::ADC1_CHANNEL_MAX) {
        // 		LogHandler::error(_TAG, "Invalid Battery voltage pin: %ld", SettingsHandler::Battery_Voltage_PIN);
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
    }

    void getBatteryLevel() {
    	LogHandler::verbose(_TAG, "Enter getBatteryLevel");
        if(m_adc1Channel == adc1_channel_t::ADC1_CHANNEL_MAX) {
            return;
        }
        // https://esp32tutorials.com/esp32-adc-esp-idf/
        //int adc_value = adc1_get_raw(m_adc1Channel);
        uint16_t raw = analogRead(SettingsHandler::Battery_Voltage_PIN); // analogRead is less accurate than adc1_get_raw
        m_batteryVoltage = (raw * 3.3 ) / 4095;
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
        Serial.print("m_batteryVoltage: ");
        Serial.println(m_batteryVoltage);
        // Serial.print("m_batteryVoltage calc: ");
        // Serial.println((adc_value * 3.3 ) / 4095);
    }

private:
    const char* _TAG = TagHandler::BatteryHandler;
    static bool m_battery_connected;
	double m_batteryVoltage = 0.0;
	esp_adc_cal_characteristics_t m_adc1_chars;
	esp_adc_cal_value_t m_val_type;
	adc1_channel_t m_adc1Channel;
};

bool BatteryHandler::m_battery_connected = false;