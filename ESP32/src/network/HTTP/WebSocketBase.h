#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
// #include "LogHandler.h"
#include "sensors/BatteryHandler.h"
#include "TCode/MotorHandler.h"
#include "PowerHandler.h"
#include "settings/SettingsHandler.h"

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

    void compileCommand(char* buf, size_t bufSize, const char* command, const char* message = 0) {
        if (!buf || bufSize == 0 || !command) {
            return;
        }
        if(LogHandler::getLogLevel() == LogLevel::DEBUG) {
            if(message)
                Serial.printf("Sending WS commands: %s, Message: %s\n", command, message);
            else
                Serial.printf("Sending WS commands: %s\n",command);
        }
        int written = 0;
        if(!message)
            written = snprintf(buf, bufSize, "{ \"command\": \"%s\" }", command);
        else if(strpbrk(message, "{") != nullptr)
            written = snprintf(buf, bufSize, "{ \"command\": \"%s\" , \"message\": %s }", command, message);
        else
            written = snprintf(buf, bufSize, "{ \"command\": \"%s\" , \"message\": \"%s\" }", command, message);

        if (written < 0 || static_cast<size_t>(written) >= bufSize) {
            LogHandler::warning(Tags::WebSocketServer, "WebSocket payload truncated for command '%s'", command);
            buf[bufSize - 1] = '\0';
        }
    }
    void processWebSocketTextMessage(const char* msg)
    {
        if (strpbrk(msg, "{") == nullptr)
        {
            LogHandler::verbose(Tags::WebSocketServer, "Websocket tcode in: %s", msg);
            extern void feedMotorCommand(const char* cmd, size_t len);
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