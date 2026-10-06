
#pragma once

#include <Print.h>
#include "LogHandler.h"
#include "settingsFactory.h"
#include "utils.h"

template <unsigned int bufferLen>
class PrintX: public Print {
public:

    PrintX() {}
    
    size_t write(uint8_t buffer) override
    {
        if (buffer == 0)
            return 0;
        return writeBuffer((const char*)&buffer, 1);
    }

    size_t write(const uint8_t *buffer, size_t size) override
    {
        if (buffer == 0)
            return 0;
        return writeBuffer((const char*)buffer, size);
    }
protected:
    virtual size_t onWrite(const char* buffer, size_t len) = 0;

private:
    // const static size_t m_outbufferLen = 256;
    // size_t m_outbufferCurrentLen = 0;
    char m_outbuffer[bufferLen] = {0};

    
    size_t writeBuffer(const char* buffer, size_t len)
    {
        bool isNewLine = strcmp(buffer, "\r\n") == 0;
        int currentBufferLen = strlen(m_outbuffer);
        // LogHandler::info("PrintX", "content: %s, len: %i, current bufferLen: %i", buffer, len, strlen(m_outbuffer));
        if(!isNewLine && (currentBufferLen + len) < bufferLen)// How to handle larger values if its even an issue here.
            strncat(m_outbuffer, buffer, len);
        currentBufferLen = strlen(m_outbuffer);
        // if(strcmp(buffer, "\r\n") == 0 || strcmp(buffer, "\n") == 0 || strlen(m_outbuffer) == bufferLen) 
        if(isNewLine || currentBufferLen == bufferLen)
        {
            // SettingsFactory::getInstance()->addMotorStatus(m_name, m_outbuffer);
            // LogHandler::info(m_name, m_outbuffer);

            onWrite(m_outbuffer, currentBufferLen);
            m_outbuffer[0] = {0};
        }
        // if(LogHandler::getLogLevel() >= LogLevel::INFO)
        //     LogHandler::raw(buffer);
        return len;
    }

};