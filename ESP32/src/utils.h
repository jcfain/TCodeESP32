
#pragma once

#include <Arduino.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"

int getposition(const char *array, size_t size, char c)
{
    for (size_t i = 0; i < size; i++)
    {
        if (array[i] == c)
            return (int)i;
    }
    return -1;
}

char* substr(const char* arr, int begin, int len)
{
    char* res = new char[len + 1];
    for (int i = 0; i < len; i++)
        res[i] = *(arr + begin + i);
    res[len] = 0;
    return res;
}

void strtrim(char* str) {
    int start = 0; // number of leading spaces
    char* buffer = str;
    while (*str && *str++ == ' ') ++start;
    while (*str++); // move to end of string
    int end = str - buffer - 1; 
    while (end > 0 && buffer[end - 1] == ' ') --end; // backup over trailing spaces
    buffer[end] = 0; // remove trailing spaces
    if (end <= start || start == 0) return; // exit if no leading spaces or string is now empty
    str = buffer + start;
    while ((*buffer++ = *str++));  // remove leading spaces: K&R
}

double round2(double value) {
   return (int)(value * 100 + 0.5) / 100.0;
}

double mapf(double x, double in_min, double in_max, double out_min, double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

adc1_channel_t gpioToADC1(int gpioPin) {
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
// adc2_channel_t gpioToADC2(int gpioPin) {
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