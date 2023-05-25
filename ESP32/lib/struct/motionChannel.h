#pragma once
#include <Arduino.h>

struct MotionChannel {
    MotionChannel(const char nameIn[3]) { 
        strcpy(name, nameIn);
    }
    char name[3];
    float phase = 0;
    bool reverse = 0;
};