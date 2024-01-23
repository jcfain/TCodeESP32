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

    void toJson(JsonObject& obj) {
        obj["Name"] = Name;
        obj["FriendlyName"] = FriendlyName;
        obj["min"] = min;
        obj["mid"] = mid;
        obj["max"] = max;
        obj["isSwitch"] = isSwitch;
        obj["sr6Only"] = sr6Only;
    }
    void fromJson(const JsonObject& obj) {
        Name = obj["Name"];
        FriendlyName = obj["FriendlyName"];
        min = obj["min"] | 0;
        mid = obj["mid"] | 5000;
        max = obj["max"] | 9999;
        isSwitch = obj["isSwitch"];
        sr6Only = obj["sr6Only"];
    }
};