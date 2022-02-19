/* MIT License

Copyright (c) 2020 Jason C. Fain

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

#include <AsyncJson.h>
#include <list>

AsyncWebSocket ws("/ws");

class WebSocketHandler {
    public: 
        void setup(AsyncWebServer* server) {
            ws.onEvent([&](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
                onWsEvent(server, client, type, arg, data, len);
            });
            server->addHandler(&ws);
        }

        void sendCommand(String command, String message = "", AsyncWebSocketClient* client = 0)
        {
            // Serial.print("Sending WS command: ");
            // Serial.print(command);
            // Serial.print(", Message: ");
            // Serial.println(message);
            String commandJson;
            if(message.isEmpty())
                commandJson = "{ \"command\": \""+command+"\" }";
            else if(message.startsWith("{"))
                commandJson = "{ \"command\": \""+command+"\", \"message\": "+message+" }";
            else
                commandJson = "{ \"command\": \""+command+"\", \"message\": \""+message+"\" }";
            if(client)
                client->printf(commandJson.c_str());
            else
                for (AsyncWebSocketClient *pClient : m_clients)
                    pClient->printf(commandJson.c_str());
        }

        String getTCode() 
        {
            if(!m_tcodeCommands.empty()) 
            {
                String tcode = m_tcodeCommands.front();
                m_tcodeCommands.erase(m_tcodeCommands.begin());
                // if(!tcode.equals(temp)) {
                //     Serial.println("Top tcode: "+tcode);
                //     temp = tcode;
                // }
                return tcode;
            }
            return "";
        }

    private:
        std::list<AsyncWebSocketClient *> m_clients;
        std::list<String> m_tcodeCommands;

        String temp;
        void processWebSocketTextMessage(String msg) 
        {
            if(msg.indexOf("{") == -1) 
            {
                m_tcodeCommands.push_back(msg);
                if(msg.equals("D1")) 
                {
                    sendCommand(SettingsHandler::TCodeVersionName);
                }
            }
            else
            {
                DynamicJsonDocument doc(255);
                DeserializationError error = deserializeJson(doc, msg);
                if (error) 
                {
                    Serial.println(F("Failed to read websocket json"));
                    return;
                }
                JsonObject jsonObj = doc.as<JsonObject>();

                if(jsonObj["command"] == "tcode") 
                {
                    String message = jsonObj["message"];
                    //Serial.println("Recieved websocket tcode message: " + message);
                    m_tcodeCommands.push_back(message);
                }
            }
        }

        void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
        {
            if(type == WS_EVT_CONNECT)
            {
                // Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
                // client->printf("Hello Client %u :)", client->id());
                // client->ping();
                m_clients.push_back(client);
            } 
            else if(type == WS_EVT_DISCONNECT)
            {
                Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
                m_clients.remove(client);
            } 
            else if(type == WS_EVT_ERROR)
            {
                Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
            } 
            else if(type == WS_EVT_PONG)
            {
                Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
            } 
            else if(type == WS_EVT_DATA)
            {
                AwsFrameInfo * info = (AwsFrameInfo*)arg;
                String msg = "";
                if(info->final && info->index == 0 && info->len == len)
                {
                    //the whole message is in a single frame and we got all of it's data
                    Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

                    if(info->opcode == WS_TEXT)
                    {
                        for(size_t i=0; i < info->len; i++) 
                        {
                            msg += (char) data[i];
                        }
                    } 
                    else 
                    {
                        char buff[3];
                        for(size_t i=0; i < info->len; i++) 
                        {
                            sprintf(buff, "%02x ", (uint8_t) data[i]);
                            msg += buff ;
                        }
                        if(msg.equals("tcode")) {

                        }
                    }
                    Serial.printf("%s\n",msg.c_str());

                    if(info->opcode == WS_TEXT)
                        processWebSocketTextMessage(msg);
                    else
                        client->binary("I got your binary message");
                } 
                else 
                {
                //message is comprised of multiple frames or the frame is split into multiple packets
                    if(info->index == 0)
                    {
                        if(info->num == 0)
                            Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
                        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
                    }

                    Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

                    if(info->opcode == WS_TEXT)
                    {
                        for(size_t i=0; i < len; i++) 
                        {
                            msg += (char) data[i];
                        }
                    } 
                    else 
                    {
                        char buff[3];
                        for(size_t i=0; i < len; i++) 
                        {
                            sprintf(buff, "%02x ", (uint8_t) data[i]);
                            msg += buff ;
                        }
                    }
                    Serial.printf("%s\n",msg.c_str());

                    if((info->index + len) == info->len)
                    {
                        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
                        if(info->final)
                        {
                            Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
                            if(info->message_opcode == WS_TEXT)
                                processWebSocketTextMessage(msg);
                            else
                                client->binary("I got your binary message");
                        }
                    }
                }
            }
        }
};