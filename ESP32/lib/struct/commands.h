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

class Commands {
public:
    const char DELEMITER = '#';
    const char DELEMITER_SAVE = '$'; 
    const char DELEMITER_VALUE = ':'; 
    const char* HELP = "#help";
    const char* SAVE = "$save";
    const char* DEFAULT_ALL = "$defaultAll";
    const char* RESTART = "#restart";
    const char* CLEAR_LOGS_INCLUDE = "#clear-log-include";
    const char* CLEAR_LOGS_EXCLUDE = "#clear-log-exclude";
    const char* MOTION_ENABLE = "#motion-enable";
    const char* MOTION_DISABLE = "#motion-disable";
    const char* MOTION_TOGGLE = "#motion-toggle";
    const char* SAVE = "#motion-profile-cycle";
    const char* SAVE = "#pause-toggle";
    const char* SAVE = "#wifi-ssid";
    const char* SAVE = "#wifi-pass";
    const char* SAVE = "#log-level";
    const char* SAVE = "#add-log-include";
    const char* SAVE = "#remove-log-include";
    const char* SAVE = "#add-log-exclude";
    const char* SAVE = "#remove-log-exclude";
    const char* SAVE = "$save";
    const char* SAVE = "$save";
    const char* SAVE = "$save";
    const char* SAVE = "$save";
    const char* SAVE = "$save";
    const char* SAVE = "$save";
    const char* SAVE = "$save";
    const char* SAVE = "$save";
    const char* SAVE = "$save";
    const char* SAVE = "$save";
    const char* SAVE = "$save";
    const char* SAVE = "$save";
    const char* SAVE = "$save";
    const char* SAVE = "$save";
    const char* SAVE = "$save";
    
};