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

#include <Arduino.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <AsyncJson.h>
#include <ESPmDNS.h>
#include "WifiHandler.h"
#include <list>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

class WebHandler {
    private:
        //AsyncWebServer server = AsyncWebServer(80);
        void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
            if(!index){
                Serial.printf("UploadStart: %s\n", filename.c_str());
            }
            for(size_t i=0; i<len; i++){
                Serial.write(data[i]);
            }
            if(final){
                Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index+len);
            }
        }
        std::list<AsyncWebSocketClient *> m_clients;

    public:
        bool initialized = false;
        bool MDNSInitialized = false;
        void setup(int port, char* hostName, char* friendlyName, bool apMode = false) {
            stop();
            // Changing the port at runtime crashes the ESP32.
            // if (port == 0) {
            //     port = 80;
            // } 
            // Serial.print("Setting up web server on port: ");
            // Serial.println(port);
            // server = AsyncWebServer(port);

            if(!apMode)
            {
                startMDNS(hostName, friendlyName);
            }
            // communicate via websocket.
            ws.onEvent([&](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
                onWsEvent(server, client, type, arg, data, len);
            });
            server.addHandler(&ws);

            server.on("/userSettings", HTTP_GET, [](AsyncWebServerRequest *request) 
            {
                request->send(SPIFFS, "/userSettings.json");
            });   

            server.on("/connectWifi", HTTP_POST, [](AsyncWebServerRequest *request) 
            {
                WifiHandler wifi;
                const size_t capacity = JSON_OBJECT_SIZE(2);
                DynamicJsonDocument doc(capacity);
                if (wifi.connect(SettingsHandler::ssid, SettingsHandler::wifiPass)) 
                {

                    doc["connected"] = true;
                    doc["IPAddress"] = wifi.ip().toString();
                }
                else 
                {
                    doc["connected"] = false;
                    doc["IPAddress"] = "0.0.0.0";

                }
                String output;
                serializeJson(doc, output);
                AsyncWebServerResponse *response = request->beginResponse(200, "application/json", output);
                request->send(response);
            });

            server.on("/toggleContinousTwist", HTTP_POST, [](AsyncWebServerRequest *request) 
            {
				SettingsHandler::continousTwist = !SettingsHandler::continousTwist;
				if (SettingsHandler::save()) 
				{
					char returnJson[45];
					sprintf(returnJson, "{\"msg\":\"done\", \"continousTwist\":%s }", SettingsHandler::continousTwist ? "true" : "false");
					AsyncWebServerResponse *response = request->beginResponse(200, "application/json", returnJson);
					request->send(response);
				} 
				else 
				{
					AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"Error saving settings\"}");
					request->send(response);
				}
            });

            server.on("/systemInfo", HTTP_GET, [](AsyncWebServerRequest *request) 
            {
                // AsyncResponseStream *response = request->beginResponseStream("application/json");
                // DynamicJsonBuffer jsonBuffer;
                // JsonObject &root = jsonBuffer.createObject();
                // root["ssid"] = WiFi.SSID();
                // root["freeHeap"] = String(ESP.getFreeHeap())
                // root.printTo(*response);
                // request->send(response);
            });

            server.on("^\\/sensor\\/([0-9]+)$", HTTP_GET, [] (AsyncWebServerRequest *request) 
            {
                String sensorId = request->pathArg(0);
            });

            // upload a file to /upload
            // server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
            //     request->send(200);
            // }, handleUpload);server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request){

            server.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request)
            {
                request->send(200, "text/plain",String("Restarting device, wait about 10-20 seconds and navigate to ") + (SettingsHandler::hostname) + ".local or the network IP address in your browser address bar.");
                delay(2000);
                Serial.println("Device restarting...");
                ESP.restart();
                delay(5000);
            });
            server.on("/default", HTTP_POST, [](AsyncWebServerRequest *request)
            {
                Serial.println("Settings default");
				SettingsHandler::reset();
            });

            AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/settings", [](AsyncWebServerRequest *request, JsonVariant &json)
			{
                Serial.println("Save settings...");
                JsonObject jsonObj = json.as<JsonObject>();
                if(SettingsHandler::update(jsonObj))
                {
                    if (SettingsHandler::save()) 
                    {
                        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"done\"}");
                        request->send(response);
                    } 
                    else 
                    {
                        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"Error saving settings\"}");
                        request->send(response);
                    }
                }
                else
                {
                    AsyncWebServerResponse *response = request->beginResponse(400, "application/json", "{\"msg\":\"Could not parse JSON\"}");
                    request->send(response);
                }
            }, 1500U );//Bad request? increase the size.

            server.addHandler(handler);
            
            server.onNotFound([](AsyncWebServerRequest *request) 
			{
                Serial.println("Not found...");
                if (request->method() == HTTP_OPTIONS) {
                    request->send(200);
                } else {
                    request->send(404);
                }
            });

            //server.rewrite("/", "/wifiSettings.htm").setFilter(ON_AP_FILTER);
            server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index-min.html");;
            server.begin();
            initialized = true;
        }
        void stop() {
            if(initialized) 
            {
                initialized = false;
                server.end();
            }
            if(MDNSInitialized)
            {
                 MDNS.end();
                 MDNSInitialized = false;
            }
        }

        void sendCommand(String command, String message, AsyncWebSocketClient* client = 0)
        {
            Serial.print("Sending WS command: ");
            Serial.print(command);
            Serial.print(", Message: ");
            Serial.println(message);
            String commandJson;
            if(message.isEmpty())
                commandJson = "{ \"command\": \""+command+"\"}";
            else if(message.startsWith("{"))
                commandJson = "{ \"command\": \""+command+"\", \"message\": "+message+" }";
            else
                commandJson = "{ \"command\": \""+command+"\", \"message\": \""+message+"\"}";
            if(client)
                client->printf(commandJson.c_str());
            else
                for (AsyncWebSocketClient *pClient : m_clients)
                    pClient->printf(commandJson.c_str());
        }

        private:
            void startMDNS(char* hostName, char* friendlyName)
            {
                if(MDNSInitialized)
                    MDNS.end();
                Serial.print("hostName: ");
                Serial.println(hostName);
                Serial.print("friendlyName: ");
                Serial.println(friendlyName);
                if (!MDNS.begin(hostName)) {
                    printf("MDNS Init failed");
                    return;
                }
                MDNS.setInstanceName(friendlyName);
                MDNS.addService("http", "tcp", 80);
                MDNS.addService("tcode", "udp", SettingsHandler::udpServerPort);
                MDNSInitialized = true;
            }
            
            void processWebSocketTextMessage(String msg) 
            {
                if(msg.equals("tcode")) {

                }
            }

            void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
            {
                if(type == WS_EVT_CONNECT)
                {
                    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
                    client->printf("Hello Client %u :)", client->id());
                    client->ping();
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
