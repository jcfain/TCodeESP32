#pragma once
#include <Arduino.h>
#include "LogHandler.h"

class PrintX: public Print {
    public:

    static PrintX* getInstance()
    {
        static PrintX print;
        return &print;
    }
    
    size_t write(uint8_t buffer)
    {
        if (buffer == 0) {
            return 0;
        }
        // Serial.printf("write(uint8_t buffer): %s\n", buffer);
        return LogHandler::info("PrintX", (const char*)&buffer);
    }

    size_t write(const uint8_t *buffer, size_t size)
    {
        // Serial.printf("write(const uint8_t *buffer, size_t size): %s size: %i\n", buffer, size);
        return LogHandler::info("PrintX", (const char *)buffer);
    }
};