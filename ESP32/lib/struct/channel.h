#pragma once

#include <stdint.h>

struct Channel {
    const char* Name;
    const char* FriendlyName;
    uint16_t min;
    uint16_t mid;
    uint16_t max;
    bool isSwitch;
    bool sr6Only;
};