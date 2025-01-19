
#pragma once

#include <HTTPSServer.hpp>
#include <WebsocketHandler.hpp>
#include "WebSocketBase.h"
#include "SettingsHandler.h"
#include "LogHandler.h"
#include "TagHandler.h"
#include "BatteryHandler.h"
#include "SecureWebSocketClient.hpp"


// As websockets are more complex, they need a custom class that is derived from WebsocketHandler
class SecureWebSocketHandler : public WebSocketBase {
public:
    void setup(HTTPSServer& httpsServer) {
        LogHandler::info(_TAG, "Setting up secure webSocket");
        tCodeInQueue = xQueueCreate(5, sizeof(char[255]));
        if(!tCodeInQueue || tCodeInQueue == NULL) {
            LogHandler::error(_TAG, "Error creating the tcode queue");
        }
        // Initialize the slots
        for(int i = 0; i < MAX_CLIENTS; i++) activeClients[i] = nullptr;
        instanceRef = this;
        setSecureWebSocketMessageCallback(onMessage);
        isInitialized = true;
        httpsServer.registerNode(new WebsocketNode("/ws", &SecureWebSocketClient::create));
        LogHandler::info(_TAG, "Ready");
    }

    static void onMessage(const char* message) {
        instance()->processWebSocketTextMessage(message);
    }

    void setMessageCallback(SECURE_WEBSOCKET_MESSAGE_FUNCTION_PTR_T f) // Sets the callback function used by TCode
    {
        if (f == nullptr) {
            secure_websocket_message_callback = 0;
        } else {
            secure_websocket_message_callback = f;
        }
    }

    void CommandCallback(const char* in) override 
    { //This overwrites the callback for message return
        if(isInitialized && hasClients())
            sendCommand(in);
    }

    void sendCommand(const char* command, const char* message = 0) override
    {
        if(isInitialized && command_mtx.try_lock()) {
            std::lock_guard<std::mutex> lck(command_mtx, std::adopt_lock);
            m_lastSend = millis();

            char commandJson[255];
            compileCommand(commandJson, command, message);
            // if(client)
            //     send(commandJson, SEND_TYPE_TEXT);
            // else
                sendText(commandJson);
        }
    }

    void closeAll() override
    {
        for(int i = 0; i < MAX_CLIENTS; i++) {
            activeClients[i]->close();
        }
    }
private: 
    static const char* _TAG;
    static int m_lastSend;
    static SecureWebSocketHandler* instanceRef;

    void sendText(const char* message) {
        for(int i = 0; i < MAX_CLIENTS; i++) {
        if (activeClients[i] != nullptr) {
            activeClients[i]->send(message, activeClients[i]->SEND_TYPE_TEXT);
        }
    }
    }
    bool hasClients() {
            for(int i = 0; i < MAX_CLIENTS; i++) {
                if(activeClients[i] != nullptr)
                    return true;
            }
        return false;
    }

    static SecureWebSocketHandler* instance() {
        return instanceRef;
    }
};
const char* SecureWebSocketHandler::_TAG = TagHandler::SecureWebsocketsHandler;
SecureWebSocketHandler* SecureWebSocketHandler::instanceRef;
int SecureWebSocketHandler::m_lastSend = 0;
