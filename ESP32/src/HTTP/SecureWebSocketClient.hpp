#pragma once

#include <HTTPSServer.hpp>
#include <WebsocketHandler.hpp>
#include "../LogHandler.h"
#include "../TagHandler.h"

// The HTTPS Server comes in a separate namespace. For easier use, include it here.
using namespace httpsserver;

#define MAX_CLIENTS 4

using SECURE_WEBSOCKET_MESSAGE_FUNCTION_PTR_T = void (*)(const char* message);

SECURE_WEBSOCKET_MESSAGE_FUNCTION_PTR_T secure_websocket_message_callback = 0;

void setSecureWebSocketMessageCallback(SECURE_WEBSOCKET_MESSAGE_FUNCTION_PTR_T f)
{
    if (f == nullptr) {
        secure_websocket_message_callback = 0;
    } else {
        secure_websocket_message_callback = f;
    }
}

class SecureWebSocketClient : public WebsocketHandler {
public:
    static WebsocketHandler * create();
    void onMessage(WebsocketInputStreambuf * inbuf) override;
    void onClose() override;
private:
    static const char* _TAG;
};

const char* SecureWebSocketClient::_TAG = TagHandler::SecureWebsocketClient;
SecureWebSocketClient* activeClients[MAX_CLIENTS];

WebsocketHandler * SecureWebSocketClient::create() {
    LogHandler::debug(_TAG, "Creating new chat client!");
    SettingsHandler::printFree();
    SecureWebSocketClient* handler = new SecureWebSocketClient();
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if (activeClients[i] == nullptr) {
            activeClients[i] = handler;
            break;
        }
    }
    SettingsHandler::printFree();
    return handler;
}

void SecureWebSocketClient::onClose() {
    SettingsHandler::printFree();
    LogHandler::info(_TAG, "Close wss client!");
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if (activeClients[i] == this) {
            LogHandler::info(_TAG, "Delete client!");
            activeClients[i] = nullptr;
            SettingsHandler::printFree();
        }
    }
}

void SecureWebSocketClient::onMessage(WebsocketInputStreambuf * inbuf) {
    // Get the input message
    std::ostringstream ss;
    ss << inbuf;
    const char* msg = ss.str().c_str();
    //LogHandler::verbose(_TAG, "Secure message recieved: %s", msg);
    if(secure_websocket_message_callback) {
        secure_websocket_message_callback(msg);
        // // Send it back to every client
        // for(int i = 0; i < MAX_CLIENTS; i++) {
        //     if (activeClients[i] != nullptr) {
        //         activeClients[i]->send(msg, SEND_TYPE_TEXT);
        //     }
        // }
    }
}
