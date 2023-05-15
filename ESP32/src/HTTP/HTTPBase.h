#pragma once
#include "WebSocketBase.h"
class HTTPBase {
    public:
    virtual void setup(int port, WebSocketBase* webSocketHandler, bool apMode);
    virtual void stop();
    virtual bool isRunning();
};