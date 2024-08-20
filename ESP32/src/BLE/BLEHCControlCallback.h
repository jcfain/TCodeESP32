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

#include "BLECharacteristicCallbacksBase.h"
#include "TagHandler.h"
#include "../lib/constants.h"

class BLEHCControlCallback: public BLECharacteristicCallbacksBase  
{
public:
    BLEHCControlCallback(QueueHandle_t tcodeQueue) : m_TCodeQueue(tcodeQueue) {
    }
    // Haptics connect UUID's
    // const char* NAME = "OSR-ESP32";
    // const char* SERVICE_UUID = "00004000-0000-1000-8000-0000101A2B3C";
    // const char* CHARACTERISTIC_UUID = "00002000-0001-1000-8000-0000101A2B3C";
    // const char* CHARACTERISTIC_UUID2_HC = "00002000-0002-1000-8000-0000101A2B3C";
    // At some point this signature will change because its in master so if Bluetooth breaks, check the source class signature.
    #ifdef ESP_ARDUINO3
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo)  override {
    #else
    void onWrite(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc)  override {
    #endif
        // uint16_t handle = pCharacteristic->getHandle();
        // const uint8_t* rxData = rxValue.data();
        // size_t rxLength = rxValue.length();
        
        // LogHandler::verbose(_TAG, "rxData: %ld", rxData);
        // LogHandler::verbose(_TAG, "rxLength: %ld", rxLength);

        // char tcode[MAX_COMMAND] = {0};
        // //Serial.println();
        // for (int i = 0; i < rxLength; i++) {
        //     //Serial.print(rxValue[i],HEX);
        //     if(i < MAX_COMMAND)
        //         tcode[i]=rxData[i];
        //     // if(i<rxLength)
        //     //     Serial.print(":");
        // }
        // size_t len = pCharacteristic->getDataLength();
        // const char* value = pCharacteristic->getValue().c_str();
        // LogHandler::verbose(_TAG, "Recieve tcode: %s, len: %ld", value, len);
        // if(m_motorHandler)
        //     m_motorHandler->read(value, len);
    };
// private:
//     const char* CHARACTERISTIC_UUID = "00002000-0001-1000-8000-0000101A2B3C";
//     const char* CHARACTERISTIC_UUID2 = "00002000-0002-1000-8000-0000101A2B3C";
//     BLECharacteristic* m_tcodeCharacteristic;
//     BLECharacteristic* m_tcodeCharacteristic2;
private: 
    QueueHandle_t m_TCodeQueue;
};