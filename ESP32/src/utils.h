
#pragma once

#include <Arduino.h>

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