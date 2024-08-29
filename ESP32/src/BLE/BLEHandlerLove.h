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
#include <queue>
#include <sstream>
#include "logging/LogHandler.h"

#include "BLELoveControlCallback.h"
#include "../lib/constants.h"
#include "TagHandler.h"
#include "BLEHandlerBase.h"

class BLEHandlerLove :public BLEHandlerBase {
public:
    BLEHandlerLove() : BLEHandlerBase("LOVE-H", "50300001-0023-4bd4-bbd5-a6920e4c5653") { }
private:
    const char* TXCHARACTERISTIC_UUID = "50300003-0023-4bd4-bbd5-a6920e4c5653";
    const char* RXCHARACTERISTIC_UUID = "50300002-0023-4bd4-bbd5-a6920e4c5653";
    BLECharacteristic* m_pTxCharacteristic;
    BLECharacteristic* m_pRxCharacteristic;

    void setupCharacteristics(BLEService *pService, BLEAdvertising *pAdvertising, QueueHandle_t tcodeQueue) override {
        BLEDevice::setSecurityAuth(true, true, true);
        m_pTxCharacteristic = pService->createCharacteristic(
                            TXCHARACTERISTIC_UUID,
                            NIMBLE_PROPERTY::NOTIFY
                            );
        m_pRxCharacteristic = pService->createCharacteristic(
                            RXCHARACTERISTIC_UUID,
                            NIMBLE_PROPERTY::WRITE  |
                            NIMBLE_PROPERTY::WRITE_NR
                            );
        auto callbacks = getCaracteristicCallbacks(tcodeQueue);
        static_cast<BLELoveControlCallback*>(callbacks)->setTX(m_pTxCharacteristic);
        m_pRxCharacteristic->setCallbacks(callbacks);
        pAdvertising->setScanResponse(false);
        pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
        //pService->addCharacteristic(m_pRxCharacteristic);
    }

    BLECharacteristicCallbacksBase* getCaracteristicCallbacks(QueueHandle_t tcodeQueue) {
        static BLELoveControlCallback callbacks(tcodeQueue);
        return &callbacks;
    }
    void CommandCallback(const char* in) override {
        // m_pTxCharacteristic->setValue(in);
        // m_pTxCharacteristic->notify();
    };
};