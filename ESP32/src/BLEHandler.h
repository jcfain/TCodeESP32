
#pragma once
/* This works but we cant use Bluetooth and Wifi at the same time with out performance loss due to them having the same radio....
We need all the performance we can get in wifi so commenting this out. Maybe oneday they will launch a new board with them seperate. 
For now this is only for first configs */
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <sstream>


#define SERVICE_UUID "ff1b451d-3070-4276-9c81-5dc5ea1043bc" // UART service UUID
#define CHARACTERISTIC_UUID "c5f1543e-338d-47a0-8525-01e3c621359d"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) 
    {
        LogHandler::debug(_TAG, "*********");
        LogHandler::debug(_TAG, "Device connected");
    };
    void onDisconnect(BLEServer* pServer) 
    {
        LogHandler::debug(_TAG, "*********");
        LogHandler::debug(_TAG, "Device disconnected");
        BLEDevice::startAdvertising();
    }
private:
    const char* _TAG = "BLE";
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
    const char* _TAG = "BLE";
};
class CharacteristicCallbacks: public BLECharacteristicCallbacks 
{
    String recievedJsonConfiguration = "";
    std::string sendJsonConfiguration = "";
    unsigned sendChunkIndex = 0;
    int sendMaxLen = 499;

    void onWrite(BLECharacteristic *pCharacteristic) 
    {
        SettingsHandler::printMemory();
        LogHandler::verbose(_TAG, "*** BLE onWrite");
        std::string rxValue = pCharacteristic->getValue();

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
                LogHandler::debug(_TAG, "*** BLECharacteristic Sent Value: ");
                LogHandler::debug(_TAG, "Ok");
                LogHandler::debug(_TAG, " ***");
                recievedJsonConfiguration += rxValue.data();
            }
            else 
            {
                // Save json
                LogHandler::debug(_TAG, "Done: %s", recievedJsonConfiguration);
                if(SettingsHandler::save(recievedJsonConfiguration)) 
                {
                    pCharacteristic->setValue(">>f<<"); // Finish saving
                    pCharacteristic->notify();
                    LogHandler::debug(_TAG, "*** BLEFinish saving");
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
            const char* wifiSetting = SettingsHandler::serialize();
            LogHandler::debug(_TAG, "BLE Get wifi settings: %s", wifiSetting);
            if (strlen(wifiSetting) == 0) {
                LogHandler::error(_TAG, "*** BLE onRead empty");
                return;
            }
            //LogHandler::info(_TAG, "*** Sent Value: %s", wifiSetting);
            //const int len = strlen(wifiSetting);
            //LogHandler::info(_TAG, "*** strlen: %i", strlen(wifiSetting));
            sendJsonConfiguration = std::string(wifiSetting);
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
        if(sendChunkIndex < sendJsonConfiguration.length()) {
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
private:
    const char* _TAG = "BLE";
    // QueueHandle_t debugInQueue;
    // int m_lastSend;
    // TaskHandle_t* emptyQueueHandle;
    // bool emptyQueueRunning;
};
class BLEHandler 
{
    private:
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
        LogHandler::verbose(_TAG, "*** BLE setup");
        // Create the BLE Device
        BLEDevice::init("TCodeConfigurator"); // Give it a name
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
        pCharacteristic->setCallbacks(new CharacteristicCallbacks());

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
        
        LogHandler::verbose(_TAG, "BLE setup ***");
    }

    void stop() 
    {
        if(isInitailized) 
        {
            LogHandler::info(_TAG, "*** BLE Stop");
            BLEDevice::deinit(true);
            LogHandler::info(_TAG, "BLE deinit");
            isInitailized = false;
            if(pServer != nullptr) 
                delete(pServer);
            if(pService != nullptr) 
                delete(pService);
            if(pCharacteristic != nullptr) 
                delete(pCharacteristic);
            if(pAdvertising != nullptr) 
                delete(pAdvertising);
        }
    }
private:
    const char* _TAG = "BLE";
};
