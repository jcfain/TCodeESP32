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

#include <Arduino.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <AsyncJson.h>
#include <ESPmDNS.h>
#include "SettingsHandler.h"

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
        void setup(int port, char* hostName, char* friendlyName) {
            // Changing the port at runtime crashes the ESP32.
            // if (port == 0) {
            //     port = 80;
            // } 
            // Serial.print("Setting up web server on port: ");
            // Serial.println(port);
            // server = AsyncWebServer(port);

            // This has issues running with the webserver.
            //startMDNS(hostName, friendlyName);

            //Route for root / web page
            server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(SPIFFS, "/www/index.htm", "text/html");
            });

            server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
                request->send(SPIFFS, "/www/settings.js", "text/javascript");
            });
            server.on("/userSettings", HTTP_GET, [](AsyncWebServerRequest *request) {
                request->send(SPIFFS, "/userSettings.json", "text/javascript");
            });

            server.on("/style", HTTP_GET, [](AsyncWebServerRequest *request) {
                request->send(SPIFFS, "/www/style.css", "text/css");
            });

            server.on("/systemInfo", HTTP_GET, [](AsyncWebServerRequest *request) {
                // AsyncResponseStream *response = request->beginResponseStream("application/json");
                // DynamicJsonBuffer jsonBuffer;
                // JsonObject &root = jsonBuffer.createObject();
                // root["ssid"] = WiFi.SSID();
                // root["freeHeap"] = String(ESP.getFreeHeap())
                // root.printTo(*response);
                // request->send(response);
            });

            server.on("^\\/sensor\\/([0-9]+)$", HTTP_GET, [] (AsyncWebServerRequest *request) {
                String sensorId = request->pathArg(0);
            });

            // upload a file to /upload
            // server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
            //     request->send(200);
            // }, handleUpload);server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request){

            server.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request){
                request->send(200, "text/plain","Restarting device, wait about 10 seconds and navigate to the configuration page again.");
                delay(2000);
                ESP.restart();
            });

            AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/settings", [](AsyncWebServerRequest *request, JsonVariant &json) {
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
            
            server.onNotFound([](AsyncWebServerRequest *request) {
                if (request->method() == HTTP_OPTIONS) {
                    request->send(200);
                } else {
                    request->send(404);
                }
            });
            
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
