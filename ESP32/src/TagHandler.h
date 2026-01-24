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
#include <map>

using tag_t = uint32_t;
#define MASK(i) (1 << i)

enum Tag : uint32_t {
    MAIN = 0x01,
    DISPLAY = 0x02,
    TEMPERATURE = 0x04,
    BATTERY = 0x08,
    SETTINGS = 0x10,
    WIFI = 0x20,
    UDP = 0x40,
    WEBSOCKETS_SERVER = 0x80,
    WEBSOCKET_BASE = 0x100,
    SECURE_WEBSOCKET_SERVER = 0x200,
    SECURE_WEBSOCKET_CLIENT = 0x400,
    HTTPS = 0x800,
    WEB = 0x1000,
    SYSTEM_COMMAND = 0x2000,
    BLE = 0x4000,
    BLE_CONFIGURATION = 0x8000,
    BLUETOOTH = 0x10000,
    SERVO = 0x20000,
    TCODE = 0x40000,
    MOTOR = 0x80000,
    MOTION = 0x100000,
    VOICE = 0x200000,
    BUTTON = 0x400000,
    MDNS = 0x800000,
    SETTINGS_FACTORY = 0x1000000,
};

std::map

const uint32_t ALL_TAGS = 0xFFFFFFFF;

class TagHandler {
    protected:
        static const std::map<tag_t, const char*> _tag_names;
    public:
    TagHandler() :
    _tags = {
        {MASK(1), "main"},
        {MASK()}
const char* TagHandler::MainLoop = "main-loop";
const char* TagHandler::DisplayHandler = "display-handler";
const char* TagHandler::TemperatureHandler = "temperature-handler";
const char* TagHandler::BatteryHandler = "battery-handler";
const char* TagHandler::SettingsHandler = "settings-handler";
const char* TagHandler::WifiHandler = "wifi-handler";
const char* TagHandler::UdpHandler = "udp-handler";
const char* TagHandler::WebsocketsHandler = "websocket-handler";
const char* TagHandler::WebsocketBase = "websocket-base";
const char* TagHandler::SecureWebsocketsHandler = "secure-websocket-handler";
const char* TagHandler::SecureWebsocketClient = "secure-websocket-client";
const char* TagHandler::HTTPSHandler = "https-handler";
const char* TagHandler::WebHandler = "web-handler";
const char* TagHandler::SystemCommandHandler = "system-command-handler";
const char* TagHandler::BLEHandler = "ble-handler";
const char* TagHandler::BLEConfigurationHandler = "ble-config-handler";
const char* TagHandler::BluetoothHandler = "bluetooth-handler";
const char* TagHandler::ServoHandler = "servo-handler";
const char* TagHandler::TCodeHandler = "tcode-handler";
const char* TagHandler::BLDCHandler = "bldc-handler";
const char* TagHandler::ToyHandler = "toy-handler";
const char* TagHandler::MotorHandler = "motor-handler";
const char* TagHandler::MotionHandler = "motion-handler";
const char* TagHandler::VoiceHandler = "voice-handler";
const char* TagHandler::ButtonHandler = "button-handler";
const char* TagHandler::MdnsHandler = "mdns-handler";
const char* TagHandler::SettingsFactory = "settings-factory";}
    }
    static const std::map<tag_t, const char *> tags;
    static bool HasTag(const char*);
};



const std::vector<const char *> TagHandler::AvailableTags = {
    TagHandler::Main,
    TagHandler::MainLoop,
    TagHandler::SystemCommandHandler,
    TagHandler::SettingsHandler,
    TagHandler::WifiHandler,
	#if !SECURE_WEB
    TagHandler::WebHandler,
    TagHandler::WebsocketsHandler,
    #else
    TagHandler::HTTPSHandler,
    TagHandler::SecureWebsocketsHandler,
    TagHandler::SecureWebsocketClient,
    #endif
    TagHandler::WebsocketBase,
    TagHandler::HTTPSHandler,
    TagHandler::UdpHandler,
    TagHandler::ServoHandler,
    TagHandler::TCodeHandler,
    //TagHandler::ServoHandler2,
    TagHandler::ToyHandler,
    TagHandler::BLDCHandler,
    TagHandler::DisplayHandler,
    TagHandler::TemperatureHandler,
    TagHandler::BatteryHandler,
    TagHandler::WebHandler,
    TagHandler::BLEHandler,
    TagHandler::BLEConfigurationHandler,
    TagHandler::BluetoothHandler,
    TagHandler::MotorHandler,
    TagHandler::MotionHandler,
    TagHandler::VoiceHandler,
    TagHandler::ButtonHandler,
    TagHandler::MdnsHandler,
    TagHandler::SettingsFactory

};

bool TagHandler::HasTag(const char* name) {
    return std::find_if(AvailableTags.begin(), AvailableTags.end(), 
        [name](const char *tagName) {
            return !strcmp(tagName, name);
    }) != AvailableTags.end();
};