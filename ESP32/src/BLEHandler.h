
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
        Serial.println("*********");
        Serial.print("Device connected");
    };
    void onDisconnect(BLEServer* pServer) 
    {
        Serial.println("*********");
        Serial.print("Device disconnected");
        BLEDevice::startAdvertising();
    }
};

class DescriptorCallbacks: public BLEDescriptorCallbacks 
{
    void onWrite(BLEDescriptor *pDescriptor) 
    {
        Serial.println("*********");
        Serial.print("Descriptor onWrite: ");
    }
    void onRead(BLEDescriptor *pDescriptor) 
    {
        Serial.println("*********");
        Serial.print("Descriptor onRead: ");
    }
};
class CharacteristicCallbacks: public BLECharacteristicCallbacks 
{
    String recievedJsonConfiguration = "";
    void onWrite(BLECharacteristic *pCharacteristic) 
    {
        std::string rxValue = pCharacteristic->getValue();

        if (rxValue.length() > 0) 
        {
            Serial.println("*********");
            Serial.print("Characteristic Received Value: ");

            for (int i = 0; i < rxValue.length(); i++) {
                Serial.print(rxValue[i]);
            }

            Serial.println();

            // Do stuff based on the command received from the app
            if (rxValue.find(">>r<<") == 0) // Restart
            {
                Serial.print("*** Restarting");
                ESP.restart();
            }
            else if (rxValue.find(">>t<<") == -1) 
            {
                pCharacteristic->setValue(">"); // More please (Doesnt really matter as the que is client side)
                pCharacteristic->notify();
                Serial.print("*** Characteristic Sent Value: ");
                Serial.print("Ok");
                Serial.println(" ***");
                recievedJsonConfiguration += rxValue.data();
            }
            else 
            {
                // Save json
                Serial.println("Done: ");
                Serial.print(recievedJsonConfiguration);
                if(SettingsHandler::derializeWifiSettings(recievedJsonConfiguration)) 
                {
                    pCharacteristic->setValue(">>f<<"); // Finish saving
                    pCharacteristic->notify();
                    Serial.print("*** Finish saving");
                }
                else
                {
                    pCharacteristic->setValue(">>e<<"); // Error
                    Serial.print("*** Error saving");
                    pCharacteristic->notify();
                }
                recievedJsonConfiguration = "";
            }

            Serial.println();
            Serial.println("*********");
        }
    }
    void onRead(BLECharacteristic *pCharacteristic) 
    {
        // char* sentValue = SettingsHandler::getJsonForBLE();
        String wifiSetting = SettingsHandler::serializeWifiSettings();
        Serial.print("*** Sent Value: ");
        Serial.print(wifiSetting);
        pCharacteristic->setValue(wifiSetting.c_str());
        pCharacteristic->notify(); // Send the value to the app!
        Serial.print("Characteristic Onread Ok");
        Serial.println(" ***");
    }
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
        // Create the BLE Device
        BLEDevice::init("TCodeConfigurator"); // Give it a name

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
        Serial.println("BLE waiting a client connection to notify...");
        isInitailized = true;
    }

    void stop() 
    {
        if(isInitailized) 
        {
            Serial.println("BLE Stop");
            BLEDevice::deinit(true);
            Serial.println("BLE deinit");
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
};
