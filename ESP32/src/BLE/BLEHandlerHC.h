
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

#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
//#include <NimBLE2902.h>
#include <queue>
#include <sstream>
#include "logging/LogHandler.h"

#include "BLEHCControlCallback.h"
#include "../lib/constants.h"
#include "TagHandler.h"
#include "BLEHandlerBase.h"

class BLEHandlerHC :public BLEHandlerBase {
public:
    BLEHandlerHC() : BLEHandlerBase("OSR-ESP32", "00004000-0000-1000-8000-0000101A2B3C") { }
private:
    const char* CHARACTERISTIC_UUID = "00002000-0001-1000-8000-0000101A2B3C";
    const char* CHARACTERISTIC_UUID2 = "00002000-0002-1000-8000-0000101A2B3C";
    BLECharacteristic* m_characteristic;
    BLECharacteristic* m_characteristic2;
    // Haptics connect UUID's
    // const char* NAME = "OSR-ESP32";
    // const char* SERVICE_UUID = "00004000-0000-1000-8000-0000101A2B3C";
    // const char* CHARACTERISTIC_UUID = "00002000-0001-1000-8000-0000101A2B3C";
    // const char* CHARACTERISTIC_UUID2_HC = "00002000-0002-1000-8000-0000101A2B3C";

    void setupCharacteristics(BLEService *pService, BLEAdvertising *pAdvertising, QueueHandle_t tcodeQueue) override {
        m_characteristic = new BLECharacteristic(CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE_NR);
        m_characteristic2 = new BLECharacteristic(CHARACTERISTIC_UUID2, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE_NR);
        LogHandler::debug(TagHandler::BLEHandler, "Setting up BLE TCode Characteristic Callbacks");
        m_characteristic->setCallbacks(getCaracteristicCallbacks(tcodeQueue));
        pService->addCharacteristic(m_characteristic);
        pService->addCharacteristic(m_characteristic2);
    }

    BLECharacteristicCallbacksBase* getCaracteristicCallbacks(QueueHandle_t tcodeQueue) {
        static BLEHCControlCallback callbacks(tcodeQueue);
        return &callbacks;
    }
    void CommandCallback(const char* in) override {
        // m_characteristic->setValue(in);
        // m_characteristic->notify();
    };
};