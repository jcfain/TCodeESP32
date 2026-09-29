#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <string>
// #include "LogHandler.h"
#include "sensors/BatteryHandler.h"
#include "TCode/MotorHandler.h"
#include "PowerHandler.h"
#include "settings/SettingsHandler.h"
#include "messages/SystemCommandHandler.h"

class WebSocketBase {
    public:
    virtual void CommandCallback(const char* in) = 0;
    virtual void sendCommand(const char* command, const char* message = 0) = 0;
    virtual void closeAll() = 0;

    void getTCode(char* webSocketData)
    {
        if(!tCodeInQueue || tCodeInQueue == NULL)
        {
            if(millis() >= lastMessage + messageLimit) {
                lastMessage = millis();
                LogHandler::error(Tags::WebSocketServer, "TCode queue was null");
            }
            return;
        }
        if (xQueueReceive(tCodeInQueue, webSocketData, 0))
        {
            //tcode->toCharArray(webSocketData, tcode->length() + 1);
            // Serial.print("Top tcode: ");
            // Serial.println(webSocketData);
        }
        else
        {
            webSocketData[0] = {0};
        }
    }

protected:
    bool isInitialized = false;
    QueueHandle_t tCodeInQueue;
    std::mutex command_mtx;

    // Builds the { "command": ..., "message": ... } envelope sent to the web UI.
    // A message that is itself a JSON object/array is embedded as-is; anything
    // else is sent as a JSON string and escaped, so log lines containing quotes,
    // backslashes or newlines still produce valid JSON on the client.
    std::string compileCommand(const char* command, const char* message = 0) {
        std::string json;
        if (!command) {
            return json;
        }
        #ifdef DEBUG_WS_COMPILER
        if(LogHandler::getLogLevel() == LogLevel::DEBUG) {
            if(message)
                Serial.printf("Sending WS commands: %s, Message: %s\n", command, message);
            else
                Serial.printf("Sending WS commands: %s\n",command);
        }
        #endif
        json.reserve(strlen(command) + (message ? strlen(message) : 0) + 32);
        json += "{ \"command\": \"";
        appendJsonEscaped(json, command);
        json += "\"";
        if (message) {
            json += " , \"message\": ";
            if (isJsonValue(message)) {
                json += message;
            } else {
                json += "\"";
                appendJsonEscaped(json, message);
                json += "\"";
            }
        }
        json += " }";
        return json;
    }

    static bool isJsonValue(const char* message) {
        while (*message == ' ' || *message == '\t' || *message == '\r' || *message == '\n')
            message++;
        return *message == '{' || *message == '[';
    }

    // https://stackoverflow.com/questions/7724448/simple-json-string-escape-for-c
    static void appendJsonEscaped(std::string& out, const char* s) {
        for (; *s; ++s) {
            const char c = *s;
            switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char hex[7];
                    snprintf(hex, sizeof(hex), "\\u%04x", static_cast<unsigned char>(c));
                    out += hex;
                } else {
                    out += c;
                }
            }
        }
    }
    void processWebSocketTextMessage(const char* msg)
    {
        if (strpbrk(msg, "{") == nullptr)
        {
            extern void feedMotorCommand(const char* cmd, size_t len);
            if (m_commandHandler.isCommand(msg))
            {
                // $ and # system commands typed into the web UI terminal.
                // Output is reported through LogHandler::raw.
                char command[MAX_COMMAND];
                strlcpy(command, msg, sizeof(command));
                size_t len = strlen(command);
                while (len > 0 && (command[len - 1] == '\n' || command[len - 1] == '\r'))
                    command[--len] = '\0';
                LogHandler::debug(Tags::WebSocketServer, "Websocket system command in: %s", command);
                m_commandHandler.process(command);
                // Some system commands (e.g. #device-home) queue TCode for the motor.
                char tcode[MAX_COMMAND];
                while (m_commandHandler.getTCode(tcode))
                {
                    size_t tcodeLen = strlen(tcode);
                    if (tcodeLen > 0)
                        feedMotorCommand(tcode, tcodeLen);
                }
                return;
            }
            LogHandler::verbose(Tags::WebSocketServer, "Websocket tcode in: %s", msg);
            feedMotorCommand(msg, strlen(msg));
        }
        else
        {
            JsonDocument doc; //255
            DeserializationError error = deserializeJson(doc, msg);
            if (error)
            {
                LogHandler::error(Tags::WebSocketServer, "Failed to read websocket json");
                return;
            }
            JsonObject jsonObj = doc.as<JsonObject>();

            if (!jsonObj["command"].isNull())
            {
                String command = jsonObj["command"].as<String>();
                String message = jsonObj["message"].as<String>();
                if(command == "setBatteryFull") {
                    BatteryHandler::setBatteryToFull();
                }
                else if (command == "identifyServo") {
                    extern MotorHandler* motorHandler;
                    extern PowerHandler powerHandler;
                    LogHandler::debug(Tags::WebSocketServer, "identifyServo command received: '%s' (motorHandler=%p)", message.c_str(), motorHandler);
                    bool shouldRestoreServoPower = false;
                    if (!powerHandler.isServoVoltageEnabled()) {
                        powerHandler.setServoVoltageEnabled(true);
                        shouldRestoreServoPower = true;
                    }

                    if (motorHandler)
                        motorHandler->identifyServo(message.c_str());
                    else
                        LogHandler::error(Tags::WebSocketServer, "identifyServo: motorHandler is null");

                    if (shouldRestoreServoPower) {
                        struct RestoreServoPowerParams {
                            PowerHandler* power;
                        };
                        auto* params = new RestoreServoPowerParams{ &powerHandler };
                        xTaskCreate([](void* arg) {
                            auto* p = static_cast<RestoreServoPowerParams*>(arg);
                            vTaskDelay(pdMS_TO_TICKS(2600));
                            p->power->setServoVoltageEnabled(false);
                            delete p;
                            vTaskDelete(nullptr);
                            }, "idPwrR", 2048, params, 1, nullptr);
                    }
                }
                else if (command == "setServoVoltageEnabled") {
                    extern PowerHandler powerHandler;
                    bool enabled = (message == "true");
                    powerHandler.setServoVoltageEnabled(enabled);
                    // Persist the state to settings
                    SettingsFactory* settingsFactory = SettingsFactory::getInstance();
                    if (settingsFactory) {
                        settingsFactory->setValue(SERVO_VOLTAGE_ENABLE_STATE, enabled);
                    }
                }
                // String* message = jsonObj["message"];
                // Serial.print("Recieved websocket tcode message: ");
                // Serial.println(message->c_str());
                // if(tCodeInQueue == NULL)return;
                // xQueueSend(tCodeInQueue, &message, 0);
            }
            // else
            // {
            //     LogHandler::verbose(Tags::WebSocketServer, "Websocket tcode in JSON: %s", msg);
            //     char tcode[MAX_COMMAND];
            //     SettingsHandler::processTCodeJson(tcode, msg);
            //     // Serial.print("tcode JSON converted:");
            //     // Serial.println(tcode);
            //     xQueueSend(tCodeInQueue, tcode, 0);
            // }
        }
    }

private:
    SystemCommandHandler m_commandHandler;
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