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
#include <ArduinoJson.h>
#include "constants.h"

struct ButtonModel {
    char name[26];
    uint8_t index;
    char command[MAX_COMMAND];

    ButtonModel() { }
    ButtonModel(const ButtonModel &model) {
        strcpy(name, model.name);
        index = model.index;
        strcpy(command, model.command);
    }
    
    void loadDefault(uint8_t argIndex) {
        index = argIndex;
        if(argIndex == 0) {
            sprintf(name, "Toggle motion");
            sprintf(command, "#motion-toggle");
        } else if(argIndex == 1) {
            sprintf(name, "Cycle motion profile");
            sprintf(command, "#motion-profile-cycle");
        } else if(argIndex == 2) {
            sprintf(name, "Set motion profile 3");
            sprintf(command, "#motion-profile-set:%u #motion-enable", 3);
        } else {
            sprintf(name, "Set motion profile 4");
            sprintf(command, "#motion-profile-set:%u #motion-enable", 4);
        }
    }

    void toJson(JsonObject& obj) {
        obj["name"] = name;
        obj["index"] = index;
        obj["command"] = command;
    }
    void fromJson(const JsonObject obj) {
        const char* nameTemp  = obj["name"] | "Default";
        strcpy(name, nameTemp);
        index = obj["index"];
        const char* commandTemp  = obj["command"] | "\0";
        strcpy(command, commandTemp);
    }

    void press() {
        pressed_ = true;
    }
    void release() {
        pressed_ = false;
    }
    bool isPressed() {
        return pressed_;
    }

private: 
    bool pressed_ = false;
} buttonModelRef;