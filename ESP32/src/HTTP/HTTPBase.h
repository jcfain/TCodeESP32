#pragma once
#include "WebSocketBase.h"
#include "TaskHandler.h"
class HTTPBase : public Task {
    public:
    virtual void setup_http(uint16_t port, WebSocketBase* webSocketHandler, bool apMode) = 0;
    virtual void stop() = 0;
    virtual bool isRunning() = 0;
};