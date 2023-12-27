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
        m_TCodeQueue = xQueueCreate(600, sizeof(const char*));

        BLEDevice::init(BLE_DEVICE_NAME);
        BLEServer *pServer = BLEDevice::createServer();
        pServer->setCallbacks(new ServerCallbacks());

        m_tcodeCharacteristic = new BLECharacteristic(BLE_TCODE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ  | BLECharacteristic::PROPERTY_WRITE_NR);
        m_tcodeCharacteristic->setValue("");
        m_tcodeCharacteristic->setCallbacks(new BLETCodeControlCallback());

        BLEService *pService = pServer->createService(BLE_TCODE_SERVICE_UUID);
        pService->addCharacteristic(m_tcodeCharacteristic);

        pService->start();

        BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(BLE_TCODE_SERVICE_UUID);
        pAdvertising->setScanResponse(true);

        // Functions that help with iPhone connections issue
        // https://github.com/nkolban/esp32-snippets/issues/768
        pAdvertising->setMinPreferred(0x06);  
        pAdvertising->setMaxPreferred(0x12);
        BLEDevice::startAdvertising();
        LogHandler::info(_TAG, "Started BLE Server.");
    }

    void read(char* buf) {
        if(xQueueReceive(m_TCodeQueue, buf, 0)) {
            LogHandler::verbose(_TAG, "Recieve tcode: %s", buf);
        } else {
            buf[0] = {0};
        }
    }

private: 
    static const char* _TAG;
    const char* BLE_DEVICE_NAME = "TCode ESP32";
    const char* BLE_TCODE_SERVICE_UUID = "ff1b451d-3070-4276-9c81-5dc5ea1043bc";
    const char* BLE_TCODE_CHARACTERISTIC_UUID = "c5f1543e-338d-47a0-8525-01e3c621359d";
    BLECharacteristic* m_tcodeCharacteristic;
    static QueueHandle_t m_TCodeQueue;

    // ----------------------------------------
    // Functions to handle Bluetooth LE Setup.
    //-----------------------------------------
    class BLETCodeControlCallback: public BLECharacteristicCallbacks {
        void onWrite(BLECharacteristic *pCharacteristic) {
            const char* input = pCharacteristic->getValue().c_str();
            const int length = strlen(input);

            xQueueSend(m_TCodeQueue, input, 0);
            for (int i = 0; i < length; i++) {
                //tcode.ByteInput(input[i]);
            }
        }
    };

    class ServerCallbacks: public BLEServerCallbacks {
        void onConnect(BLEServer* pServer) {
            LogHandler::info(_TAG, "A client has connected via BLE.");
        };
        void onDisconnect(BLEServer* pServer) {
            LogHandler::info(_TAG, ("A client has disconnected from BLE."));
            pServer->startAdvertising(); 
        }
    };
    
};
QueueHandle_t BLEHandler::m_TCodeQueue;
const char*  BLEHandler::_TAG = TagHandler::BLEHandler;
