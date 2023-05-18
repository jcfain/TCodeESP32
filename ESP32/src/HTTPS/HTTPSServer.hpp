#pragma once

#include <Arduino.h>
#include <string>
#include <esp_https_server.h>
#include "esp_tls.h"
#include "SettingsHandler.h"
#include "HTTP/WebSocketBase.h"
#include "HTTP/HTTPBase.h"

//typedef std::function<esp_err_t(httpd_req_t*)> TSHandlerFunction;

// using HTTPS_FUNCTION_PTR_T = esp_err_t (*)(httpd_req_t *req);
// extern "C" void invoke_function(void* ptr) {
//     (*static_cast<std::function<void()>*>(ptr))();
// }
template<typename Function>
struct function_traits;

template <typename Ret, typename... Args>
struct function_traits<Ret(Args...)> {
    typedef Ret(*ptr)(Args...);
};

template <typename Ret, typename... Args>
struct function_traits<Ret(*const)(Args...)> : function_traits<Ret(Args...)> {};

template <typename Cls, typename Ret, typename... Args>
struct function_traits<Ret(Cls::*)(Args...) const> : function_traits<Ret(Args...)> {};

using voidfun = void(*)();

template <typename F>
voidfun lambda_to_void_function(F lambda) {
    static auto lambda_copy = lambda;

    return []() {
        lambda_copy();
    };
}
// requires C++20
template <typename F>
auto lambda_to_pointer(F lambda) -> typename function_traits<decltype(&F::operator())>::ptr {
    static auto lambda_copy = lambda;
    
    return []<typename... Args>(Args... args) {
        return lambda_copy(args...);
    };
}

class HTTPSServer: public HTTPBase {
public:

    static httpd_uri_t root;

    HTTPSServer(): conf(HTTPD_SSL_CONFIG_DEFAULT()) { 
        conf.httpd.uri_match_fn = httpd_uri_match_wildcard;
        conf.httpd.stack_size = 20000;
    }

    void setup(int port, WebSocketBase* webSocketHandler, bool apMode) override {
        // Start the httpd server
        ESP_LOGI(TAG, "Starting server");
        bool certExists = SettingsHandler::readFile(cert, "/certs/servercert.pem");
        bool keyExists = SettingsHandler::readFile(privkey, "/certs/prvtkey.pem");
        if(!certExists && !keyExists)
            return;
        Serial.print("Cert: ");
        Serial.println(this->cert);
        Serial.print("Key: ");
        Serial.println(this->privkey);
        conf.cacert_pem = (const uint8_t*)this->cert;
        conf.cacert_len = strlen(this->cert) + 1;
        conf.prvtkey_pem = (const uint8_t*)this->privkey;
        conf.prvtkey_len = strlen(this->privkey) + 1;

        esp_err_t ret = httpd_ssl_start(&server, &conf);
        if (ESP_OK != ret) {
            ESP_LOGI(TAG, "Error starting server!");
            return;
        }
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        setupHandlers(webSocketHandler, apMode);
    }
    
    void stop() override {

    }
    bool isRunning() override {
        return true;
    }
    
    /* An HTTP GET handler */
    static esp_err_t root_file_handler(httpd_req_t *req)
    {
        httpd_resp_set_type(req, "text/html");
        char* buf; 
        SettingsHandler::readFile(buf, "/www/index-min.html"),
        httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
        delete buf;
        return ESP_OK;
    }
    static esp_err_t settings_file_handler(httpd_req_t *req)
    {
        httpd_resp_set_type(req, "text/html");
        char* buf; 
        SettingsHandler::readFile(buf, "/www/settings-min.js"),
        httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
        delete buf;
        return ESP_OK;
    }
    static esp_err_t style_file_handler(httpd_req_t *req)
    {
        httpd_resp_set_type(req, "text/html");
        char* buf; 
        SettingsHandler::readFile(buf, "/www/style-min.css"),
        httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
        delete buf;
        return ESP_OK;
    }

private: 
    static const char* _TAG;
    httpd_handle_t server = nullptr;
    httpd_ssl_config_t conf;
    char* cert;
    char* privkey;
    static const char contentTypes[7][6][32];
    
    void RegisterHandler(const char* path, http_method method, std::function<esp_err_t(httpd_req_t*)> handler) {
        
        httpd_uri_t* uri_handler = new httpd_uri_t();
        uri_handler->uri = path;
        uri_handler->method = method;
        uri_handler->handler = lambda_to_pointer(handler);
        httpd_register_uri_handler(this->server, uri_handler);
    }

    static esp_err_t sendResponse(httpd_req_t *req, u_int16_t code, const char* mime, const char* content) {
        httpd_resp_set_status(req, getStatus(code));
        httpd_resp_set_type(req, mime);
        httpd_resp_send(req, content, HTTPD_RESP_USE_STRLEN);
        if(code >= 400) {
            return ESP_FAIL;
        }
        return ESP_OK;
    }

// #define HTTPD_200      "200 OK"                     /*!< HTTP Response 200 */
// #define HTTPD_204      "204 No Content"             /*!< HTTP Response 204 */
// #define HTTPD_207      "207 Multi-Status"           /*!< HTTP Response 207 */
// #define HTTPD_400      "400 Bad Request"            /*!< HTTP Response 400 */
// #define HTTPD_404      "404 Not Found"              /*!< HTTP Response 404 */
// #define HTTPD_408      "408 Request Timeout"        /*!< HTTP Response 408 */
// #define HTTPD_500      "500 Internal Server Error"  /*!< HTTP Response 500 */
    static const char* getStatus(u_int16_t code) {
        switch (code)
        {
        case 200:
            return HTTPD_200;
            break;
        case 204:
            return HTTPD_204;
            break;
        case 207:
            return HTTPD_207;
            break;
        case 400:
            return HTTPD_400;
            break;
        case 404:
            return HTTPD_404;
            break;
        case 408:
            return HTTPD_408;
            break;
        case 500:
            return HTTPD_500;
            break;
        
        default:
            return HTTPD_500;
        }
    }

    void setupHandlers(WebSocketBase* webSocketHandler, bool apMode) {
        
        RegisterHandler("/", HTTP_GET, &handleSPIFFS);
        RegisterHandler("/index-min.html", HTTP_GET, &handleSPIFFS);
        RegisterHandler("/settings-min.js", HTTP_GET, &handleSPIFFS);
        RegisterHandler("/style-min.css", HTTP_GET, &handleSPIFFS);
        
        RegisterHandler("/userSettings", HTTP_GET, [this](httpd_req_t *req) -> esp_err_t
        {
            LogHandler::verbose(_TAG, "Get userSettings");
            SettingsHandler::printFree();
            ////req->send(SPIFFS, "/userSettings.json");
            // DynamicJsonDocument doc(SettingsHandler::deserialize);
            // File file = SPIFFS.open(SettingsHandler::userSettingsFilePath, "r");
            // DeserializationError error = deserializeJson(doc, file);
            char settings[6144];
            SettingsHandler::serialize(settings);
            if (strlen(settings) == 0) {
                //AsyncWebServerResponse *response = req->beginResponse(504, "application/text", "Error getting user settings");
                return sendResponse(req, 504, "application/text", "Error getting user settings");
            }
            // if(strcmp(doc["wifiPass"], SettingsHandler::defaultWifiPass) != 0 )
            //     doc["wifiPass"] = "Too bad haxor!";// Do not send password if its not default
                
            // doc["lastRebootReason"] = SettingsHandler::lastRebootReason;
            // String output;
            // serializeJson(doc, output);
            //AsyncWebServerResponse *response = req->beginResponse(200, "application/json", settings);
            return sendResponse(req, 200, "application/json", settings);
        });   
        RegisterHandler("/systemInfo", HTTP_GET, [this](httpd_req_t *req) -> esp_err_t 
        {
            LogHandler::verbose(_TAG, "systemInfo");
            SettingsHandler::printFree();
            char systemInfo[1024];
            SettingsHandler::getSystemInfo(systemInfo);
            if (strlen(systemInfo) == 0) {
                //AsyncWebServerResponse *response = req->beginResponse(504, "application/text", "Error getting user settings");
                return sendResponse(req, 504, "application/text", "Error getting user settings");
            }
            //AsyncWebServerResponse *response = req->beginResponse(200, "application/json", systemInfo);
                return sendResponse(req, 200, "application/json", systemInfo);
        });   
        
        // secureServer.registerNode(new ResourceNode("/log", "GET", [](HTTPRequest * req, HTTPResponse * res) 
        // {
        //     Serial.println("Get log...");
        //     req->send(SPIFFS, SettingsHandler::logPath);
        // }));   

        // secureServer.registerNode(new ResourceNode("/connectWifi", "POST", [](HTTPRequest * req, HTTPResponse * res) 
        // {
        //     WifiHandler wifi;
        //     const size_t capacity = JSON_OBJECT_SIZE(2);
        //     DynamicJsonDocument doc(capacity);
        //     if (wifi.connect(SettingsHandler::ssid, SettingsHandler::wifiPass)) 
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
        //     AsyncWebServerResponse *response = req->beginResponse(200, "application/json", output);
        //     req->send(response);
        // }));

        // secureServer.registerNode(new ResourceNode("/toggleContinousTwist", "POST", [](HTTPRequest * req, HTTPResponse * res) 
        // {
        //     SettingsHandler::continuousTwist = !SettingsHandler::continuousTwist;
        //     if (SettingsHandler::save()) 
        //     {
        //         char returnJson[45];
        //         sprintf(returnJson, "{\"msg\":\"done\", \"continousTwist\":%s }", SettingsHandler::continuousTwist ? "true" : "false");
        //         AsyncWebServerResponse *response = req->beginResponse(200, "application/json", returnJson);
        //         req->send(response);
        //     } 
        //     else 
        //     {
        //         AsyncWebServerResponse *response = req->beginResponse(200, "application/json", "{\"msg\":\"Error saving settings\"}");
        //         req->send(response);
        //     }
        // });

        // secureServer.registerNode(new ResourceNode("^\\/sensor\\/([0-9]+)$", "GET", [] (HTTPRequest * req, HTTPResponse * res) 
        // {
        //     String sensorId = req->pathArg(0);
        // }));

        RegisterHandler("/restart", HTTP_POST, [this](httpd_req_t *req) -> esp_err_t
        {
            LogHandler::verbose(_TAG, "restart");
            SettingsHandler::printFree();
            //if(apMode) {
                //req->send(200, "text/plain",String("Restarting device, wait about 10-20 seconds and navigate to ") + (SettingsHandler::hostname) + ".local or the network IP address in your browser address bar.");
            //}
            String message = "{\"msg\":\"restarting\",\"apMode\":false}";
            //message += apMode ? "true}" : "false}";
            //AsyncWebServerResponse *response = req->beginResponse(200, "application/json", message);
            auto response = sendResponse(req, 200, "application/json", message.c_str());
            delay(2000);
            //webSocketHandler->closeAll();
            SystemCommandHandler::restart();
            return response;
        });

        RegisterHandler("/default", HTTP_POST, [this](httpd_req_t *req) -> esp_err_t
        {
            Serial.println("Settings default");
            SettingsHandler::defaultAll();
            return sendResponse(req, 200, "application/text", "OK");
        });
        RegisterHandler("/settings", HTTP_POST, [this](httpd_req_t *req) -> esp_err_t
        {
            LogHandler::verbose(_TAG, "Post settings");
            SettingsHandler::printFree();
            const int capacity = SettingsHandler::getDeserializeSize();
            DynamicJsonDocument doc(capacity);

            // Create buffer to read request
            char buffer[capacity + 1];
            memset(buffer, 0, capacity+1);

    /* Destination buffer for content of HTTP POST request.
     * httpd_req_recv() accepts char* only, but content could
     * as well be any binary data (needs type casting).
     * In case of string data, null termination will be absent, and
     * content length would give length of string */
    //char content[100];

    /* Truncate if content length larger than the buffer */
    //size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, buffer, capacity+1);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            httpd_resp_send_408(req);
        }
        /* In case of error, returning ESP_FAIL will
         * ensure that the underlying socket is closed */
        return ESP_FAIL;
    }
            // If the request is still not read completely, we cannot process it.
            // if (!req->requestComplete()) {
            //     // res->setStatusCode(413);
            //     // res->setStatusText("Request entity too large");
            //     // res->println("413 Request entity too large");
            //     return sendResponse(req, 413, "application/text", "413 Request entity too large");
            // }
            
            DeserializationError error = deserializeJson(doc, buffer);
            if (error)
            {
                return sendResponse(req, 504, "application/text", "Error deserializing settings json");
            }
            
            JsonObject reqObj = doc.as<JsonObject>();
             if(SettingsHandler::update(reqObj))
            {
                if (SettingsHandler::save()) 
                {
                    // AsyncWebServerResponse *response = req->beginResponse(200, "application/json", "{\"msg\":\"done\"}");
                    return sendResponse(req, 200, "application/json", "{\"msg\":\"done\"}");
                } 
                else 
                {
                    // AsyncWebServerResponse *response = req->beginResponse(200, "application/json", "{\"msg\":\"Error saving settings\"}");
                    return sendResponse(req, 200, "application/json", "{\"msg\":\"Error saving settings\"}");
                }
            }
            else
            {
                // AsyncWebServerResponse *response = req->beginResponse(400, "application/json", "{\"msg\":\"Could not parse JSON\"}");
                return sendResponse(req, 400, "application/json", "{\"msg\":\"Could not parse JSON\"}");
            }
            return sendResponse(req, 200, "application/text", "OK");
        });

        // AsyncCallbackJsonWebHandler* settingsUpdateHandler = new AsyncCallbackJsonWebHandler("/settings", [](HTTPRequest * req, HTTPResponse * res, JsonVariant &json)
        // {
        //     Serial.println("API save settings...");
        //     JsonObject jsonObj = json.as<JsonObject>();
        //     if(SettingsHandler::update(jsonObj))
        //     {
        //         if (SettingsHandler::save()) 
        //         {
        //             AsyncWebServerResponse *response = req->beginResponse(200, "application/json", "{\"msg\":\"done\"}");
        //             req->send(response);
        //         } 
        //         else 
        //         {
        //             AsyncWebServerResponse *response = req->beginResponse(200, "application/json", "{\"msg\":\"Error saving settings\"}");
        //             req->send(response);
        //         }
        //     }
        //     else
        //     {
        //         AsyncWebServerResponse *response = req->beginResponse(400, "application/json", "{\"msg\":\"Could not parse JSON\"}");
        //         req->send(response);
        //     }
        // }, 6000U );//Bad req? increase the size.
    }
    
    /**
     * This handler function will try to load the requested resource from SPIFFS's /public folder.
     * 
     * If the method is not GET, it will throw 405, if the file is not found, it will throw 404.
     */
    static esp_err_t handleSPIFFS(httpd_req_t *req) {
        
        // auto buf_len = httpd_req_get_url_query_len(req) + 1;
        // LogHandler::info(_TAG, "Spiffs: %ld", buf_len);
        // char* uri = new char[buf_len];
        // if(httpd_req_get_url_query_str(req, uri, buf_len) != ESP_OK) {
        //     delete[] uri;
        //     return sendResponse(req, 500, "application/text","Error parsing query");
        //     // httpd_resp_send_500(req);
        //     // return ESP_FAIL;
        // }
        LogHandler::info(_TAG, "Spiffs: %s", req->uri);
        SettingsHandler::printFree();
        // We only handle GET here
        if (req->method == HTTP_GET) {
            // Redirect / to /index.html
            std::string reqFile = strcmp(req->uri,"/") == 0 ? "/index-min.html" : std::string(req->uri);
            LogHandler::info(_TAG, "Get File: %s", reqFile.c_str());

            // Try to open the file
            std::string filename = std::string("/www") + reqFile;

            // Check if the file exists
            if (!SPIFFS.exists(filename.c_str())) {
                LogHandler::error(_TAG, "Spiffs file not found: %s", filename.c_str());
                return sendResponse(req, 404, "application/text", filename.c_str());
            }

            File file = SPIFFS.open(filename.c_str());

            // Set length
            httpd_resp_set_hdr(req, "Content-Length", String(file.size()).c_str());

            // Content-Type is guessed using the definition of the contentTypes-table defined above
            int cTypeIdx = 0;
            do {
            if(reqFile.rfind(contentTypes[cTypeIdx][0])!=std::string::npos) {
                httpd_resp_set_hdr(req, "Content-Type", contentTypes[cTypeIdx][1]);
                break;
            }
            cTypeIdx+=1;
            } while(strlen(contentTypes[cTypeIdx][0])>0);

            // Read the file and write it to the response
            uint8_t buffer[256];
            size_t length = 0;
            do {
                length = file.read(buffer, 256);
                httpd_resp_sendstr_chunk(req, (char*)buffer);
            } while (length > 0);

            file.close();
        } else {
            // If there's any body, discard it
            return sendResponse(req, 405, "application/text","Method not allowed");
        }
        return sendResponse(req, 200, "application/text", "OK");
    }
};

const char* HTTPSServer::_TAG = TagHandler::HTTPSHandler;
const char HTTPSServer::contentTypes[7][6][32] = {
        {".html", "text/html"},
        {".css",  "text/css"},
        {".js",   "application/javascript"},
        {".json", "application/json"},
        {".png",  "image/png"},
        {".jpg",  "image/jpg"},
        {"", ""}
    };