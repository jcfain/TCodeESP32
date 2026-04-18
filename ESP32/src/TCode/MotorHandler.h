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
#include "Global.h"
#include "TCodeBase.h"
#include "MCPWMServo.h"
#include "settings/SettingsHandler.h"
#include "logging/TagHandler.h"

class MotorHandler
{
public:
    virtual void setup() = 0;
    virtual void read(byte inByte) = 0;
    virtual void read(const String &input) = 0;
    virtual void read(const char *input, size_t len) = 0;
    virtual void execute() = 0;
    virtual void setMessageCallback(TCODE_FUNCTION_PTR_T function) = 0;

protected:
    /**
     * Attach a servo-frequency PWM output using the MCPWM peripheral.
     * Duty values use the same 0..(2^SERVO_PWM_RES - 1) range as before.
     */
    void attachServoPin(const char *name, uint8_t pin, uint32_t freq, int8_t channel = -1)
    {
        LogHandler::debug(Tags::Motor, "Connecting %s servo to pin: %d @ freq: %d (MCPWM)", name, pin, freq);
        if (!MCPWMServo::getInstance().attachPin(pin, freq, SERVO_PWM_RES))
        {
            LogHandler::error(Tags::Motor, "Error attaching %s MCPWM servo on pin %d", name, pin);
        }
    }

    /**
     * Write a duty value to an MCPWM servo output.
     */
    void writeServo(uint8_t pin, uint32_t duty)
    {
        MCPWMServo::getInstance().write(pin, duty);
    }

    /**
     * Attach a LEDC PWM output (for vibration motors, lube, heater, fan, etc.).
     */
#ifdef ESP_ARDUINO3
    void attachLedcPin(const char *name, uint8_t pin, uint32_t freq, int8_t channel = -1, uint8_t res = 8)
    {
        bool success = false;
        if (channel > -1)
        {
            LogHandler::debug(Tags::Motor, "Connecting %s to pin: %d @ freq: %d channel: %d resolution: %d (LEDC)", name, pin, freq, channel, res);
            success = ledcAttachChannel(pin, freq, res, channel);
        }
        else
        {
            LogHandler::debug(Tags::Motor, "Connecting %s to pin: %d @ freq: %d resolution: %d (LEDC)", name, pin, freq, res);
            success = ledcAttach(pin, freq, res);
        }
        if (!success)
        {
            LogHandler::error(Tags::Motor, "Error attaching %s LEDC pin", name);
        }
    }
#else
    void attachLedcPin(const char *name, uint8_t pin, uint32_t freq, int8_t channel, int8_t res = 8)
    {
        LogHandler::debug(Tags::Motor, "Connecting %s to pin: %d @ freq: %d on channel: %d resolution: %d (LEDC)", name, pin, freq, channel, res);
        ledcSetup(channel, freq, res);
        ledcAttachPin(pin, channel);
    }
#endif

    /**
     * This method gets the period of the frequency 1/f
     * and converts the units to microseconds * 1000000
     */
    int frequencyToMicroseconds(int freq)
    {
        return 1000000 / freq;
    }
};