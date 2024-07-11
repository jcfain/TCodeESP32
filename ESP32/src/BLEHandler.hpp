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

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <sstream>
#include "LogHandler.h"
#include "TagHandler.h"

class ServerCallbacks;
class BLETCodeControlCallback;

class BLEHandler {
public:
    void setup () {
        LogHandler::info(_TAG, "Setting up BLE Tcode handler");
        m_TCodeQueue = xQueueCreate(25, MAX_COMMAND);

        LogHandler::debug(_TAG, "Setting up BLE init device");
        BLEDevice::init(m_isHC ? BLE_DEVICE_NAME_HC : BLE_DEVICE_NAME);
        LogHandler::debug(_TAG, "Setting up BLE Create server");
        BLEServer *pServer = BLEDevice::createServer();
        pServer->setCallbacks(new ServerCallbacks());

        LogHandler::debug(_TAG, "Setting up BLE Characteristics");
        if(m_isHC) {
            m_tcodeCharacteristic = new BLECharacteristic(BLE_TCODE_CHARACTERISTIC_UUID_HC, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
            m_tcodeCharacteristic2 = new BLECharacteristic(BLE_TCODE_CHARACTERISTIC_UUID2_HC, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
        } else {
            m_tcodeCharacteristic = new BLECharacteristic(BLE_TCODE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE_NR);
        }
        LogHandler::debug(_TAG, "Setting up BLE Callbacks");
        //m_tcodeCharacteristic->setValue("");
        m_tcodeCharacteristic->setCallbacks(new BLETCodeControlCallback());
        if(m_isHC) {
            m_tcodeCharacteristic2->setCallbacks(new BLETCodeControlCallback());
        }

        LogHandler::debug(_TAG, "Setting up BLE Service");
        BLEService *pService = pServer->createService(m_isHC ? BLE_TCODE_SERVICE_UUID_HC : BLE_TCODE_SERVICE_UUID);
        pService->addCharacteristic(m_tcodeCharacteristic);
        
        if(m_isHC) {
            pService->addCharacteristic(m_tcodeCharacteristic2);
        }

        LogHandler::debug(_TAG, "Starting BLE Service");
        pService->start();

        BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(BLE_TCODE_SERVICE_UUID);
        pAdvertising->setScanResponse(true);

        // Functions that help with iPhone connections issue
        // https://github.com/nkolban/esp32-snippets/issues/768
        pAdvertising->setMinPreferred(0x06);  
        pAdvertising->setMaxPreferred(0x12);
        
        LogHandler::debug(_TAG, "Starting BLE Advertising");
        BLEDevice::startAdvertising();
        LogHandler::info(_TAG, "Started BLE Server.");
    }

    void read(char* buf) {
        if(xQueueReceive(m_TCodeQueue, buf, 0)) {
            //LogHandler::verbose(_TAG, "Recieve tcode: %s", buf);
        } else {
            buf[0] = {0};
        }
    }

private: 
    static const char* _TAG;
    const char* BLE_DEVICE_NAME = "TCODE-ESP32";
    const char* BLE_TCODE_SERVICE_UUID = "ff1b451d-3070-4276-9c81-5dc5ea1043bc";
    const char* BLE_TCODE_CHARACTERISTIC_UUID = "c5f1543e-338d-47a0-8525-01e3c621359d";

    bool m_isHC = false;
    // Haptics connect UUID's
    const char* BLE_DEVICE_NAME_HC = "OSR-ESP32";
    const char* BLE_TCODE_SERVICE_UUID_HC = "00004000-0000-1000-8000-0000101A2B3C";
    const char* BLE_TCODE_CHARACTERISTIC_UUID_HC = "00002000-0001-1000-8000-0000101A2B3C";
    const char* BLE_TCODE_CHARACTERISTIC_UUID2_HC = "00002000-0002-1000-8000-0000101A2B3C";

    BLECharacteristic* m_tcodeCharacteristic;
    BLECharacteristic* m_tcodeCharacteristic2;
    static QueueHandle_t m_TCodeQueue;

    // ----------------------------------------
    // Functions to handle Bluetooth LE Setup.
    //-----------------------------------------
    class BLETCodeControlCallback: public BLECharacteristicCallbacks {
        void onWrite(BLECharacteristic *pCharacteristic) {
            // uint16_t handle = pCharacteristic->getHandle();
            uint8_t* rxValue = pCharacteristic->getData();
            size_t rxLength = pCharacteristic->getLength();
            
            // LogHandler::info(_TAG, "handle: %ld", handle);
            // LogHandler::info(_TAG, "rxLength: %ld", rxLength);

            char tcode[MAX_COMMAND] = {0};
            //Serial.println();
            for (int i = 0; i < rxLength; i++) {
                //Serial.print(rxValue[i],HEX);
                if(i < MAX_COMMAND)
                    tcode[i]=rxValue[i];
                // if(i<rxLength)
                //     Serial.print(":");
            }
            xQueueSend(m_TCodeQueue, tcode, 0);
            //Serial.println();
        }
    };

    class ServerCallbacks: public BLEServerCallbacks {
        void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) {
            LogHandler::info(_TAG, "A client has connected via BLE: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
                param->connect.remote_bda[0],
                param->connect.remote_bda[1],
                param->connect.remote_bda[2],
                param->connect.remote_bda[3],
                param->connect.remote_bda[4],
                param->connect.remote_bda[5]
            );
        };
        void onDisconnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) {
            LogHandler::info(_TAG, "A client has disconnected from BLE: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
                param->connect.remote_bda[0],
                param->connect.remote_bda[1],
                param->connect.remote_bda[2],
                param->connect.remote_bda[3],
                param->connect.remote_bda[4],
                param->connect.remote_bda[5]
            );
            pServer->startAdvertising(); 
        }
    };
    
};
QueueHandle_t BLEHandler::m_TCodeQueue;
const char*  BLEHandler::_TAG = TagHandler::BLEHandler;
