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

#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
//#include <NimBLE2902.h>
#include <queue>
#include <sstream>
#include "logging/LogHandler.h"

#include "BLETCodeControlCallback.h"
#include "constants.h"
#include "TagHandler.h"
#include "BLEHandlerBase.h"

class BLEHandlerTCode :public BLEHandlerBase {
public:
    BLEHandlerTCode() : BLEHandlerBase("TCODE-ESP32", "ff1b451d-3070-4276-9c81-5dc5ea1043bc") { }
private:
    const char* CHARACTERISTIC_UUID = "c5f1543e-338d-47a0-8525-01e3c621359d";
    NimBLECharacteristic* m_characteristic = 0;
    
    void setupCharacteristics(NimBLEService *pService, NimBLEAdvertising *pAdvertising, QueueHandle_t tcodeQueue) override {
        LogHandler::checkHeapIntegrity(TagHandler::BLEHandler, "at begining of setupCharacteristics");
        m_characteristic = new NimBLECharacteristic(CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE_NR);
        LogHandler::checkHeapIntegrity(TagHandler::BLEHandler, "after new NimBLECharacteristic");
        m_characteristic->setCallbacks(getCaracteristicCallbacks(tcodeQueue));
        pService->addCharacteristic(m_characteristic);
    }

    BLECharacteristicCallbacksBase* getCaracteristicCallbacks(QueueHandle_t tcodeQueue) {
        static BLETCodeControlCallback callbacks(tcodeQueue);
        return &callbacks;
    }
    void CommandCallback(const char* in) override {
        //m_characteristic->setValue(in);
        if(m_characteristic)
            m_characteristic->notify((const uint8_t*)in, strlen(in));
    };
};