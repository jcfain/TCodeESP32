#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <iomanip>
// #include "LogHandler.h"
#include "BatteryHandler.h"
#include "TCodeInterface.h"

class WebSocketBase : public TCodeInterface {
    public:
    void init() 
    {
        isBaseInitialized = true;
    }

    virtual void sendCommand(const char* command, size_t commandLen, const char* message = 0, size_t messageLen = MAX_WS_MESSAGE) = 0;
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

    void sendError(const char* message, size_t len = MAX_WS_MESSAGE) 
    {
        sendCommand("error", 6, message, len);
    }
    void sendRawLog(const char* message, size_t len = MAX_WS_MESSAGE) 
    {
        sendCommand("log", 4, message, len);
    }
    void sendLog(LogLevel level, const char* message, size_t len = MAX_WS_MESSAGE) {

		switch (level) 
        {
            case LogLevel::NONE:
                sendRawLog(message, len);
                break;
            case LogLevel::INFO:
                sendCommand("info", 5, message, len);
                break;
            case LogLevel::WARNING:
                sendCommand("warn", 5, message, len);
                break;
            case LogLevel::ERROR:
                sendError(message, len);
                break;
            case LogLevel::VERBOSE:
                sendCommand("verbose", 8, message, len);
                break;
            case LogLevel::DEBUG:
                sendCommand("debug", 6, message, len);
                break;
            default:
                break;
		}
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
    static const int COMMAND_PADDING = 100;

    size_t compileCommand(char* buf, const char* command, size_t commandLen, const char* message = 0, size_t messageLen = 0) {
        if(commandLen > MAX_WS_COMMAND || strlen(command) > MAX_WS_COMMAND) 
        {
            Serial.printf("[WebSocketBase] compileCommand: ERROR Sending WS commands: %s, \nCommand len (%i) it too long! MAX(%i)\n", command, commandLen, MAX_WS_COMMAND);
            return 0;
        }
        if(message && (messageLen > MAX_WS_MESSAGE || strlen(message) > MAX_WS_MESSAGE)) 
        {
            Serial.printf("[WebSocketBase] compileCommand: ERROR Sending WS commands: %s, Message: %s,\nMessage len (%i) it too long! MAX(%i)\n", command, message, messageLen, MAX_WS_MESSAGE);
            return 0;
        }
        if(LogHandler::getLogLevel() > LogLevel::DEBUG) 
        {
            if(message)
                Serial.printf("[WebSocketBase] compileCommand: Sending WS commands: %s, Message: %s\n", command, message);
            else
                Serial.printf("[WebSocketBase] compileCommand: Sending WS commands: %s\n",command);
        }
        size_t jsonLen = 0;
        size_t len = commandLen + messageLen + COMMAND_PADDING;
        // Serial.printf("WS commands: %s, Message: %s\n", command, message);
        if(!message) 
        {
            char commandEscaped[MAX_WS_COMMAND];
            escape_json(commandEscaped, command, commandLen);
            jsonLen = snprintf(buf, len, "{\"command\":\"%s\"}", commandEscaped);
        } 
        else if(strpbrk(message, "{") != nullptr) 
        {
            jsonLen = snprintf(buf, len, "{\"command\":\"%s\",\"message\":%s}", command, message);
        }
        else 
        {
            char messageEscaped[MAX_WS_MESSAGE];
            escape_json(messageEscaped, message, messageLen);
            jsonLen = snprintf(buf, len, "{\"command\":\"%s\",\"message\":\"%s\"}", command, messageEscaped);
            // Serial.printf("Sanitized WS commands: %s, Message: %s\n", command, messageEscaped);
        }
        return jsonLen;
    }

    // https://stackoverflow.com/questions/7724448/simple-json-string-escape-for-c
    size_t escape_json(char* out, const char* s, const size_t& len) {
        std::ostringstream o;
        for (auto i = 0; i<len; i++) 
        {
            switch (s[i]) {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if ('\x00' <= s[i] && s[i] <= '\x1f') {
                    o << "\\u"
                    << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(s[i]);
                } else {
                    o << s[i];
                }
            }
        }
        std::string outStr = o.str();
        size_t newLen = outStr.length();
        strncpy(out, outStr.c_str(), newLen);
        //snprintf(out, newLen, "%s", outStr.c_str());
        out[newLen] = {0};
        return newLen;
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
            DeserializationError::Code code;
            if (LogHandler::logDeserializationError(_TAG, deserializeJson(doc, msg), "websocket json", code)) 
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
                            // sendError("Error saving setting!");
                            char returnBuffer[MAX_WS_MESSAGE] = {0};
                            messageObj["status"] = "Invalid setting type";
                            size_t returnlen = serializeJson(messageObj, returnBuffer);
                            sendCommand("saveFail", 9, returnBuffer, returnlen);
                            // TODO figure out how to callback in front end to revert UI
                            // sendCommand("saveFail", returnBuffer, MAX_COMMAND_LENGTH, returnlen);
                            return;
                        }
                        
                    }
                    if(file == SettingFile::NONE) {
                        LogHandler::error(_TAG, "[WebSocketBase] processWebSocketTextMessage: Setting not found: %s.", name);
                        // sendError("Error saving setting!");
                        char returnBuffer[MAX_WS_MESSAGE] = {0};
                        messageObj["status"] = "Setting not found";
                        size_t returnlen = serializeJson(messageObj, returnBuffer);
                        sendCommand("saveFail", 9, returnBuffer, returnlen);
                        // TODO figure out how to callback in front end to revert UI
                        // sendCommand("saveFail", returnBuffer, MAX_COMMAND_LENGTH, returnlen);
                        return;
                    }
                    char returnBuffer[MAX_WS_MESSAGE] = {0};
                    messageObj["status"] = "Success";
                    size_t returnlen = serializeJson(messageObj, returnBuffer);
                    sendCommand("saveSuccess", 12, returnBuffer, returnlen);
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