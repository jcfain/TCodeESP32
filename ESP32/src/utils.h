/* MIT License

Copyright (c) 2024 Jason C. Fain

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#pragma once

#include <Arduino.h>
#include <sstream>
#include <vector>
#include <math.h>
// #ifdef ESP32
// #include "driver/adc.h"
// #include "esp_adc_cal.h"
// // #include "sp_adc/adc_cali.h"
// #endif

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

// #ifdef ESP32
// adc1_channel_t gpioToADC1(const int& gpioPin) {
//     switch(gpioPin) {
//         case 36:
//             return ADC1_CHANNEL_0;
//         case 37:
//             return ADC1_CHANNEL_1;
//         case 38:
//             return ADC1_CHANNEL_2;
//         case 39:
//             return ADC1_CHANNEL_3;
//         case 32:
//             return ADC1_CHANNEL_4;
//         case 33:
//             return ADC1_CHANNEL_5;
//         case 34:
//             return ADC1_CHANNEL_6;
//         case 35:
//             return ADC1_CHANNEL_7;
//         default: return ADC1_CHANNEL_MAX;
//     }
// }
// #endif

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

bool startsWith(const char* value, const char* startsWith) {
    auto startsWithLen = strlen(startsWith);
    if (startsWithLen && startsWithLen <= strlen(value) && ( strncmp(startsWith,value,startsWithLen) == 0 )) {
        return true;
    }
    return false;
}

bool endsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return false;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return false;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

bool contains(const char* in, const char* contains) {
    return strstr(in, contains) != nullptr;
}

bool match(const char* in, const char* match) {
    return strcmp(in, match) == 0;
}

void appendNewline(char* out, const char* input) {
    strcpy(out, input);
    if(!endsWith(out, "\n")) {;
        strcat(out, "\n");
    }
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