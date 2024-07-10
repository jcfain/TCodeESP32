#pragma once
#include <algorithm>
#include "setting.h"

class SettingMap
{
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
        {ssid, "Wifi ssid", SettingType::String, {SettingProfile::Wifi}},
        {wifiPass, "Wifi pass",SettingType::String, {SettingProfile::Wifi}},
        {boardType, "Board type", SettingType::Number, {SettingProfile::System}},
        {logLevel, "Log level", SettingType::Number, {SettingProfile::System}},
        {fullBuild, "Full build", SettingType::Boolean, {SettingProfile::System}},
        {TCodeVersion, "TCode version", SettingType::Number, {SettingProfile::System}},
        {udpServerPort, "Udp port", SettingType::Number, {SettingProfile::Wifi}},
        {webServerPort, "Web port", SettingType::Number, {SettingProfile::Wifi}},
        {hostname, "Hostname", SettingType::String, {SettingProfile::Wifi}},
        {friendlyName, "Friendly name", SettingType::String, {SettingProfile::Wifi}},
        {bluetoothEnabled, "Bluetooth enabled", SettingType::Boolean, {SettingProfile::Bluetooth}},
        {pitchFrequencyIsDifferent, "Pitch frewuency is different", SettingType::Boolean, {SettingProfile::Servo}},
        {msPerRad, "Ms per rad", SettingType::Number, {SettingProfile::Servo}},
        {servoFrequency, "Servo frequency", SettingType::Number, {SettingProfile::Servo}},
        {pitchFrequency, "Pitch frequency", SettingType::Number, {SettingProfile::Servo}},
        {valveFrequency, "Valve frequency", SettingType::Number, {SettingProfile::Servo}},
        {twistFrequency, "Twist frequency", SettingType::Number, {SettingProfile::Servo}},
        {squeezeFrequency, "Squeeze frequency", SettingType::Number, {SettingProfile::Servo}},
        {continuousTwist, "Continous twist", SettingType::Boolean, {SettingProfile::Servo}},
        {feedbackTwist, "Feedback twist", SettingType::Boolean, {SettingProfile::Servo}},
        {analogTwist, "Analog twist", SettingType::Boolean, {SettingProfile::Servo}},
        {TwistFeedBack_PIN, "Twist feedback PIN", SettingType::Number, {SettingProfile::Pin}},
        {RightServo_PIN, "Right servo PIN", SettingType::Number, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {LeftServo_PIN, "LEft servo PIN", SettingType::Number, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {RightUpperServo_PIN, "Right upper servo PIN", SettingType::Number, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {LeftUpperServo_PIN, "Left upper servo PIN", SettingType::Number, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {PitchLeftServo_PIN, "Pitch left servo PIN", SettingType::Number, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {PitchRightServo_PIN, "Pitch right servo PIN", SettingType::Number, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {ValveServo_PIN, "Valve servo PIN", SettingType::Number, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {TwistServo_PIN, "Twist servo PIN", SettingType::Number, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {Squeeze_PIN, "Squeeze servo PIN", SettingType::Number, {SettingProfile::Servo, SettingProfile::PWM, SettingProfile::Pin}},
        {Vibe0_PIN, "Vibe1 PIN", SettingType::Number, {SettingProfile::PWM, SettingProfile::Pin}},
        {Vibe1_PIN, "Vibe2 PIN", SettingType::Number, {SettingProfile::PWM, SettingProfile::Pin}},
        {Vibe2_PIN, "Vibe3 PIN", SettingType::Number, {SettingProfile::PWM, SettingProfile::Pin}},
        {Vibe3_PIN, "Vibe4 PIN", SettingType::Number, {SettingProfile::PWM, SettingProfile::Pin}},
        {Case_Fan_PIN, "Case fan PIN", SettingType::Number, {SettingProfile::PWM, SettingProfile::Pin}},
        {LubeButton_PIN, "Lube button PIN", SettingType::Number, {SettingProfile::PWM, SettingProfile::Pin}},
        {Internal_Temp_PIN, "Internal temp PIN", SettingType::Number, {SettingProfile::Analog, SettingProfile::Pin}},
        {BLDC_UsePWM, "Use PWM", SettingType::Boolean, {SettingProfile::Bldc}},
        {BLDC_UseMT6701, "Use MT6701", SettingType::Boolean, {SettingProfile::Bldc}},
        {BLDC_UseHallSensor, "Wifi pass", SettingType::Boolean, {SettingProfile::Bldc}},
        {BLDC_Pulley_Circumference, "Use hall sensor", SettingType::Number, {SettingProfile::Bldc}},
        {BLDC_Encoder_PIN, "Encoder PIN", SettingType::Number, {SettingProfile::Bldc, SettingProfile::Pin}},
        {BLDC_ChipSelect_PIN, "Chipselect PIN", SettingType::Number, {SettingProfile::Bldc, SettingProfile::Pin}},
        {BLDC_Enable_PIN, "Enable PIN", SettingType::Number, {SettingProfile::Bldc, SettingProfile::Pin}},
        {BLDC_HallEffect_PIN, "Halleffect PIN", SettingType::Number, {SettingProfile::Bldc, SettingProfile::Pin}},
        {BLDC_PWMchannel1_PIN, "PWM channel1 PIN", SettingType::Number, {SettingProfile::Bldc, SettingProfile::Pin}},
        {BLDC_PWMchannel2_PIN, "PWM channel2 PIN", SettingType::Number, {SettingProfile::Bldc, SettingProfile::Pin}},
        {BLDC_PWMchannel3_PIN, "PWM channel3 PIN", SettingType::Number, {SettingProfile::Bldc, SettingProfile::Pin}},
        {BLDC_MotorA_Voltage, "Motor A voltage", SettingType::Float, {SettingProfile::Bldc}},
        {BLDC_MotorA_Current, "Motor A current", SettingType::Float, {SettingProfile::Bldc}},
        {BLDC_MotorA_ParametersKnown, "Motor A parameters known", SettingType::Boolean, {SettingProfile::Bldc}},
        {BLDC_MotorA_ZeroElecAngle, "Motor A ZeroElecAngle", SettingType::Float, {SettingProfile::Bldc}},
        {BLDC_RailLength, "Rail length", SettingType::Number, {SettingProfile::Bldc}},
        {BLDC_StrokeLength, "Stroke length", SettingType::Number, {SettingProfile::Bldc}},
        {staticIP, "Static IP", SettingType::Boolean, {SettingProfile::Wifi}},
        {localIP, "Local IP", SettingType::String, {SettingProfile::Wifi}},
        {gateway, "Gateway", SettingType::String, {SettingProfile::Wifi}},
        {subnet, "Subnet", SettingType::String, {SettingProfile::Wifi}},
        {dns1, "DNS1", SettingType::String, {SettingProfile::Wifi}},
        {dns2, "DSN2", SettingType::String, {SettingProfile::Wifi}},
        {sr6Mode, "SR6 m,ode", SettingType::Boolean, {SettingProfile::System}},
        {RightServo_ZERO, "Right servo zero", SettingType::Number, {SettingProfile::Servo}},
        {LeftServo_ZERO, "LEft servo zero", SettingType::Number, {SettingProfile::Servo}},
        {RightUpperServo_ZERO, "Right upper servo zero", SettingType::Number, {SettingProfile::Servo}},
        {LeftUpperServo_ZERO, "Left upper servo zero", SettingType::Number, {SettingProfile::Servo}},
        {PitchLeftServo_ZERO, "Pitch left servo zero", SettingType::Number, {SettingProfile::Servo}},
        {PitchRightServo_ZERO, "Pitch right servo zero", SettingType::Number, {SettingProfile::Servo}},
        {TwistServo_ZERO, "Twist servo zero", SettingType::Number, {SettingProfile::Servo}},
        {ValveServo_ZERO, "Valve servo zero", SettingType::Number, {SettingProfile::Servo}},
        {Squeeze_ZERO, "Squeeze servo zero", SettingType::Number, {SettingProfile::Servo}},
        {autoValve, "Auto valve", SettingType::Boolean, {SettingProfile::Servo}},
        {inverseValve, "Inverse valve", SettingType::Boolean, {SettingProfile::Servo}},
        {valveServo90Degrees, "Valve 90 degree servo", SettingType::Boolean, {SettingProfile::Servo}},
        {inverseStroke, "Inverse stroke", SettingType::Boolean, {SettingProfile::Servo}},
        {inversePitch, "Inverse pitch", SettingType::Boolean, {SettingProfile::Servo}},
        {lubeAmount, "Lube amount", SettingType::Number, {SettingProfile::System}},
        {lubeEnabled, "Lube enabled", SettingType::Boolean, {SettingProfile::System}},
        {displayEnabled, "Display enabled", SettingType::Boolean, {SettingProfile::Display}},
        {sleeveTempDisplayed, "Sleeve temp displayed", SettingType::Boolean, {SettingProfile::Display}},
        {versionDisplayed, "Version displayed", SettingType::Boolean, {SettingProfile::Display}},
        {internalTempDisplayed, "Internal temp displayed", SettingType::Boolean, {SettingProfile::Display}},
        {tempSleeveEnabled, "Temp sleeve enabled", SettingType::Boolean, {SettingProfile::Temperature}},
        {Display_Screen_Width, "Display screen width", SettingType::Number, {SettingProfile::Display}},
        {Display_Screen_Height, "Display screen height", SettingType::Number, {SettingProfile::Display}},
        {TargetTemp, "Target temp", SettingType::Float, {SettingProfile::Temperature}},
        {HeatPWM, "Heat PWM", SettingType::Number, {SettingProfile::Temperature}},
        {HoldPWM, "Hold PWM", SettingType::Number, {SettingProfile::Temperature}},
        {Display_I2C_Address, "Display I2C address", SettingType::String, {SettingProfile::Display}},
        {Display_Rst_PIN, "Display Rst PIN", SettingType::Number, {SettingProfile::Display, SettingProfile::Pin}},
        {Temp_PIN, "Temp pin", SettingType::Number, {SettingProfile::Pin}},
        {Heater_PIN, "Heater PIN", SettingType::Number, {SettingProfile::Temperature, SettingProfile::Pin}},
        {heaterThreshold, "Heater thresh hold", SettingType::Float, {SettingProfile::Temperature}},
        {heaterResolution, "Heater resolution", SettingType::Number, {SettingProfile::Temperature}},
        {heaterFrequency, "Heater frequency", SettingType::Number, {SettingProfile::Temperature}},
        {fanControlEnabled, "Fan control enabled", SettingType::Boolean, {SettingProfile::Temperature}},
        {caseFanFrequency, "Case fan frequency", SettingType::Number, {SettingProfile::Temperature}},
        {caseFanResolution, "case fan resolution", SettingType::Number, {SettingProfile::Temperature}},
        {internalTempForFan, "Internal temp for fan", SettingType::Number, {SettingProfile::Temperature}},
        {internalMaxTemp, "Internal max temp", SettingType::Number, {SettingProfile::Temperature}},
        {tempInternalEnabled, "Temp Internal enabled", SettingType::Boolean, {SettingProfile::Temperature}},
        {batteryLevelEnabled, "Battery level enabled", SettingType::Boolean, {SettingProfile::Battery}},
        {batteryLevelNumeric, "Battery level numeric", SettingType::Number, {SettingProfile::Battery}},
        {batteryVoltageMax, "Battery voltage max", SettingType::Float, {SettingProfile::Battery}},
        {batteryCapacityMax, "Battery capcity max", SettingType::Number, {SettingProfile::Battery}},
        {voiceEnabled, "Voice enabled", SettingType::Boolean, {SettingProfile::Voice}},
        {voiceMuted, "Voice muted", SettingType::Boolean, {SettingProfile::Voice}},
        {voiceWakeTime, "Voice wake time", SettingType::Number, {SettingProfile::Voice}},
        {voiceVolume, "Voice volume", SettingType::Number, {SettingProfile::Voice}},
        {logIncludeTags, "Log included tags", SettingType::Array, {SettingProfile::System}},
        {logExcludeTags, "Log excluded tags", SettingType::Array, {SettingProfile::System}},
    };

    const Setting* findSetting(const char* name)
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
};