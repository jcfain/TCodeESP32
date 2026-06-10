/* MIT License

Copyright (c) 2026 Jason C. Fain

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

// #include <AsyncJson.h>
#include <AsyncWebSocket.h>
#include <mutex>
#include <list>
#include "HTTP/WebSocketBase.h"
#include "SettingsHandler.h"
// #include "LogHandler.h"
#include "TagHandler.h"
#include "BatteryHandler.h"
#include "TaskHandler.hpp"

AsyncWebSocket ws("/ws");

struct WebSocketCommand {
    const char* command;
    const char* message;
};

class WebSocketHandler : public WebSocketBase {
public: 
    void setup(AsyncWebServer* server) 
    {
        LogHandler::info(_TAG, "Setting up webSocket");
        ws.onEvent([&](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
            onWsEvent(server, client, type, arg, data, len);
        });
        server->addHandler(&ws);
        m_TCodeQueue = xQueueCreate(25, sizeof(char[MAX_COMMAND]));
        if(m_TCodeQueue == NULL) {
            LogHandler::error(_TAG, "Error creating the tcode queue");
        }
        if(SettingsFactory::getInstance()->getWebsocketLoggingEnabled())
            TaskHandler::getInstance()->startWebsocketLogging(this);
        m_xMutex = xSemaphoreCreateMutex();
        isInitialized = true;
    }
    
    void send(const char* in) override 
    {
        if(isInitialized && ws.count()) 
        {
            // size_t len = strlen(in);
            // if(len > MAX_WS_MESSAGE)
            // {
            //     LogHandler::error(_TAG, "[WebSocketHandler] [send] Return value was too long for buffer: %s", in);
            //     return;
            // }
            // char messageEscaped[MAX_WS_MESSAGE];
            // size_t lenEscaped = escape_json(messageEscaped, in, len);
            // ws.textAll(messageEscaped, lenEscaped);
            sendCommand("tcode", 6, in, strlen(in));
        }
    }

    void sendCommand(const char* command, size_t commandLen, const char* message = 0, size_t messageLen = 0) override
    {
        if(isInitialized && ws.count()) 
        {
            xSemaphoreTake(m_xMutex, portMAX_DELAY);
            m_lastSend = millis();

            char commandJson[commandLen + messageLen + COMMAND_PADDING];
            size_t len = compileCommand(commandJson, command, commandLen, message, messageLen);
            if(!len)
                return;
            ws.textAll(commandJson, len);
            xSemaphoreGive(m_xMutex);
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
    //     if(m_TCodeQueue == NULL)
    //     {
    //         LogHandler::error(_TAG, "TCode queue was null");
    //         return;
    //     } 
    // 	if(xQueueReceive(m_TCodeQueue, webSocketData, 0)) 
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

    void sendLogTask(void *webSocketHandler) override
    {
        sendLogTaskRunning = true;
        Serial.printf("[sendLogTask]: init\n");
        // char lastMessage[LogHandler::internal_buffer_length];
        LogMessage logMessage;
        while (sendLogTaskRunning) 
        {
            if(ws.count() > 0) 
            {
                if(LogHandler::getLog(&logMessage)) 
                {
                    // strncpy(lastMessage, logMessage.message, LogHandler::internal_buffer_length);
                    const char* level = "info";
                    switch(logMessage.level)
                    {
                        case LogLevel::INFO:
                            level = "info";
                        case LogLevel::DEBUG:
                            level = "debug";
                        break;
                        case LogLevel::WARNING:
                            level = "warn";
                        break;
                        case LogLevel::ERROR:
                            level = "error";
                        break;
                        case LogLevel::VERBOSE:
                            level = "verbose";
                        break;
                        default:
                            level = "log";
                        break;
                    }
                    // multiSeq = logMessage.multipart;
                    // Serial.printf("sending log: level: %s message: %s, multi: %i\n", level, logMessage.message, multiSeq);
                    ((WebSocketHandler*)webSocketHandler)->sendCommand(level, strlen(level), logMessage.message, logMessage.len);
                }
            }
            vTaskDelay(100/portTICK_PERIOD_MS);
        }
        Serial.printf("[sendLogTask]: end\n");
        vTaskDelete(NULL);
    }

    void closeAll() override
    {
        for (AsyncWebSocketClient *pClient : m_clients)
            pClient->close();
    }

private:
    bool isInitialized = false;
    // std::mutex command_mtx;
    const char* _TAG = TagHandler::WebsocketsHandler;
// unsigned long lastCall;
    std::list<AsyncWebSocketClient *> m_clients;
    // QueueHandle_t m_TCodeQueue;
    // static QueueHandle_t debugInQueue;
    // std::mutex serial_mtx;

    std::mutex serial_mtx;
    int m_lastSend;

    void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
    {
        if(type == WS_EVT_CONNECT)
        {
            LogHandler::debug(_TAG, "ws[%s][%u] connect", server->url(), client->id());
            //client->printf("Hello Client %u :)", client->id());
            // client->ping();
            // client->client()->setNoDelay(true);
            m_clients.push_back(client);
        } 
        else if(type == WS_EVT_DISCONNECT)
        {
            LogHandler::debug(_TAG, "ws[%s][%u] disconnect", server->url(), client->id());
            m_clients.remove(client);
        } 
        else if(type == WS_EVT_ERROR)
        {
            LogHandler::debug(_TAG, "ws[%s][%u] error(%u): %s", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
        } 
        else if(type == WS_EVT_PONG)
        {
            LogHandler::debug(_TAG, "ws[%s][%u] pong[%u]: %s", server->url(), client->id(), len, (len)?(char*)data:"");
        } 
        else if(type == WS_EVT_DATA)
        {
            AwsFrameInfo * info = (AwsFrameInfo*)arg;
            //String msg = "";
            if(info->final && info->index == 0 && info->len == len)
            {
                //the whole message is in a single frame and we got all of it's data
                //Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

                // if(info->opcode == WS_TEXT)
                // {
                //     for(size_t i=0; i < info->len; i++) 
                //     {
                //         msg += (char) data[i];
                //     }
                // } 
                // else 
                // {
                //     char buff[3];
                //     for(size_t i=0; i < info->len; i++) 
                //     {
                //         sprintf(buff, "%02x ", (uint8_t) data[i]);
                //         msg += buff ;
                //     }
                // }
                // Serial.printf("%s\n",msg.c_str());

                if(info->opcode == WS_TEXT) 
                {
                    data[len] = 0;
                    processWebSocketTextMessage((char*) data);
                }
                else
                    client->binary("I got your binary message");
            } 
            else 
            {
            //message is comprised of multiple frames or the frame is split into multiple packets
                // if(info->index == 0)
                // {
                //     if(info->num == 0)
                //         Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
                //     Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
                // }

                // Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

                // if(info->opcode == WS_TEXT)
                // {
                //     for(size_t i=0; i < len; i++) 
                //     {
                //         msg += (char) data[i];
                //     }
                // } 
                // else 
                // {
                //     char buff[3];
                //     for(size_t i=0; i < len; i++) 
                //     {
                //         sprintf(buff, "%02x ", (uint8_t) data[i]);
                //         msg += buff ;
                //     }
                // }
                // Serial.printf("%s\n",msg.c_str());

                if((info->index + len) == info->len)
                {
                    //Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
                    if(info->final)
                    {
                        //Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
                        if(info->message_opcode == WS_TEXT) 
                        {
                            data[len] = 0;
                            processWebSocketTextMessage((char*)data);
                        }
                        else
                            client->binary("I got your binary message");
                    }
                }
            }
        }
    }
};