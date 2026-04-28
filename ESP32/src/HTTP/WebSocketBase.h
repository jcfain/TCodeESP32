#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
// #include "LogHandler.h"
#include "BatteryHandler.h"
#include "TCodeInterface.h"

class WebSocketBase : public TCodeInterface {
    public:
    virtual void sendCommand(const char* command, const char* message = 0) = 0;
    virtual void closeAll() = 0;

    size_t available() override
    {
        return m_TCodeQueue && uxQueueMessagesWaiting(m_TCodeQueue);
    }

    size_t read(char* buf)  override
    {
        if(!m_TCodeQueue || m_TCodeQueue == NULL)
        {
            if(millis() >= lastMessage + messageLimit) {
                lastMessage = millis();
                LogHandler::error(_TAG, "Websocket TCode queue was null");
            }
            return 0;
        } 
        if(!xQueueReceive(m_TCodeQueue, buf, 0)) 
        {
            buf[0] = {0};
            return 0;
        }
        //tcode->toCharArray(webSocketData, tcode->length() + 1);
        // Serial.print("Top tcode: ");
        // Serial.println(webSocketData);
        return strnlen(buf, MAX_COMMAND);
    }

protected:
    bool isInitialized = false;
    QueueHandle_t m_TCodeQueue;
    std::mutex command_mtx;

    void compileCommand(char* buf, const char* command, const char* message = 0) {
        if(LogHandler::getLogLevel() == LogLevel::DEBUG) {
            if(message)
                Serial.printf("Sending WS commands: %s, Message: %s\n", command, message);
            else
                Serial.printf("Sending WS commands: %s\n",command);
        }
        if(!message)
            sprintf(buf, "{ \"command\": \"%s\" }", command);
        else if(strpbrk(message, "{") != nullptr)
            sprintf(buf, "{ \"command\": \"%s\" , \"message\": %s }", command, message);
        else
            sprintf(buf, "{ \"command\": \"%s\" , \"message\": \"%s\" }", command, message);
    }
    void processWebSocketTextMessage(const char* msg) 
    {
        if(strpbrk(msg, "{") == nullptr)  
        {
            if(!m_TCodeQueue || m_TCodeQueue == NULL)
            {
                LogHandler::error(_TAG, "TCode queue was null");
            } 
            else 
            {
                
                LogHandler::verbose(_TAG, "Websocket tcode in: %s", msg);
                xQueueSend(m_TCodeQueue, msg, 0);
// Serial.print("Time between ws calls: ");
// Serial.println(millis() - lastCall);
// //Serial.println(msg);
// lastCall = millis();
                //executeTCode(msg);
            }
            // if (strcmp(msg, SettingsHandler::HandShakeChannel) == 0) 
            // {
            //     sendCommand(SettingsHandler::TCodeVersionName);
            // }
        }
        else
        {
            JsonDocument doc; //255
            DeserializationError error = deserializeJson(doc, msg);
            if (error) 
            {
                LogHandler::error(_TAG, "Failed to read websocket json");
                return;
            }
            JsonObject jsonObj = doc.as<JsonObject>();

            if(!jsonObj["command"].isNull()) 
            {
                String command = jsonObj["command"].as<String>();
                String message = jsonObj["message"].as<String>();
                if(command == "setBatteryFull") {
                    BatteryHandler::setBatteryToFull();
                }
                // String* message = jsonObj["message"];
                // Serial.print("Recieved websocket tcode message: ");
                // Serial.println(message->c_str());
                // if(m_TCodeQueue == NULL)return;
                // xQueueSend(m_TCodeQueue, &message, 0);
            } 
            // else 
            // {
            //     LogHandler::verbose(_TAG, "Websocket tcode in JSON: %s", msg);
            //     char tcode[MAX_COMMAND];
            //     SettingsHandler::processTCodeJson(tcode, msg);
            //     // Serial.print("tcode JSON converted:");
            //     // Serial.println(tcode);
            //     xQueueSend(m_TCodeQueue, tcode, 0);
            // }
        }
    }

private:
    const char* _TAG = "webSocket-base";
    // std::mutex serial_mtx;
    // static QueueHandle_t debugInQueue;
    // static TaskHandle_t* emptyQueueHandle;
    // static bool emptyQueueRunning;
    int messageLimit = 5000;
    unsigned long lastMessage = millis();
};


// bool WebSocketBase::emptyQueueRunning = false;
// QueueHandle_t WebSocketBase::debugInQueue;
// TaskHandle_t* WebSocketBase::emptyQueueHandle = NULL;