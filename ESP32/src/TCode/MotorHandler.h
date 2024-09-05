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
#include "../SettingsHandler.h"
#include "../TagHandler.h"

class MotorHandler {
public:
    virtual void setup() = 0;
    virtual void read(byte inByte) = 0;
    virtual void read(const String &input) = 0;
    virtual void read(const char* input, size_t len) = 0;
    virtual void execute() = 0;
    virtual void setMessageCallback(TCODE_FUNCTION_PTR_T function) = 0;
protected:
    #ifdef ESP_ARDUINO3
    // void attachPin(const char* name, uint8_t pin, uint32_t freq, int8_t res = -1) {
    void attachPin(const char* name, uint8_t pin, uint32_t freq, int8_t channel = -1, uint8_t res = 0) {
        uint8_t resolution = res > 0 ? res : SERVO_PWM_RES;
        bool success = false;
        if(channel > -1) 
        {
            LogHandler::debug(TagHandler::MotorHandler, "Connecting %s servo to pin: %d @ freq: %d channel: %d resolution: %d", name, pin, freq, channel, resolution);
            success = ledcAttachChannel(pin, freq, resolution, channel);
        } 
        else
        {
            LogHandler::debug(TagHandler::MotorHandler, "Connecting %s servo to pin: %d @ freq: %d resolution: %d", name, pin, freq, resolution);
            success = ledcAttach(pin, freq, resolution);
        }
        if(!success) {
            LogHandler::error(TagHandler::MotorHandler, "Error attaching %s pin", name);
        }
    }
    #else
    void attachPin(const char* name, uint8_t pin, uint32_t freq, int8_t channel, int8_t res = -1) {
        uint8_t resolution = res > -1 ? res : SERVO_PWM_RES;
        LogHandler::debug(TagHandler::MotorHandler, "Connecting %s servo to pin: %d @ freq: %d on channel: %d", name, pin, freq, channel);
        ledcSetup(channel,freq,resolution);
        ledcAttachPin(pin,channel);
    }
    #endif
    

    int calcInt(int freq) {
        return 1000000/freq;
    }
};