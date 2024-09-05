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
#include <vector>
#include <ArduinoJson.h>
//#include <variant>
#include <mpark/variant.hpp>
//#include <variant.hpp>

enum class SettingType
{
    NONE,
    Boolean,
    Number,
    String,
    Float,
    Double,
    ArrayString,
    ArrayInt,
    MAX
};

enum class SettingProfile
{
    System,
    Wireless,
    Wifi,
    Button,
    MotionProfile,
    Temperature,
    Display,
    Servo,
    Pin,
    Timer,
    Bldc,
    Battery,
    Voice,
    PWM,
    Analog,
    Bluetooth,
    Ble,
    ChannelRanges,
    Vib,
    Disabled,
    MAX
};

enum class RestartRequired {
    NO,
    YES
};

struct Setting
{
    const char* name;
    const char* friendlyName;
    const char* description;
    SettingType type;
    mpark::variant<const int, const char*, const float, const double, const bool> defaultValue;
    //JsonVariant value;
    RestartRequired isRestartRequired;
    std::vector<SettingProfile> profiles;
    bool hasProfile(const SettingProfile &profileToSearch) const {
        return std::find_if(profiles.begin(), profiles.end(), [profileToSearch](const SettingProfile &profile) {
            return profile == profileToSearch;
        }) != profiles.end();
    }
};

enum class SettingFile
{
    NONE,
    Common,
    Network,
    Pins,
    MotionProfile,
    ButtonSet,
    ESPTimers
};

class SettingFileInfo {
public:
    const char* path;
    SettingFile file;
    JsonDocument doc;
    const std::vector<Setting> settings;
    const Setting* getSetting(const char* name) {
        std::vector<Setting>::const_iterator it = 
                std::find_if(settings.begin(), settings.end(), 
                    [name](const Setting &setting) {
                        return setting.name == name;
                });
        if(it != settings.end()) {
            return it.base();
        }
        return 0;
    }
};