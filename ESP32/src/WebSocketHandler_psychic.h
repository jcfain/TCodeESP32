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

//#include <AsyncJson.h>
#include <mutex>
#include <list>
#include <PsychicHttp.h>
//#include "HTTP/WebSocketBase.h"
#include "SettingsHandler.h"
// #include "LogHandler.h"
#include "TagHandler.h"
#include "BatteryHandler.h"

PsychicWebSocketHandler ws;

struct WebSocketCommand {
    const char* command;
    const char* message;
};
//("/ws")
class WebSocketHandler : public WebSocketBase {
    public: 
        void setup(PsychicHttpServer* server) {
            LogHandler::info(_TAG, "Setting up webSocket");
            // ws.onEvent([&](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
            //     onWsEvent(server, client, type, arg, data, len);
            // });
            tCodeInQueue = xQueueCreate(5, sizeof(char[MAX_COMMAND]));
            if(tCodeInQueue == NULL) {
                LogHandler::error(_TAG, "Error creating the tcode queue");
            }
            ws.onOpen([this](PsychicWebSocketClient *client) {
                LogHandler::info(_TAG, "Connection #%u connected from %s\n", client->socket(), client->remoteIP().toString().c_str());
                client->sendMessage("Hello!");
                m_clients.push_back(client);
            });
            ws.onFrame([this](PsychicWebSocketRequest *request, httpd_ws_frame *frame) {
                LogHandler::info(_TAG, "[socket] #%d sent: %s\n", request->client()->socket(), (char *)frame->payload);
                processWebSocketTextMessage((char *)frame->payload);
                return request->reply(frame);

            });
            ws.onClose([this](PsychicWebSocketClient *client) {
                LogHandler::info(_TAG, "Connection #%u closed from %s\n", client->socket(), client->remoteIP().toString().c_str());
                m_clients.remove(client);
            });
            server->on("/ws")->setHandler(&ws);
            // debugInQueue = xQueueCreate(10, sizeof(char[MAX_COMMAND]));
            // if(debugInQueue == NULL) {
            //     LogHandler::error(_TAG, "Error creating the debug queue");
            // }
            // xTaskCreate(this->emptyQueue, "emptyDebugQueue", 2042, this, tskIDLE_PRIORITY, emptyQueueHandle);
            isInitialized = true;
        }
        
        void CommandCallback(const char* in) override { //This overwrites the callback for message return
            if(isInitialized && ws.count() > 0)
                sendCommand(in);
        }

        // void sendDebug(const char* message, LogLevel level) {
            // if (level != LogLevel::VERBOSE && isInitialized && debugInQueue != NULL && uxQueueMessagesWaiting(debugInQueue) < 10 && serial_mtx.try_lock()) {
            //     std::lock_guard<std::mutex> lck(serial_mtx, std::adopt_lock);
            //         // char messageToSend[MAX_COMMAND];
            //         // if(sizeof(message) > 253) {
            //         //     strncpy(messageToSend, message, 253);
            //         //     messageToSend[254] = '\0';
            //         //     Serial.println("truncated");
            //         // } else {
            //         //     strcpy(messageToSend, message);
            //         //     messageToSend[strlen(message)] = '\0';
            //         // }
            //         // if(level >= LogLevel::DEBUG) {
            //         //     Serial.print("insert to q: ");
            //         //     Serial.println(message);
            //         // }
            //         xQueueSend(debugInQueue, message, 0);
            // }
        // }

        void sendCommand(const char* command, const char* message = 0) override
        {
            if(isInitialized && command_mtx.try_lock()) {
                std::lock_guard<std::mutex> lck(command_mtx, std::adopt_lock);
                m_lastSend = millis();

                char commandJson[MAX_COMMAND];
                compileCommand(commandJson, command, message);
                // if(client)
                //     client->text(commandJson);
                // else
                    ws.sendAll(commandJson);
            }
        }

        // // Did not work last I tried it. Gave up.
        // // template <size_t N>
        // // void sendCommands(WebSocketCommand (&commands)[N], AsyncWebSocketClient* client = 0)
        // // {
        // //     if(isInitialized && command_mtx.try_lock()) {
        // //         std::lock_guard<std::mutex> lck(command_mtx, std::adopt_lock);
        // //         m_lastSend = millis();

        // //         char commandsJson[MAX_COMMAND];
        // //         std::strcat(commandsJson, "[");
        // //         for (int i = 0; i < N; i++) 
        // //         {
        // //             if(commands[i].command) {
        // //                 char commandJson[128];
        // //                 compileCommand(commandJson, commands[i].command, commands[i].message);
        // //                 Serial.print("compileCommand: ");
        // //                 Serial.println(commandJson);
        // //                 std::strcat(commandsJson, commandJson);
        // //                 if(i < N-1)
        // //                     std::strcat(commandsJson, ",");
        // //             }
        // //         }
        // //         std::strcat(commandsJson, "]");
        // //         Serial.print("commandsJson: ");
        // //         Serial.println(commandsJson);
        // //         if(client)
        // //             client->text(commandsJson);
        // //         else
        // //             ws.textAll(commandsJson);
        // //     }
        // // }

        // void getTCode(char* webSocketData) 
        // {
        //     if(tCodeInQueue == NULL)
        //     {
        //         LogHandler::error(_TAG, "TCode queue was null");
        //         return;
        //     } 
		// 	if(xQueueReceive(tCodeInQueue, webSocketData, 0)) 
        //     {
        //         //tcode->toCharArray(webSocketData, tcode->length() + 1);
        //         // Serial.print("Top tcode: ");
        //         // Serial.println(webSocketData);
        //     }
        //     else 
        //     {
      	//         webSocketData[0] = {0};
        //     }
        //     ws.cleanupClients();
        // }

        void closeAll() override
        {
            for (PsychicWebSocketClient  *pClient : m_clients)
                pClient->close();
        }

    private:
        bool isInitialized = false;
        // std::mutex serial_mtx;
        // std::mutex command_mtx;
        const char* _TAG = TagHandler::WebsocketsHandler;
// unsigned long lastCall;
        std::list<PsychicWebSocketClient *> m_clients;
        // QueueHandle_t tCodeInQueue;
        // static QueueHandle_t debugInQueue;
        static int m_lastSend;
        // static TaskHandle_t* emptyQueueHandle;
        // static bool emptyQueueRunning;

        // static void emptyQueue(void *webSocketHandler) {
        //     while (true) {
        //         if(ws.count() > 0 && millis() - m_lastSend > 50 && uxQueueMessagesWaiting(debugInQueue)) {
		// 		    char lastMessage[MAX_COMMAND];
        //             if(xQueueReceive(debugInQueue, lastMessage, 0)) {
        //                 if(LogHandler::getLogLevel() == LogLevel::VERBOSE)
        //                     Serial.printf("read from q: %s\n", lastMessage);
        //                 ((WebSocketHandler*)webSocketHandler)->sendCommand("debug", lastMessage);
        //             }
        //         }
        // 	    vTaskDelay(100/portTICK_PERIOD_MS);
        //     }
        //     vTaskDelete(NULL);
        // }
};
// bool WebSocketHandler::emptyQueueRunning = false;
// QueueHandle_t WebSocketHandler::debugInQueue;
int WebSocketHandler::m_lastSend = 0;
// TaskHandle_t* WebSocketHandler::emptyQueueHandle = NULL;