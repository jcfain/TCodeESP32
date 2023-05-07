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
#include <ArduinoJson.h>

#define maxMotionProfileCount 5
#define maxMotionProfileNameLength 31
#define motionDefaultProfileName "Profile"
#define motionUpdateGlobalDefault 100
#define motionPeriodGlobalDefault 2000
#define motionAmplitudeGlobalDefault 60
#define motionOffsetGlobalDefault 5000
#define motionPeriodGlobalRandomDefault false
#define motionPeriodGlobalRandomMinDefault 500
#define motionPeriodGlobalRandomMaxDefault 2000
#define motionAmplitudeGlobalRandomDefault false
#define motionAmplitudeGlobalRandomMinDefault 20
#define motionAmplitudeGlobalRandomMaxDefault 60
#define motionOffsetGlobalRandomDefault false
#define motionOffsetGlobalRandomMinDefault 3000
#define motionOffsetGlobalRandomMaxDefault 7000
#define motionPhaseGlobalDefault 0.0f
#define motionReversedGlobalDefault false
#define motionRandomChangeMinDefault 3000
#define motionRandomChangeMaxDefault 30000

struct MotionProfile {
    MotionProfile() { }
    MotionProfile(int profileNumber) {
        snprintf(motionProfileName, sizeof(motionProfileName), "Profile %ld", profileNumber);
    }
    // const int motionUpdateGlobalDefault = 100;
    // const int motionPeriodGlobalDefault = 2000;
    // const int motionAmplitudeGlobalDefault = 60;
    // const int motionOffsetGlobalDefault = 5000;
    // const bool motionPeriodGlobalRandomDefault = false;
    // const int motionPeriodGlobalRandomMinDefault = 500;
    // const int motionPeriodGlobalRandomMaxDefault = 2000;
    // const bool motionAmplitudeGlobalRandomDefault = false;
    // const int motionAmplitudeGlobalRandomMinDefault = 20;
    // const int motionAmplitudeGlobalRandomMaxDefault = 60;
    // const bool motionOffsetGlobalRandomDefault = false;
    // const int motionOffsetGlobalRandomMinDefault = 3000;
    // const int motionOffsetGlobalRandomMaxDefault = 7000;
    // const float motionPhaseGlobalDefault = 0;
    // const bool motionReversedGlobalDefault = false;
    // const int motionRandomChangeMinDefault = 3000;
    // const int motionRandomChangeMaxDefault = 30000;

    char motionProfileName[maxMotionProfileNameLength] = motionDefaultProfileName;
    int motionUpdateGlobal = motionUpdateGlobalDefault;
    int motionPeriodGlobal = motionPeriodGlobalDefault;
    int motionAmplitudeGlobal = motionAmplitudeGlobalDefault;
    int motionOffsetGlobal = motionOffsetGlobalDefault;
    bool motionPeriodGlobalRandom = motionPeriodGlobalRandomDefault;
    int motionPeriodGlobalRandomMin = motionPeriodGlobalRandomMinDefault;
    int motionPeriodGlobalRandomMax = motionPeriodGlobalRandomMaxDefault;
    bool motionAmplitudeGlobalRandom = motionAmplitudeGlobalRandomDefault;
    int motionAmplitudeGlobalRandomMin = motionAmplitudeGlobalRandomMinDefault;
    int motionAmplitudeGlobalRandomMax = motionAmplitudeGlobalRandomMaxDefault;
    bool motionOffsetGlobalRandom = motionOffsetGlobalRandomDefault;
    int motionOffsetGlobalRandomMin = motionOffsetGlobalRandomMinDefault;
    int motionOffsetGlobalRandomMax = motionOffsetGlobalRandomMaxDefault;
    float motionPhaseGlobal = motionPhaseGlobalDefault;
    bool motionReversedGlobal = motionReversedGlobalDefault;
    int motionRandomChangeMin = motionRandomChangeMinDefault;
    int motionRandomChangeMax = motionRandomChangeMaxDefault;

    void toJson(JsonObject &obj) {
        obj["motionProfileName"] = motionProfileName;
        obj["motionUpdateGlobal"] = motionUpdateGlobal;
        obj["motionPeriodGlobal"] = motionPeriodGlobal;
        obj["motionAmplitudeGlobal"] = motionAmplitudeGlobal;
        obj["motionOffsetGlobal"] = motionOffsetGlobal;
        obj["motionPhaseGlobal"] = motionPhaseGlobal;
        obj["motionReversedGlobal"] = motionReversedGlobal;
        obj["motionPeriodGlobalRandom"] = motionPeriodGlobalRandom;
        obj["motionPeriodGlobalRandomMin"] = motionPeriodGlobalRandomMin;
        obj["motionPeriodGlobalRandomMax"] = motionPeriodGlobalRandomMax;
        obj["motionAmplitudeGlobalRandom"] = motionAmplitudeGlobalRandom;
        obj["motionAmplitudeGlobalRandomMin"] = motionAmplitudeGlobalRandomMin;
        obj["motionAmplitudeGlobalRandomMax"] = motionAmplitudeGlobalRandomMax;
        obj["motionOffsetGlobalRandom"] = motionOffsetGlobalRandom;
        obj["motionOffsetGlobalRandomMin"] = motionOffsetGlobalRandomMin;
        obj["motionOffsetGlobalRandomMax"] = motionOffsetGlobalRandomMax;
        obj["motionRandomChangeMin"] = motionRandomChangeMin;
        obj["motionRandomChangeMax"] = motionRandomChangeMax;
    }

    void fromJson(JsonObject obj) {
        const char* motionProfileNameTemp  = obj["motionProfileName"] | "Profile";
        strcpy(motionProfileName, motionProfileNameTemp);
        // Serial.print("IN motionUpdateGlobal: ");
        // Serial.println(motionUpdateGlobal);
        motionUpdateGlobal = obj["motionUpdateGlobal"] | motionUpdateGlobalDefault;
        // Serial.print("IN obj[motionUpdateGlobal]): ");
        // Serial.println((int)obj["motionUpdateGlobal"]);
        motionPeriodGlobal = obj["motionPeriodGlobal"] | motionPeriodGlobalDefault;
        motionAmplitudeGlobal = obj["motionAmplitudeGlobal"] | motionAmplitudeGlobalDefault;
        motionOffsetGlobal = obj["motionOffsetGlobal"] | motionOffsetGlobalDefault;
        motionPhaseGlobal = obj["motionPhaseGlobal"] | motionPhaseGlobalDefault;
        motionReversedGlobal = obj["motionReversedGlobal"] | motionReversedGlobalDefault;
        motionPeriodGlobalRandom = obj["motionPeriodGlobalRandom"] | motionPeriodGlobalRandomDefault;
        motionPeriodGlobalRandomMin = obj["motionPeriodGlobalRandomMin"] | motionPeriodGlobalRandomMinDefault;
        motionPeriodGlobalRandomMax = obj["motionPeriodGlobalRandomMax"] | motionPeriodGlobalRandomMaxDefault;
        motionAmplitudeGlobalRandom = obj["motionAmplitudeGlobalRandom"] | motionAmplitudeGlobalRandomDefault;
        motionAmplitudeGlobalRandomMin = obj["motionAmplitudeGlobalRandomMin"] | motionAmplitudeGlobalRandomMinDefault;
        motionAmplitudeGlobalRandomMax = obj["motionAmplitudeGlobalRandomMax"] | motionAmplitudeGlobalRandomMaxDefault;
        motionOffsetGlobalRandom = obj["motionOffsetGlobalRandom"] | motionOffsetGlobalRandomDefault;
        motionOffsetGlobalRandomMin = obj["motionOffsetGlobalRandomMin"] | motionOffsetGlobalRandomMinDefault;
        motionOffsetGlobalRandomMax = obj["motionOffsetGlobalRandomMax"] | motionOffsetGlobalRandomMaxDefault;
        motionRandomChangeMin = obj["motionRandomChangeMin"] | motionRandomChangeMinDefault;
        motionRandomChangeMax = obj["motionRandomChangeMax"] | motionRandomChangeMaxDefault;
    }
};