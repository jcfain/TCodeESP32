#pragma once
#include <Arduino.h>
#include "LogHandler.h"

// Adapts the Arduino Print interface onto LogHandler so libraries that take a
// Print* can log through the normal level/tag filtering.
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
        char c[2] = {(char)buffer, '\0'};
        LogHandler::info(Tags::Main, "%s", c);
        return 1;
    }

    size_t write(const uint8_t *buffer, size_t size)
    {
        LogHandler::info(Tags::Main, "%.*s", (int)size, (const char *)buffer);
        return size;
    }
};
