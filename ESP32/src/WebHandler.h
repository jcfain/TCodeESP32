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

AsyncWebServer server(80);

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

    public:
        void setup(int port, char* hostName, char* friendlyName, bool apMode = false) {
            // Changing the port at runtime crashes the ESP32.
            // if (port == 0) {
            //     port = 80;
            // } 
            // Serial.print("Setting up web server on port: ");
            // Serial.println(port);
            // server = AsyncWebServer(port);

            // This has issues running with the webserver.
            //startMDNS(hostName, friendlyName);

            // Route for root / web page
            // if (apMode) 
            // {
            //     server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            //     {
            //         request->send(SPIFFS, "/www/wifiSettings.htm");
            //     });
            // }
            // else 
            // {
            //     server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            //     {
            //         request->send(SPIFFS, "/www/index.htm");
            //     });
            // }

            // server.on("/wifiSettings", HTTP_GET, [](AsyncWebServerRequest *request)
            // {
            //     request->send(SPIFFS, "/www/wifiSettings.htm");
            // });

            // server.on("/jquery", HTTP_GET, [](AsyncWebServerRequest *request)
            // {
            //     request->send(SPIFFS, "/www/jquery-3.5.1.min.js");
            // });

            // server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) 
            // {
            //     request->send(SPIFFS, "/www/settings.js");
            // });

            // server.on("/style", HTTP_GET, [](AsyncWebServerRequest *request) 
            // {
            //     request->send(SPIFFS, "/www/style.css", "text/css");
            // });

            server.on("/userSettings", HTTP_GET, [](AsyncWebServerRequest *request) 
            {
                request->send(SPIFFS, "/userSettings.json");
            });   

            server.on("/jquery", HTTP_GET, [](AsyncWebServerRequest* request) 
			{
                AsyncWebServerResponse* response = request->beginResponse(SPIFFS, "/www/js/jquery-3.5.1.min.js.gz", "text/javascript");
                response->addHeader("Content-Encoding", "gzip");
                request->send(response);
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
                request->send(200, "text/plain","Restarting device, wait about 10-20 seconds and navigate to the network IP address in your browser address bar.");
                delay(2000);
                Serial.println("Device restarting...");
                ESP.restart();
                delay(5000);
            });

            AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/settings", [](AsyncWebServerRequest *request, JsonVariant &json) 
			{
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
            });

            server.addHandler(handler);
            
            server.onNotFound([](AsyncWebServerRequest *request) 
			{
                if (request->method() == HTTP_OPTIONS) {
                    request->send(200);
                } else {
                    request->send(404);
                }
            });

            server.rewrite("/", "/wifiSettings.htm").setFilter(ON_AP_FILTER);
            //server.serveStatic("/js/", SPIFFS, "/www/js/").setCacheControl("max-age=31536000");
            server.serveStatic("/", SPIFFS, "/www/");
            // server.serveStatic("/wifiSettings", SPIFFS, "/www/wifiSettings.htm");
            // server.serveStatic("/style", SPIFFS, "/www/style.css");
            // server.serveStatic("/userSettings", SPIFFS, "/userSettings.json");
            // server.serveStatic("/settings", SPIFFS, "/www/settings.js");
            // server.serveStatic("/jquery", SPIFFS, "/www/jquery-3.5.1.min.js");
            server.begin();
        }

        private:
            void startMDNS(char* hostName, char* friendlyName)
            {
                Serial.print("hostName: ");
                Serial.println(hostName);
                Serial.print("friendlyName: ");
                Serial.println(friendlyName);
                //initialize mDNS service
                esp_err_t err = mdns_init();
                if (err) {
                    printf("MDNS Init failed: %d\n", err);
                    return;
                }

                //set hostname
                mdns_hostname_set(hostName);
                //set default instance
                mdns_instance_name_set(friendlyName);

                mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
                mdns_service_add(NULL, "_tcodeRemote", "_udp", SettingsHandler::udpServerPort, NULL, 0);
            }
};
