
    #pragma once
    //#include "math.h"
    #include <stddef.h>
class TCodeInterface
    {
    public:
        virtual size_t read(char* buf) = 0;
        virtual void send(const char* buf) = 0;
        virtual size_t available() = 0;
    };