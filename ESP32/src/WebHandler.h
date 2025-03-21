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
#include <WebServer.h>
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
#if !CONFIG_HTTPD_WS_SUPPORT
#error This example cannot be used unless HTTPD_WS_SUPPORT is enabled in esp-http-server component configuration
#endif
class WebHandler : public HTTPBase {
    public:
        // bool MDNSInitialized = false;
        void setup(uint16_t port, WebSocketBase* webSocketHandler, bool apMode) override {
            stop();
            if (port < 1 || port > 65535) 
                port = 80;
		    LogHandler::info(_TAG, "Starting web server on port: %i", port);
            server = new AsyncWebServer(port);
            ((WebSocketHandler*)webSocketHandler)->setup(server);
            m_settingsFactory = SettingsFactory::getInstance();
            server->on("/wifiSettings", HTTP_GET, [](AsyncWebServerRequest *request) 
            {
                char info[400];
                SettingsHandler::getWifiInfo(info);
                if (strlen(info) == 0) {
                    AsyncWebServerResponse *response = request->beginResponse(504, "application/text", "Error getting wifi settings");
                    request->send(response);
                    return;
                }
                AsyncWebServerResponse *response = request->beginResponse(200, "application/json", info);
                request->send(response);
            });  

            server->on("/settings", HTTP_GET, [this](AsyncWebServerRequest *request)
            {
                request->send(LittleFS, COMMON_SETTINGS_PATH, "application/json");
                //sendChunked(request, COMMON_SETTINGS_PATH);
            });

            server->on("/pins", HTTP_GET, [this](AsyncWebServerRequest *request) 
            {
                request->send(LittleFS, PIN_SETTINGS_PATH, "application/json");
                //sendChunked(request, PIN_SETTINGS_PATH);
            }); 

            server->on("/systemInfo", HTTP_GET, [](AsyncWebServerRequest *request) 
            {
                if(SettingsHandler::restartRequired > -1) {
                    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"status\": \"restarting\"}");
                    request->send(response);
                    return;
                }
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

            server->on("/motionProfiles", HTTP_GET, [this](AsyncWebServerRequest *request) 
            {
                request->send(LittleFS, MOTION_PROFILE_SETTINGS_PATH, "application/json");
                //sendChunked(request, MOTION_PROFILE_SETTINGS_PATH);
            });   

            server->on("/buttonSettings", HTTP_GET, [this](AsyncWebServerRequest *request) 
            {
                request->send(LittleFS, BUTTON_SETTINGS_PATH, "application/json");
                //sendChunked(request, BUTTON_SETTINGS_PATH);
            });  
            
            // server->on("/log", HTTP_GET, [this](AsyncWebServerRequest *request) 
            // {
            //     Serial.println("Get log...");
            //     //request->send(LittleFS, LOG_PATH);
            //     sendChunked(request, LOG_PATH);
            // });   

            // server->on("/connectWifi", HTTP_POST, [this](AsyncWebServerRequest *request) 
            // {
            //     WifiHandler wifi;
            //     //const size_t capacity = JSON_OBJECT_SIZE(2);
            //     JsonDocument doc;
            //     char ssid[SSID_LEN] = {0};
            //     char pass[WIFI_PASS_LEN] = {0};
            //     m_settingsFactory->getValue(SSID_SETTING, ssid, SSID_LEN);
            //     m_settingsFactory->getValue(WIFI_PASS_SETTING, pass, WIFI_PASS_LEN);
            //     if (wifi.connect(ssid, pass)) 
            //     {

            //         doc["connected"] = true;
            //         doc["IPAddress"] = wifi.ip().toString();
            //     }
            //     else 
            //     {
            //         doc["connected"] = false;
            //         doc["IPAddress"] = "0.0.0.0";

            //     }
            //     String output;
            //     serializeJson(doc, output);
            //     AsyncWebServerResponse *response = request->beginResponse(200, "application/json", output);
            //     request->send(response);
            // });

            // server->on("/toggleContinousTwist", HTTP_POST, [this](AsyncWebServerRequest *request) 
            // {
			// 	m_settingsFactory->setValue(CONTINUOUS_TWIST, !m_settingsFactory->getContinuousTwist());
			// 	if (m_settingsFactory->saveCommon()) 
			// 	{
			// 		char returnJson[45];
			// 		sprintf(returnJson, "{\"msg\":\"done\", \"continousTwist\":%s }", m_settingsFactory->getContinuousTwist() ? "true" : "false");
			// 		AsyncWebServerResponse *response = request->beginResponse(200, "application/json", returnJson);
			// 		request->send(response);
			// 	} 
			// 	else 
			// 	{
			// 		AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"Error saving settings\"}");
			// 		request->send(response);
			// 	}
            // });

            server->on("^\\/sensor\\/([0-9]+)$", HTTP_GET, [] (AsyncWebServerRequest *request) 
            {
                String sensorId = request->pathArg(0);
            });


            server->on("^\\/changeBoard\\/([0-9]+)$", HTTP_POST, [this](AsyncWebServerRequest *request)
            {
                auto boardTypeString = request->pathArg(0);
                int boardType = boardTypeString.isEmpty() ? (int)BoardType::DEVKIT : boardTypeString.toInt();
                if(boardType == (int)BoardType::CRIMZZON || boardType == (int)BoardType::ISAAC) {
                    m_settingsFactory->setValue(DEVICE_TYPE, DeviceType::SR6);
                } else if(boardType == (int)BoardType::SSR1PCB) {
                    m_settingsFactory->setValue(DEVICE_TYPE, DeviceType::SSR1);
                    m_settingsFactory->setValue(BLDC_ENCODER, BLDCEncoderType::MT6701);
                }
                Serial.println("Settings pinout default");
                m_settingsFactory->setValue(BOARD_TYPE_SETTING, boardType);
                if(m_settingsFactory->saveCommon() && SettingsHandler::defaultPinout())
				//if (m_settingsFactory->resetPins()) // Settings handler executes resetPins
                {
                    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"done\"}");
                    request->send(response);
                } 
                else 
                {
                    AsyncWebServerResponse *response = request->beginResponse(500, "application/json", "{\"msg\":\"Error changing board type\"}");
                    request->send(response);
                }
            });
            server->on("^\\/changeDevice\\/([0-9]+)$", HTTP_POST, [this](AsyncWebServerRequest *request)
            {
                auto deviceTypeString = request->pathArg(0);
                int deviceType = deviceTypeString.isEmpty() ? (int)DeviceType::OSR : deviceTypeString.toInt();
                Serial.println("Settings pinout default");
                m_settingsFactory->setValue(DEVICE_TYPE, deviceType);
                if(m_settingsFactory->saveCommon() && SettingsHandler::defaultPinout())
				//if (m_settingsFactory->resetPins()) // Settings handler executes resetPins
                {
                    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"done\"}");
                    request->send(response);
                } 
                else 
                {
                    AsyncWebServerResponse *response = request->beginResponse(500, "application/json", "{\"msg\":\"Error changing device type\"}");
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
                webSocketHandler->closeAll();
                SettingsHandler::restart(2);
            });

            server->on("/default", HTTP_POST, [this](AsyncWebServerRequest *request)
            {
                Serial.println("Settings default");
                if(m_settingsFactory->resetAll()) {
                    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"done\"}");
                    request->send(response);
			        SettingsHandler::restart(5);
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
            }, 500U );//Bad request? increase the size.

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
                Serial.printf("AsyncWebServerRequest Not found: %s", request->url().c_str());
                if (request->method() == HTTP_OPTIONS) {
                    request->send(200);
                } else {
                    AsyncWebServerResponse *response = request->beginResponse(404, "application/text", String("AsyncWebServerRequest Not found") + request->url());
                    request->send(response);
                }
            });
            // server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request)
            // {
            //     // request->send(LittleFS, COMMON_SETTINGS_PATH, "application/json");
            //     Serial.println("index");
            //     sendChunked(request, "/index-min.html", "application/html");
            // });
            //"^\\/pinoutDefault\\/([0-9]+)$"
            // server->on("\\/.*\\.js", HTTP_GET, [this](AsyncWebServerRequest *request)
            // {
            //     // request->send(LittleFS, COMMON_SETTINGS_PATH, "application/json");
            //     const char* filename = request->pathArg(0).c_str();
            //     Serial.printf("JS file: %s\n", filename);
            //     sendChunked(request, filename, "application/javascript");
            // });
            // server->on("/settings-min.js", HTTP_GET, [this](AsyncWebServerRequest *request)
            // {
            //     sendChunked(request, "/www/settings-min.js", 4096, "application/javascript");
            // });
            // server->on("/motion-generator-min.js", HTTP_GET, [this](AsyncWebServerRequest *request)
            // {
            //     sendChunked(request, "/www/motion-generator-min.js", 1024, "application/javascript");
            // });

            //server->rewrite("/", "/wifiSettings.htm").setFilter(ON_AP_FILTER);
            server->serveStatic("/", LittleFS, "/www/")
                .setDefaultFile("index-min.html");
                //.setCacheControl("max-age=60000");
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
            const char* lastError = LogHandler::getLastError();
            char responseMessage[1057];
            sprintf(responseMessage, "{\"msg\":\"Error: %s\"}", strlen(lastError) > 0 ? lastError : "Unknown error");
            AsyncWebServerResponse *response = request->beginResponse(code, "application/json", responseMessage);
            request->send(response);
        }

        void sendChunked(AsyncWebServerRequest *request, const char* filePath, uint16_t chunkSize = 512, const char* mimeType = "application/json") {
		        LogHandler::debug(_TAG,"Open file: %s\n", filePath);
                File file{LittleFS.open(filePath, FILE_READ)};

                AsyncWebServerResponse *response = request->beginChunkedResponse(
                    mimeType,
                    [this, file, chunkSize](
                        uint8_t* buffer,
                        const size_t max_len,
                        const size_t index) mutable -> size_t
                    {
		                LogHandler::debug(_TAG,"Enter chunked file: %s\n", file.name());
                        size_t length;

                        // Restrict chunk size so we don't run out of RAM
                        static const size_t max_chunk{chunkSize};
                        if (max_chunk < max_len)
                        {
		                    LogHandler::debug(_TAG,"Max chunk %u Max len %u for: %s\n", chunkSize, max_len, file.name());
                            length = file.read(buffer, max_chunk);
                        }
                        else
                        {
		                    LogHandler::debug(_TAG,"Max len %u exceded max chunk %u for: %s\n", max_len, chunkSize, file.name());
                            length = file.read(buffer, max_len);
                        }

                        if (length == 0)
                        {
		                    LogHandler::debug(_TAG,"Close file: %s\n", file.name());
                            file.close();
                        }

                        return length;
                    });

                // Force download
                //response->addHeader("Content-Disposition", "attachment; filename=\"userSettings.json\"");
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
