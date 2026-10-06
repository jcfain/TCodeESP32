/* MIT License

Copyright (c) 2026 Jason C. Fain

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

int getposition(const char *array, const size_t& size, const char& c);

void substr(char* out, const char* in, const int& begin, const int& len);

void strtrim(char* buf);

double round2(const double &value);
float round2(const float &value);

double mapf(const double& x, const double& in_min, const double& in_max, const double& out_min, const double& out_max);

// #ifdef ESP32
// adc1_channel_t gpioToADC1(const int& gpioPin);
// #endif

bool contains_duplicate(const std::vector<const char*>& values );

void hexToString(const int &inByte, char* buf);

int stringToHex(std::string buff);

bool startsWith(const char* value, const char* startsWith);

bool endsWith(const char *str, const char *suffix);

bool contains(const char* in, const char* contains);

bool match(const char* in, const char* match);

void appendNewline(char* out, const char* input);

struct StrCompare
{
   bool operator()(char const *a, char const *b) const
   {
      return strcmp(a, b) < 0;
   }
};

struct Chunker {
    Chunker(const char* in, const size_t& len, const size_t& maxLen) : in(in), len(len), maxLen(maxLen), sent(0) { }

    // size_t getChunkSize() {
    //     if(len > maxLen) 
    //     {
    //         int mod = len % maxLen;
    //         int chunkTotal = len - mod;
    //         int sendChunks = chunkTotal / maxLen;
    //         int chunkAmount = chunkTotal / sendChunks;
    //         return chunkAmount;
    //     }
    //     return len;
    // }
    
    size_t operator()(char* out) 
    {
        if(sent < len)
        {
            int maxLenMinusNewLine = maxLen - 1;
            if(len > maxLenMinusNewLine) 
            {
                int mod = len % maxLenMinusNewLine;
                int chunkTotal = len - mod;
                int fullChunkCount = chunkTotal / maxLenMinusNewLine;
                int chunkAmount = chunkTotal / fullChunkCount;
                // Serial.printf("[Chunker] len: %i, fullChunkCount: %i, chunkAmount: %i\n", len, fullChunkCount, chunkAmount);
                // if(mod)
                //     Serial.printf("[Chunker] left over mod: %i, totalChunks: %i\n", mod, fullChunkCount +1);
                // Serial.printf("[Chunker] sent: %i\n", sent);
                if(len - sent > mod)
                {
                    strncpy(out, in + sent, chunkAmount);
                    out[chunkAmount] = '\0';
                    sent += chunkAmount;
                    Serial.printf("[Chunker] truncated: %s\n", out);
                    // Serial.printf("[Chunker] sent: %i\n", sent);
                    return chunkAmount;
                }
                else if(mod)
                {
                    strncpy(out, in + sent, mod);
                    out[mod] = '\0';
                    strcat(out, "\n"); // qwen3.8 27b said:  // If out buffer is exactly (mod+1), this overflows. 
                    // Pretty I removed all usage of this function when I created it. Ignore until its used gain. I think it was working correct...
                    sent += mod;
                    Serial.printf("[Chunker] truncated mod: %i, message: %s\n", mod, out);
                    // Serial.printf("[Chunker] sent: %i\n", sent);
                    return mod +1;
                }
                else
                {
                    out[0] = '\0';
                    strcat(out, "\n");
                    return 2;
                }
            } 
            else 
            {
                strncpy(out, in, len);
                out[len] = '\0';
                sent += len;
                strcat(out, "\n");
                return len +1;
            }
        }
        return 0;
    }

private:
    const char* in;
    const size_t len;
    const size_t maxLen;
    size_t sent = 0;
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
