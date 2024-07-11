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
#include <algorithm>
#include <ArduinoJson.h>
#include "setting.h"

class SettingMap
{
public:
    const char* ssid = "ssid";
    const char* wifiPass = "wifiPass";
    const char* boardType = "boardType";
    const char* logLevel = "logLevel";
    const char* fullBuild = "fullBuild";
    const char* TCodeVersion = "TCodeVersion";
    const char* udpServerPort = "udpServerPort";
    const char* webServerPort = "webServerPort";
    const char* hostname = "hostname";
    const char* friendlyName = "friendlyName";
    const char* bluetoothEnabled = "bluetoothEnabled";
    const char* pitchFrequencyIsDifferent = "pitchFrequencyIsDifferent";
    const char* msPerRad = "msPerRad";
    const char* servoFrequency = "servoFrequency";
    const char* pitchFrequency = "pitchFrequency";
    const char* valveFrequency = "valveFrequency";
    const char* twistFrequency = "twistFrequency";
    const char* squeezeFrequency = "squeezeFrequency";
    const char* continuousTwist = "continuousTwist";
    const char* feedbackTwist = "feedbackTwist";
    const char* analogTwist = "analogTwist";
    const char* TwistFeedBack_PIN = "TwistFeedBack_PIN";
    const char* RightServo_PIN = "RightServo_PIN";
    const char* LeftServo_PIN = "LeftServo_PIN";
    const char* RightUpperServo_PIN = "RightUpperServo_PIN";
    const char* LeftUpperServo_PIN = "LeftUpperServo_PIN";
    const char* PitchLeftServo_PIN = "PitchLeftServo_PIN";
    const char* PitchRightServo_PIN = "PitchRightServo_PIN";
    const char* ValveServo_PIN = "ValveServo_PIN";
    const char* TwistServo_PIN = "TwistServo_PIN";
    const char* Squeeze_PIN = "Squeeze_PIN";
    const char* Vibe0_PIN = "Vibe0_PIN";
    const char* Vibe1_PIN = "Vibe1_PIN";
    const char* Vibe2_PIN = "Vibe2_PIN";
    const char* Vibe3_PIN = "Vibe3_PIN";
    const char* Case_Fan_PIN = "Case_Fan_PIN";
    const char* LubeButton_PIN = "LubeButton_PIN";
    const char* Internal_Temp_PIN = "Internal_Temp_PIN";
    const char* BLDC_UsePWM = "BLDC_UsePWM";
    const char* BLDC_UseMT6701 = "BLDC_UseMT6701";
    const char* BLDC_UseHallSensor = "BLDC_UseHallSensor";
    const char* BLDC_Pulley_Circumference = "BLDC_Pulley_Circumference";
    const char* BLDC_Encoder_PIN = "BLDC_Encoder_PIN";
    const char* BLDC_ChipSelect_PIN = "BLDC_ChipSelect_PIN";
    const char* BLDC_Enable_PIN = "BLDC_Enable_PIN";
    const char* BLDC_HallEffect_PIN = "BLDC_HallEffect_PIN";
    const char* BLDC_PWMchannel1_PIN = "BLDC_PWMchannel1_PIN";
    const char* BLDC_PWMchannel2_PIN = "BLDC_PWMchannel2_PIN";
    const char* BLDC_PWMchannel3_PIN = "BLDC_PWMchannel3_PIN";
    const char* BLDC_MotorA_Voltage = "BLDC_MotorA_Voltage";
    const char* BLDC_MotorA_Current = "BLDC_MotorA_Current";
    const char* BLDC_MotorA_ParametersKnown = "BLDC_MotorA_ParametersKnown";
    const char* BLDC_MotorA_ZeroElecAngle = "BLDC_MotorA_ZeroElecAngle";
    const char* BLDC_RailLength = "BLDC_RailLength";
    const char* BLDC_StrokeLength = "BLDC_StrokeLength";
    const char* staticIP = "staticIP";
    const char* localIP = "localIP";
    const char* gateway = "gateway";
    const char* subnet = "subnet";
    const char* dns1 = "dns1";
    const char* dns2 = "dns2";
    const char* sr6Mode = "sr6Mode";
    const char* RightServo_ZERO = "RightServo_ZERO";
    const char* LeftServo_ZERO = "LeftServo_ZERO";
    const char* RightUpperServo_ZERO = "RightUpperServo_ZERO";
    const char* LeftUpperServo_ZERO = "LeftUpperServo_ZERO";
    const char* PitchLeftServo_ZERO = "PitchLeftServo_ZERO";
    const char* PitchRightServo_ZERO = "PitchRightServo_ZERO";
    const char* TwistServo_ZERO = "TwistServo_ZERO";
    const char* ValveServo_ZERO = "ValveServo_ZERO";
    const char* Squeeze_ZERO = "Squeeze_ZERO";
    const char* autoValve = "autoValve";
    const char* inverseValve = "inverseValve";
    const char* valveServo90Degrees = "valveServo90Degrees";
    const char* inverseStroke = "inverseStroke";
    const char* inversePitch = "inversePitch";
    const char* lubeAmount = "lubeAmount";
    const char* lubeEnabled = "lubeEnabled";
    const char* displayEnabled = "displayEnabled";
    const char* sleeveTempDisplayed = "sleeveTempDisplayed";
    const char* versionDisplayed = "versionDisplayed";
    const char* internalTempDisplayed = "internalTempDisplayed";
    const char* tempSleeveEnabled = "tempSleeveEnabled";
    const char* Display_Screen_Width = "Display_Screen_Width";
    const char* Display_Screen_Height = "Display_Screen_Height";
    const char* TargetTemp = "TargetTemp";
    const char* HeatPWM = "HeatPWM";
    const char* HoldPWM = "HoldPWM";
    const char* Display_I2C_Address = "Display_I2C_Address";
    const char* Display_Rst_PIN = "Display_Rst_PIN";
    const char* Temp_PIN = "Temp_PIN";
    const char* Heater_PIN = "Heater_PIN";
    const char* heaterThreshold = "heaterThreshold";
    const char* heaterResolution = "heaterResolution";
    const char* heaterFrequency = "heaterFrequency";
    const char* fanControlEnabled = "fanControlEnabled";
    const char* caseFanFrequency = "caseFanFrequency";
    const char* caseFanResolution = "caseFanResolution";
    const char* internalTempForFan = "internalTempForFan";
    const char* internalMaxTemp = "internalMaxTemp";
    const char* tempInternalEnabled = "tempInternalEnabled";
    const char* batteryLevelEnabled = "batteryLevelEnabled";
    const char* batteryLevelNumeric = "batteryLevelNumeric";
    const char* batteryVoltageMax = "batteryVoltageMax";
    const char* batteryCapacityMax = "batteryCapacityMax";
    const char* voiceEnabled = "voiceEnabled";
    const char* voiceMuted = "voiceMuted";
    const char* voiceWakeTime = "voiceWakeTime";
    const char* voiceVolume = "voiceVolume";
    const char* logIncludeTags = "log-include-tags";
    const char* logExcludeTags = "log-exclude-tags";
    const std::vector<Setting> settings = {
        {ssid, "Wifi ssid", "The ssid of the WiFi AP", SettingType::String, RestartRequired::YES, {SettingProfile::Wifi}},
        {wifiPass, "Wifi pass", "The password for the WiFi AP", SettingType::String, RestartRequired::YES, {SettingProfile::Wifi}},
        {boardType, "Board type", "The physical board type", SettingType::Number, RestartRequired::YES, {SettingProfile::System}},
        {logLevel, "Log level", "The loglevel that will output", SettingType::Number, RestartRequired::NO, {SettingProfile::System}},
        //{fullBuild, "Full build", "", SettingType::Boolean, RestartRequired::YES, {SettingProfile::System}},
        {TCodeVersion, "TCode version", "The version of TCode", SettingType::Number, RestartRequired::YES, {SettingProfile::System}},
        {udpServerPort, "Udp port", "The UDP port for TCode input", SettingType::Number, RestartRequired::YES, {SettingProfile::Wifi}},
        {webServerPort, "Web port", "The Web port for the web server", SettingType::Number, RestartRequired::YES, {SettingProfile::Wifi}},
        {hostname, "Hostname", "The hostname for network com", SettingType::String, RestartRequired::YES, {SettingProfile::Wifi}},
        {friendlyName, "Friendly name", "The friendly name displayed when connecting", SettingType::String, RestartRequired::YES, {SettingProfile::Wifi}},
        {bluetoothEnabled, "BLE enabled", "BLE tcode", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Bluetooth}},
        {pitchFrequencyIsDifferent, "Pitch frewuency is different", "", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Servo}},
        {msPerRad, "Ms per rad", "Micro seconds per radian for servos", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo}},
        {servoFrequency, "Servo frequency", "Base servo frequenbcy", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo}},
        {pitchFrequency, "Pitch frequency", "Pitch servo frequency if different than base", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo}},
        {valveFrequency, "Valve frequency", "Valve servo frequency", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo}},
        {twistFrequency, "Twist frequency", "Twist servo frequency if different than base", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo}},
        {squeezeFrequency, "Squeeze frequency", "Squeeze servo frequency if different than base", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo}},
        {continuousTwist, "Continous twist", "Ignores any feedback signal", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Servo}},
        {feedbackTwist, "Feedback twist", "For feed back servos", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Servo}},
        {analogTwist, "Analog twist", "Analog feedback servo", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Servo}},
        {TwistFeedBack_PIN, "Twist feedback PIN", "Feedback servo pin", SettingType::Number, RestartRequired::YES, {SettingProfile::Pin}},
        {RightServo_PIN, "Right servo PIN", "Pin the right servo is on", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {LeftServo_PIN, "Left servo PIN", "Pin the left servo is on", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {RightUpperServo_PIN, "Right upper servo PIN", "Pin the right upper servo is on", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {LeftUpperServo_PIN, "Left upper servo PIN", "Pin the left servo is on", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {PitchLeftServo_PIN, "Pitch left servo PIN", "Pin the pitch left servo is on", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {PitchRightServo_PIN, "Pitch right servo PIN", "Pin the pitch right servo is on", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {ValveServo_PIN, "Valve servo PIN", "Pin the valve servo is on", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {TwistServo_PIN, "Twist servo PIN", "Pin the twist servo is on", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {Squeeze_PIN, "Squeeze servo PIN", "Pin the squeeze servo is on", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {Vibe0_PIN, "Vibe1 PIN", "Pin the vibe 1 is on", SettingType::Number, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
        {Vibe1_PIN, "Vibe2 PIN", "Pin the vibe 2 is on", SettingType::Number, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
        {Vibe2_PIN, "Vibe3 PIN", "Pin the vibe 3 is on", SettingType::Number, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
        {Vibe3_PIN, "Vibe4 PIN", "Pin the vibe 4 is on", SettingType::Number, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
        {Case_Fan_PIN, "Case fan PIN", "Pin the case fan is on", SettingType::Number, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
        {LubeButton_PIN, "Lube button PIN", "Pin the lube button is on", SettingType::Number, RestartRequired::YES, {SettingProfile::PWM, SettingProfile::Pin}},
        {Internal_Temp_PIN, "Internal temp PIN", "Pin the internal temp sensor is on", SettingType::Number, RestartRequired::YES, {SettingProfile::Analog, SettingProfile::Pin}},
        {BLDC_UsePWM, "Use PWM", "Use PWM for BLDC sensor", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Bldc}},
        {BLDC_UseMT6701, "Use MT6701", "Use MT6701 for BLDC sensor", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Bldc}},
        {BLDC_UseHallSensor, "Use hall sensor", "Use Hall sensor for BLDC sensor", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Bldc}},
        {BLDC_Pulley_Circumference, "Pull circumference", "The pulley circumference for BLDC motor", SettingType::Number, RestartRequired::YES, {SettingProfile::Bldc}},
        {BLDC_Encoder_PIN, "Encoder PIN", "Pin the BLDC encoder is on", SettingType::Number, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin}},
        {BLDC_ChipSelect_PIN, "Chipselect PIN", "Pin the BLDC chip select is on", SettingType::Number, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin}},
        {BLDC_Enable_PIN, "Enable PIN", "Pin the BLDC enable is on", SettingType::Number, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin}},
        {BLDC_HallEffect_PIN, "Halleffect PIN", "Pin the hall effect is on", SettingType::Number, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin}},
        {BLDC_PWMchannel1_PIN, "PWM channel1 PIN", "Pin for the BLDC PWM 1", SettingType::Number, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin, SettingProfile::PWM}},
        {BLDC_PWMchannel2_PIN, "PWM channel2 PIN", "Pin for the BLDC PWM 2", SettingType::Number, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin, SettingProfile::PWM}},
        {BLDC_PWMchannel3_PIN, "PWM channel3 PIN", "Pin for the BLDC PWM 3", SettingType::Number, RestartRequired::YES, {SettingProfile::Bldc, SettingProfile::Pin, SettingProfile::PWM}},
        {BLDC_MotorA_Voltage, "Motor A voltage", "BLDC Motor A voltage", SettingType::Float, RestartRequired::YES, {SettingProfile::Bldc}},
        {BLDC_MotorA_Current, "Motor A current", "BLDC Motor A current", SettingType::Float, RestartRequired::YES, {SettingProfile::Bldc}},
        {BLDC_MotorA_ParametersKnown, "Motor A parameters known", "BLDC Motor A params known", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Bldc}},
        {BLDC_MotorA_ZeroElecAngle, "Motor A ZeroElecAngle", "BLDC Motor A ZeroElecAngle", SettingType::Float, RestartRequired::YES, {SettingProfile::Bldc}},
        {BLDC_RailLength, "Rail length", "SSR1 rail length", SettingType::Number, RestartRequired::YES, {SettingProfile::Bldc}},
        {BLDC_StrokeLength, "Stroke length", "SSR1 stroke length", SettingType::Number, RestartRequired::YES, {SettingProfile::Bldc}},
        {staticIP, "Static IP", "Enable static IP for this device", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Wifi}},
        {localIP, "Local IP", "The static IP of this device", SettingType::String, RestartRequired::YES, {SettingProfile::Wifi}},
        {gateway, "Gateway", "The networks gateway", SettingType::String, RestartRequired::YES, {SettingProfile::Wifi}},
        {subnet, "Subnet", "The networks subnet", SettingType::String, RestartRequired::YES, {SettingProfile::Wifi}},
        {dns1, "DNS1", "The networks first DNS", SettingType::String, RestartRequired::YES, {SettingProfile::Wifi}},
        {dns2, "DSN2", "The networks second DNS", SettingType::String, RestartRequired::YES, {SettingProfile::Wifi}},
        {sr6Mode, "SR6 mode", "The current device mode", SettingType::Boolean, RestartRequired::YES, {SettingProfile::System}},
        {RightServo_ZERO, "Right servo zero", "The zero calibration for the right servo", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo}},
        {LeftServo_ZERO, "Left servo zero", "The zero calibration for the left servo", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo}},
        {RightUpperServo_ZERO, "Right upper servo zero", "The zero calibration for the right upper servo", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo}},
        {LeftUpperServo_ZERO, "Left upper servo zero", "The zero calibration for the left upper servo", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo}},
        {PitchLeftServo_ZERO, "Pitch left servo zero", "The zero calibration for the pitch left servo", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo}},
        {PitchRightServo_ZERO, "Pitch right servo zero", "The zero calibration for the pitch right servo", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo}},
        {TwistServo_ZERO, "Twist servo zero", "The zero calibration for the twist servo", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo}},
        {ValveServo_ZERO, "Valve servo zero", "The zero calibration for the valve servo", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo}},
        {Squeeze_ZERO, "Squeeze servo zero", "The zero calibration for the squeeze servo", SettingType::Number, RestartRequired::YES, {SettingProfile::Servo}},
        {autoValve, "Auto valve", "Enable valve auto behavior", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Servo}},
        {inverseValve, "Inverse valve", "Inverse the valve movement", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Servo}},
        {valveServo90Degrees, "Valve 90 degree servo", "The valve is 90 degrees max", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Servo}},
        {inverseStroke, "Inverse stroke", "Inverse stroke", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Servo}},
        {inversePitch, "Inverse pitch", "Inverse pitch", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Servo}},
        {lubeAmount, "Lube amount", "Amount of lube in PWM", SettingType::Number, RestartRequired::YES, {SettingProfile::System}},
        {lubeEnabled, "Lube enabled", "Enable lube", SettingType::Boolean, RestartRequired::YES, {SettingProfile::System}},
        {displayEnabled, "Display enabled", "Enable the OLED display", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Display}},
        {sleeveTempDisplayed, "Sleeve temp displayed", "Display the sleeve temp on the OLED", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Display}},
        {versionDisplayed, "Version displayed", "Display the version on the OLED", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Display}},
        {internalTempDisplayed, "Internal temp displayed", "Display the internal temp on the OLED", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Display}},
        {tempSleeveEnabled, "Temp sleeve enabled", "Enable sleeve temp", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Temperature}},
        {Display_Screen_Width, "Display screen width", "Screen width of the OLED", SettingType::Number, RestartRequired::YES, {SettingProfile::Display}},
        {Display_Screen_Height, "Display screen height", "Screen height of the OLED", SettingType::Number, RestartRequired::YES, {SettingProfile::Display}},
        {TargetTemp, "Target sleeve temp", "Target temp for the sleeve", SettingType::Float, RestartRequired::YES, {SettingProfile::Temperature}},
        {HeatPWM, "Heat PWM", "PWM when the sleeve needs heating", SettingType::Number, RestartRequired::YES, {SettingProfile::Temperature}},
        {HoldPWM, "Hold PWM", "PWM when the sleeve is at target", SettingType::Number, RestartRequired::YES, {SettingProfile::Temperature}},
        {Display_I2C_Address, "Display I2C address", "I2C address of the display", SettingType::String, RestartRequired::YES, {SettingProfile::Display}},
        {Display_Rst_PIN, "Display Rst PIN", "Reset pin for the display", SettingType::Number, RestartRequired::YES, {SettingProfile::Display, SettingProfile::Pin}},
        {Temp_PIN, "Temp pin", "Pin the sleeve temperture is on", SettingType::Number, RestartRequired::YES, {SettingProfile::Pin, SettingProfile::Analog}},
        {Heater_PIN, "Heater PIN", "Pin the heater is on", SettingType::Number, RestartRequired::YES, {SettingProfile::Temperature, SettingProfile::Pin, SettingProfile::PWM}},
        {heaterThreshold, "Heater thresh hold", "", SettingType::Float, RestartRequired::YES, {SettingProfile::Temperature}},// TODo: what is this exactly
        {heaterResolution, "Heater resolution", "Resolution for the Heater PWM", SettingType::Number, RestartRequired::YES, {SettingProfile::Temperature}},
        {heaterFrequency, "Heater frequency", "Frequence for the heater PWM", SettingType::Number, RestartRequired::YES, {SettingProfile::Temperature}},
        {fanControlEnabled, "Fan control enabled", "Enable PWM fan", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Temperature}},
        {caseFanFrequency, "Fan frequency", "Fan frequency", SettingType::Number, RestartRequired::YES, {SettingProfile::Temperature}},
        {caseFanResolution, "Fan resolution", "Fan resolution", SettingType::Number, RestartRequired::YES, {SettingProfile::Temperature}},
        {internalTempForFan, "Internal temp for fan", "Fan temp sensor", SettingType::Double, RestartRequired::YES, {SettingProfile::Temperature}},
        {internalMaxTemp, "Internal max temp", "Max temp for the fan temp", SettingType::Double, RestartRequired::YES, {SettingProfile::Temperature}},
        {tempInternalEnabled, "Temp Internal enabled", "Enable the internal temp sensor", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Temperature}},
        {batteryLevelEnabled, "Battery level enabled", "Enable the battery level", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Battery}},
        {batteryLevelNumeric, "Battery level numeric", "", SettingType::Number, RestartRequired::YES, {SettingProfile::Battery}},// TODO what is this exactly?
        {batteryVoltageMax, "Battery voltage max", "The max voltage for the battery", SettingType::Double, RestartRequired::YES, {SettingProfile::Battery}},
        {batteryCapacityMax, "Battery capcity max", "The battery max capacity", SettingType::Number, RestartRequired::YES, {SettingProfile::Battery}},
        {voiceEnabled, "Voice enabled", "Enable voice control", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Voice}},
        {voiceMuted, "Voice muted", "Voice talk back muted", SettingType::Boolean, RestartRequired::YES, {SettingProfile::Voice}},
        {voiceWakeTime, "Voice wake time", "How long to keep the voice module awake listening for commands", SettingType::Number, RestartRequired::YES, {SettingProfile::Voice}},
        {voiceVolume, "Voice volume", "The volume of the voice talk back", SettingType::Number, RestartRequired::YES, {SettingProfile::Voice}},
        {logIncludeTags, "Log included tags", "Log tags to be included in the output", SettingType::Array, RestartRequired::YES, {SettingProfile::System}},
        {logExcludeTags, "Log excluded tags", "Log tags to be excluded in the output", SettingType::Array, RestartRequired::YES, {SettingProfile::System}}
    };
    const Setting* getSetting(const char* name)
    {
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

    const bool hasProfile(const char* name, const SettingProfile profile, const Setting* setting = 0)
    {
        if(!setting)
            setting = getSetting(name);
        if(!setting)
            return false;
        return std::find_if(setting->profile.begin(), setting->profile.end(), 
                            [profile](const SettingProfile &profileIn) {
                                return profile == profileIn;
        }) != setting->profile.end();
    }
    template<typename T>
    const void getValue(const char* name, T &value)
    {
        if(systemInfoDoc.containsKey(name))
            value = systemInfoDoc[name].to<T>();
        else if (wifiDoc.containsKey(name))
            value = wifiDoc[name].to<T>();
        else if (commonDoc.containsKey(name))
            value = commonDoc[name].to<T>();
        //else
    }
    template<typename T>
    const void setValue(const char* name, const T &value)
    {
        if(systemInfoDoc.containsKey(name))
            systemInfoDoc[name] = value;
        else if (wifiDoc.containsKey(name))
            wifiDoc[name] = value;
        else if (commonDoc.containsKey(name))
            commonDoc[name] = value;
        //else
    }
private:
    const int commonDeserializeSize = 32768;
    const int commonSerializeSize = 24576;
    StaticJsonDocument<3500> systemInfoDoc;
    StaticJsonDocument<100> wifiDoc;
    StaticJsonDocument<32768> commonDoc;
};