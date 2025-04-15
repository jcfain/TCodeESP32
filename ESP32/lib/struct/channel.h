#pragma once

#include <stdint.h>
#include <ArduinoJson.h>
#include "settingConstants.h"

struct Channel {
    const char* Name;
    const char* FriendlyName;
    bool isSwitch;
    bool sr6Only;
    uint16_t min;
    uint16_t mid;
    uint16_t max;
    uint16_t userMin;
    uint16_t userMid;
    uint16_t userMax;
    bool rangeLimitEnabled = false;

    void toJson(JsonObject& obj) {
        obj[CHANNEL_NAME] = Name;
        obj[CHANNEL_FRIENDLY_NAME] = FriendlyName;
        obj[CHANNEL_MIN] = min;
        obj[CHANNEL_MID] = mid;
        obj[CHANNEL_MAX] = max;
        obj[CHANNEL_USER_MIN] = userMin;
        obj[CHANNEL_USER_MID] = userMid;
        obj[CHANNEL_USER_MAX] = userMax;
        obj[CHANNEL_RANGE_LIMIT_ENABLED] = rangeLimitEnabled;
        obj[CHANNEL_IS_SWITCH] = isSwitch;
        obj[CHANNEL_SR6_ONLY] = sr6Only;
    }
    
    void fromJson(const JsonObject& obj) {
        Name = obj[CHANNEL_NAME];
        FriendlyName = obj[CHANNEL_FRIENDLY_NAME];
        min = obj[CHANNEL_MIN] | 0;
        mid = obj[CHANNEL_MID] | 5000;
        max = obj[CHANNEL_MAX] | 9999;
        userMin = obj[CHANNEL_USER_MIN] | 0;
        userMid = obj[CHANNEL_USER_MID] | 5000;
        userMax = obj[CHANNEL_USER_MAX] | 9999;
        rangeLimitEnabled = obj[CHANNEL_RANGE_LIMIT_ENABLED];
        isSwitch = obj[CHANNEL_IS_SWITCH];
        sr6Only = obj[CHANNEL_SR6_ONLY];
    }
};