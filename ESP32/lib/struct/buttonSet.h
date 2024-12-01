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

#include <ArduinoJson.h>
#include "buttonModel.h"
#include "constants.h"

struct ButtonSet {
    char name[26] = "Default";
    int8_t pin = -1;
    gpio_pull_mode_t pullMode = gpio_pull_mode_t::GPIO_PULLDOWN_ONLY;
    ButtonModel buttons[MAX_BUTTONS];
    ButtonSet() { }
    ButtonSet(const ButtonSet &set) {
        strcpy(name, set.name);
        pin = set.pin;
        pullMode = set.pullMode;
        for (size_t i = 0; i < MAX_BUTTONS; i++)
        {
            buttons[i] = ButtonModel(set.buttons[i]);
        }
    }

//Doesnt work
    void toJson(JsonObject& obj) {
        obj["name"] = name;
        obj["pin"] = pin;
        obj["pullMode"] = (uint8_t)pullMode;
        auto array = obj["buttons"].to<JsonArray>();
        for(int i = 0; i<MAX_BUTTONS; i++) {
            JsonObject buttonOBJ;
            buttons[i].toJson(buttonOBJ);
            array.add(buttonOBJ);
        }
        // for(size_t i = 0; i<MAX_BUTTONS; i++) {
        //     buttons[i].toJson(obj["buttons"][i]);
        // }
    }

    void fromJson(const JsonObject& obj) {
        const char* nameTemp  = obj["name"] | "Default";
        strcpy(name, nameTemp);
        pin = obj["pin"] | -1;
        pullMode = (gpio_pull_mode_t)(obj["pullMode"] | (uint8_t)gpio_pull_mode_t::GPIO_PULLDOWN_ONLY);
        JsonArray array = obj["buttons"].as<JsonArray>();
        for(int i = 0; i<MAX_BUTTONS; i++) {
            auto profile = ButtonModel();
            profile.fromJson(array[i].as<JsonObject>());
            buttons[i] = profile;
        }
    }
};