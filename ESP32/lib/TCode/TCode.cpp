// TCODE LIBRARY
// by TempestMAx 1-6-26
// This class acts as a handler for all TCode axes
// v0.1 Experimental build
//
//
// MIT License
//
// Copyright (c) 2021 Richard Unger
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#include "Axis.h"       // TCode Axis library
#include "TCode.h"      // TCode library header
#include <ctype.h>      // toupper, isdigit
#include <stdlib.h>     // strtol
#include <string.h>     // strlen

// Constructor
TCode::TCode() = default;  // nothing to initialize beyond the = {nullptr} in the header

// Function to register the address of each new axis
bool TCode::addAxis(const char* channel, Axis& axis) {
    char letter;
    uint8_t index;
    if (!parseChannel(channel, letter, index)) return false;     // invalid channel like "X5"

    Axis** target = nullptr;                                     // pointer-to-pointer: we'll decide which array
    switch (letter) {
        case 'L': target = _Lin; break;
        case 'R': target = _Rot; break;
        case 'V': target = _Vib; break;
        case 'A': target = _Aux; break;
    }
    if (target[index] != nullptr) return false;                  // already occupied

    target[index] = &axis;                                       // store the address of the caller's Axis
    _count++;                                                    // just for your convenience
    return true;
}

Axis* TCode::Lin(uint8_t index) const { return (index < 10) ? _Lin[index] : nullptr; }
Axis* TCode::Rot(uint8_t index) const { return (index < 10) ? _Rot[index] : nullptr; }
Axis* TCode::Vib(uint8_t index) const { return (index < 10) ? _Vib[index] : nullptr; }
Axis* TCode::Aux(uint8_t index) const { return (index < 10) ? _Aux[index] : nullptr; }

// Function to get a pointer to a named axis
Axis* TCode::getAxis(const char* channel) const {
    char letter;
    uint8_t index;
    if (!parseChannel(channel, letter, index)) return nullptr;
    switch (letter) {
        case 'L': return Lin(index);
        case 'R': return Rot(index);
        case 'V': return Vib(index);
        case 'A': return Aux(index);
    }
    return nullptr;
}

// Function to feed in incoming data for interpretation one byte at a time
void TCode::byteInput(uint8_t byte) {
    if (byte == ' ' || byte == '\n' || byte == '\r') {
        if (_cmdLen > 0) {
            _cmdBuffer[_cmdLen] = '\0';
            processCommand(_cmdBuffer);
            _cmdLen = 0;
        }
        if (byte == '\n' || byte == '\r') {
            executeAll();
        }
    } else if (_cmdLen < 47) {
        _cmdBuffer[_cmdLen++] = (char)byte;
    }
}

// Function to feed in incoming data for interpretation as a string
void TCode::stringInput(const char* str) {
    if (!str) return;
    while (*str) {
        byteInput((uint8_t)*str);
        ++str;
    }
}

// Output buffer availablity function, similar to the Serial buffer
size_t TCode::available() const {
    return _outputLen - _outputPos;
}

// Output buffer read function, similar to the Serial buffer
int TCode::read() {
    if (_outputPos >= _outputLen) return -1;
    return (unsigned char)_outputBuffer[_outputPos++];
}

// Function to stop all axes
void TCode::stopAll() {
    for (uint8_t i = 0; i < 10; ++i) {
        if (_Lin[i]) _Lin[i]->stop();
        if (_Rot[i]) _Rot[i]->stop();
        if (_Vib[i]) _Vib[i]->setPos(0);   // Vibration channels go to zero
        if (_Aux[i]) _Aux[i]->stop();
    }
}

void TCode::setTCodeCallback(TCodeCallback f)
{
    if (f == nullptr)
    {
        tcode_callback = 0;
    }
    else
    {
        tcode_callback = f;
    }
}

//Top-level command dispatcher
void TCode::processCommand(const char* cmd) {
    if (!cmd || strlen(cmd) == 0) return;

    char first = toupper(cmd[0]);

    if (first == 'L' || first == 'R' || first == 'V' || first == 'A') {
        processAxisCommand(cmd);
    } else if (first == 'D') {
        processDeviceCommand(cmd);
    // } else if (first == '$') {
    //     processSaveCommand(cmd);
    } else {
        if(tcode_callback)
            tcode_callback(cmd);
    }
}

// Function to process axis commands
void TCode::processAxisCommand(const char* cmd) {
    char letter;
    uint8_t idx;
    if (!parseChannel(cmd, letter, idx)) return; // Check it's a valid axis address type

    // Check it's an axis that's registered
    Axis* ax = nullptr;
    switch (letter) {
        case 'L': ax = Lin(idx); break;
        case 'R': ax = Rot(idx); break;
        case 'V': ax = Vib(idx); break;
        case 'A': ax = Aux(idx); break;
    }
    if (!ax) return;

    // Parse target position (up to first 4 digits after channel)
    uint16_t targetPos = 0;
    size_t i = 2;  // skip letter + digit
    size_t len = strlen(cmd);
    if (len < 3) return;
    if (!isdigit(cmd[i])) return;
    while (i < len && isdigit(cmd[i]) && i < 6) { // digits 2-5 can be axis position
        targetPos = targetPos * 10 + (cmd[i] - '0');
        ++i;
    }
    size_t j = i;
    while (j < 6) { targetPos *= 10; ++j; } // fill in any missing digits 3-5

    // Set default values
    InputType inputType = InputType::SHORT;
    uint32_t extendedParam = 0;
    MoveStyle style = MoveStyle::RAMPED;
    int32_t gradient = 0;
    bool hasEaseIn = false;
    bool hasEaseOut = false;
    bool hasGradient = false;

    // Read the speed or interval parameter
    if (i < len) {
        char c = toupper(cmd[i]);
        if (c == 'S' || c == 'I') {
            char* endptr;
            long val = strtol(cmd + i + 1, &endptr, 10);
            if ((endptr > cmd + i + 1) && val > 0) {
                //if (val > LONG_COMMAND_MAX) val = LONG_COMMAND_MAX; // Limit this in the Axis class
                extendedParam = (uint32_t)val;
                inputType = (c == 'S') ? InputType::SPEED : InputType::INTERVAL;
                i = endptr - cmd;
            }
        }
    }

    // Look for easing parameters
    while (i < len) {
        char c = toupper(cmd[i]);
        if (c == '<') {
            hasEaseIn = true;
        } else if (c == '>') {
            hasEaseOut = true;
        } else if (c == 'G') {  // Shift to gradient interpreter
            break;
        }
        ++i;
    }

    // Read the gradient parameter
    if (i < len) {
        char c = toupper(cmd[i]);
        if (c == 'G') {
            char* endptr;
            long val = strtol(cmd + i + 1, &endptr, 10);
            if (endptr > cmd + i + 1) {
                gradient = (int32_t)val;
                hasGradient = true;
                i = endptr - cmd;
            }
        }
    }

    // Decide final MoveStyle 
    if (inputType == InputType::SHORT) {    // No easing or gradient if it's a short
        style = MoveStyle::RAMPED;
        gradient = 0;
    } else if (hasGradient) {               // G overrides any < >
        style = MoveStyle::GRADIENT;
    } else if (hasEaseIn && hasEaseOut) {
        style = MoveStyle::EASE_BOTH;
    } else if (hasEaseIn) {
        style = MoveStyle::EASE_IN;
    } else if (hasEaseOut) {
        style = MoveStyle::EASE_OUT;
    }

    // Send the interpretted values to the axis
    ax->prepAxis(targetPos, inputType, extendedParam, style, gradient);
}

// Function to process device commands
void TCode::processDeviceCommand(const char* cmd) {
    if (strcmp(cmd, "D0") == 0) {
        queueResponse(TCODE_DEVICE_INFO "\r\n");
    } else if (strcmp(cmd, "D1") == 0) {
        queueResponse(TCODE_VERSION_INFO "\r\n");
    } else if (strcmp(cmd, "D2") == 0) {
        // TODO: eventually output range values for each axis
        queueResponse("D2 not implemented yet\r\n");
    } else if (strcmp(cmd, "DSTOP") == 0) {
        stopAll();
        queueResponse("STOP\r\n");
    }
    // anything else is silently ignored
}

// Function to process preference save commands
// CURRENTLY A STUB
void TCode::processSaveCommand(const char* cmd) {
    // TODO: handle save commands ($)
    (void)cmd;  // silence unused warning
}

// Function that executes all buffered TCode commands
void TCode::executeAll() {
    for (uint8_t i = 0; i < 10; ++i) {
        if (_Lin[i]) _Lin[i]->setAxis();
        if (_Rot[i]) _Rot[i]->setAxis();
        if (_Vib[i]) _Vib[i]->setAxis();
        if (_Aux[i]) _Aux[i]->setAxis();
    }
}


void TCode::queueResponse(const char* text) {
    _outputLen = 0;
    _outputPos = 0;
    if (text) {
        while (*text && _outputLen < 63) {
            _outputBuffer[_outputLen++] = *text++;
        }
    }
}


bool TCode::parseChannel(const char* cmd, char& letter, uint8_t& index) const {
    if (!cmd || strlen(cmd) < 2) return false;
    letter = toupper(cmd[0]);
    if (cmd[1] < '0' || cmd[1] > '9') return false;
    index = cmd[1] - '0';
    return (letter == 'L' || letter == 'R' || letter == 'V' || letter == 'A');
}
