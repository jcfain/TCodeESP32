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
#include <PsychicHttp.h>
// #if ESP8266 == 1
// #include <ESPAsyncTCP.h>
// #else
// #include <AsyncTCP.h>
// #endif
//#include <AsyncJson.h>
#include "HTTP/HTTPBase.h"
#include "WifiHandler.h"
#include "WebSocketHandler_psychic.h"
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
		    LogHandler::info(_TAG, "Starting psychic web server on port: %i", port);
            server = new PsychicHttpServer();
            m_settingsFactory = SettingsFactory::getInstance();

            server->listen(port);
            ((WebSocketHandler*)webSocketHandler)->setup(server);

            server->on("/wifiSettings", HTTP_GET, [](PsychicRequest *request) 
            {
                char info[255];
                SettingsHandler::getWifiInfo(info);
                if (strlen(info) == 0) {
                    //PsychicResponse *response = request->beginResponse(504, "application/text", "Error getting wifi settings");
                    return request->reply(504, "application/text", "Error getting wifi settings");
                }
                // PsychicResponse *response = request->beginResponse(200, "application/json", info);
                return request->reply(200, "application/json", info);
            });  

            server->on("/pins", HTTP_GET, [](PsychicRequest *request) 
            {
                PsychicFileResponse response(request, LittleFS, PIN_SETTINGS_PATH, "application/json");
                return response.send();
            }); 

            server->on("/settings", HTTP_GET, [](PsychicRequest *request) 
            {
                PsychicFileResponse response(request, LittleFS, COMMON_SETTINGS_PATH, "application/json");
                return response.send();
            });   

            server->on("/systemInfo", HTTP_GET, [](PsychicRequest *request) 
            {
                String systemInfo;
                SettingsHandler::getSystemInfo(systemInfo);
                if (systemInfo.length() == 0) {
                    return request->reply(504, "application/text", "Error getting user settings");
                }
                return request->reply(200, "application/json", systemInfo.c_str());
            }); 

            server->on("/motionProfiles", HTTP_GET, [](PsychicRequest *request) 
            {
                PsychicFileResponse response(request, LittleFS, MOTION_PROFILE_SETTINGS_PATH, "application/json");
                return response.send();
            });   

            server->on("/buttonSettings", HTTP_GET, [](PsychicRequest *request) 
            {
                PsychicFileResponse response(request, LittleFS, BUTTON_SETTINGS_PATH, "application/json");
                return response.send();
            });  
            
            
            server->on("/log", HTTP_GET, [](PsychicRequest *request) 
            {
                Serial.println("Get log...");
                PsychicFileResponse response(request, LittleFS, LOG_PATH);
                return response.send();
            });   

            server->on("/connectWifi", HTTP_POST, [this](PsychicRequest *request) 
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
                //PsychicResponse *response = request->beginResponse(200, "application/json", output);
                return request->reply(200, "application/json", output.c_str());
            });

            server->on("/toggleContinousTwist", HTTP_POST, [this](PsychicRequest *request) 
            {
				m_settingsFactory->setValue(CONTINUOUS_TWIST, !m_settingsFactory->getContinuousTwist());
				if (m_settingsFactory->saveCommon()) 
				{
					char returnJson[45];
					sprintf(returnJson, "{\"msg\":\"done\", \"continousTwist\":%s }", m_settingsFactory->getContinuousTwist() ? "true" : "false");
					//PsychicResponse *response = request->beginResponse(200, "application/json", returnJson);
					return request->reply(200, "application/json", returnJson);
				} 
				else 
				{
					//PsychicResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"Error saving settings\"}");
					return request->reply(200, "application/json", "{\"msg\":\"Error saving settings\"}");
				}
            });

            // server->on("^\\/sensor\\/([0-9]+)$", HTTP_GET, [] (PsychicRequest *request) 
            // {
            //     String sensorId = request->pathArg(0);
            // });


            server->on("^\\/pinoutDefault\\/([0-9]+)$", HTTP_POST, [this](PsychicRequest *request)
            {
                auto boardType = request->getParam("");
                String boardTypeString = boardType->value();
                int boardTypeInt = boardTypeString.isEmpty() ? (int)BoardType::DEVKIT : boardTypeString.toInt();
                Serial.println("Settings pinout default");
                m_settingsFactory->setValue(BOARD_TYPE_SETTING, boardTypeInt);
                if(SettingsHandler::defaultPinout())
				//if (m_settingsFactory->resetPins()) // Settings handler executes resetPins
                {
                    return request->reply(200, "application/json", "{\"msg\":\"done\"}");
                } 
                else 
                {
                    //PsychicResponse *response = request->beginResponse(500, "application/json", "{\"msg\":\"Error defaulting pinout settings\"}");
                    return request->reply(500, "application/json", "{\"msg\":\"Error defaulting pinout settings\"}");
                }
            });

            // upload a file to /upload
            // server->on("/upload", HTTP_POST, [](PsychicRequest *request){
            //     request->send(200);
            // }, handleUpload);server->on("/reset", HTTP_POST, [](PsychicRequest *request){

            server->on("/restart", HTTP_POST, [webSocketHandler, apMode](PsychicRequest *request)
            {
                //if(apMode) {
                    //request->send(200, "text/plain",String("Restarting device, wait about 10-20 seconds and navigate to ") + (SettingsHandler::hostname) + ".local or the network IP address in your browser address bar.");
                //}
                String message = "{\"msg\":\"restarting\",\"apMode\":";
                message += apMode ? "true}" : "false}";
                //PsychicResponse *response = request->beginResponse(200, "application/json", message);
                auto value = request->reply(200, "application/json", message.c_str());
                delay(2000);
                webSocketHandler->closeAll();
                SettingsHandler::restart();
                return value;
            });

            server->on("/default", HTTP_POST, [this](PsychicRequest *request)
            {
                Serial.println("Settings default");
                if(m_settingsFactory->resetAll()) {
                    esp_err_t ret = request->reply(200, "application/json", "{\"msg\":\"done\"}");
			        SettingsHandler::restart(5);
                    return ret;
                } else {
                    return sendError(request);
                }
            });

            server->on("/settings", HTTP_POST, [this](PsychicRequest *request, JsonVariant &json)
			{
                Serial.println("API save settings...");
                JsonObject jsonObj = json.as<JsonObject>();
                if (m_settingsFactory->saveCommon(jsonObj)) 
                {
                    return request->reply(200, "application/json", "{\"msg\":\"done\"}");
                } 
                return request->reply(500, "application/json", "{\"msg\":\"Error saving settings\"}");
            });

            server->on("/pins", [this](PsychicRequest *request, JsonVariant &json)
			{
                Serial.println("API save pins...");
                JsonObject jsonObj = json.as<JsonObject>();
                if (m_settingsFactory->savePins(jsonObj)) 
                {
                    return request->reply(200, "application/json", "{\"msg\":\"done\"}");
                } 
                return request->reply(500, "application/json", "{\"msg\":\"Error saving pin settings\"}");
            });

            server->on("/wifiSettings", [this](PsychicRequest *request, JsonVariant &json)
			{
                Serial.println("API save wifi settings...");
                JsonObject jsonObj = json.as<JsonObject>();
                if (m_settingsFactory->saveWifi(jsonObj)) 
                {
                    return request->reply(200, "application/json", "{\"msg\":\"done\"}");
                } 
                return request->reply(500, "application/json", "{\"msg\":\"Error saving wifi settings\"}");
            });

            server->on("/motionProfiles", [this](PsychicRequest *request, JsonVariant &json)
			{
                Serial.println("API save motion profiles...");
                JsonObject jsonObj = json.as<JsonObject>();
                if (SettingsHandler::saveMotionProfiles(jsonObj)) 
                {
                    return request->reply(200, "application/json", "{\"msg\":\"done\"}");
                } 
                return request->reply(500, "application/json", "{\"msg\":\"Error saving motion profiles settings\"}");
            });
            
            server->on("/buttonSettings", [this](PsychicRequest *request, JsonVariant &json)
			{
                Serial.println("API save button settings...");
                JsonObject jsonObj = json.as<JsonObject>();
                if (SettingsHandler::saveButtons(jsonObj)) 
                {
                    return request->reply(200, "application/json", "{\"msg\":\"done\"}");
                } 
                return request->reply(500, "application/json", "{\"msg\":\"Error saving button settings\"}");
            });

            server->onNotFound([](PsychicRequest *request) 
			{
                Serial.printf("PsychicRequest Not found: %s", request->url());
                if (request->method() == HTTP_OPTIONS) {
                    return request->reply(200);
                } else {
                    //PsychicResponse *response = request->beginResponse(404, "application/text", String("PsychicRequest Not found") + request->url());
                    return request->reply(404, "application/text", (String("PsychicRequest Not found") + request->url()).c_str());
                }
            });
            //server->rewrite("/", "/wifiSettings.htm").setFilter(ON_AP_FILTER);
            server->serveStatic("/", LittleFS, "/www/")->setDefaultFile("index-min.html");
            //server->s;
            initialized = true;
        }

        void stop() override {
            if(initialized) 
            {
                initialized = false;
                server->stop();
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
        PsychicHttpServer* server;

        void handleUpload(PsychicRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
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
        esp_err_t sendError(PsychicRequest *request, int code = 500) {
            const char* lastError = LogHandler::getLastError();
            char responseMessage[1024];
            sprintf(responseMessage, "{\"msg\":\"Error setting default: %s\"}", strlen(lastError) > 0 ? lastError : "Unknown error");
            //PsychicResponse *response = request->beginResponse(code, "application/json", responseMessage);
            return request->reply(code, "application/json", responseMessage);
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
        //     MDNS.addService("tcode", "udp", SettingsHandler::udpServerPort);
        //     MDNSInitialized = true;
        // }
};
