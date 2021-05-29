/* 
This works but we cant use Bluetooth and Wifi at the same time with out performance loss due to them having the same radio....
We need all the performance we can get in wifi so commenting this out. Maybe oneday they will launch a new board with them seperate.
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <sstream>

#pragma once

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
        Serial.print("Descriptor onWrite: ");
        uint8_t* value = pDescriptor->getValue();
        pDescriptor->setValue("Descriptor onWrite");
    }
    void onRead(BLEDescriptor *pDescriptor) 
    {
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
            if (rxValue.find(">><<") == -1) 
            {
                pCharacteristic->setValue(">");
                pCharacteristic->notify(); // Send the value to the app!
                Serial.print("*** Characteristic Sent Value: ");
                Serial.print("Ok");
                Serial.println(" ***");
                recievedJsonConfiguration += rxValue.data();
            }
            else if (rxValue.find(">>r<<") == 0) 
            {
                ESP.restart();
            }
            else 
            {
                // Save json
                Serial.println("Done: ");
                Serial.print(recievedJsonConfiguration);
                SettingsHandler::parse(recievedJsonConfiguration);
            }

            Serial.println();
            Serial.println("*********");
        }
    }
    void onRead(BLECharacteristic *pCharacteristic) 
    {
        char* sentValue = SettingsHandler::getJsonForBLE();
        pCharacteristic->setValue(sentValue);
        pCharacteristic->notify(); // Send the value to the app!
        Serial.print("*** Sent Value: ");
        Serial.print("Characteristic Onread Ok");
        Serial.println(" ***");
    }
};
class BLEHandler 
{
    private:
        BLECharacteristic *pCharacteristic;
        bool deviceConnected = false;
        float txValue = 0;
    public:
    void setup() 
    {
        // Create the BLE Device
        BLEDevice::init("TCodeESP32"); // Give it a name

        // Create the BLE Server
        BLEServer *pServer = BLEDevice::createServer();
        pServer->setCallbacks(new MyServerCallbacks());
        // Create the BLE Service
        BLEService *pService = pServer->createService(SERVICE_UUID);

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
        BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(SERVICE_UUID);
        pAdvertising->setScanResponse(true);
        pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
        BLEDevice::startAdvertising();
        Serial.println("BLE waiting a client connection to notify...");
    }

};
 */