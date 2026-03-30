#pragma once

#include <Arduino.h>

class BenchHandler
{
public:
    static BenchHandler* getInstance() {
        static BenchHandler instance;
        return &instance;
    }
    void init() 
    {
        
    }
    void enable()
    {
        benchEnable = true;
    }
    void disable()
    {
        benchEnable = false;
    }
    void benchStart(int benchNumber)
    {
        if(!benchEnable)
            return;
        if (benchEnable || (benchEnableZero && benchNumber == 0))
            bench[benchNumber] = micros();
    }
    void benchFinish(const char *systemUnderBench, int benchNumber)
    {
        if(!benchEnable)
            return;
        if (benchEnable || (benchEnableZero && benchNumber == 0))
        {
            unsigned long timeTaken = micros() - bench[benchNumber];
            if (timeTaken > benchThreshHold)
            {
                Serial.printf("%s:							%lu\n", systemUnderBench, timeTaken);
                bench[benchNumber] = 0;
                benchLast[benchNumber] = timeTaken;
            }
        }
    }
private:
    unsigned long bench[10];
    unsigned long benchLast[10];
    bool benchEnable = false;
    bool benchEnableZero = false;
    unsigned long benchThreshHold = 1300;
};