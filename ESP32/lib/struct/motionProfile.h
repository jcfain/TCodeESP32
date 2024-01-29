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
#include "motionChannel.h"

#define maxMotionProfileCount 5
#define maxMotionProfileNameLength 31
#define motionDefaultProfileName "Profile"

struct MotionProfile {
    MotionProfile() { }
    MotionProfile(int profileNumber) {
        snprintf(motionProfileName, sizeof(motionProfileName), "Profile %ld", profileNumber);
    }
    char motionProfileName[maxMotionProfileNameLength] = motionDefaultProfileName;
    bool edited = false;
    std::vector<MotionChannel> channels;

    void addDefaultChannel(const char name[3]) {
        edited = true;
        auto channel = MotionChannel(name);
        channel.edited = true;
        channels.push_back(channel);
    }

    int getMotionChannelIndex(const char name[3]) {
        for (size_t i = 0; i < channels.size(); i++)
        {
            if(strcmp(channels[i].name, name) != 0) {
                return i;
            }
        }
        return -1;
    }

//Doesnt work
    void toJson(JsonObject &obj) {
        obj["name"] = motionProfileName;
        obj["edited"] = edited;
        auto array = obj.createNestedArray("channels");
        for(size_t i = 0; i<channels.size(); i++) {
            JsonObject channelObj;
            channels[i].toJson(channelObj);
            array.add(channelObj);
        }
    }

    void fromJson(const JsonObject& obj) {
        const char* motionProfileNameTemp  = obj["name"] | "Profile";
        strcpy(motionProfileName, motionProfileNameTemp);
        edited = obj["edited"];
        channels.clear();
        JsonArray array = obj["channels"].as<JsonArray>();
        for (size_t i = 0; i < array.size(); i++)
        {
            auto channel = MotionChannel(array[i]["name"].as<const char*>());
            channel.fromJson(array[i].as<JsonObject>());
            channels.push_back(channel);
        }
    }
};