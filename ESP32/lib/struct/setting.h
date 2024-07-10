#pragma once
#include <vector>
enum class SettingType
{
    Boolean,
    Number,
    String,
    Float,
    Double,
    Array
};

enum class SettingProfile
{
    System,
    Wifi,
    Button,
    MotionProfile,
    Temperature,
    Display,
    Servo,
    Pin,
    Bldc,
    Battery,
    Voice,
    PWM,
    Analog,
    Bluetooth,
    Ble
};

struct Setting
{
    const char* name;
    const char* friendlyName;
    SettingType type;
    std::vector<SettingProfile> profile;
};
