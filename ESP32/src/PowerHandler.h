/* MIT License

Copyright (c) 2025 Austen Bartels

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
#include <MCP45HVX1.h>
#include "SettingsHandler.h"
#include "utils.h"
// #include "driver/adc.h"
// #include "esp_adc_cal.h"
#include "LogHandler.h"
#include "TagHandler.h"

#define MCP45HVX1_ADDRESS (0x3C)

// R2 = (R1 * 1.221) / (VOUT - 1.221)
// R2 (VOUT - 1.221) = R1 * 1.221
// VOUT - 1.221 = (R1 * 1.221) / R2
// VOUT = 1.221 + ((R1 * 1.221) / R2)
constexpr float R1 =  47000.0f;
constexpr float R2_MAX = 15000.0f;
constexpr float R2_MIN = 5000.0f;
constexpr float VOUT(const float r2) {
    return 1.221f + ((R1 * 1.221f) / r2);
};
constexpr float VMIN = VOUT(R2_MAX);
constexpr float VMAX = VOUT(R2_MIN);

float derive_r2(float voltage)
{
    return (R1 * 1.221f) / (voltage - 1.221f);
}

float derive_voltage(float r2)
{
    return 1.221f + ((R1 * 1.221f) / r2);
}


using POWER_STATE_FUNCTION_PTR_T = void (*)(float servo_voltage, float input_voltage);
/** This class is setup for a specific board with an LTC2944 gas guage
 * The module used is CJMCU-294.
*/
class PowerHandler {
public:

    enum PDLevels {
        PD5V,
        PD9V,
        PD12V,
        PD15V,
        PD20V,
        MAX
    };

	static void startLoop(void * parameter) {
		((PowerHandler*)parameter)->loop();
	}

    // PD CFG pins
    void setPDLevel(PDLevels level)
    {
        LogHandler::debug(_TAG, "Setting PDLevel to %d", level);
        switch(level) {
            case PD5V:
                writePDMatrix(1,0,0);
            break;
            case PD9V:
                writePDMatrix(0,0,0);
            break;
            case PD12V:
                writePDMatrix(0,0,1);
            case PD15V:
                writePDMatrix(0,1,1);
            break;
            case PD20V:
                writePDMatrix(0,1,0);
            break;
            default:
                LogHandler::error(_TAG, "Invalid power delivery level");
            break;
        }
    }

    // VServo level
    void setServoVoltage(float volts)
    {
        LogHandler::debug(_TAG, "Setting servo voltage %f", volts);
        if (volts < VMIN || volts > VMAX)
        {
            // We can't practically get this low
            LogHandler::error(_TAG, "Invalid voltage level");
            return;
        }
        // Resistance range = 5k +/- 10k
        // R1 = 47k
        float r2 = derive_r2(volts) - 5000.0f;
        if (r2 < 0.0f || r2 > 10000.0f)
        {
            LogHandler::error(_TAG, "Invalid resistor value requested (%f)", r2);
            return;
        }
        LogHandler::verbose(_TAG, "Writing digipot %f ohm (%f v)", r2, volts);
        writeDigipot(r2);
    }

    void enableServoVoltage(bool en)
    {
        LogHandler::debug(_TAG, "Setting servo enabled %s", en ? "true" : "false");
        if (m_pinmap)
        {
            int8_t pin = m_pinmap->servoVoltageEnPin();
            if (pin > 0)
            {
                if (en) {
                    digitalWrite(pin, HIGH);
                } else {
                    digitalWrite(pin, LOW);
                }
            }
        }
    }

    float readServoVoltage()
    {
        float voltage = -1.0f;
        if (m_pinmap)
        {
            int8_t pin = m_pinmap->servoVoltagePin();
            if (pin > 0)
            {
                float raw_value = (float)analogRead(pin);
                voltage = translateADCVoltage(raw_value);
            }
        }
        return voltage;
    }

    float readInputVoltage()
    {
        float voltage = -1.0f;
        if (m_pinmap)
        {
            int8_t pin = m_pinmap->inputVoltagePin();
            if (pin > 0)
            {
                float raw_value = (float)analogRead(pin);
                voltage = translateADCVoltage(raw_value);
            }
        }
        return voltage;
    }

    void initPowerPins()
    {
        if(m_pinmap)
        {
            int8_t servoEn = m_pinmap->servoVoltageEnPin();
            int8_t servoRead = m_pinmap->servoVoltagePin();
            int8_t inputRead = m_pinmap->inputVoltagePin();
            int8_t pdCFG1 = m_pinmap->PDCFGPin(0);
            int8_t pdCFG2 = m_pinmap->PDCFGPin(1);
            int8_t pdCFG3 = m_pinmap->PDCFGPin(2);
            if (servoEn > 0)
            {
                pinMode(servoEn, OUTPUT);
            }
            if (servoRead > 0)
            {
                pinMode(servoRead, INPUT);
            }
            if (inputRead > 0)
            {
                pinMode(inputRead, INPUT);
            }
            if (pdCFG1 > 0)
            {
                pinMode(pdCFG1, OUTPUT);
            }
            if (pdCFG2 > 0)
            {
                pinMode(pdCFG2, OUTPUT);
            }
            if (pdCFG3 > 0)
            {
                pinMode(pdCFG3, OUTPUT);
            }


        }
    }

    bool setup() {
        LogHandler::info(_TAG, "Setup servo regulator");
		if(!SettingsHandler::waitForI2CDevices(MCP45HVX1_ADDRESS)) {
            return false;
        }
        Wire.begin();
        long timeout = millis() + 10000;
        m_digipot.begin();
        m_digipot.startup();
        SettingsFactory* settingsFactory = SettingsFactory::getInstance();
        m_pinmap = settingsFactory->getPins();
        initPowerPins();
        _isSetup = true;
        setPDLevel((PowerHandler::PDLevels)settingsFactory->getPDLevel());
        enableServoVoltage(settingsFactory->getServoVoltageEn());
        setServoVoltage(settingsFactory->getServoVoltage());
        LogHandler::debug(_TAG, "Complete");
        return true;
    }

	void setMessageCallback(POWER_STATE_FUNCTION_PTR_T f) {
		if (f == nullptr) {
			message_callback = 0;
		} else {
			message_callback = f;
		}
	}

    void loop() {
		_isRunning = true;
		LogHandler::debug(_TAG, "Power task cpu core: %u", xPortGetCoreID());
        TickType_t pxPreviousWakeTime = millis();
		while(_isRunning) {
            if(m_pinmap && (millis() >= lastTick)) {
                lastTick = millis() + tick;
                float servo_v =  readServoVoltage();
                float input_v = readInputVoltage();

                if(message_callback)
                    message_callback(servo_v, input_v);
            }
            xTaskDelayUntil(&pxPreviousWakeTime, 5000/portTICK_PERIOD_MS);
        }

		vTaskDelete( NULL );
    }

private:
    const float DIVIDER_COEFF = 7.7f;
    static const char* _TAG;
	POWER_STATE_FUNCTION_PTR_T message_callback = 0;
    MCP45HVX1 m_digipot;
    unsigned long lastTick = 0;
    int tick = 5000;
    bool _isRunning = false;
    bool _isSetup = false;
    PinMap* m_pinmap = NULL;

    void writePDMatrix(int8_t cfg1, int8_t cfg2, int8_t cfg3)
    {
        if (m_pinmap && _isSetup)
        {
            int8_t pin1 = m_pinmap->PDCFGPin(0);
            int8_t pin2 = m_pinmap->PDCFGPin(1);
            int8_t pin3 = m_pinmap->PDCFGPin(2);
            if (pin1 > 0)
            {
                digitalWrite(pin1, cfg1);
            }
            if (pin2 > 0)
            {
                digitalWrite(pin2, cfg2);
            }
            if (pin3 > 0)
            {
                digitalWrite(pin3, cfg3);
            }
            LogHandler::debug(_TAG, "Wrote matrix: %d %d %d", cfg1, cfg2, cfg3);
        }
    }

    void writeDigipot(float resistance) {
        if (_isSetup)
        {
            int wiper = ((int)round(resistance) * 127) / 10000;
            LogHandler::debug(_TAG, "Setting resistance %f (%d)", resistance, wiper);
            m_digipot.writeWiper(wiper);
        }
    }

    int readDigipot() {
        int res = -1;
        if (_isSetup)
        {
            res = m_digipot.readWiper();
        }
        return res;
    }
    
    float translateADCVoltage(float input, bool curve_fitting=true)
    {
        const float ADC_MAXIMUM = 4095.0f; // ESP32
        const float DIVIDER_COEFF = 1150.0f / 150.0f; // Hardware dependent
        const float VOLTAGE_REFERENCE = 3.3f;
        float voltage = (input * DIVIDER_COEFF * VOLTAGE_REFERENCE) / ADC_MAXIMUM;
        if (curve_fitting)
        {
            // Values determined from emperical curve fitting, may vary with hardware/temperature/current
            voltage = (0.95 * voltage) + 1.22;
        }
        return voltage;
    }
};

const char* PowerHandler::_TAG = TagHandler::PowerHandler;
