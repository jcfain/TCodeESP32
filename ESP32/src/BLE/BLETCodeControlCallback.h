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
#include "constants.h"

class BLETCodeControlCallback: public BLECharacteristicCallbacksBase 
{
public:
    BLETCodeControlCallback(QueueHandle_t tcodeQueue): m_TCodeQueue(tcodeQueue){ }
    // const char* NAME = "TCODE-ESP32";
    // const char* SERVICE_UUID = "ff1b451d-3070-4276-9c81-5dc5ea1043bc";
    // At some point this signature will change because its in master so if Bluetooth breaks, check the source class signature.
    #ifdef NIMBLE_LATEST
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo)  override {
    #else
    void onWrite(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc)  override {
    #endif

        size_t len = pCharacteristic->getLength();
        if(len) {
            NimBLEAttValue rxValue = pCharacteristic->getValue();
            LogHandler::verbose(TagHandler::BLEHandler, "Recieve tcode: %s", rxValue.c_str());
            if(xQueueSend(m_TCodeQueue, rxValue.c_str(), 0) != pdTRUE) {
                LogHandler::error(TagHandler::BLEHandler, "Failed to write to queue");
            }
        }
    };
    
    #ifdef NIMBLE_LATEST
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    #else
    void onRead(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc) override {
    #endif
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onRead(), value: ");
        Serial.println(pCharacteristic->getValue().c_str());
    };
    void onNotify(NimBLECharacteristic* pCharacteristic) {
        Serial.println("Sending notification to clients");
    };
private: 
    QueueHandle_t m_TCodeQueue;
};