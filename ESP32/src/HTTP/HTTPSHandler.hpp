
#pragma once
// Include certificate data (see note above)
#include "cert.h"
#include "private_key.h"

// Binary data for the favicon
//#include "favicon.h"


// Includes for the server
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <ArduinoJson.h>

#include "HTTPBase.h"
#include "SecureWebSocketHandler.hpp"
#include "SettingsHandler.h"
#include "TagHandler.h"
#include "LogHandler.h"
#include "SystemCommandHandler.h"

// The HTTPS Server comes in a separate namespace. For easier use, include it here.
using namespace httpsserver;
class HTTPSHandler : public HTTPBase {
public:
    void setup(int port, WebSocketBase* webSocketHandler, bool apMode) override {

        setupHandlers(webSocketHandler, apMode);
        // The websocket handler can be linked to the server by using a WebsocketNode:
        // (Note that the standard defines GET as the only allowed method here,
        // so you do not need to pass it explicitly)
        static_cast<SecureWebSocketHandler*>(webSocketHandler)->setup(secureServer);

        LogHandler::info(_TAG, "Starting secure https server...");
        secureServer.start();
        if (secureServer.isRunning()) {
            LogHandler::info(_TAG, "Server ready.");
            m_connected = true;
        }
    }

    void stop() override {
        m_connected = false;
        secureServer.stop();
    }

    bool isRunning() override {
        return m_connected;
    }
	static void startLoop(void* ref)
	{
		//LogHandler::debug(TagHandler::DisplayHandler, "Starting loop");
		//if(((DisplayHandler*)displayHandlerRef)->isConnected())
			((HTTPSHandler*)ref)->loop();
	}

    void loop() {
        while(m_connected) {
            // This call will let the server do its work
            secureServer.loop();

        	vTaskDelay(1/portTICK_PERIOD_MS);
        }
        
  		vTaskDelete( NULL );
    }

//     static void handleRoot(HTTPRequest * req, HTTPResponse * res) {
//         // Status code is 200 OK by default.
//         // We want to deliver a simple HTML page, so we send a corresponding content type:
//         res->setHeader("Content-Type", "text/html");
// //res->
//             //server->serveStatic("/", SPIFFS, "/www/").setDefaultFile("index-min.html");
//         // The response implements the Print interface, so you can use it just like
//         // you would write to Serial etc.
//         res->print("<!DOCTYPE HTML>\n"
//     "<html>\n"
//     "   <head>\n"
//     "   <title>ESP32 Chat</title>\n"
//     "</head>\n"
//     "<body>\n"
//     "    <div style=\"width:500px;border:1px solid black;margin:20px auto;display:block\">\n"
//     "        <form onsubmit=\"return false\">\n"
//     "            Your Name: <input type=\"text\" id=\"txtName\" value=\"ESP32 user\">\n"
//     "            <button type=\"submit\" id=\"btnConnect\">Connect</button>\n"
//     "        </form>\n"
//     "        <form onsubmit=\"return false\">\n"
//     "            <div style=\"overflow:scroll;height:400px\" id=\"divOut\">Not connected...</div>\n"
//     "            Your Message: <input type=\"text\" id=\"txtChat\" disabled>\n"
//     "            <button type=\"submit\" id=\"btnSend\" disabled>Send</button>\n"
//     "        </form>\n"
//     "    </div>\n"
//     "    <script type=\"text/javascript\">\n"
//     "        const elem = id => document.getElementById(id);\n"
//     "        const txtName = elem(\"txtName\");\n"
//     "        const txtChat = elem(\"txtChat\");\n"
//     "        const btnConnect = elem(\"btnConnect\");\n"
//     "        const btnSend = elem(\"btnSend\");\n"
//     "        const divOut = elem(\"divOut\");\n"
//     "\n"
//     "        class Chat {\n"
//     "            constructor() {\n"
//     "                this.connecting = false;\n"
//     "                this.connected = false;\n"
//     "                this.name = \"\";\n"
//     "                this.ws = null;\n"
//     "            }\n"
//     "            connect() {\n"
//     "                if (this.ws === null) {\n"
//     "                    this.connecting = true;\n"
//     "                    txtName.disabled = true;\n"
//     "                    this.name = txtName.value;\n"
//     "                    btnConnect.innerHTML = \"Connecting...\";\n"
//     "                    this.ws = new WebSocket(\"wss://\" + document.location.host + \"/chat\");\n"
//     "                    this.ws.onopen = e => {\n"
//     "                        this.connecting = false;\n"
//     "                        this.connected = true;\n"
//     "                        divOut.innerHTML = \"<p>Connected.</p>\";\n"
//     "                        btnConnect.innerHTML = \"Disconnect\";\n"
//     "                        txtChat.disabled=false;\n"
//     "                        btnSend.disabled=false;\n"
//     "                        this.ws.send(this.name + \" joined!\");\n"
//     "                    };\n"
//     "                    this.ws.onmessage = e => {\n"
//     "                        divOut.innerHTML+=\"<p>\"+e.data+\"</p>\";\n"
//     "                        divOut.scrollTo(0,divOut.scrollHeight);\n"
//     "                    }\n"
//     "                    this.ws.onclose = e => {\n"
//     "                        this.disconnect();\n"
//     "                    }\n"
//     "                }\n"
//     "            }\n"
//     "            disconnect() {\n"
//     "                if (this.ws !== null) {\n"
//     "                    this.ws.send(this.name + \" left!\");\n"
//     "                    this.ws.close();\n"
//     "                    this.ws = null;\n"
//     "                }\n"
//     "                if (this.connected) {\n"
//     "                    this.connected = false;\n"
//     "                    txtChat.disabled=true;\n"
//     "                    btnSend.disabled=true;\n"
//     "                    txtName.disabled = false;\n"
//     "                    divOut.innerHTML+=\"<p>Disconnected.</p>\";\n"
//     "                    btnConnect.innerHTML = \"Connect\";\n"
//     "                }\n"
//     "            }\n"
//     "            sendMessage(msg) {\n"
//     "                if (this.ws !== null) {\n"
//     "                    this.ws.send(this.name + \": \" + msg);\n"
//     "                }\n"
//     "            }\n"
//     "        };\n"
//     "        let chat = new Chat();\n"
//     "        btnConnect.onclick = () => {\n"
//     "            if (chat.connected) {\n"
//     "                chat.disconnect();\n"
//     "            } else if (!chat.connected && !chat.connecting) {\n"
//     "                chat.connect();\n"
//     "            }\n"
//     "        }\n"
//     "        btnSend.onclick = () => {\n"
//     "            chat.sendMessage(txtChat.value);\n"
//     "            txtChat.value=\"\";\n"
//     "            txtChat.focus();\n"
//     "        }\n"
//     "    </script>\n"
//     "</body>\n"
//     "</html>\n");
//     }

    static void handleFavicon(HTTPRequest * req, HTTPResponse * res) {
        // // Set Content-Type
        // res->setHeader("Content-Type", "image/vnd.microsoft.icon");
        // // Write data from header file
        // res->write(FAVICON_DATA, FAVICON_LENGTH);
    }


private:

    static const char* _TAG;
    // Create an SSL certificate object from the files included above
    SSLCert cert = SSLCert(
        example_crt_DER, example_crt_DER_len,
        example_key_DER, example_key_DER_len
    );
    bool m_connected = false;
    // Create an SSL-enabled server that uses the certificate
    // The contstructor takes some more parameters, but we go for default values here.
    HTTPSServer secureServer = HTTPSServer(&cert);
    
    static const char contentTypes[7][6][32];

    static void beginResponse(HTTPResponse * res, int code, const char* contentType, const char* message) {
        res->setStatusCode(code);
        res->setHeader("Content-Type", contentType);
        res->setHeader("Content-Length", httpsserver::intToString(strlen(message)));
        //res->setStatusText(message);
        res->println(message);
    }

    void setupHandlers(WebSocketBase* webSocketHandler, bool apMode) {
        
        // For every resource available on the server, we need to create a ResourceNode
        // The ResourceNode links URL and HTTP method to a handler function
        ResourceNode * nodeRoot    = new ResourceNode("/", "GET", &handleSPIFFS);
        ResourceNode * nodeFavicon = new ResourceNode("/favicon.ico", "GET", &handleFavicon);
        ResourceNode * spiffsNode = new ResourceNode("", "", &handleSPIFFS);
        
        secureServer.registerNode(nodeRoot);
        secureServer.setDefaultNode(spiffsNode);
        secureServer.registerNode(nodeFavicon);
        
        secureServer.registerNode(new ResourceNode("/userSettings", "GET", [](HTTPRequest * req, HTTPResponse * res) 
        {
            LogHandler::verbose(_TAG, "Get userSettings");
            SettingsHandler::printFree();
            ////req->send(SPIFFS, "/userSettings.json");
            // DynamicJsonDocument doc(SettingsHandler::deserialize);
            // File file = SPIFFS.open(SettingsHandler::userSettingsFilePath, "r");
            // DeserializationError error = deserializeJson(doc, file);
            char settings[40000];
            SettingsHandler::serialize(settings);
            if (strlen(settings) == 0) {
                //AsyncWebServerResponse *response = req->beginResponse(504, "application/text", "Error getting user settings");
                HTTPSHandler::beginResponse(res, 504, "application/text", "Error getting user settings");
                return;
            }
            // if(strcmp(doc["wifiPass"], SettingsHandler::defaultWifiPass) != 0 )
            //     doc["wifiPass"] = "Too bad haxor!";// Do not send password if its not default
                
            // doc["lastRebootReason"] = SettingsHandler::lastRebootReason;
            // String output;
            // serializeJson(doc, output);
            //AsyncWebServerResponse *response = req->beginResponse(200, "application/json", settings);
            HTTPSHandler::beginResponse(res, 200, "application/json", settings);
        }));   
        secureServer.registerNode(new ResourceNode("/systemInfo", "GET", [](HTTPRequest * req, HTTPResponse * res) 
        {
            LogHandler::verbose(_TAG, "systemInfo");
            SettingsHandler::printFree();
            char systemInfo[1024];
            SettingsHandler::getSystemInfo(systemInfo);
            if (strlen(systemInfo) == 0) {
                //AsyncWebServerResponse *response = req->beginResponse(504, "application/text", "Error getting user settings");
                HTTPSHandler::beginResponse(res, 504, "application/text", "Error getting user settings");
                return;
            }
            //AsyncWebServerResponse *response = req->beginResponse(200, "application/json", systemInfo);
                HTTPSHandler::beginResponse(res, 200, "application/json", systemInfo);
        }));   
        
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

        secureServer.registerNode(new ResourceNode("/restart", "POST", [](HTTPRequest * req, HTTPResponse * res)
        {
            LogHandler::verbose(_TAG, "restart");
            SettingsHandler::printFree();
            //if(apMode) {
                //req->send(200, "text/plain",String("Restarting device, wait about 10-20 seconds and navigate to ") + (SettingsHandler::hostname) + ".local or the network IP address in your browser address bar.");
            //}
            String message = "{\"msg\":\"restarting\",\"apMode\":false}";
            //message += apMode ? "true}" : "false}";
            //AsyncWebServerResponse *response = req->beginResponse(200, "application/json", message);
            HTTPSHandler::beginResponse(res, 200, "application/json", message.c_str());
            delay(2000);
            //webSocketHandler->closeAll();
            SystemCommandHandler::restart();
        }));

        secureServer.registerNode(new ResourceNode("/default", "POST", [](HTTPRequest * req, HTTPResponse * res)
        {
            Serial.println("Settings default");
            SettingsHandler::defaultAll();
        }));
        secureServer.registerNode(new ResourceNode("/settings", "POST", [](HTTPRequest * req, HTTPResponse * res)
        {
            LogHandler::verbose(_TAG, "Puot settings");
            SettingsHandler::printFree();
            const int capacity = SettingsHandler::getDeserializeSize();
            DynamicJsonDocument doc(capacity);

            // Create buffer to read request
            char buffer[capacity + 1];
            memset(buffer, 0, capacity+1);

            // Try to read request into buffer
            size_t idx = 0;
            while (!req->requestComplete() && idx < capacity) {
                idx += req->readChars(buffer + idx, capacity-idx);
            }
            // If the request is still not read completely, we cannot process it.
            if (!req->requestComplete()) {
                // res->setStatusCode(413);
                // res->setStatusText("Request entity too large");
                // res->println("413 Request entity too large");
                HTTPSHandler::beginResponse(res, 413, "application/text", "413 Request entity too large");
                // Clean up
                return;
            }
            
            DeserializationError error = deserializeJson(doc, buffer);
            if (error)
            {
                HTTPSHandler::beginResponse(res, 504, "application/text", "Error deserializing settings json");
                return;
            }
            
            JsonObject reqObj = doc.as<JsonObject>();
             if(SettingsHandler::update(reqObj))
            {
                if (SettingsHandler::save()) 
                {
                    // AsyncWebServerResponse *response = req->beginResponse(200, "application/json", "{\"msg\":\"done\"}");
                    HTTPSHandler::beginResponse(res, 200, "application/json", "{\"msg\":\"done\"}");
                } 
                else 
                {
                    // AsyncWebServerResponse *response = req->beginResponse(200, "application/json", "{\"msg\":\"Error saving settings\"}");
                    HTTPSHandler::beginResponse(res, 200, "application/json", "{\"msg\":\"Error saving settings\"}");
                }
            }
            else
            {
                // AsyncWebServerResponse *response = req->beginResponse(400, "application/json", "{\"msg\":\"Could not parse JSON\"}");
                HTTPSHandler::beginResponse(res, 400, "application/json", "{\"msg\":\"Could not parse JSON\"}");
            }
        }));

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
    static void handleSPIFFS(HTTPRequest * req, HTTPResponse * res) {
        LogHandler::verbose(_TAG, "Spiffs: %s", req->getRequestString().c_str());
        SettingsHandler::printFree();
        // We only handle GET here
        if (req->getMethod() == "GET") {
            // Redirect / to /index.html
            std::string reqFile = req->getRequestString()=="/" ? "/index-min.html" : req->getRequestString();
            LogHandler::verbose(_TAG, "Get File: %s", reqFile.c_str());

            // Try to open the file
            std::string filename = std::string("/www") + reqFile;

            // Check if the file exists
            if (!SPIFFS.exists(filename.c_str())) {
                LogHandler::error(_TAG, "Spiffs file not found: %s", filename.c_str());
                HTTPSHandler::beginResponse(res, 404, "application/text", filename.c_str());
                return;
            }

            File file = SPIFFS.open(filename.c_str());

            // Set length
            res->setHeader("Content-Length", httpsserver::intToString(file.size()));

            // Content-Type is guessed using the definition of the contentTypes-table defined above
            int cTypeIdx = 0;
            do {
            if(reqFile.rfind(contentTypes[cTypeIdx][0])!=std::string::npos) {
                res->setHeader("Content-Type", contentTypes[cTypeIdx][1]);
                break;
            }
            cTypeIdx+=1;
            } while(strlen(contentTypes[cTypeIdx][0])>0);

            // Read the file and write it to the response
            uint8_t buffer[256];
            size_t length = 0;
            do {
                length = file.read(buffer, 256);
                res->write(buffer, length);
            } while (length > 0);

            file.close();
        } else {
            // If there's any body, discard it
            req->discardRequestBody();
            // Send "405 Method not allowed" as response
            res->setStatusCode(405);
            res->setStatusText("Method not allowed");
            res->println("405 Method not allowed");
        }
    }
};
const char* HTTPSHandler::_TAG = TagHandler::HTTPSHandler;
const char HTTPSHandler::contentTypes[7][6][32] = {
        {".html", "text/html"},
        {".css",  "text/css"},
        {".js",   "application/javascript"},
        {".json", "application/json"},
        {".png",  "image/png"},
        {".jpg",  "image/jpg"},
        {"", ""}
    };