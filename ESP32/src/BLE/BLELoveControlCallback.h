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

class BLELoveControlCallback: public BLECharacteristicCallbacksBase   
{
public:
    BLELoveControlCallback(QueueHandle_t tcodeQueue): m_TCodeQueue(tcodeQueue) {
        LogHandler::debug(TagHandler::BLEHandler, "Setting up BLE love Characteristic Callbacks");
        
        SettingsFactory::getInstance()->getValue(BLE_LOVE_DEVICE_TYPE, m_bleLoveDeviceType);
    }
    // At some point this signature will change because its in master so if Bluetooth breaks, check the source class signature.
    #ifdef NIMBLE_LATEST
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo)  override {
    #else
    void onWrite(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc)  override {
    #endif
        //assert(pCharacteristic == pRxCharacteristic);
        int len = pCharacteristic->getDataLength();
        if (len) {
            std::string rxValue = pCharacteristic->getValue();
            //LogHandler::verbose(TagHandler::BLEHandler, "*********");
            LogHandler::verbose(TagHandler::BLEHandler, "Received Value: %s", rxValue.c_str());
            if(m_bleLoveDeviceType == BLELoveDeviceType::EDGE) {
                executeEdgeDevice(rxValue);
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

    void setTX(BLECharacteristic* pTxCharacteristic) {
        m_pTxCharacteristic = pTxCharacteristic;
    }

private:
    char tcodeBuffer[MAX_COMMAND];
    QueueHandle_t m_TCodeQueue;
    BLELoveDeviceType m_bleLoveDeviceType;
    BLECharacteristic* m_pTxCharacteristic = 0;

    void buildTCode(const char* channel, const int &loveValue, const u_int16_t &interval, char* buf) {
        sprintf(buf, "%s%04ldI%u\n", channel, map(loveValue, 0,20,0,9999), interval);
    }
    void buildTCode(const char* channel, const int &loveValue, char* buf) {
        sprintf(buf, "%s%04ld\n", channel, map(loveValue, 0,20,0,9999));
    }
    void buildTCodeSpeed(const char* channel, const int &loveValue, const u_int16_t &speed, char* buf) {
        sprintf(buf, "%s%04ldS%u\n", channel, map(loveValue, 0,20,0,9999), speed);
    }

void executeEdgeDevice(std::string rxValue) {
        // Serial.println();
        // Serial.println("*********");
        if(m_pTxCharacteristic)
        {
            static uint8_t messageBuf[64];
            if (rxValue == "DeviceType;") {
                Serial.println("$Responding to Device Enquiry");
                //memmove(messageBuf, "P:37:FFFFFFFFFFFF;", 18); // Edge
                memmove(messageBuf, "H:11:0082059AD3BD;", 18); //H solace EA gravity 9AD3BD
                //memmove(messageBuf, "C:11:0082059AD3BD;", 18); // C Nora
                // memmove(messageBuf, "W:11:0082059AD3BD;", 18); //W: -domi
                //memmove(messageBuf, "H:11:FFFFFFFFFFFF;", 18); //H solace EA gravity 9AD3BD
                // CONFIGURATION:               ^ Use a BLE address of the Lovense device you're cloning.
                m_pTxCharacteristic->setValue(messageBuf, 18);
                m_pTxCharacteristic->notify();
            } else if (rxValue == "Battery;") {
                memmove(messageBuf, "69;", 3);
                m_pTxCharacteristic->setValue(messageBuf, 3);
                m_pTxCharacteristic->notify();
            } else if (rxValue == "PowerOff;") {
                memmove(messageBuf, "OK;", 3);
                m_pTxCharacteristic->setValue(messageBuf, 3);
                m_pTxCharacteristic->notify();
            } else if (rxValue == "RotateChange;") {
                memmove(messageBuf, "OK;", 3);
                m_pTxCharacteristic->setValue(messageBuf, 3);
                m_pTxCharacteristic->notify();
            } else if (rxValue.rfind("Status:", 0) == 0) {
                memmove(messageBuf, "2;", 2);
                m_pTxCharacteristic->setValue(messageBuf, 3);
                m_pTxCharacteristic->notify();
            } else if (rxValue.rfind("Vibrate:", 0) == 0) {
                int vibration = std::atoi(rxValue.substr(8).c_str());
                memmove(messageBuf, "OK;", 3);
                m_pTxCharacteristic->setValue(messageBuf, 3);
                m_pTxCharacteristic->notify();
                buildTCode("V0", vibration, tcodeBuffer);
            } else if (rxValue.rfind("Vibrate1:", 0) == 0) {
                int vibration = std::atoi(rxValue.substr(9).c_str());
                memmove(messageBuf, "OK;", 3);
                m_pTxCharacteristic->setValue(messageBuf, 3);
                m_pTxCharacteristic->notify();
                buildTCode("V0", vibration, tcodeBuffer);
            } else if (rxValue.rfind("Vibrate2:", 0) == 0) {
                int vibration = std::atoi(rxValue.substr(9).c_str());
                memmove(messageBuf, "OK;", 3);
                m_pTxCharacteristic->setValue(messageBuf, 3);
                m_pTxCharacteristic->notify();
                buildTCode("V1", vibration, tcodeBuffer);
            } else {
                LogHandler::warning(TagHandler::BLEHandler, "$Unknown request");        
                memmove(messageBuf, "ERR;", 4);
                m_pTxCharacteristic->setValue(messageBuf, 4);
                m_pTxCharacteristic->notify();
            }
        }
        LogHandler::verbose(TagHandler::BLEHandler, "Recieve love tcode: %s", tcodeBuffer);
        if(xQueueSend(m_TCodeQueue, tcodeBuffer, 0) != pdTRUE) {
            LogHandler::error(TagHandler::BLEHandler, "Failed to write to queue");
        }
    }
};