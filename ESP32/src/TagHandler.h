/* MIT License

Copyright (c) 2023 Jason C. Fain

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

class TagHandler {
    public:
    static const char* Main;
    static const char* MainLoop;
    static const char* DisplayHandler;
    static const char* TemperatureHandler;
    static const char* BatteryHandler;
    static const char* SettingsHandler;
    static const char* WifiHandler;
    static const char* UdpHandler;
    static const char* WebsocketsHandler;
    static const char* WebHandler;
    static const char* WebsocketBase;
    static const char* SecureWebsocketsHandler;
    static const char* SecureWebsocketClient;
    static const char* HTTPSHandler;
    static const char* SystemCommandHandler;
    static const char* BLEHandler;
    static const char* BluetoothHandler;
    static const char* ServoHandler3;
    static const char* ServoHandler2;
    static const char* TCodeHandler;
    static const char* BLDCHandler;
    static const char* ToyHandler;
    static const char* MotorHandler;
    static const char* MotionHandler;
    static const char* VoiceHandler;
    static const char* ButtonHandler;

    static const std::vector<const char *> AvailableTags;
};

const char* TagHandler::Main = "main";
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
const char* TagHandler::BluetoothHandler = "bluetooth-handler";
const char* TagHandler::ServoHandler3 = "servo-3-handler";
const char* TagHandler::ServoHandler2 = "servo-2-handler";
const char* TagHandler::TCodeHandler = "tcode-handler";
const char* TagHandler::BLDCHandler = "bldc-handler";
const char* TagHandler::ToyHandler = "toy-handler";
const char* TagHandler::MotorHandler = "motor-handler";
const char* TagHandler::MotionHandler = "motion-handler";
const char* TagHandler::VoiceHandler = "voice-handler";
const char* TagHandler::ButtonHandler = "button-handler";

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
    TagHandler::ServoHandler3,
    TagHandler::TCodeHandler,
    TagHandler::ServoHandler2,
    TagHandler::ToyHandler,
    TagHandler::BLDCHandler,
    TagHandler::DisplayHandler,
    TagHandler::TemperatureHandler,
    TagHandler::BatteryHandler,
    TagHandler::WebHandler,
    TagHandler::BLEHandler,
    TagHandler::BluetoothHandler,
    TagHandler::MotorHandler,
    TagHandler::MotionHandler,
    TagHandler::VoiceHandler,
    TagHandler::ButtonHandler

};