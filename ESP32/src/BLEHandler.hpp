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
#include "esp_coexist.h"
#include "../lib/constants.h"
#include "LogHandler.h"
#include "TagHandler.h"
#include "TCode/MotorHandler.h"

class ServerCallbacks;
class BLETCodeControlCallback;

class BLEHandler {
public:
    void setup () {
        LogHandler::info(_TAG, "Setting up BLE Tcode handler");
        m_TCodeQueue = xQueueCreate(25, sizeof(char[MAX_COMMAND]));

        LogHandler::debug(_TAG, "Setting up BLE init device");
        BLEDevice::init(m_isHC ? BLE_DEVICE_NAME_HC : BLE_DEVICE_NAME);
        LogHandler::debug(_TAG, "Setting up BLE Create server");
        BLEServer *pServer = BLEDevice::createServer();
        pServer->setCallbacks(getServerCallbacks());
        pServer->advertiseOnDisconnect(true);

        LogHandler::debug(_TAG, "Setting up BLE Characteristics");
        if(m_isHC) {
            m_tcodeCharacteristic = new BLECharacteristic(BLE_TCODE_CHARACTERISTIC_UUID_HC, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
            m_tcodeCharacteristic2 = new BLECharacteristic(BLE_TCODE_CHARACTERISTIC_UUID2_HC, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
        } else {
            m_tcodeCharacteristic = new BLECharacteristic(BLE_TCODE_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE_NR);
        }
        LogHandler::debug(_TAG, "Setting up BLE Characteristic Callbacks");
        m_tcodeCharacteristic->setCallbacks(getCaracteristicCallbacks());
        if(m_isHC) {
            m_tcodeCharacteristic2->setCallbacks(getCaracteristicCallbacks());
        }

        LogHandler::debug(_TAG, "Setting up BLE Service");
        BLEService *pService = pServer->createService(m_isHC ? BLE_TCODE_SERVICE_UUID_HC : BLE_TCODE_SERVICE_UUID);
        pService->addCharacteristic(m_tcodeCharacteristic);
        
        if(m_isHC) {
            pService->addCharacteristic(m_tcodeCharacteristic2);
        }

        LogHandler::debug(_TAG, "Starting BLE Service");
        if(pService->start()) {
            LogHandler::info(_TAG, "Started BLE service");
        } else {
            LogHandler::error(_TAG, "Failed to start BLE service.");
        }

        BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(BLE_TCODE_SERVICE_UUID);
        pAdvertising->setScanResponse(true);

        // Functions that help with iPhone connections issue
        // https://github.com/nkolban/esp32-snippets/issues/768
        // pAdvertising->setMinPreferred(0x06);  
        // pAdvertising->setMaxPreferred(0x12);
        
        LogHandler::debug(_TAG, "Starting BLE Advertising");
        if(BLEDevice::startAdvertising())
            LogHandler::info(_TAG, "Started BLE Server.");
        else
            LogHandler::error(_TAG, "Failed to start BLE advertising.");
        // //https://github.com/espressif/esp-idf/issues/12434
        // // Before sending BLE command
        // esp_coex_status_bit_set(ESP_COEX_ST_TYPE_BLE, ESP_COEX_BLE_ST_MESH_CONFIG);

        // // After sending
        // esp_coex_status_bit_clear(ESP_COEX_ST_TYPE_BLE, ESP_COEX_BLE_ST_MESH_CONFIG);
        // Before sending BLE command
        //esp_coex_preference_set(ESP_COEX_PREFER_BT);

        //After sending
        //esp_coex_preference_set(ESP_COEX_PREFER_WIFI);
        
		// xTaskCreatePinnedToCore(
		// 	bleTask,/* Function to implement the task */
		// 	"BLETask", /* Name of the task */
		// 	configMINIMAL_STACK_SIZE*4,  /* Stack size in words */
		// 	static_cast<void*>(this),  /* Task input parameter */
		// 	tskIDLE_PRIORITY,  /* Priority of the task */
		// 	&m_bleTask,  /* Task handle. */
		// 	CONFIG_BT_NIMBLE_PINNED_TO_CORE); /* Core where the task should run */
    }

    static void bleTask(void* arg) {
        BLEHandler* handler = static_cast<BLEHandler*>(arg);
        TickType_t pxPreviousWakeTime = millis();
        while(1) {
            auto len = handler->m_tcodeCharacteristic->getDataLength();
            if(len) {
                const char* value = handler->m_tcodeCharacteristic->getValue().c_str();
                LogHandler::verbose(_TAG, "Recieve tcode: %s", value);
                //strncpy(buf, value, m_tcodeCharacteristic->getDataLength());
                // if(strlen(value))
                //     handler->m_motorHandler->read(value, len);
                //handler->m_motorHandler->read(value);
                if(xQueueSend(m_TCodeQueue, value, 0) != pdTRUE) {
                    //LogHandler::error(_TAG, "Failed to write to queue");
                }
            }
            xTaskDelayUntil(&pxPreviousWakeTime, 10/portTICK_PERIOD_MS);
        }
    }

    void read(char* buf) {
        // if(m_tcodeCharacteristic->getDataLength()) {
        //     const char* value = m_tcodeCharacteristic->getValue().c_str();
        //     strncpy(buf, value, m_tcodeCharacteristic->getDataLength());
        // }
        // else
        // {
        //     buf[0] = {0};
        // }
        if(xQueueReceive(m_TCodeQueue, buf, 0)) {
            //LogHandler::verbose(_TAG, "Recieve tcode: %s", buf);
        } else {
            //LogHandler::error(_TAG, "Failed to read from queue");
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
    TaskHandle_t m_bleTask;

    // ----------------------------------------
    // Functions to handle Bluetooth LE Setup.
    //-----------------------------------------
    class BLETCodeControlCallback: public NimBLECharacteristicCallbacks   {
        // At some point this signature will change because its in master so if Bluetooth breaks, check the source class signature.
        //void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
        void onWrite(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc) {
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

            size_t len = pCharacteristic->getDataLength();
            if(len) {
                NimBLEAttValue rxValue = pCharacteristic->getValue();
                LogHandler::verbose(_TAG, "Recieve tcode: %s", rxValue.c_str());
                if(xQueueSend(m_TCodeQueue, rxValue.c_str(), 0) != pdTRUE) {
                    LogHandler::error(_TAG, "Failed to write to queue");
                }
            }
            // size_t len = pCharacteristic->getDataLength();
            // const char* value = pCharacteristic->getValue().c_str();
            // LogHandler::verbose(_TAG, "Recieve tcode: %s, len: %ld", value, len);
            // if(m_motorHandler)
            //     m_motorHandler->read(value, len);
        };
        
        //void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo){
        void onRead(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc){
            Serial.print(pCharacteristic->getUUID().toString().c_str());
            Serial.print(": onRead(), value: ");
            Serial.println(pCharacteristic->getValue().c_str());
        };
        void onNotify(NimBLECharacteristic* pCharacteristic) {
            Serial.println("Sending notification to clients");
        };


        /**
         *  The value returned in code is the NimBLE host return code.
         */
        //void onStatus(NimBLECharacteristic* pCharacteristic, int code) {
        void onStatus(NimBLECharacteristic* pCharacteristic, Status s, int code) {
            String str = ("Notification/Indication return code: ");
            str += code;
            str += ", ";
            str += NimBLEUtils::returnCodeToString(code);
            Serial.println(str);
        };

        //void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) {
        void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) {
            String str = "";
            //"Client ID: ";
            // str += connInfo.getConnHandle();
            // str += " Address: ";
            // str += connInfo.getAddress().toString().c_str();
            if(subValue == 0) {
                str += " Unsubscribed to ";
            }else if(subValue == 1) {
                str += " Subscribed to notfications for ";
            } else if(subValue == 2) {
                str += " Subscribed to indications for ";
            } else if(subValue == 3) {
                str += " Subscribed to notifications and indications for ";
            }
            str += std::string(pCharacteristic->getUUID()).c_str();

            Serial.println(str);
        };
    };

    class ServerCallbacks: public NimBLEServerCallbacks {
        // Next version of NimBLE
        // void onConnect(BLEServer* pServer, NimBLEConnInfo& param) {
        //     LogHandler::info(_TAG, "A client has connected via BLE: %s",
        //         param.getAddress().toString().c_str()
        //     );
        // };
        // void onDisconnect(BLEServer* pServer, NimBLEConnInfo& param) {
        //     LogHandler::info(_TAG, "A client has disconnected from BLE: %s",
        //         param.getAddress().toString().c_str()
        //     );
        //     pServer->startAdvertising(); 
        // }        
        void onConnect(BLEServer* pServer, ble_gap_conn_desc* desc) {
            LogHandler::info(_TAG, "A client has connected via BLE");
        };
        void onDisconnect(BLEServer* pServer, ble_gap_conn_desc* desc) {
            LogHandler::info(_TAG, "A client has disconnected from BLE");
            //pServer->startAdvertising(); 
        }
    };
    ServerCallbacks* getServerCallbacks() {
        static ServerCallbacks callbacks;
        return &callbacks;
    };
    BLETCodeControlCallback* getCaracteristicCallbacks() {
        static BLETCodeControlCallback callbacks;
        return &callbacks;
    };
};
QueueHandle_t BLEHandler::m_TCodeQueue;
const char*  BLEHandler::_TAG = TagHandler::BLEHandler;
