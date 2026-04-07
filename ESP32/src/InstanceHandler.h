/* MIT License

Copyright (c) 2026 Jason C. Fain

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
#include "constants.h"
#include "SerialHandler.h"
#include <TCode.h>
#include "LogHandler.h"
#include "SettingsHandler.h"
#include "SystemCommandHandler.h"
#if WIFI_TCODE
#include "WifiHandler.h"
#endif

#if BUILD_TEMP
#include "TemperatureHandler.h"
#endif
#if BUILD_DISPLAY
#include "DisplayHandler.h"
#endif
#if BLUETOOTH_TCODE
#include "BluetoothHandler.h"
#endif
#include "TCode/MotorHandler.h"
// #include "BLEConfigurationHandler.h"

#ifdef MOTOR_TYPE_SERVO
#include "ServoHandler0_3.h"
#include "ServoHandler0_4.h"
#elif defined MOTOR_TYPE_BLDC
#include "BLDCHandler0_3.h"
#include "BLDCHandler0_4.h"
#endif

#if WIFI_TCODE
#include "UdpHandler.h"
// #include "TcpHandler.h"
#include "HTTP/HTTPBase.h"
#include "HTTP/WebSocketBase.h"
#if !SECURE_WEB
#include "WebHandler.h"
//#include "WebHandler_psychic.h"
#else
#include "HTTP\HTTPSHandler.hpp"
#endif
#include "MDNSHandler.hpp"
#endif
// #include "OTAHandler.h"
#if BLE_TCODE
#include "BLEHandler.hpp"
#endif

#if WIFI_TCODE
#if !SECURE_WEB
#include "WebSocketHandler.h"
//#include "WebSocketHandler_psychic.h"
#else
#include "HTTP/SecureWebSocketHandler.hpp"
#endif
#endif

#include "BatteryHandler.h"
#include "MotionHandler.hpp"
#include "VoiceHandler.hpp"
#include "ButtonHandler.hpp"

#include "TaskHandler.hpp"

SerialHandler *serialHandler;
SystemCommandHandler *systemCommandHandler = 0;
MotorHandler *motorHandler = 0;
BatteryHandler *batteryHandler = 0;
MotionHandler *motionHandler = 0;
VoiceHandler *voiceHandler;
ButtonHandler *buttonHandler = 0;
#if WIFI_TCODE
    Udphandler *udpHandler = 0;
    WifiHandler wifi;
    MDNSHandler mdnsHandler;
    HTTPBase *webHandler = 0;
    WebSocketBase *webSocketHandler = 0;
#endif
#if BUILD_TEMP
    TemperatureHandler *temperatureHandler = 0;
#endif
#if BLE_TCODE
    BLEHandler *bleHandler = 0;
#endif
#if BLUETOOTH_TCODE
    BluetoothHandler *bluetoothHandler = 0;
#endif
#if BUILD_DISPLAY
    DisplayHandler *displayHandler = 0;
#endif


void tcodeCommandCallback(const char *in)
{

    if (systemCommandHandler->isCommand(in))
    {
        systemCommandHandler->process(in);
    }
    else
    {
#if BLUETOOTH_TCODE
        if (bluetoothHandler && bluetoothHandler->isConnected())
            bluetoothHandler->CommandCallback(in);
#endif
#if BLE_TCODE
        if (bleHandler && bleHandler->isConnected())
            bleHandler->send(in);
#endif
#if WIFI_TCODE
        if (webSocketHandler)
            webSocketHandler->send(in);
        if (udpHandler)
            udpHandler->send(in);
#endif
        serialHandler->send(in);
    }
}

void tcodePassthroughCommandCallback(const char *in)
{
    if (systemCommandHandler->isCommand(in))
    {
        // This seems wrong but since we are only calling this from one place its fine for now.
        char temp[strlen(in) + 2];
        temp[0] = {0};
        strcpy(temp, in);
        strcat(temp, "\n");
//////////////////////////////////////////////////////////////////////////////////////
#if BLUETOOTH_TCODE
        if (bluetoothHandler && bluetoothHandler->isConnected())
            bluetoothHandler->send(temp);
#endif
#if BLE_TCODE

#endif
#if WIFI_TCODE
        if (webSocketHandler)
            webSocketHandler->send(temp);
        if (udpHandler)
            udpHandler->send(temp);
#endif
        serialHandler->send(temp);
    }
}

void profileChangeCallback(uint8_t profile)
{
}

void logCallBack(const char *input, const size_t& length, const LogLevel& level)
{
#if WIFI_TCODE
    // if(webSocketHandler) {
    // 	webSocketHandler->sendDebug(in, level);
    // }
#endif
}

#if BUILD_TEMP
void tempChangeCallBack(const TemperatureType& type, const char *message, const float& temp)
{
#if WIFI_TCODE
    if (webSocketHandler)
    {
        if (strpbrk(message, "{") == nullptr)
        {
            webSocketHandler->sendCommand(message);
        }
        else
        {
            if (type == TemperatureType::SLEEVE)
            {
                webSocketHandler->sendCommand("sleeveTempStatus", message);
            }
            else
            {
                webSocketHandler->sendCommand("internalTempStatus", message);
            }
        }
    }
#endif
#if BUILD_DISPLAY
    if (displayHandler)
    {
        if (type == TemperatureType::SLEEVE)
        {
            displayHandler->setSleeveTemp(temp);
        }
        else
        {
            displayHandler->setInternalTemp(temp);
        }
    }
#endif
}

void tempStateChangeCallBack(const TemperatureType& type, const char *state)
{
#if BUILD_DISPLAY
    if (displayHandler)
    {
        if (type == TemperatureType::SLEEVE)
        {
            LogHandler::verbose(TagHandler::Main, "tempStateChangeCallBack heat: %s", state);
            displayHandler->setHeateState(state);
            if (temperatureHandler)
                displayHandler->setHeateStateShort(temperatureHandler->getShortSleeveControlStatus(state));
        }
        else
        {
            LogHandler::verbose(TagHandler::Main, "tempStateChangeCallBack fan: %s", state);
            displayHandler->setFanState(state);
        }
    }
#endif
}
#endif

void batteryVoltageCallback(const float& capacityRemainingPercentage, const float& capacityRemaining, const float& voltage, const float& temperature)
{
#if BUILD_DISPLAY
    if (displayHandler)
    {
        displayHandler->setBatteryInformation(capacityRemainingPercentage, voltage, temperature);
    }
#endif
#if WIFI_TCODE
    if (webSocketHandler)
    {
        String statusJson("{\"batteryCapacityRemaining\":\"" + String(capacityRemaining) + "\", \"batteryCapacityRemainingPercentage\":\"" + String(capacityRemainingPercentage) + "\", \"batteryVoltage\":\"" + String(voltage) + "\", \"batteryTemperature\":\"" + String(temperature) + "\"}");
        webSocketHandler->sendCommand("batteryStatus", statusJson.c_str());
    }
#endif
}

void settingChangeCallback(const SettingProfile &profile, const char *settingThatChanged)
{
    LogHandler::verbose(TagHandler::Main, "settingChangeCallback: %s", settingThatChanged);
    SettingsFactory* settingsFactory = SettingsFactory::getInstance();
    if (profile == SettingProfile::System)
    {
        if (!strcmp(settingThatChanged, LOG_LEVEL_SETTING))
        {
            
            #if DEBUG_BUILD != 1
                LogHandler::setLogLevel(settingsFactory->getLogLevel());
            #endif
        }
        else if (!strcmp(settingThatChanged, LOG_INCLUDETAGS))
        {
            LogHandler::setIncludes(settingsFactory->getLogIncludes());
        }
        else if (!strcmp(settingThatChanged, LOG_EXCLUDETAGS))
        {
            LogHandler::setExcludes(settingsFactory->getLogExcludes());
        }
    }
    else if (profile == SettingProfile::MotionProfile)
    {
        if (strcmp(settingThatChanged, MOTION_PROFILE_SELECTED_INDEX) == 0 || strcmp(settingThatChanged, MOTION_PROFILES) == 0) {
            motionHandler->setMotionChannels(SettingsHandler::getMotionChannels());
        //} else if(strcmp(settingThatChanged, "motionChannels") == 0) {
        // 	motionHandler->setMotionChannels(SettingsHandler::getGetMotionChannels()());
        } else if (strcmp(settingThatChanged, MOTION_ENABLED) == 0) {
            LogHandler::verbose(TagHandler::Main, "MOTION_ENABLED: %d", SettingsHandler::getMotionEnabled());
            motionHandler->setEnabled(SettingsHandler::getMotionEnabled());
        }
        // else if(strcmp(settingThatChanged, "motionAmplitudeGlobal") == 0)
        // 	motionHandler->setAmplitude(SettingsHandler::getGetMotionAmplitudeGlobal()());
        // else if(strcmp(settingThatChanged, "motionOffsetGlobal") == 0)
        // 	motionHandler->setOffset(SettingsHandler::getGetMotionOffsetGlobal()());
        // else if(strcmp(settingThatChanged, "motionPeriodGlobal") == 0)
        // 	motionHandler->setPeriod(SettingsHandler::getGetMotionPeriodGlobal()());
        // else if(strcmp(settingThatChanged, "motionUpdateGlobal") == 0)
        // 	motionHandler->setUpdate(SettingsHandler::getGetMotionUpdateGlobal()());
        // else if(strcmp(settingThatChanged, "motionPhaseGlobal") == 0)
        // 	motionHandler->setPhase(SettingsHandler::getGetMotionPhaseGlobal()());
        // else if(strcmp(settingThatChanged, "motionReversedGlobal") == 0)
        // 	motionHandler->setReverse(SettingsHandler::getGetMotionReversedGlobal()());
        // else if(strcmp(settingThatChanged, "motionAmplitudeGlobalRandom") == 0)
        // 	motionHandler->setAmplitudeRandom(SettingsHandler::getGetMotionAmplitudeGlobalRandom()());
        // else if(strcmp(settingThatChanged, "motionAmplitudeGlobalRandomMin") == 0)
        // 	motionHandler->setAmplitudeRandomMin(SettingsHandler::getGetMotionAmplitudeGlobalRandomMin()());
        // else if(strcmp(settingThatChanged, "motionAmplitudeGlobalRandomMax") == 0)
        // 	motionHandler->setAmplitudeRandomMax(SettingsHandler::getGetMotionAmplitudeGlobalRandomMax()());
        // else if(strcmp(settingThatChanged, "motionPeriodGlobalRandom") == 0)
        // 	motionHandler->setPeriodRandom(SettingsHandler::getGetMotionPeriodGlobalRandom()());
        // else if(strcmp(settingThatChanged, "motionPeriodGlobalRandomMin") == 0)
        // 	motionHandler->setPeriodRandomMin(SettingsHandler::getGetMotionPeriodGlobalRandomMin()());
        // else if(strcmp(settingThatChanged, "motionPeriodGlobalRandomMax") == 0)
        // 	motionHandler->setPeriodRandomMax(SettingsHandler::getGetMotionPeriodGlobalRandomMax()());
        // else if(strcmp(settingThatChanged, "motionOffsetGlobalRandom") == 0)
        // 	motionHandler->setOffsetRandom(SettingsHandler::getGetMotionOffsetGlobalRandom()());
        // else if(strcmp(settingThatChanged, "motionOffsetGlobalRandomMin") == 0)
        // 	motionHandler->setOffsetRandomMin(SettingsHandler::getGetMotionOffsetGlobalRandomMin()());
        // else if(strcmp(settingThatChanged, "motionOffsetGlobalRandomMax") == 0)
        // 	motionHandler->setOffsetRandomMax(SettingsHandler::getGetMotionOffsetGlobalRandomMax()());
        // else if(strcmp(settingThatChanged, "motionRandomChangeMin") == 0)
        // 	motionHandler->setMotionRandomChangeMin(SettingsHandler::getGetMotionRandomChangeMin()());
        // else if(strcmp(settingThatChanged, "motionRandomChangeMax") == 0)
        // 	motionHandler->setMotionRandomChangeMax(SettingsHandler::getGetMotionRandomChangeMax()());
    }
    else if (voiceHandler && profile == SettingProfile::Voice)
    {
        if (strcmp(settingThatChanged, "voiceMuted") == 0)
        {
            voiceHandler->setMuteMode(settingsFactory->getVoiceMuted());
        }
        else if (strcmp(settingThatChanged, "voiceVolume") == 0)
        {
            voiceHandler->setVolume(settingsFactory->getVoiceVolume());
        }
        else if (strcmp(settingThatChanged, "voiceWakeTime") == 0)
        {
            voiceHandler->setWakeTime(settingsFactory->getVoiceWakeTime());
        }
    }
    else if (buttonHandler && profile == SettingProfile::Button)
    {
        if (strcmp(settingThatChanged, "bootButtonCommand") == 0)
            buttonHandler->updateBootButtonCommand(settingsFactory->getBootButtonCommand());
        else if (strcmp(settingThatChanged, "analogButtonCommands") == 0)
        {
            buttonHandler->updateAnalogButtonCommands(settingsFactory->getButtonSets());
        }
        else if (strcmp(settingThatChanged, "buttonAnalogDebounce") == 0)
        {
            buttonHandler->updateAnalogDebounce(settingsFactory->getButtonAnalogDebounce());
        }
    }
    else if (profile == SettingProfile::ChannelRanges)
    { 
        if (strcmp(settingThatChanged, CHANNEL_PROFILE) == 0) {
            // TODO add channe; specific updates when moving to its own save...maybe...
            motionHandler->updateChannelRanges();
        } else if (strcmp(settingThatChanged, "channelRangesEnabled") == 0) {
            webSocketHandler->sendCommand("channelRangesEnabled", SettingsHandler::getChannelRangesEnabled() ? "true" : "false");
        }
        
    }
};



/// Functor ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class TCodeCommandCallback
// {
// public:
//     TCodeCommandCallback(
//         SystemCommandHandler* systemCommandHandler, 
//         WebSocketHandler* webSocketHandler, 
//         Udphandler* udpHandler,
//         BLEHandler* bleHandler) 
//     :   systemCommandHandler(systemCommandHandler), 
//         udpHandler(udpHandler),
//         bleHandler(bleHandler),
//         webSocketHandler(webSocketHandler) {}   
//     void operator()(const char* in) const {

//         if (systemCommandHandler->isCommand(in))
//         {
//             systemCommandHandler->process(in);
//         }
//         else
//         {
//             #if BLUETOOTH_TCODE
//                 if (btHandler && btHandler->isConnected())
//                     btHandler->send(in);
//             #endif
//             #if BLE_TCODE
//                 if (bleHandler && bleHandler->isConnected())
//                     bleHandler->send(in);
//             #endif
//             #if WIFI_TCODE
//                 if (webSocketHandler)
//                     webSocketHandler->send(in);
//                 if (udpHandler)
//                     udpHandler->send(in);
//             #endif
//                 if (Serial)
//                     Serial.println(in);
//         }
//     }
// private:
//     SystemCommandHandler* systemCommandHandler;
//     Udphandler* udpHandler;
//     BLEHandler* bleHandler;
//     DisplayHandler* displayHandler;
//     WebSocketHandler* webSocketHandler;
//     TemperatureHandler* temperatureHandler;
//     MotionHandler* motionHandler;
//     ButtonHandler* buttonHandler;
//     VoiceHandler* voiceHandler;
// };

// class LogCallback
// {
// public:
//     LogCallback(WebSocketHandler* webSocketHandler) : webSocketHandler(webSocketHandler) {}   
//     void operator()(const char* in, LogLevel level) const {
//         #if WIFI_TCODE
//             // if(webSocketHandler) {
//             // 	webSocketHandler->sendDebug(in, level);
//             // }
//         #endif
//     }
// private:
//     WebSocketHandler* webSocketHandler;
// };

// class TempChangeCallback
// {
// public:
//     TempChangeCallback(DisplayHandler* displayHandler, WebSocketHandler* webSocketHandler) : 
//         displayHandler(displayHandler),
//         webSocketHandler(webSocketHandler) {}   
//     void operator()(const char* message, TemperatureType type, float temp) const 
//     {
//         #if WIFI_TCODE
//             if (webSocketHandler)
//             {
//                 if (strpbrk(message, "{") == nullptr)
//                 {
//                     webSocketHandler->sendCommand(message);
//                 }
//                 else
//                 {
//                     if (type == TemperatureType::SLEEVE)
//                     {
//                         webSocketHandler->sendCommand("sleeveTempStatus", message);
//                     }
//                     else
//                     {
//                         webSocketHandler->sendCommand("internalTempStatus", message);
//                     }
//                 }
//             }
//         #endif
//         #if BUILD_DISPLAY
//             if (displayHandler)
//             {
//                 if (type == TemperatureType::SLEEVE)
//                 {
//                     displayHandler->setSleeveTemp(temp);
//                 }
//                 else
//                 {
//                     displayHandler->setInternalTemp(temp);
//                 }
//             }
//         #endif
//     }
// private:
//     WebSocketHandler* webSocketHandler;
//     DisplayHandler* displayHandler;
// };

// class TempChangeStateCallback
// {
// public:
//     TempChangeStateCallback(DisplayHandler* displayHandler, TemperatureHandler* temperatureHandler) : 
//         displayHandler(displayHandler),
//         temperatureHandler(temperatureHandler) {}   
//     void operator()(TemperatureType type, const char *state) const 
//     {
//         #if BUILD_DISPLAY
//             if (displayHandler)
//             {
//                 if (type == TemperatureType::SLEEVE)
//                 {
//                     LogHandler::verbose(TagHandler::Main, "tempStateChangeCallBack heat: %s", state);
//                     displayHandler->setHeateState(state);
//                     if (temperatureHandler)
//                         displayHandler->setHeateStateShort(temperatureHandler->getShortSleeveControlStatus(state));
//                 }
//                 else
//                 {
//                     LogHandler::verbose(TagHandler::Main, "tempStateChangeCallBack fan: %s", state);
//                     displayHandler->setFanState(state);
//                 }
//             }
//         #endif
//     }
// private:
//     DisplayHandler* displayHandler;
//     TemperatureHandler* temperatureHandler;
// };

// class BatteryVoltageCallback
// {
// public:
//     BatteryVoltageCallback(DisplayHandler* displayHandler, WebSocketHandler* webSocketHandler) : 
//         displayHandler(displayHandler),
//         webSocketHandler(webSocketHandler) {}   
//     void operator()(float capacityRemainingPercentage, float capacityRemaining, float voltage, float temperature) const 
//     {
//         #if BUILD_DISPLAY
//             if (displayHandler)
//             {
//                 displayHandler->setBatteryInformation(capacityRemainingPercentage, voltage, temperature);
//             }
//         #endif
//         #if WIFI_TCODE
//             if (webSocketHandler)
//             {
//                 String statusJson("{\"batteryCapacityRemaining\":\"" + String(capacityRemaining) + "\", \"batteryCapacityRemainingPercentage\":\"" + String(capacityRemainingPercentage) + "\", \"batteryVoltage\":\"" + String(voltage) + "\", \"batteryTemperature\":\"" + String(temperature) + "\"}");
//                 webSocketHandler->sendCommand("batteryStatus", statusJson.c_str());
//             }
//         #endif
//     }
// private:
//     DisplayHandler* displayHandler;
//     WebSocketHandler* webSocketHandler;
// };

// // #if WIFI_TCODE
// // class WifiStatusCallBack
// // {
// // public:
// //     WifiStatusCallBack(
// //         SettingsFactory* settingsFactory,
// //         DisplayHandler* displayHandler,
// //         MotionHandler* motionHandler,
// //         ButtonHandler* buttonHandler,
// //         VoiceHandler* voiceHandler) :
// //             settingsFactory(settingsFactory),
// //             displayHandler(displayHandler) {}   
// //     void operator()(WiFiStatus status, WiFiReason reason) const 
// //     {
// //         if (status == WiFiStatus::CONNECTED)
// //         {
// //             LogHandler::debug(TagHandler::Main, "wifiStatusCallBack WiFiStatus::CONNECTED");
// //             if (reason == WiFiReason::AP_MODE)
// //             {
// //                 LogHandler::debug(TagHandler::Main, "wifiStatusCallBack WiFiReason::AP_MODE");
// //                 // if(bleConfigurationHandler)
// //                 //   bleConfigurationHandler->stop(); // If a client connects to the ap stop the BLE to save memory.
// //             }
// //         }
// //         else if(status == WiFiStatus::DISCONNECTED)
// //         {
// //             // wifi.dispose();
// //             // startApMode();
// //             LogHandler::debug(TagHandler::Main, "wifiStatusCallBack Not connected");
// //             if (reason == WiFiReason::NO_AP || reason == WiFiReason::UNKNOWN)
// //             {
// //                 LogHandler::debug(TagHandler::Main, "wifiStatusCallBack WiFiReason::NO_AP || WiFiReason::UNKNOWN");
// //                 startConfigMode(
// //                     settingsFactory->getWebServerPort(),
// //                     settingsFactory->getUdpServerPort(),
// //                     settingsFactory->getHostname(),
// //                     settingsFactory->getFriendlyName());
// //             }
// //             else if (reason == WiFiReason::AUTH)
// //             {
// //                 LogHandler::debug(TagHandler::Main, "wifiStatusCallBack WiFiReason::AUTH");
// //                 LogHandler::warning(TagHandler::Main, "Connection auth failed: Resetting wifi password and restarting");
// //                 settingsFactory->defaultValue(WIFI_PASS_SETTING);
// //                 ESP.restart();
// //             }
// //             else if (reason == WiFiReason::AP_MODE)
// //             {
// //                 LogHandler::debug(TagHandler::Main, "wifiStatusCallBack WiFiReason::AP_MODE");
// //                 // #ifdef !ESP32_DA
// //                 // if(bleConfigurationHandler)
// //                 // 	bleConfigurationHandler->setup();
// //                 // #endif
// //             }
// //         } 
// //         else if(status == WiFiStatus::IP) 
// //         {
// //             #if BUILD_DISPLAY
// //                 if(displayHandler) 
// //                 {
// //                     String ipaddress = wifi.ip().toString();
// //                     displayPrint("Connected IP: " + ipaddress);
// //                     displayHandler->setLocalIPAddress(wifi.ip());
// //                 }
// //             #endif
// //         }
// //     }
// // private:

// //     DisplayHandler* displayHandler;
// //     SettingsFactory* settingsFactory;
// // };

// class SettingsChangeCallback
// {
// public:
//     SettingsChangeCallback(
//         SettingsFactory* settingsFactory,
//         WebSocketHandler* webSocketHandler,
//         MotionHandler* motionHandler,
//         ButtonHandler* buttonHandler,
//         VoiceHandler* voiceHandler) :
//             settingsFactory(settingsFactory),
//             webSocketHandler(webSocketHandler),
//             motionHandler(motionHandler),
//             buttonHandler(buttonHandler),
//             voiceHandler(voiceHandler) {}   
//     void operator()(const SettingProfile &profile, const char *settingThatChanged) const 
//     {
//            LogHandler::verbose(TagHandler::Main, "settingChangeCallback: %s", settingThatChanged);
//         if (profile == SettingProfile::System)
//         {
//             if (!strcmp(settingThatChanged, LOG_LEVEL_SETTING))
//             {
//                 LogHandler::setLogLevel(settingsFactory->getLogLevel());
//             }
//             else if (!strcmp(settingThatChanged, LOG_INCLUDETAGS))
//             {
//                 LogHandler::setIncludes(settingsFactory->getLogIncludes());
//             }
//             else if (!strcmp(settingThatChanged, LOG_EXCLUDETAGS))
//             {
//                 LogHandler::setExcludes(settingsFactory->getLogExcludes());
//             }
//         }
//         else if (profile == SettingProfile::MotionProfile)
//         {
//             if (strcmp(settingThatChanged, MOTION_PROFILE_SELECTED_INDEX) == 0 || strcmp(settingThatChanged, MOTION_PROFILES) == 0) {
//                 motionHandler->setMotionChannels(SettingsHandler::getMotionChannels());
//             //} else if(strcmp(settingThatChanged, "motionChannels") == 0) {
//             // 	motionHandler->setMotionChannels(SettingsHandler::getGetMotionChannels()());
//             } else if (strcmp(settingThatChanged, MOTION_ENABLED) == 0) {
//                 LogHandler::verbose(TagHandler::Main, "MOTION_ENABLED: %d", SettingsHandler::getMotionEnabled());
//                 motionHandler->setEnabled(SettingsHandler::getMotionEnabled());
//             }
//             // else if(strcmp(settingThatChanged, "motionAmplitudeGlobal") == 0)
//             // 	motionHandler->setAmplitude(SettingsHandler::getGetMotionAmplitudeGlobal()());
//             // else if(strcmp(settingThatChanged, "motionOffsetGlobal") == 0)
//             // 	motionHandler->setOffset(SettingsHandler::getGetMotionOffsetGlobal()());
//             // else if(strcmp(settingThatChanged, "motionPeriodGlobal") == 0)
//             // 	motionHandler->setPeriod(SettingsHandler::getGetMotionPeriodGlobal()());
//             // else if(strcmp(settingThatChanged, "motionUpdateGlobal") == 0)
//             // 	motionHandler->setUpdate(SettingsHandler::getGetMotionUpdateGlobal()());
//             // else if(strcmp(settingThatChanged, "motionPhaseGlobal") == 0)
//             // 	motionHandler->setPhase(SettingsHandler::getGetMotionPhaseGlobal()());
//             // else if(strcmp(settingThatChanged, "motionReversedGlobal") == 0)
//             // 	motionHandler->setReverse(SettingsHandler::getGetMotionReversedGlobal()());
//             // else if(strcmp(settingThatChanged, "motionAmplitudeGlobalRandom") == 0)
//             // 	motionHandler->setAmplitudeRandom(SettingsHandler::getGetMotionAmplitudeGlobalRandom()());
//             // else if(strcmp(settingThatChanged, "motionAmplitudeGlobalRandomMin") == 0)
//             // 	motionHandler->setAmplitudeRandomMin(SettingsHandler::getGetMotionAmplitudeGlobalRandomMin()());
//             // else if(strcmp(settingThatChanged, "motionAmplitudeGlobalRandomMax") == 0)
//             // 	motionHandler->setAmplitudeRandomMax(SettingsHandler::getGetMotionAmplitudeGlobalRandomMax()());
//             // else if(strcmp(settingThatChanged, "motionPeriodGlobalRandom") == 0)
//             // 	motionHandler->setPeriodRandom(SettingsHandler::getGetMotionPeriodGlobalRandom()());
//             // else if(strcmp(settingThatChanged, "motionPeriodGlobalRandomMin") == 0)
//             // 	motionHandler->setPeriodRandomMin(SettingsHandler::getGetMotionPeriodGlobalRandomMin()());
//             // else if(strcmp(settingThatChanged, "motionPeriodGlobalRandomMax") == 0)
//             // 	motionHandler->setPeriodRandomMax(SettingsHandler::getGetMotionPeriodGlobalRandomMax()());
//             // else if(strcmp(settingThatChanged, "motionOffsetGlobalRandom") == 0)
//             // 	motionHandler->setOffsetRandom(SettingsHandler::getGetMotionOffsetGlobalRandom()());
//             // else if(strcmp(settingThatChanged, "motionOffsetGlobalRandomMin") == 0)
//             // 	motionHandler->setOffsetRandomMin(SettingsHandler::getGetMotionOffsetGlobalRandomMin()());
//             // else if(strcmp(settingThatChanged, "motionOffsetGlobalRandomMax") == 0)
//             // 	motionHandler->setOffsetRandomMax(SettingsHandler::getGetMotionOffsetGlobalRandomMax()());
//             // else if(strcmp(settingThatChanged, "motionRandomChangeMin") == 0)
//             // 	motionHandler->setMotionRandomChangeMin(SettingsHandler::getGetMotionRandomChangeMin()());
//             // else if(strcmp(settingThatChanged, "motionRandomChangeMax") == 0)
//             // 	motionHandler->setMotionRandomChangeMax(SettingsHandler::getGetMotionRandomChangeMax()());
//         }
//         else if (voiceHandler && profile == SettingProfile::Voice)
//         {
//             if (strcmp(settingThatChanged, "voiceMuted") == 0)
//             {
//                 voiceHandler->setMuteMode(settingsFactory->getVoiceMuted());
//             }
//             else if (strcmp(settingThatChanged, "voiceVolume") == 0)
//             {
//                 voiceHandler->setVolume(settingsFactory->getVoiceVolume());
//             }
//             else if (strcmp(settingThatChanged, "voiceWakeTime") == 0)
//             {
//                 voiceHandler->setWakeTime(settingsFactory->getVoiceWakeTime());
//             }
//         }
//         else if (buttonHandler && profile == SettingProfile::Button)
//         {
//             if (strcmp(settingThatChanged, "bootButtonCommand") == 0)
//                 buttonHandler->updateBootButtonCommand(settingsFactory->getBootButtonCommand());
//             else if (strcmp(settingThatChanged, "analogButtonCommands") == 0)
//             {
//                 buttonHandler->updateAnalogButtonCommands(settingsFactory->getButtonSets());
//             }
//             else if (strcmp(settingThatChanged, "buttonAnalogDebounce") == 0)
//             {
//                 buttonHandler->updateAnalogDebounce(settingsFactory->getButtonAnalogDebounce());
//             }
//         }
//         else if (profile == SettingProfile::ChannelRanges)
//         { 
//             if (strcmp(settingThatChanged, CHANNEL_PROFILE) == 0) {
//                 // TODO add channe; specific updates when moving to its own save...maybe...
//                 motionHandler->updateChannelRanges();
//             } else if (strcmp(settingThatChanged, "channelRangesEnabled") == 0) {
//                 webSocketHandler->sendCommand("channelRangesEnabled", SettingsHandler::getChannelRangesEnabled() ? "true" : "false");
//             }
            
//         }
//     }
// private:
//     SettingsFactory* settingsFactory;
//     WebSocketHandler* webSocketHandler;
//     MotionHandler* motionHandler;
//     ButtonHandler* buttonHandler;
//     VoiceHandler* voiceHandler;
// };
// // class TCodeCommandCallbackPassthrough
// // {
// // public:
// //     TCodeCommandCallbackPassthrough(
// //         SystemCommandHandler* systemCommandHandler, 
// //         WebSocketHandler* webSocketHandler, 
// //         Udphandler* udpHandler,
// //         BLEHandler* bleHandler) 
// //     :   systemCommandHandler(systemCommandHandler), 
// //         udpHandler(udpHandler),
// //         bleHandler(bleHandler),
// //         webSocketHandler(webSocketHandler) {}   
// //     void operator()(const char* in) const {

// //         if (systemCommandHandler->isCommand(in))
// //         {
// //             // This seems wrong but since we are only calling this from one place its fine for now.
// //             char temp[strlen(in) + 2];
// //             temp[0] = {0};
// //             strcpy(temp, in);
// //             strcat(temp, "\n");
// //             //////////////////////////////////////////////////////////////////////////////////////
// //             #if BLUETOOTH_TCODE
// //                 if (btHandler && btHandler->isConnected())
// //                     btHandler->send(temp);
// //             #endif
// //             #if BLE_TCODE

// //             #endif
// //             #if WIFI_TCODE
// //                 if (webSocketHandler)
// //                     webSocketHandler->send(temp);
// //                 if (udpHandler)
// //                     udpHandler->send(temp);
// //             #endif
// //                 if (Serial)
// //                     Serial.println(temp);
// //         }
// //         if (systemCommandHandler->isCommand(in))
// //         {
// //             systemCommandHandler->process(in);
// //         }
// //         else
// //         {
// //             #if BLUETOOTH_TCODE
// //                 if (btHandler && btHandler->isConnected())
// //                     btHandler->send(in);
// //             #endif
// //             #if BLE_TCODE
// //                 if (bleHandler && bleHandler->isConnected())
// //                     bleHandler->send(in);
// //             #endif
// //             #if WIFI_TCODE
// //                 if (webSocketHandler)
// //                     webSocketHandler->send(in);
// //                 if (udpHandler)
// //                     udpHandler->send(in);
// //             #endif
// //                 if (Serial)
// //                     Serial.println(in);
// //         }
// //     }
// // private:
// //     SystemCommandHandler* systemCommandHandler;
// //     Udphandler* udpHandler;
// //     BLEHandler* bleHandler;
// //     DisplayHandler* displayHandler;
// //     WebSocketHandler* webSocketHandler;
// //     TemperatureHandler* temperatureHandler;
// //     MotionHandler* motionHandler;
// //     ButtonHandler* buttonHandler;
// //     VoiceHandler* voiceHandler;
// // };

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////