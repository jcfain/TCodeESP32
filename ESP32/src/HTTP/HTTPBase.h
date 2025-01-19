#pragma once
#include "WebSocketBase.h"
class HTTPBase {
    public:
    virtual void setup(uint16_t port, WebSocketBase* webSocketHandler, bool apMode) = 0;
    virtual void stop() = 0;
    virtual bool isRunning() = 0;
};