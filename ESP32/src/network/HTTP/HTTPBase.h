#pragma once
#include "WebSocketBase.h"
#include "tasks/TaskHandler.h"
class HTTPBase : public TaskHandler::Task
{
public:
    HTTPBase() : Task(TaskHandler::Rates::ONDEMAND) {}
    virtual void setup_http(uint16_t port, WebSocketBase *webSocketHandler, bool apMode) = 0;
    virtual void stop() = 0;
    virtual bool isRunning() = 0;
};