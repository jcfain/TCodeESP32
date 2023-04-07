/* MIT License

Copyright (c) 2023 Jason C. Fain

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
#include <ESPAsyncWebserver.h>
#include <AsyncTCP.h>
#include <AsyncJson.h>
#include <ESPmDNS.h>
#include "WifiHandler.h"
#include "WebSocketHandler.h"
#include "TagHandler.h"

AsyncWebServer* server;
class WebHandler {
    public:
        bool initialized = false;
        bool MDNSInitialized = false;
        void setup(int port, char* hostName, char* friendlyName, WebSocketHandler* webSocketHandler, bool apMode = false) {
            stop();
            if (port < 1) 
                port = 80;
		    LogHandler::info(_TAG, "Starting web server on port: %i", port);
            server = new AsyncWebServer(port);
            webSocketHandler->setup(server);

            if(!apMode)
            {
                startMDNS(hostName, friendlyName);
            }

            server->on("/userSettings", HTTP_GET, [](AsyncWebServerRequest *request) 
            {
                ////request->send(SPIFFS, "/userSettings.json");
                // DynamicJsonDocument doc(SettingsHandler::deserialize);
                // File file = SPIFFS.open(SettingsHandler::userSettingsFilePath, "r");
                // DeserializationError error = deserializeJson(doc, file);
                char settings[3072];
                SettingsHandler::serialize(settings);
                if (strlen(settings) == 0) {
                    AsyncWebServerResponse *response = request->beginResponse(504, "application/text", "Error getting user settings");
                    request->send(response);
                    return;
                }
                // if(strcmp(doc["wifiPass"], SettingsHandler::defaultWifiPass) != 0 )
                //     doc["wifiPass"] = "Too bad haxor!";// Do not send password if its not default
                    
                // doc["lastRebootReason"] = SettingsHandler::lastRebootReason;
                // String output;
                // serializeJson(doc, output);
                AsyncWebServerResponse *response = request->beginResponse(200, "application/json", settings);
                request->send(response);
            });   
            server->on("/systemInfo", HTTP_GET, [](AsyncWebServerRequest *request) 
            {
                char systemInfo[1024];
                SettingsHandler::getSystemInfo(systemInfo);
                if (strlen(systemInfo) == 0) {
                    AsyncWebServerResponse *response = request->beginResponse(504, "application/text", "Error getting user settings");
                    request->send(response);
                    return;
                }
                AsyncWebServerResponse *response = request->beginResponse(200, "application/json", systemInfo);
                request->send(response);
            });   
            
            server->on("/log", HTTP_GET, [](AsyncWebServerRequest *request) 
            {
                Serial.println("Get log...");
                request->send(SPIFFS, SettingsHandler::logPath);
            });   

            server->on("/connectWifi", HTTP_POST, [](AsyncWebServerRequest *request) 
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

            server->on("/toggleContinousTwist", HTTP_POST, [](AsyncWebServerRequest *request) 
            {
				SettingsHandler::continuousTwist = !SettingsHandler::continuousTwist;
				if (SettingsHandler::save()) 
				{
					char returnJson[45];
					sprintf(returnJson, "{\"msg\":\"done\", \"continousTwist\":%s }", SettingsHandler::continuousTwist ? "true" : "false");
					AsyncWebServerResponse *response = request->beginResponse(200, "application/json", returnJson);
					request->send(response);
				} 
				else 
				{
					AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"Error saving settings\"}");
					request->send(response);
				}
            });

            server->on("^\\/sensor\\/([0-9]+)$", HTTP_GET, [] (AsyncWebServerRequest *request) 
            {
                String sensorId = request->pathArg(0);
            });

            // upload a file to /upload
            // server->on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
            //     request->send(200);
            // }, handleUpload);server->on("/reset", HTTP_POST, [](AsyncWebServerRequest *request){

            server->on("/restart", HTTP_POST, [webSocketHandler, apMode](AsyncWebServerRequest *request)
            {
                //if(apMode) {
                    //request->send(200, "text/plain",String("Restarting device, wait about 10-20 seconds and navigate to ") + (SettingsHandler::hostname) + ".local or the network IP address in your browser address bar.");
                //}
                String message = "{\"msg\":\"restarting\",\"apMode\":";
                message += apMode ? "true}" : "false}";
                AsyncWebServerResponse *response = request->beginResponse(200, "application/json", message);
                request->send(response);
                delay(2000);
                webSocketHandler->closeAll();
                SystemCommandHandler::restart();
            });

            server->on("/default", HTTP_POST, [](AsyncWebServerRequest *request)
            {
                Serial.println("Settings default");
				SettingsHandler::defaultAll();
            });

            AsyncCallbackJsonWebHandler* settingsUpdateHandler = new AsyncCallbackJsonWebHandler("/settings", [](AsyncWebServerRequest *request, JsonVariant &json)
			{
                Serial.println("API save settings...");
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
            }, 2400U );//Bad request? increase the size.

            // //To upload through terminal you can use: curl -F "image=@firmware.bin" esp8266-webupdate.local/update
            // server->on("/update", HTTP_POST, [this](AsyncWebServerRequest *request){
            //         // the request handler is triggered after the upload has finished... 
            //         // create the response, add header, and send response
            //         AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", (Update.hasError())?"FAIL":"OK");
            //         response->addHeader("Connection", "close");
            //         response->addHeader("Access-Control-Allow-Origin", "*");
            //         SettingsHandler::restartRequired = true;
            //         request->send(response);
            //     },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            //         //Upload handler chunks in data
                    
            //         if(!index){ // if index == 0 then this is the first frame of data
            //         Serial.printf("UploadStart: %s\n", filename.c_str());
            //         Serial.setDebugOutput(true);
                    
            //         // calculate sketch space required for the update
            //         uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
            //         if(!Update.begin(maxSketchSpace)){//start with max available size
            //             Update.printError(Serial);
            //         }
            //             Update.runAsync(true); // tell the updaterClass to run in async mode
            //     }

            //     //Write chunked data to the free sketch space
            //     if(Update.write(data, len) != len){
            //         Update.printError(Serial);
            //     }
                
            //     if(final){ // if the final flag is set then this is the last frame of data
            //     if(Update.end(true)){ //true to set the size to the current progress
            //         Serial.printf("Update Success: %u B\nRebooting...\n", index+len);
            //         } else {
            //         Update.printError(Serial);
            //         }
            //         Serial.setDebugOutput(false);
            //     }
            // });

            server->addHandler(settingsUpdateHandler);
            
            server->onNotFound([](AsyncWebServerRequest *request) 
			{
                Serial.println("Not found...");
                if (request->method() == HTTP_OPTIONS) {
                    request->send(200);
                } else {
                    request->send(404);
                }
            });

            //server->rewrite("/", "/wifiSettings.htm").setFilter(ON_AP_FILTER);
            server->serveStatic("/", SPIFFS, "/www/").setDefaultFile("index-min.html");;
            server->begin();
            initialized = true;
        }
        void stop() {
            if(initialized) 
            {
                initialized = false;
                server->end();
            }
            if(MDNSInitialized)
            {
                 MDNS.end();
                 MDNSInitialized = false;
            }
        }

    private:
        const char* _TAG = TagHandler::WebHandler;
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
};
