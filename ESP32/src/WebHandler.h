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

#include <Arduino.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#if ESP8266 == 1
#include <ESPAsyncTCP.h>
#else
#include <AsyncTCP.h>
#endif
#include <AsyncJson.h>
#include "HTTP/HTTPBase.h"
#include "WifiHandler.h"
#include "WebSocketHandler.h"
#include "TagHandler.h"
#include "SystemCommandHandler.h"
// #if !CONFIG_HTTPD_WS_SUPPORT
// #error This example cannot be used unless HTTPD_WS_SUPPORT is enabled in esp-http-server component configuration
// #endif
class WebHandler : public HTTPBase {
    public:
        // bool MDNSInitialized = false;
        void setup(int port, WebSocketBase* webSocketHandler, bool apMode) override {
            stop();
            if (port < 1) 
                port = 80;
		    LogHandler::info(_TAG, "Starting web server on port: %i", port);
            server = new AsyncWebServer(port);
            ((WebSocketHandler*)webSocketHandler)->setup(server);
            m_settingsFactory = SettingsFactory::getInstance();
            server->on("/wifiSettings", HTTP_GET, [](AsyncWebServerRequest *request) 
            {
                char info[100];
                SettingsHandler::getWifiInfo(info);
                if (strlen(info) == 0) {
                    AsyncWebServerResponse *response = request->beginResponse(504, "application/text", "Error getting wifi settings");
                    request->send(response);
                    return;
                }
                AsyncWebServerResponse *response = request->beginResponse(200, "application/json", info);
                request->send(response);
            });  

            server->on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) 
            {
                request->send(LittleFS, COMMON_SETTINGS_PATH, "application/json");
            });

            server->on("/pins", HTTP_GET, [](AsyncWebServerRequest *request) 
            {
                request->send(LittleFS, PIN_SETTINGS_PATH, "application/json");
            }); 

            server->on("/systemInfo", HTTP_GET, [](AsyncWebServerRequest *request) 
            {
                String systemInfo;
                SettingsHandler::getSystemInfo(systemInfo);
                if (!systemInfo.length()) {
                    AsyncWebServerResponse *response = request->beginResponse(504, "application/text", "Error getting user settings");
                    request->send(response);
                    return;
                }
                AsyncWebServerResponse *response = request->beginResponse(200, "application/json", systemInfo.c_str());
                request->send(response);
            }); 

            server->on("/motionProfiles", HTTP_GET, [](AsyncWebServerRequest *request) 
            {
                request->send(LittleFS, MOTION_PROFILE_SETTINGS_PATH, "application/json");
            });   

            server->on("/buttonSettings", HTTP_GET, [](AsyncWebServerRequest *request) 
            {
                request->send(LittleFS, BUTTON_SETTINGS_PATH, "application/json");
            });  
            
            server->on("/log", HTTP_GET, [](AsyncWebServerRequest *request) 
            {
                Serial.println("Get log...");
                request->send(LittleFS, LOG_PATH);
            });   

            server->on("/connectWifi", HTTP_POST, [this](AsyncWebServerRequest *request) 
            {
                WifiHandler wifi;
                //const size_t capacity = JSON_OBJECT_SIZE(2);
                JsonDocument doc;
                char ssid[SSID_LEN] = {0};
                char pass[WIFI_PASS_LEN] = {0};
                m_settingsFactory->getValue(SSID_SETTING, ssid, SSID_LEN);
                m_settingsFactory->getValue(WIFI_PASS_SETTING, pass, WIFI_PASS_LEN);
                if (wifi.connect(ssid, pass)) 
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

            server->on("/toggleContinousTwist", HTTP_POST, [this](AsyncWebServerRequest *request) 
            {
				m_settingsFactory->setValue(CONTINUOUS_TWIST, !m_settingsFactory->getContinuousTwist());
				if (m_settingsFactory->saveCommon()) 
				{
					char returnJson[45];
					sprintf(returnJson, "{\"msg\":\"done\", \"continousTwist\":%s }", m_settingsFactory->getContinuousTwist() ? "true" : "false");
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


            server->on("^\\/pinoutDefault\\/([0-9]+)$", HTTP_POST, [this](AsyncWebServerRequest *request)
            {
                auto boardTypeString = request->pathArg(0);
                int boardType = boardTypeString.isEmpty() ? (int)BoardType::DEVKIT : boardTypeString.toInt();
                Serial.println("Settings pinout default");
                m_settingsFactory->setValue(BOARD_TYPE_SETTING, boardType);
                if(SettingsHandler::defaultPinout())
				//if (m_settingsFactory->resetPins()) // Settings handler executes resetPins
                {
                    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"done\"}");
                    request->send(response);
                } 
                else 
                {
                    AsyncWebServerResponse *response = request->beginResponse(500, "application/json", "{\"msg\":\"Error defaulting pinout settings\"}");
                    request->send(response);
                }
            });

            // upload a file to /upload
            // server->on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
            //     request->send(200);
            // }, handleUpload);server->on("/reset", HTTP_POST, [](AsyncWebServerRequest *request){

            server->on("/restart", HTTP_POST, [webSocketHandler, apMode](AsyncWebServerRequest *request)
            {
                //if(apMode) {
                    //request->send(200, "text/plain",String("Restarting device, wait about 10-20 seconds and navigate to ") + (SettingsHandler::getHostname()) + ".local or the network IP address in your browser address bar.");
                //}
                String message = "{\"msg\":\"restarting\",\"apMode\":";
                message += apMode ? "true}" : "false}";
                AsyncWebServerResponse *response = request->beginResponse(200, "application/json", message);
                request->send(response);
                delay(2000);
                webSocketHandler->closeAll();
                SettingsHandler::restart();
            });

            server->on("/default", HTTP_POST, [this](AsyncWebServerRequest *request)
            {
                Serial.println("Settings default");
                if(SettingsHandler::defaultAll()) {
                    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"done\"}");
                    request->send(response);
                } else {
                    sendError(request);
                }
            });

            AsyncCallbackJsonWebHandler* settingsUpdateHandler = new AsyncCallbackJsonWebHandler("/settings", [this](AsyncWebServerRequest *request, JsonVariant &json)
			{
                Serial.println("API save settings...");
                JsonObject jsonObj = json.as<JsonObject>();
                if (m_settingsFactory->saveCommon(jsonObj)) 
                {
                    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"done\"}");
                    request->send(response);
                } 
                else 
                {
                    AsyncWebServerResponse *response = request->beginResponse(500, "application/json", "{\"msg\":\"Error saving settings\"}");
                    request->send(response);
                }
            }, 32768U );//Bad request? increase the size.

            AsyncCallbackJsonWebHandler* pinsHandler = new AsyncCallbackJsonWebHandler("/pins", [this](AsyncWebServerRequest *request, JsonVariant &json)
			{
                Serial.println("API save pins...");
                JsonObject jsonObj = json.as<JsonObject>();
                if (m_settingsFactory->savePins(jsonObj)) 
                {
                    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"done\"}");
                    request->send(response);
                } 
                else 
                {
                    AsyncWebServerResponse *response = request->beginResponse(500, "application/json", "{\"msg\":\"Error saving pins\"}");
                    request->send(response);
                }
            }, 1000U );//Bad request? increase the size.

            AsyncCallbackJsonWebHandler* wifiUpdateHandler = new AsyncCallbackJsonWebHandler("/wifiSettings", [this](AsyncWebServerRequest *request, JsonVariant &json)
			{
                Serial.println("API save wifi settings...");
                JsonObject jsonObj = json.as<JsonObject>();
                if (m_settingsFactory->saveWifi(jsonObj)) 
                {
                    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"done\"}");
                    request->send(response);
                } 
                else 
                {
                    AsyncWebServerResponse *response = request->beginResponse(500, "application/json", "{\"msg\":\"Error saving wifi settings\"}");
                    request->send(response);
                }
            }, 100U );//Bad request? increase the size.

            AsyncCallbackJsonWebHandler* motionProfileUpdateHandler = new AsyncCallbackJsonWebHandler("/motionProfiles", [](AsyncWebServerRequest *request, JsonVariant &json)
			{
                Serial.println("API save motion profiles...");
                JsonObject jsonObj = json.as<JsonObject>();
                if (SettingsHandler::saveMotionProfiles(jsonObj)) 
                {
                    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"done\"}");
                    request->send(response);
                } 
                else 
                {
                    AsyncWebServerResponse *response = request->beginResponse(500, "application/json", "{\"msg\":\"Error saving motion profiles\"}");
                    request->send(response);
                }
            }, 30000U );//Bad request? increase the size.
            
            AsyncCallbackJsonWebHandler* buttonsUpdateHandler = new AsyncCallbackJsonWebHandler("/buttonSettings", [](AsyncWebServerRequest *request, JsonVariant &json)
			{
                Serial.println("API save button settings...");
                JsonObject jsonObj = json.as<JsonObject>();
                if (SettingsHandler::saveButtons(jsonObj)) 
                {
                    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"done\"}");
                    request->send(response);
                } 
                else 
                {
                    AsyncWebServerResponse *response = request->beginResponse(500, "application/json", "{\"msg\":\"Error saving button settings\"}");
                    request->send(response);
                }
            }, 10000U );//Bad request? increase the size.
            // //To upload through terminal you can use: curl -F "image=@firmware.bin" esp8266-webupdate.local/update
            // server->on("/update", HTTP_POST, [this](AsyncWebServerRequest *request){
            //         // the request handler is triggered after the upload has finished... 
            //         // create the response, add header, and send response
            //         AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", (Update.hasError())?"FAIL":"OK");
            //         response->addHeader("Connection", "close");
            //         response->addHeader("Access-Control-Allow-Origin", "*");
            //         SettingsHandler::getRestartRequired() = true;
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
            server->addHandler(pinsHandler);
            server->addHandler(wifiUpdateHandler);
            server->addHandler(motionProfileUpdateHandler);
            server->addHandler(buttonsUpdateHandler);
            
            server->onNotFound([](AsyncWebServerRequest *request) 
			{
                Serial.printf("AsyncWebServerRequest Not found: %s", request->url());
                if (request->method() == HTTP_OPTIONS) {
                    request->send(200);
                } else {
                    AsyncWebServerResponse *response = request->beginResponse(404, "application/text", String("AsyncWebServerRequest Not found") + request->url());
                    request->send(response);
                }
            });

            //server->rewrite("/", "/wifiSettings.htm").setFilter(ON_AP_FILTER);
            server->serveStatic("/", LittleFS, "/www/").setDefaultFile("index-min.html");
            server->begin();
            initialized = true;
        }
        void stop() override {
            if(initialized) 
            {
                initialized = false;
                server->end();
            }
            // if(MDNSInitialized)
            // {
            //      MDNS.end();
            //      MDNSInitialized = false;
            // }
        }
        bool isRunning() override {
            return initialized;
        }
    private:
        bool initialized = false;
        const char* _TAG = TagHandler::WebHandler;
    	SettingsFactory* m_settingsFactory;
        AsyncWebServer* server;

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
        void sendError(AsyncWebServerRequest *request, int code = 500) {
            char lastError[1024];
            LogHandler::getLastError(lastError);
            char responseMessage[1024];
            sprintf(responseMessage, "{\"msg\":\"Error setting default: %s\"}", strlen(lastError) > 0 ? lastError : "Unknown error");
            AsyncWebServerResponse *response = request->beginResponse(code, "application/json", responseMessage);
            request->send(response);
        }
        // void startMDNS(char* hostName, char* friendlyName)
        // {
        //     if(MDNSInitialized)
        //         MDNS.end();
        //     Serial.print("hostName: ");
        //     Serial.println(hostName);
        //     Serial.print("friendlyName: ");
        //     Serial.println(friendlyName);
        //     if (!MDNS.begin(hostName)) {
        //         printf("MDNS Init failed");
        //         return;
        //     }
        //     MDNS.setInstanceName(friendlyName);
        //     MDNS.addService("http", "tcp", 80);
        //     MDNS.addService("tcode", "udp", SettingsHandler::getUdpServerPort());
        //     MDNSInitialized = true;
        // }
};
