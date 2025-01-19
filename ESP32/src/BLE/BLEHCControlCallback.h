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

#include <climits>
#include <queue>

#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>

#include "logging/LogHandler.h"

#include "BLECharacteristicCallbacksBase.h"
#include "TagHandler.h"
#include "constants.h"

class BLEHCControlCallback: public BLECharacteristicCallbacksBase  
{
public:
    BLEHCControlCallback(QueueHandle_t tcodeQueue): m_TCodeQueue(tcodeQueue) { }
    // At some point this signature will change because its in master so if Bluetooth breaks, check the source class signature.
    #ifdef NIMBLE_LATEST
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo)  override {
    #else
    void onWrite(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc)  override {
    #endif
        // uint16_t handle = pCharacteristic->getHandle();
        NimBLEAttValue rxValue = pCharacteristic->getValue();
        size_t rxLength = rxValue.length();
        const uint8_t* rxData = rxValue.data();
        if(rxLength > 4) {
            Serial.println("Warning: it seems the format of HC data has changed.");
            return;
        }

        // // //esp_gatt_char_prop_t  prop = pCharacteristic->;
        // // //Serial.println(*rxValue,HEX);
        // // //00F418713F41
        // // // uint8_t x;
        // // // sscanf(rxValue, "%x", &x);
        // // //int value = (int)strtol(rxValue, NULL, 0);
        // // // LogHandler::info(_TAG, "rxValue: %u", *rxValue);
        // Serial.print("rxLength: ");
        // Serial.print(rxLength);
        // Serial.println();
        // Serial.print("handle: ");
        // Serial.print(handle);
        // Serial.println();
        // // LogHandler::info(m_TAG, "handle: %d", handle);
        // // LogHandler::info(m_TAG, "rxLength: %d", rxLength);

        // // // xQueueSend(m_TCodeQueue, input, 0);
        // // Serial.println();
        // for (int i = 0; i < rxLength; i++) {
        //     //tcode.ByteInput(input[i]);
        //     //tcode.ByteInput(input[i]);
        //     Serial.print(rxData[i] < 16 ? "0" : "");
        //     Serial.print(rxData[i],HEX);
        //     if(i<rxLength -1)
        //         Serial.print(":");
        // }
        // Serial.println();
        uint8_t n = rxLength;
        uint64_t totalBytes = 0ul;
        while (n--) totalBytes = totalBytes * 256 + rxData[n];// Concatenate and convert to big endian.
        uint16_t tcodeBytes = totalBytes & 0x0000FFFF;
        uint16_t speedBytes = (totalBytes & 0xFFFF0000) >> 16;
        tcode[MAX_COMMAND] = {0};
        snprintf(tcode, MAX_COMMAND, "L0%03dI%d\n", tcodeBytes, speedBytes);
        
        LogHandler::verbose(TagHandler::BLEHandler, "Receive HC tcode: %s", tcode);
        if(xQueueSend(m_TCodeQueue, tcode, 0) != pdTRUE) {
            LogHandler::error(TagHandler::BLEHandler, "Failed to write to queue");
        }
    }
private: 
    const char* m_TAG = TagHandler::BLEHandler;
    QueueHandle_t m_TCodeQueue;
    char tcode[MAX_COMMAND] = {0};

    template <typename T>
    T swap_endian(T u)
    {
        static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

        union
        {
            T u;
            unsigned char u8[sizeof(T)];
        } source, dest;

        source.u = u;

        for (size_t k = 0; k < sizeof(T); k++)
            dest.u8[k] = source.u8[sizeof(T) - k - 1];

        return dest.u;
    }
};