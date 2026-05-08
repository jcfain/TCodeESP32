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
#include <memory>
#if ESP8266 == 1
#include <ESPAsyncTCP.h>
#else
#include <AsyncTCP.h>
#endif
#include <AsyncJson.h>
#include "HTTP/HTTPBase.h"
#include "network/WifiHandler.h"
#include "TCode/MotorHandler.h"
#include "TCode/PwmManager.h"
#include "WebSocketHandler.h"
#include "logging/TagHandler.h"
#include "messages/SystemCommandHandler.h"
#include "tasks/TaskHandler.h"
class WebHandler : public HTTPBase
{
public:
    WebHandler() = default;
    // bool MDNSInitialized = false;
    void setup_http(uint16_t port, WebSocketBase *webSocketHandler, bool apMode) override
    {
        stop();
        if (port < 1 || port > 65535)
            port = 80;
        LogHandler::info(Tags::Web, "Starting web server on port: %i (free heap: %u)", port, ESP.getFreeHeap());
        server = new AsyncWebServer(port);
        if (!server)
        {
            LogHandler::error(Tags::Web, "Failed to allocate AsyncWebServer!");
            return;
        }
        ((WebSocketHandler *)webSocketHandler)->setup(server);
        m_settingsFactory = SettingsFactory::getInstance();
        server->on("/wifiSettings", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                char info[1024];
                SettingsHandler::getWifiInfo(info, sizeof(info));
                if (strlen(info) == 0) {
                    AsyncWebServerResponse *response = request->beginResponse(504, "application/text", "Error getting wifi settings");
                    request->send(response);
                    return;
                }
                AsyncWebServerResponse *response = request->beginResponse(200, "application/json", info);
                request->send(response); });

        server->on("/settings", HTTP_GET, [](AsyncWebServerRequest *request)
                   { request->send(LittleFS, COMMON_SETTINGS_PATH, "application/json"); });

        server->on("/pins", HTTP_GET, [](AsyncWebServerRequest *request)
                   { request->send(LittleFS, PIN_SETTINGS_PATH, "application/json"); });

        server->on("/systemInfo", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                if(SettingsHandler::restartRequired > -1 || !SettingsHandler::initialized) {
                    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"status\": \"restarting\"}");
                    request->send(response);
                    return;
                }
                auto systemInfo = std::make_shared<String>();
                SettingsHandler::getSystemInfo(*systemInfo);
                if (!systemInfo->length()) {
                    AsyncWebServerResponse *response = request->beginResponse(504, "application/text", "Error getting user settings");
                    request->send(response);
                    return;
                }
                // Use a non-chunked response so Content-Length is set and _ack()
                // allocates min(space, remaining) per call instead of space every call.
                // AsyncBasicResponse copies the string internally so the shared_ptr
                // can be released immediately after send().
                request->send(200, "application/json", *systemInfo); });

        server->on("/motionProfiles", HTTP_GET, [](AsyncWebServerRequest *request)
                   { request->send(LittleFS, MOTION_PROFILE_SETTINGS_PATH, "application/json"); });

        server->on("/channelsProfile", HTTP_GET, [](AsyncWebServerRequest *request)
                   { request->send(LittleFS, CHANNELS_SETTINGS_PATH, "application/json"); });

        server->on("/buttonSettings", HTTP_GET, [](AsyncWebServerRequest *request)
                   { request->send(LittleFS, BUTTON_SETTINGS_PATH, "application/json"); });

        server->on("/debugInfo", HTTP_GET, [](AsyncWebServerRequest* request)
            {
                if (LittleFS.exists(DEBUG_INFO_PATH)) {
                    request->send(LittleFS, DEBUG_INFO_PATH, "application/json");
                }
                else {
                    // No debug info recorded yet; return an empty doc so the
                    // client can populate its UI without treating it as a fault.
                    request->send(200, "application/json", "{\"" DEBUG_INFO_LAST_BOOT_REASONS "\":[]}");
                }
            });

        server->on("/debugInfo", HTTP_POST, [](AsyncWebServerRequest* request)
            {
                // Clear the persisted debug info file so the client can reset
                // recorded boot reasons.
                if (LittleFS.exists(DEBUG_INFO_PATH)) {
                    LittleFS.remove(DEBUG_INFO_PATH);
                }
                request->send(200, "application/json", "{\"msg\":\"done\"}");
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

        server->on("/changeBoard", HTTP_POST, [this](AsyncWebServerRequest *request)
                   {
                String boardTypeString = request->hasParam("value") ? request->getParam("value")->value() : "";
                int boardType = boardTypeString.isEmpty() ? (int)BoardType::DEVKIT : boardTypeString.toInt();
                // if(boardType == (int)BoardType::CRIMZZON || boardType == (int)BoardType::ISAAC) {
                //     m_settingsFactory->setValue(DEVICE_TYPE, DeviceType::SR6);
                // } else if(boardType == (int)BoardType::SSR1PCB) {
                //     m_settingsFactory->setValue(DEVICE_TYPE, DeviceType::SSR1);
                //     m_settingsFactory->setValue(BLDC_ENCODER, BLDCEncoderType::MT6701);
                // }
                // Serial.println("Settings pinout default");
                // m_settingsFactory->setValue(BOARD_TYPE_SETTING, boardType);
                if(m_settingsFactory->changeBoardType(boardType))
                {
                    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"done\"}");
                    request->send(response);
                }
                else
                {
                    AsyncWebServerResponse *response = request->beginResponse(500, "application/json", "{\"msg\":\"Error changing board type\"}");
                    request->send(response);
                } });
        server->on("/changeDevice", HTTP_POST, [this](AsyncWebServerRequest *request)
                   {
                        String deviceTypeString = request->hasParam("value") ? request->getParam("value")->value() : "";
                int deviceType = deviceTypeString.isEmpty() ? (int)DeviceType::OSR : deviceTypeString.toInt();
                // Serial.println("Settings pinout default");
                // m_settingsFactory->setValue(DEVICE_TYPE, deviceType);
                // if(m_settingsFactory->saveCommon() && m_settingsFactory->defaultPinout())
				if (m_settingsFactory->changeDeviceType(deviceType))
                {
                    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"done\"}");
                    request->send(response);
                }
                else
                {
                    AsyncWebServerResponse *response = request->beginResponse(500, "application/json", "{\"msg\":\"Error changing device type\"}");
                    request->send(response);
                } });

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
                SettingsHandler::restart(2); });

        server->on("/reapplyPwm", HTTP_POST, [](AsyncWebServerRequest* request)
            {
                LogHandler::info(Tags::Web, "/reapplyPwm requested");
                MotorHandler::requestReapply();
                AsyncWebServerResponse* response = request->beginResponse(200, "application/json", "{\"msg\":\"reapplying\"}");
                request->send(response); });

        // Lightweight liveness probe used by the web UI's post-restart poll
        // and by external health checks. GET or POST both work.
        auto pingHandler = [](AsyncWebServerRequest* request) {
            AsyncWebServerResponse* response = request->beginResponse(200, "application/json", "{\"status\":\"ok\"}");
            request->send(response);
            };
        server->on("/ping", HTTP_GET, pingHandler);
        server->on("/ping", HTTP_POST, pingHandler);

        server->on("/default", HTTP_POST, [this](AsyncWebServerRequest *request)
                   {
                Serial.println("Settings default");
                if(m_settingsFactory->resetAll()) {
                    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"done\"}");
                    request->send(response);
			        SettingsHandler::restart(5);
                } else {
                    sendError(request);
                } });

        AsyncCallbackJsonWebHandler *settingsUpdateHandler = new AsyncCallbackJsonWebHandler("/settings", [this](AsyncWebServerRequest *request, JsonVariant &json)
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
                } }); //, 32768U );//Bad request? increase the size.

        AsyncCallbackJsonWebHandler *pinsHandler = new AsyncCallbackJsonWebHandler("/pins", [this](AsyncWebServerRequest *request, JsonVariant &json)
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
                } }); //, 1000U );//Bad request? increase the size.

        AsyncCallbackJsonWebHandler *wifiUpdateHandler = new AsyncCallbackJsonWebHandler("/wifiSettings", [this](AsyncWebServerRequest *request, JsonVariant &json)
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
                } }); //, 500U );//Bad request? increase the size.

        AsyncCallbackJsonWebHandler *motionProfileUpdateHandler = new AsyncCallbackJsonWebHandler("/motionProfiles", [](AsyncWebServerRequest *request, JsonVariant &json)
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
                } }); //, 30000U );//Bad request? increase the size.

        AsyncCallbackJsonWebHandler *channelsProfileUpdateHandler = new AsyncCallbackJsonWebHandler("/channelsProfile", [](AsyncWebServerRequest *request, JsonVariant &json)
                                                                                                    {
                Serial.println("API save channels profile...");
                JsonObject jsonObj = json.as<JsonObject>();
                if (SettingsHandler::saveChannels(jsonObj))
                {
                    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"msg\":\"done\"}");
                    request->send(response);
                }
                else
                {
                    AsyncWebServerResponse *response = request->beginResponse(500, "application/json", "{\"msg\":\"Error saving channels profile\"}");
                    request->send(response);
                } }); //, 5000U );//Bad request? increase the size.

        AsyncCallbackJsonWebHandler *buttonsUpdateHandler = new AsyncCallbackJsonWebHandler("/buttonSettings", [](AsyncWebServerRequest *request, JsonVariant &json)
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
                } }); //, 10000U );//Bad request? increase the size.

                // ----------------------------------------------------------------
                // /pwmTest — manual override: bind {pin} to LEDC or MCPWM at the
                // requested freq+resolution and write a duty %. Bypasses normal
                // settings; only undone by reapplyPwm or reboot. Used by the GUI
                // "Test PWM output" panel under Advanced settings.
                // Body: { "pin": <int>, "driver": 0|1, "freq": <hz>,
                //         "resolution": <bits>, "dutyPct": <0..100> }
                // ----------------------------------------------------------------
                AsyncCallbackJsonWebHandler* pwmTestHandler = new AsyncCallbackJsonWebHandler("/pwmTest", [](AsyncWebServerRequest* request, JsonVariant& json)
                    {
                        JsonObject obj = json.as<JsonObject>();
                        int pin = obj["pin"] | -1;
                        int driver = obj["driver"] | -1; // 0 = MCPWM, 1 = LEDC (matches PwmDriver enum)
                        int freq = obj["freq"] | 0;
                        int resolution = obj["resolution"] | 0;
                        float dutyPct = obj["dutyPct"] | 0.0f;

                        if (pin < 0 || freq <= 0 || resolution <= 0 || resolution > 20 || dutyPct < 0.0f || dutyPct > 100.0f)
                        {
                            AsyncWebServerResponse* response = request->beginResponse(400, "application/json",
                                "{\"msg\":\"Invalid pwmTest payload\"}");
                            request->send(response);
                            return;
                        }

                        PwmManager::Backend wantBackend =
                            (driver == 0) ? PwmManager::Backend::MCPWM : PwmManager::Backend::LEDC;

                        PwmManager::Backend got = PwmManager::instance().attachExclusive(
                            "pwmTest", (int8_t)pin, (uint32_t)freq, (uint8_t)resolution, wantBackend);

                        if (got == PwmManager::Backend::NONE)
                        {
                            AsyncWebServerResponse* response = request->beginResponse(500, "application/json",
                                "{\"msg\":\"pwmTest attach failed\"}");
                            request->send(response);
                            return;
                        }

                        uint32_t maxDuty = (resolution >= 32) ? 0xFFFFFFFFu : ((1u << resolution) - 1u);
                        uint32_t duty = (uint32_t)((dutyPct / 100.0f) * (float)maxDuty + 0.5f);
                        if (duty > maxDuty) duty = maxDuty;
                        bool ok = PwmManager::instance().write((int8_t)pin, duty);

                        LogHandler::info(Tags::Web,
                            "/pwmTest pin=%d driver=%s freq=%d res=%d duty=%u/%u ok=%d",
                            pin, got == PwmManager::Backend::MCPWM ? "MCPWM" : "LEDC",
                            freq, resolution, (unsigned)duty, (unsigned)maxDuty, ok);

                        char body[160];
                        snprintf(body, sizeof(body),
                            "{\"msg\":\"%s\",\"backend\":\"%s\",\"duty\":%u,\"maxDuty\":%u}",
                            ok ? "done" : "write failed",
                            got == PwmManager::Backend::MCPWM ? "MCPWM" : "LEDC",
                            (unsigned)duty, (unsigned)maxDuty);
                        AsyncWebServerResponse* response = request->beginResponse(ok ? 200 : 500, "application/json", body);
                        request->send(response);
                    });

                // /pwmTestStop — detach the pin so it stops driving (high-Z input).
                // Body: { "pin": <int> }
                AsyncCallbackJsonWebHandler* pwmTestStopHandler = new AsyncCallbackJsonWebHandler("/pwmTestStop", [](AsyncWebServerRequest* request, JsonVariant& json)
                    {
                        JsonObject obj = json.as<JsonObject>();
                        int pin = obj["pin"] | -1;
                        if (pin < 0)
                        {
                            AsyncWebServerResponse* response = request->beginResponse(400, "application/json",
                                "{\"msg\":\"missing pin\"}");
                            request->send(response);
                            return;
                        }
                        bool ok = PwmManager::instance().detach((int8_t)pin);
                        pinMode((uint8_t)pin, INPUT);
                        LogHandler::info(Tags::Web, "/pwmTestStop pin=%d ok=%d", pin, ok);
                        AsyncWebServerResponse* response = request->beginResponse(200, "application/json",
                            ok ? "{\"msg\":\"detached\"}" : "{\"msg\":\"not attached\"}");
                        request->send(response);
                    });

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
        server->addHandler(channelsProfileUpdateHandler);
        server->addHandler(buttonsUpdateHandler);
        server->addHandler(pwmTestHandler);
        server->addHandler(pwmTestStopHandler);

        server->onNotFound([this](AsyncWebServerRequest *request)
                           {
                LogHandler::info(Tags::Web, "Request: %s %s (heap: %u)", request->methodToString(), request->url().c_str(), ESP.getFreeHeap());
                if (handleStaticFile(request)) return;
                Serial.printf("AsyncWebServerRequest Not found: %s", request->url().c_str());
                if (request->method() == HTTP_OPTIONS) {
                    request->send(200);
                } else {
                    AsyncWebServerResponse *response = request->beginResponse(404, "application/text", String("AsyncWebServerRequest Not found") + request->url());
                    request->send(response);
                } });
        // server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request)
        // {
        //     // request->send(LittleFS, COMMON_SETTINGS_PATH, "application/json");
        //     Serial.println("index");
        //     sendChunked(request, "/www/index-min.html", "text/html");
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
        //     sendChunked(request, "/www/settings-min.js", 4096, "text/javascript");
        // });
        // server->on("/motion-generator-min.js", HTTP_GET, [this](AsyncWebServerRequest *request)
        // {
        //     sendChunked(request, "/www/motion-generator-min.js", 1024, "text/javascript");
        // });

        // Static files are served via handleStaticFile() in onNotFound.
        // We use request->send(FS, path, mime) with the ORIGINAL (non-.gz)
        // path — AsyncFileResponse's FS constructor automatically tries
        // path.gz and sets Content-Encoding: gzip correctly.  The File-based
        // constructor used by serveStatic does NOT do this properly.
        LogHandler::info(Tags::Web, "Calling server->begin() (free heap: %u, max block: %u)", ESP.getFreeHeap(), ESP.getMaxAllocHeap());
        server->begin();
        initialized = true;
        LogHandler::info(Tags::Web, "Web server started (free heap: %u)", ESP.getFreeHeap());
    }
    void stop() override
    {
        if (initialized)
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
    bool isRunning() override
    {
        return initialized;
    }

    void setup() override
    {
    }

    void loop() override
    {
    }

private:
    bool initialized = false;
    SettingsFactory *m_settingsFactory;
    AsyncWebServer *server;

    void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
    {
        if (!index)
        {
            Serial.printf("UploadStart: %s\n", filename.c_str());
        }
        for (size_t i = 0; i < len; i++)
        {
            Serial.write(data[i]);
        }
        if (final)
        {
            Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index + len);
        }
    }
    void sendError(AsyncWebServerRequest *request, int code = 500)
    {
        const char *lastError = LogHandler::getLastError();
        char responseMessage[1057];
        sprintf(responseMessage, "{\"msg\":\"Error: %s\"}", strlen(lastError) > 0 ? lastError : "Unknown error");
        AsyncWebServerResponse *response = request->beginResponse(code, "application/json", responseMessage);
        request->send(response);
    }

    void sendChunked(AsyncWebServerRequest *request, const char *filePath, const char *mimeType = "application/json", const bool &isGZip = false)
    {
        LogHandler::debug(Tags::Web, "[sendChunked] Open file: %s\n", filePath);
        File file{LittleFS.open(filePath, FILE_READ)};

        if (!file)
        {
            LogHandler::error(Tags::Web, "[sendChunked] Failed to open: %s", filePath);
            request->send(500, "text/plain", "File read error");
            return;
        }

        AsyncWebServerResponse *response = request->beginChunkedResponse(
            mimeType,
            [this, file](
                uint8_t *buffer,
                const size_t max_len,
                const size_t index) mutable -> size_t
            {
                LogHandler::debug(Tags::Web, "[beginChunkedResponse] Enter chunked file: %s\n", file.name());
                size_t length;

                // Cap chunk size to one TCP MSS to limit peak memory per
                // send.  Smaller allocations are more likely to succeed when
                // the heap is fragmented by WiFi+BLE coexistence.
                static constexpr size_t MAX_CHUNK = 1460;
                size_t readLen = max_len > MAX_CHUNK ? MAX_CHUNK : max_len;
                length = file.read(buffer, readLen);

                if (length == 0)
                {
                    LogHandler::debug(Tags::Web, "[beginChunkedResponse] Close file: %s\n", file.name());
                    file.close();
                }

                return length;
            });

        if (!response)
        {
            LogHandler::error(Tags::Web, "[sendChunked] Failed to create response (heap: %u)", ESP.getFreeHeap());
            file.close();
            request->send(503, "text/plain", "Out of memory");
            return;
        }

        // Force download
        // response->addHeader("Content-Disposition", "attachment; filename=\"userSettings.json\"");
        if (isGZip)
            response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    }

    bool handleStaticFile(AsyncWebServerRequest *request)
    {
        String requestUrl = request->url();
        LogHandler::debug(Tags::Web, "[handleStaticFile] request url: %s", requestUrl);
        String path = "/www" + requestUrl;
        LogHandler::debug(Tags::Web, "[handleStaticFile] static path: %s", path);

        if (path.endsWith("/"))
            path += F("index-min.html");

        // Detect mime type from the original path (before any .gz check)
        const char *mimeType;
        if (path.endsWith(".html"))
            mimeType = "text/html";
        else if (path.endsWith(".js"))
            mimeType = "text/javascript";
        else if (path.endsWith(".json"))
            mimeType = "application/json";
        else if (path.endsWith(".css"))
            mimeType = "text/css";
        else if (path.endsWith(".ico"))
            mimeType = "image/x-icon";
        else
            mimeType = "application/octet-stream";

        // Use the FS-based send which sets Content-Length (known file size)
        // instead of chunked transfer encoding.  Content-Length responses
        // require less per-ACK buffer allocation, reducing the chance of
        // _ack() "Failed to allocate" errors under memory pressure.
        // Pass the gz path directly (when it exists) so AsyncFileResponse
        // does not attempt to open the non-gz path first, which would
        // generate a spurious VFS "does not exist" error in the log.
        String pathWithGz = path + ".gz";
        if (LittleFS.exists(pathWithGz))
        {
            AsyncWebServerResponse *response = request->beginResponse(LittleFS, pathWithGz, mimeType);
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
            return true;
        }
        if (LittleFS.exists(path))
        {
            request->send(LittleFS, path, mimeType);
            return true;
        }
        return false;
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
