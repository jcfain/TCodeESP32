#pragma once
#include <stddef.h>
#include "enum.h"
/// Call backs instance share//////////////////////////////////////////////////////////////
// using TCodeCommandCallback = std::function<void(const char*)>
// using LogCallback =  std::function<void(const char* in, LogLevel level)>
// using TempChangeCallback = std::function<void(const char* message, TemperatureType type, float temp)>
// using TempChangeStateCallback = std::function<void(TemperatureType type, const char *state)>
// using BatteryVoltageCallback = std::function<void(float capacityRemainingPercentage, float capacityRemaining, float voltage, float temperature)>
// using SettingsChangeCallback = std::function<void(const SettingProfile &profile, const char *settingThatChanged)>
// // using TCodeCommandCallbackPassthrough = std::function<void(const char*)>
// // using WifiStatusCallBack = std::function<void(WiFiStatus status, WiFiReason reason)>

using TCodeCommandCallback = void (*)(const char *input);
using LogCallback = void (*)(const char *input, const size_t& length, const LogLevel& level);
using TempChangeCallback = void (*)(const TemperatureType& type, const char* message, const float& temp);
using TempChangeStateCallback = void (*)(const TemperatureType& type, const char *state);
using BatteryVoltageCallback = void (*)(const float& capacityRemainingPercentage, const float& capacityRemaining, const float& voltage, const float& temperature);
using SettingsChangeCallback = void (*)(const SettingProfile &profile, const char *settingThatChanged);
// using TCodeCommandCallbackPassthrough = void (*)(const char*)
// using WifiStatusCallBack = void (*)(WiFiStatus status, WiFiReason reason)

/////////////////////////////////////////////////////////////////////////////////////////