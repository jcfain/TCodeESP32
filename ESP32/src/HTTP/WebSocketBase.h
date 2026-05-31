#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
// #include "LogHandler.h"
#include "BatteryHandler.h"
#include "TCodeInterface.h"

class WebSocketBase : public TCodeInterface {
    public:
    void init() 
    {
        isBaseInitialized = true;
    }

    virtual void sendCommand(const char* command, const char* message = 0, size_t len = MAX_COMMAND) = 0;
    virtual void closeAll() = 0;
    virtual void sendLogTask(void *webSocketHandler) = 0;

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

    void sendError(const char* message, size_t len = MAX_COMMAND) 
    {
        sendCommand("error", message, len);
    }

    static void startLoggingTask(void *taskStartParameters)
    {
        ((WebSocketBase*)taskStartParameters)->sendLogTask(taskStartParameters);
    }
    static void stopLoggingTask(void *taskStartParameters)
    {
        ((WebSocketBase*)taskStartParameters)->sendLogTaskRunning = false;
    }

protected:
    QueueHandle_t m_TCodeQueue;
    std::mutex command_mtx;
    bool sendLogTaskRunning = false;

    void compileCommand(char* buf, const char* command, const char* message = 0, size_t len = MAX_COMMAND) {
        if(LogHandler::getLogLevel() == LogLevel::DEBUG) {
            if(message)
                Serial.printf("Sending WS commands: %s, Message: %s\n", command, message);
            else
                Serial.printf("Sending WS commands: %s\n",command);
        }
        if(!message)
            snprintf(buf, len, "{ \"command\": \"%s\" }", command);
        else if(strpbrk(message, "{") != nullptr)
            snprintf(buf, len, "{ \"command\": \"%s\" , \"message\": %s }", command, message);
        else
            snprintf(buf, len, "{ \"command\": \"%s\" , \"message\": \"%s\" }", command, message);
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
            LogHandler::debug(_TAG, "Websocket command in: %s", msg);
            JsonDocument doc; //255
            if (LogHandler::logDeserializationError(_TAG, deserializeJson(doc, msg), "websocket json")) 
            {
                return;
            }
            JsonObject jsonObj = doc.as<JsonObject>();

            if(!jsonObj["command"].isNull()) 
            {
                String command = jsonObj["command"].as<String>();
                if(command == "setBatteryFull") {
                    BatteryHandler::setBatteryToFull();
                } else if(command == "setting") {
                    SettingFile file = SettingFile::NONE;
                    JsonObject messageObj = jsonObj["message"].as<JsonObject>();
                    SettingType type = messageObj["type"].as<SettingType>();
                    const char* name = messageObj["name"].as<const char*>();
                    switch(type) {
                        case SettingType::String: {
                            const char* value = messageObj["value"].as<const char*>();
                            file = SettingsFactory::getInstance()->setValue(name, value, true);
                            break;
                        }
                        case SettingType::Number: {
                            int value = messageObj["value"].as<int>();
                            file = SettingsFactory::getInstance()->setValue(name, value, true);
                            break;
                        }
                        case SettingType::Boolean: {
                            bool value = messageObj["value"].as<bool>();
                            file = SettingsFactory::getInstance()->setValue(name, value, true);
                            break;
                        }
                        case SettingType::Float: {
                            float value = messageObj["value"].as<float>();
                            file = SettingsFactory::getInstance()->setValue(name, value, true);
                            break;
                        }
                        case SettingType::Double: {
                            double value = messageObj["value"].as<double>();
                            file = SettingsFactory::getInstance()->setValue(name, value, true);
                            break;
                        }
                        case SettingType::ArrayInt: {
                            std::vector<int> value = messageObj["value"].as<std::vector<int>>();
                            file = SettingsFactory::getInstance()->setValue(name, value, true);
                            break;
                        }
                        case SettingType::ArrayString: {
                            std::vector<const char*> value = messageObj["value"].as<std::vector<const char*>>();
                            file = SettingsFactory::getInstance()->setValue(name, value, true);
                            break;
                        }
                        default: {
                            LogHandler::error(_TAG, "[WebSocketBase] processWebSocketTextMessage: invalid setting type: %i for %s.", type, name);
                            sendError("Error saving setting!");
                            return;
                        }
                        
                    }
                    if(file == SettingFile::NONE) {
                        LogHandler::error(_TAG, "[WebSocketBase] processWebSocketTextMessage: Setting not found: %s.", name);
                        sendError("Error saving setting!");
                        return;
                    }
                    sendCommand("saved", name);
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
    bool isBaseInitialized = false;
    int messageLimit = 5000;
    unsigned long lastMessage = millis();
};