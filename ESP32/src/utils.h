
#pragma once

#include <Arduino.h>
#include <sstream>
#include <vector>
#include <math.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"

int getposition(const char *array, const size_t& size, const char& c)
{
    for (size_t i = 0; i < size; i++)
    {
        if (array[i] == c)
            return (int)i;
    }
    return -1;
}

char* substr(const char* arr, const int& begin, const int& len)
{
    char* res = new char[len + 1];
    for (int i = 0; i < len; i++)
        res[i] = *(arr + begin + i);
    res[len] = {0};
    return res;
}

void strtrim(char* buf) {
    int start = 0; // number of leading spaces
    char* buffer = buf;
    while (*buf && *buf++ == ' ') ++start;
    while (*buf++); // move to end of string
    int end = buf - buffer - 1; 
    while (end > 0 && buffer[end - 1] == ' ') --end; // backup over trailing spaces
    buffer[end] = 0; // remove trailing spaces
    if (end <= start || start == 0) return; // exit if no leading spaces or string is now empty
    buf = buffer + start;
    while ((*buffer++ = *buf++));  // remove leading spaces: K&R
}

double round2(const double &value) {
    return double(int(value * 100 + 0.5) / 100.0);
}
float round2(const float &value) {
    return float(int(value * 100 + 0.5) / 100.0);
}

double mapf(const double& x, const double& in_min, const double& in_max, const double& out_min, const double& out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

adc1_channel_t gpioToADC1(const int& gpioPin) {
    switch(gpioPin) {
        case 36:
            return ADC1_CHANNEL_0;
        case 37:
            return ADC1_CHANNEL_1;
        case 38:
            return ADC1_CHANNEL_2;
        case 39:
            return ADC1_CHANNEL_3;
        case 32:
            return ADC1_CHANNEL_4;
        case 33:
            return ADC1_CHANNEL_5;
        case 34:
            return ADC1_CHANNEL_6;
        case 35:
            return ADC1_CHANNEL_7;
        default: return ADC1_CHANNEL_MAX;
    }
}

bool contains_duplicate(const std::vector<const char*>& values ) {
    for( std::size_t i = 0 ; i < values.size() ; ++i )
        for( std::size_t j = i+1 ; j < values.size() ; ++j ) // for each number after the one at i
            if( values[i] == values[j] ) return true ; // found a duplicate

    return false ;
}

void hexToString(const int &inByte, char* buf) {
    std::stringstream addressString;
    addressString << "0x" << std::hex << inByte;
    strcpy(buf, addressString.str().c_str());
}
int stringToHex(std::string buff) {
    return (int)strtol(buff.c_str(), NULL, 0);
}

struct StrCompare
{
   bool operator()(char const *a, char const *b) const
   {
      return strcmp(a, b) < 0;
   }
};

// adc2_channel_t gpioToADC2(int gpioPinc:\Users\jfain\AppData\Local\Programs\Microsoft VS Code\resources\app\out\vs\code\electron-sandbox\workbench\workbench.html) {
//     switch(gpioPin) {
//         case 4:
//             return ADC2_CHANNEL_0;
//         case 0:
//             return ADC2_CHANNEL_1;
//         case 2:
//             return ADC2_CHANNEL_2;
//         case 15:
//             return ADC2_CHANNEL_3;
//         case 13:
//             return ADC2_CHANNEL_4;
//         case 12:
//             return ADC2_CHANNEL_5;
//         case 14:
//             return ADC2_CHANNEL_6;
//         case 27:
//             return ADC2_CHANNEL_7;
//         case 25:
//             return ADC2_CHANNEL_8;
//         case 26:
//             return ADC2_CHANNEL_9;
//         default: return ADC2_CHANNEL_MAX;
//     }
// }