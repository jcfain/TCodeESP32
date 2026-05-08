#ifndef _OPERATING_MODE_HANDLER_H_
#define _OPERATING_MODE_HANDLER_H_

#include "tasks/TaskHandler.h"
#include "settings/ConfigurationHandler.h"
#include "logging/LogHandler.h"
#include "logging/TagHandler.h"

class OperatingModeHandler : public TaskHandler::Task {
private:
    inline static OperatingModeHandler* _singleton = nullptr;
    OperatingMode _currentMode = OperatingMode::STARTUP_MODE;
public:
    OperatingModeHandler() : TaskHandler::Task(TaskHandler::Rates::SLOW) {}

    static void init()
    {
        if (!_singleton)
        {
            _singleton = new OperatingModeHandler();
            TaskHandler::global().add(_singleton);
        }
    }
    static OperatingModeHandler* global()
    {
        return _singleton;
    }

    static OperatingMode getOperatingMode()
    {
        return global()->_currentMode;
    }

    static void setOperatingMode(OperatingMode mode)
    {
        OperatingModeHandler* self = global();
        if (mode == self->_currentMode)
        {
            return;
        }

        switch(mode)
        {
            case OperatingMode::STARTUP_MODE:
                LogHandler::info(Tags::Settings, "Switching to STARTUP_MODE");
                break;
            case OperatingMode::CONFIGURATION_MODE:
                LogHandler::info(Tags::Settings, "Switching to CONFIGURATION_MODE");
                break;
            case OperatingMode::OTA_MODE:
                LogHandler::info(Tags::Settings, "Switching to OTA_MODE");
                break;
            case OperatingMode::NORMAL_OPERATION:
                LogHandler::info(Tags::Settings, "Switching to NORMAL_OPERATION");
                break;
        }
        global()->_currentMode = mode;
    }
    void setup() override
    {
    }

    void loop() override
    {
        switch(_currentMode)
        {
            case OperatingMode::STARTUP_MODE:
                // Handle startup tasks
                break;
            case OperatingMode::CONFIGURATION_MODE:
                // Handle configuration tasks
                break;
            case OperatingMode::OTA_MODE:
                // Handle OTA tasks
                break;
            case OperatingMode::NORMAL_OPERATION:
                // Handle normal operation tasks
                break;
        }
    }
};


#endif // _OPERATING_MODE_HANDLER_H_
