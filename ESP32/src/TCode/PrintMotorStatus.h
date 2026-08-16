
#pragma once

#include "PrintX.h"

template <unsigned int bufferLen>
class PrintMotorStatus: public PrintX<bufferLen> {
    public:

    PrintMotorStatus(const char* name) : m_name(name) {}

    void setName(const char* name) 
    {
        m_name = name;
    }

    size_t onWrite(const char* buffer, size_t len) override
    {
        // if((strlen(this->m_outbuffer) + len) < bufferLen)// How to handle larger values if its even an issue here.
        //     strncat(this->m_outbuffer, buffer, len);
        // if(strcmp(buffer, "\r\n") == 0 || strlen(this->m_outbuffer) == bufferLen) 
        // {
            SettingsFactory::getInstance()->addMotorStatus(m_name, buffer);
            LogHandler::info("PrintMotorStatus", "[%s] %s", m_name, buffer);
        //     this->m_outbuffer[bufferLen] = {0};
        // }
        // // if(LogHandler::getLogLevel() >= LogLevel::INFO)
        // //     LogHandler::raw(buffer);
        return len;
    }
private:
    const char* m_name;
};