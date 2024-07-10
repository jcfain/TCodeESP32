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
/* This works but we cant use Bluetooth and Wifi at the same time with out performance loss due to them having the same radio....
We need all the performance we can get in wifi so commenting this out. Maybe oneday they will launch a new board with them seperate. 
For now this is only for first configs */
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <sstream>
#include "SettingsHandler.h"
#include "LogHandler.h"
#include "TagHandler.h"


#define SERVICE_UUID "ff1b451d-3070-4276-9c81-5dc5ea1043bc" // UART service UUID
#define CHARACTERISTIC_UUID "c5f1543e-338d-47a0-8525-01e3c621359d"

class CharacteristicCallbacks: public BLECharacteristicCallbacks 
{
    String recievedJsonConfiguration = "";
    
    #ifdef ESP_ARDUINO3
        String sendJsonConfiguration = "";
    #else
        std::string sendJsonConfiguration = "";
    #endif
    unsigned sendChunkIndex = 0;
    bool sentLength = false;
    int sendMaxLen = 499;


    void onWrite(BLECharacteristic *pCharacteristic) 
    {
        SettingsHandler::printMemory();
        LogHandler::verbose(_TAG, "*** BLE onWrite");
        #ifdef ESP_ARDUINO3
            String rxValue = pCharacteristic->getValue();
        #else
            std::string rxValue = pCharacteristic->getValue();
        #endif

        if (rxValue.length() > 0) 
        {
            LogHandler::debug(_TAG, "*********");
            LogHandler::debug(_TAG, "BLE Characteristic Received Value: ");

            for (int i = 0; i < rxValue.length(); i++) {
                Serial.print(rxValue[i]);
            }

            Serial.println();

            // Do stuff based on the command received from the app
            if (rxValue.find(">>r<<") == 0) // Restart
            {
                LogHandler::debug(_TAG, "*** BLE Restarting");
                ESP.restart();
            }
            else if (rxValue.find(">>t<<") == -1) 
            {
                pCharacteristic->setValue(">"); // More please (Doesnt really matter as the que is client side)
                pCharacteristic->notify();
                LogHandler::debug(_TAG, "*** BLE Characteristic Sent Value: ");
                LogHandler::debug(_TAG, "Ok");
                LogHandler::debug(_TAG, " ***");
                recievedJsonConfiguration += rxValue.data();
            }
            else 
            {
                // Save json
                LogHandler::debug(_TAG, "Done: %s", recievedJsonConfiguration);
                if(SettingsHandler::saveAll(recievedJsonConfiguration)) 
                {
                    pCharacteristic->setValue(">>f<<"); // Finish saving
                    pCharacteristic->notify();
                    LogHandler::debug(_TAG, "*** BLE Finish saving");
                }
                else
                {
                    pCharacteristic->setValue(">>e<<"); // Error
                    LogHandler::error(_TAG, "*** BLE Error saving");
                    pCharacteristic->notify();
                }
                recievedJsonConfiguration = "";
            }

            Serial.println();
            LogHandler::verbose(_TAG, "BLE onWrite ***");
        }
    }
    void onRead(BLECharacteristic *pCharacteristic) 
    {
        LogHandler::verbose(_TAG, "*** BLE onRead");
        // char* sentValue = SettingsHandler::getJsonForBLE();
        if(sendJsonConfiguration.empty()) {
            char settings[2048] = {0};
            #warning need to send settings somehow
            //SettingsHandler::ser(settings); 
            LogHandler::debug(_TAG, "BLE Get wifi settings: %s", settings);
            if (strlen(settings) == 0) {
                LogHandler::error(_TAG, "*** BLE onRead empty");
                pCharacteristic->setValue(">>e<<");
                pCharacteristic->notify(); 
                return;
            }
            //LogHandler::info(_TAG, "*** Sent Value: %s", wifiSetting);
            //const int len = strlen(wifiSetting);
            //LogHandler::info(_TAG, "*** strlen: %i", strlen(wifiSetting));
            sendJsonConfiguration = std::string(settings);
            LogHandler::debug(_TAG, "BLE Get wifi string: %s", sendJsonConfiguration.c_str());
        }

        // size_t chunksize = wifiSettingsString.size()/19+1;
        // for(size_t i=0; i<wifiSettingsString.size(); i+=chunksize)
        // {  
        //     std::string value = wifiSettingsString.substr(i,chunksize);
        //     printf("loop %d, i*maxLen: %i : %s\n", i, chunksize, value.c_str());
        //     pCharacteristic->setValue(value);
        //     pCharacteristic->notify(); 
        // }
        if(!sentLength) {
            sentLength = true;
            char lengthNotify[10];
            sprintf(lengthNotify, ">>l<<:%zu", sendJsonConfiguration.length());
            pCharacteristic->setValue(lengthNotify);
            pCharacteristic->notify(); 

        } else if(sendChunkIndex < sendJsonConfiguration.length()) {
            if(sendJsonConfiguration.length() > sendMaxLen) {
                std::string value = sendJsonConfiguration.substr(sendChunkIndex,sendMaxLen);
                printf("index %d, length: %i, %s\n", sendChunkIndex, sendJsonConfiguration.length(), value.c_str());
                pCharacteristic->setValue(value);
                pCharacteristic->notify(); 
                // printf("ESP.getFreeHeap() %i\n", ESP.getFreeHeap());
                // printf("ESP.getHeapSize() %i\n", ESP.getHeapSize());
                sendChunkIndex += sendMaxLen;
                
                //delay(3);
            } else {
                pCharacteristic->setValue(sendJsonConfiguration);
                pCharacteristic->notify(); 
            }
        } else {
            LogHandler::debug(_TAG, ">>f<<");
            pCharacteristic->setValue(">>f<<");
            pCharacteristic->notify(); 
            sendJsonConfiguration = "";
            sentLength = false;
            sendChunkIndex = 0;
        }
        // int i = 0;
        // int maxLen = 19;
        // if(len > maxLen) {
        //     while (i*maxLen < len) {
        //         char chunk[maxLen + 1];
        //         memset(chunk, '\0', sizeof(chunk));
        //         memcpy(chunk, wifiSetting+(i*maxLen), maxLen);
        //         pCharacteristic->setValue((uint8_t*)chunk, maxLen);
        //         pCharacteristic->notify(); 

        //         printf("loop %d, i*maxLen: %i : %s\n", i, i*maxLen, chunk);
        //         delay(3);
        //         i++;
        //     }
        // } else {
        //     pCharacteristic->setValue(wifiSetting);
        //     pCharacteristic->notify(); 
        // }
        Serial.println();
        LogHandler::debug(_TAG, "BLE Onread Ok ***");
    }
public:
    void resetState() {
        recievedJsonConfiguration = "";
        sendJsonConfiguration = "";
        sentLength = false;
        sendChunkIndex = 0;
    }
private:
    const char* _TAG = TagHandler::BLEHandler;
    // QueueHandle_t debugInQueue;
    // int m_lastSend;
    // TaskHandle_t* emptyQueueHandle;
    // bool emptyQueueRunning;
};
CharacteristicCallbacks *characteristicCallbacks = new CharacteristicCallbacks();

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) 
    {
        LogHandler::debug(_TAG, "*********");
        LogHandler::debug(_TAG, "Device connected");
        characteristicCallbacks->resetState();
    };
    void onDisconnect(BLEServer* pServer) 
    {
        LogHandler::debug(_TAG, "*********");
        LogHandler::debug(_TAG, "Device disconnected");
        characteristicCallbacks->resetState();
        BLEDevice::startAdvertising();
    }
private:
    const char* _TAG = TagHandler::BLEHandler;
};

class DescriptorCallbacks: public BLEDescriptorCallbacks 
{
    void onWrite(BLEDescriptor *pDescriptor) 
    {
        LogHandler::debug(_TAG, "*********");
        LogHandler::debug(_TAG, "Descriptor onWrite: ");
    }
    void onRead(BLEDescriptor *pDescriptor) 
    {
        LogHandler::debug(_TAG, "*********");
        LogHandler::debug(_TAG, "Descriptor onRead: ");
    }
private:
    const char* _TAG = TagHandler::BLEHandler;
};

class BLEConfigurationHandler 
{
    private:
        const char* _TAG = TagHandler::BLEHandler;
        BLECharacteristic *pCharacteristic;
        bool deviceConnected = false;
        bool isInitailized = false;
        float txValue = 0;
        BLEServer *pServer;
        BLEService *pService;
        BLEAdvertising *pAdvertising;
    public:
    void setup() 
    {
        // Create the BLE Device
        LogHandler::info(_TAG, "Setup BLE: %s", "TCodeConfig");
        BLEDevice::init("TCodeConfig"); // Give it a name
        //BLEDevice::setMTU(23);
        // Create the BLE Server
        pServer = BLEDevice::createServer();
        pServer->setCallbacks(new MyServerCallbacks());
        // Create the BLE Service
        pService = pServer->createService(SERVICE_UUID);

        // Create a BLE Characteristic
          // Create a BLE Characteristic
        pCharacteristic = pService->createCharacteristic(
                            CHARACTERISTIC_UUID,
                            BLECharacteristic::PROPERTY_READ   |
                            BLECharacteristic::PROPERTY_WRITE  |
                            BLECharacteristic::PROPERTY_WRITE_NR |
                            BLECharacteristic::PROPERTY_NOTIFY |
                            BLECharacteristic::PROPERTY_INDICATE
                            );
        BLEDescriptor* p2902Descriptor = new BLEDescriptor(CHARACTERISTIC_UUID);
        p2902Descriptor->setCallbacks(new DescriptorCallbacks());
        pCharacteristic->addDescriptor(p2902Descriptor);
        pCharacteristic->setCallbacks(characteristicCallbacks);

        // Start the service
        pService->start();

        // Start advertising
        // Start advertising
        pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(SERVICE_UUID);
        pAdvertising->setScanResponse(true);
        pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
        BLEDevice::startAdvertising();
        LogHandler::info(_TAG, "BLE waiting a client connection to notify...");
        
        isInitailized = true;
        
        LogHandler::debug(_TAG, "Exit setup");
    }

    void stop() 
    {
        if(isInitailized) 
        {
            LogHandler::info(_TAG, "Stop");
            BLEDevice::deinit(true);
            LogHandler::debug(_TAG, "deinit");
            isInitailized = false;
            if(pServer != nullptr) 
                delete(pServer);
            if(pService != nullptr) 
                delete(pService);
            if(pCharacteristic != nullptr) 
                delete(pCharacteristic);
            if(pAdvertising != nullptr) 
                delete(pAdvertising);
            if(characteristicCallbacks != nullptr)
                delete(characteristicCallbacks);
        }
    }
};


