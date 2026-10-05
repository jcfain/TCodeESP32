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
#include "esp_coexist.h"
#include "constants.h"
// #include "LogHandler.h"
#include "TagHandler.h"
#include "logging/LogHandler.h"
#include "BLEServerCallbacksBase.h"

class BLEHandlerBase {
public:
    const char* NAME = "TCODE-ESP32";
    const char* SERVICE_UUID = "ff1b451d-3070-4276-9c81-5dc5ea1043bc";
    BLEHandlerBase(const char* name, const char* serviceUUID): 
        NAME(name), 
        SERVICE_UUID(serviceUUID) { }
        
    void setup(QueueHandle_t tcodeQueue) 
    {
        LogHandler::checkHeapIntegrity(TagHandler::BLEHandler, "failed at begining of setup");
        LogHandler::info(TagHandler::BLEHandler, "Setting up BLE handler: %s", NAME);
        NimBLEDevice::init(std::string(NAME));
        esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); 
        esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
        esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN ,ESP_PWR_LVL_P9);
        LogHandler::debug(TagHandler::BLEHandler, "Setting up BLE Create server");
        LogHandler::checkHeapIntegrity(TagHandler::BLEHandler, "after NimBLEDevice::init()");
        NimBLEServer *pServer = NimBLEDevice::createServer();
        LogHandler::checkHeapIntegrity(TagHandler::BLEHandler, "after NimBLEDevice::createServer()");
        pServer->setCallbacks(new ServerCallbacks());
        LogHandler::checkHeapIntegrity(TagHandler::BLEHandler, "after pServer->setCallbacks");
        pServer->advertiseOnDisconnect(true);

        NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(SERVICE_UUID);
        pAdvertising->enableScanResponse(true);
        // Functions that help with iPhone connections issue
        // https://github.com/nkolban/esp32-snippets/issues/768
        pAdvertising->setPreferredParams(0x06, 0x12);

        LogHandler::debug(TagHandler::BLEHandler, "Setting up BLE service");
        NimBLEService *pService = pServer->createService(SERVICE_UUID);
        LogHandler::checkHeapIntegrity(TagHandler::BLEHandler, "failed after pServer->createService");

        setupCharacteristics(pService, pAdvertising, tcodeQueue);
        LogHandler::checkHeapIntegrity(TagHandler::BLEHandler, "after setupCharacteristics");
        
        // LogHandler::debug(TagHandler::BLEHandler, "Starting BLE service");
        // if(pService->start()) {
        //     LogHandler::info(TagHandler::BLEHandler, "Started BLE service");
        // } else {
        //     LogHandler::error(TagHandler::BLEHandler, "Failed to start BLE service.");
        // }
        
        LogHandler::debug(TagHandler::BLEHandler, "Starting BLE advertising");
        if(pAdvertising->start())
            LogHandler::info(TagHandler::BLEHandler, "Started BLE server.");
        else
            LogHandler::error(TagHandler::BLEHandler, "Failed to start BLE advertising.");

    }

    bool isConnected() 
    {
        return getServerCallbacks()->isConnected();
    }

    virtual void CommandCallback(const char* in) = 0;

private:
    bool m_connected = false;
    ServerCallbacks* getServerCallbacks() 
    {
        static ServerCallbacks callbacks;
        return &callbacks;
    };
    virtual void setupCharacteristics(BLEService *pService, BLEAdvertising *pAdvertising, QueueHandle_t tcodeQueue) = 0;
};