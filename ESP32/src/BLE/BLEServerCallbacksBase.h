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

#include <queue>

#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>

#include "logging/LogHandler.h"

#include "TagHandler.h"
#include "../lib/constants.h"


class ServerCallbacks: public NimBLEServerCallbacks {  
    #ifdef ESP_ARDUINO3
    void onConnect(BLEServer* pServer, NimBLEConnInfo& connInfo) override  {
        LogHandler::info(TagHandler::BLEHandler, "A client has connected via BLE: %s",
            connInfo.getAddress().toString().c_str()
        );
    #else
    void onConnect(BLEServer* pServer, ble_gap_conn_desc* desc) override  {
        LogHandler::info(TagHandler::BLEHandler, "A client has connected via BLE");
    #endif
    };
    #ifdef ESP_ARDUINO3
    void onDisconnect(BLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        LogHandler::info(TagHandler::BLEHandler, "A client has disconnected from BLE: %s",
            connInfo.getAddress().toString().c_str()
        );
    #else
    void onDisconnect(BLEServer* pServer, ble_gap_conn_desc* desc) override  {
        LogHandler::info(TagHandler::BLEHandler, "A client has disconnected from BLE");
    #endif
    }
};