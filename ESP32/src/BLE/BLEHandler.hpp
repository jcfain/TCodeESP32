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
// #include <NimBLE2902.h>
#include <queue>
#include <sstream>
#include "esp_coexist.h"
#include "constants.h"
// #include "LogHandler.h"
#include "TagHandler.h"
#include "TCode/MotorHandler.h"
#include "logging/LogHandler.h"
#include "BLEHandlerBase.h"
#include "BLEHandlerTCode.h"
#include "BLEHandlerLove.h"
#include "BLEHandlerHC.h"

class BLEHandler
{
public:
    BLEHandler()
    {
        m_TCodeQueue = xQueueCreate(25, sizeof(char[MAX_COMMAND]));
        m_callBackQueue = xQueueCreate(5, sizeof(char[MAX_COMMAND]));
    }
    void setup()
    {
        // auto callbacks = getCaracteristicCallbacks();
        SettingsFactory::getInstance()->getValue(BLE_DEVICE_TYPE, m_bleDeviceType);

        m_subHandler = getHandler();
        m_subHandler->setup(m_TCodeQueue);

        // LogHandler::debug(_TAG, "Setting up BLE Characteristics");
        //  if(m_isHC) {
        //      m_tcodeCharacteristic = new BLECharacteristic(callbacks->CHARACTERISTIC_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
        //      m_tcodeCharacteristic2 = new BLECharacteristic(callbacks->CHARACTERISTIC_UUID2, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
        //      m_tcodeCharacteristic2->setCallbacks(callbacks);
        //  } else {
        //      m_tcodeCharacteristic = new BLECharacteristic(callbacks->CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE_NR);
        //  }
        //  LogHandler::debug(_TAG, "Setting up BLE Characteristic Callbacks");
        //  m_tcodeCharacteristic->setCallbacks(callbacks);

        // pService->addCharacteristic(m_tcodeCharacteristic);

        // if(m_isHC) {
        //     pService->addCharacteristic(m_tcodeCharacteristic2);
        // }

        // //https://github.com/espressif/esp-idf/issues/12434
        // // Before sending BLE command
        // esp_coex_status_bit_set(ESP_COEX_ST_TYPE_BLE, ESP_COEX_BLE_ST_MESH_CONFIG);

        // // After sending
        // esp_coex_status_bit_clear(ESP_COEX_ST_TYPE_BLE, ESP_COEX_BLE_ST_MESH_CONFIG);
        // Before sending BLE command
        // esp_coex_preference_set(ESP_COEX_PREFER_BT);

        // After sending
        // esp_coex_preference_set(ESP_COEX_PREFER_WIFI);

        // xTaskCreatePinnedToCore(
        // 	bleTask,/* Function to implement the task */
        // 	"BLETask", /* Name of the task */
        // 	configMINIMAL_STACK_SIZE*4,  /* Stack size in words */
        // 	static_cast<void*>(this),  /* Task input parameter */
        // 	tskIDLE_PRIORITY,  /* Priority of the task */
        // 	&m_bleTask,  /* Task handle. */
        // 	CONFIG_BT_NIMBLE_PINNED_TO_CORE); /* Core where the task should run */
    }

    // static void bleTask(void* arg) {
    //     BLEHandler* handler = static_cast<BLEHandler*>(arg);
    //     TickType_t pxPreviousWakeTime = millis();
    //     while(1) {
    //         auto len = handler->m_tcodeCharacteristic->getDataLength();
    //         if(len) {
    //             const char* value = handler->m_tcodeCharacteristic->getValue().c_str();
    //             LogHandler::verbose(_TAG, "Recieve tcode: %s", value);
    //             //strncpy(buf, value, m_tcodeCharacteristic->getDataLength());
    //             // if(strlen(value))
    //             //     handler->m_motorHandler->read(value, len);
    //             //handler->m_motorHandler->read(value);
    //             if(xQueueSend(m_TCodeQueue, value, 0) != pdTRUE) {
    //                 //LogHandler::error(_TAG, "Failed to write to queue");
    //             }
    //         }
    //         xTaskDelayUntil(&pxPreviousWakeTime, 10/portTICK_PERIOD_MS);
    //     }
    // }

    void read(char *buf)
    {
        // if(m_tcodeCharacteristic->getDataLength()) {
        //     const char* value = m_tcodeCharacteristic->getValue().c_str();
        //     strncpy(buf, value, m_tcodeCharacteristic->getDataLength());
        // }
        // else
        // {
        //     buf[0] = {0};
        // }
        if (xQueueReceive(m_TCodeQueue, buf, 0))
        {
            // LogHandler::verbose(_TAG, "Recieve tcode: %s", buf);
        }
        else
        {
            // LogHandler::error(_TAG, "Failed to read from queue");
            buf[0] = {0};
        }
    }

    bool isConnected()
    {
        if(!m_subHandler)
            return false;
        return m_subHandler->isConnected();
    }

    void CommandCallback(const char *in)
    {
        if(m_subHandler)
            m_subHandler->CommandCallback(in);
    }

    static void disable()
    {
        LogHandler::info(_TAG, "Disable BLE");
        //BLEDevice::deinit();
        //esp_err_t disable = esp_bt_controller_deinit();
        esp_err_t disable = esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);
        // esp_bluedroid_disable(); 
        // esp_bluedroid_deinit(); 
        // esp_bt_controller_disable(); 
        // esp_bt_controller_deinit();
        //esp_err_t disable = esp_bt_mem_release(ESP_BT_MODE_BTDM);
        if (disable != ESP_OK)
        {
            LogHandler::error(_TAG, "Disable fail: %s", esp_err_to_name(disable));
        }
    };

private:
    // friend class BLETCodeControlCallback;
    // friend class BLELoveControlCallback;
    static const char *_TAG;
    // const char* BLE_DEVICE_NAME = "TCODE-ESP32";
    // const char* BLE_TCODE_SERVICE_UUID = "ff1b451d-3070-4276-9c81-5dc5ea1043bc";
    // const char* BLE_TCODE_CHARACTERISTIC_UUID = "c5f1543e-338d-47a0-8525-01e3c621359d";
    BLEDeviceType m_bleDeviceType;
    bool m_isHC = false;
    bool m_isLove = true;
    BLEHandlerBase *m_subHandler = 0;

    QueueHandle_t m_TCodeQueue;
    QueueHandle_t m_callBackQueue;
    // // Haptics connect UUID's
    // const char* BLE_DEVICE_NAME_HC = "OSR-ESP32";
    // const char* BLE_TCODE_SERVICE_UUID_HC = "00004000-0000-1000-8000-0000101A2B3C";
    // const char* BLE_TCODE_CHARACTERISTIC_UUID_HC = "00002000-0001-1000-8000-0000101A2B3C";
    // const char* BLE_TCODE_CHARACTERISTIC_UUID2_HC = "00002000-0002-1000-8000-0000101A2B3C";

    TaskHandle_t m_bleTask;
    BLEHandlerBase *getHandler()
    {
        if (m_bleDeviceType == BLEDeviceType::LOVE)
        {
            LogHandler::info(_TAG, "Setting up BLE Love handler");
            static BLEHandlerLove bleHandler;
            return &bleHandler;
        }
        if (m_bleDeviceType == BLEDeviceType::HC)
        {
            LogHandler::info(_TAG, "Setting up BLE HC handler");
            static BLEHandlerHC bleHandler;
            return &bleHandler;
        }
        LogHandler::info(_TAG, "Setting up BLE Tcode handler");
        static BLEHandlerTCode bleHandler;
        return &bleHandler;
    };
};
const char *BLEHandler::_TAG = TagHandler::BLEHandler;
