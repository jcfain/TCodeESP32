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


#ifndef TCODE_H
#define TCODE_H

#include "Axis.h"      // 
#include <Arduino.h>   // provides uint32_t, uint16_t, etc.

#ifndef TCODE_DEVICE_INFO
#define TCODE_DEVICE_INFO   "Generic TCode Device"
#endif

#ifndef TCODE_VERSION_INFO
#define TCODE_VERSION_INFO  "TCode v0.4"
#endif

using TCodeCallback = void (*)(const char *input);

// ============================================
//           TCODE CLASS
// ============================================
class TCode {
public:
    TCode();

    // Register an axis to its official TCode channel.
    // Pass the channel exactly as the protocol expects ("L0", "R3", "V7", "A1", etc.).
    // Returns false if the channel is invalid or already taken.
    bool addAxis(const char* channel, Axis& axis);

    // Typed accessors
    Axis* Lin(uint8_t index) const;  // L0..L9
    Axis* Rot(uint8_t index) const;  // R0..R9
    Axis* Vib(uint8_t index) const;  // V0..V9
    Axis* Aux(uint8_t index) const;  // A0..A9

    // General lookup
    Axis* getAxis(const char* channel) const;

    // Utility
    uint8_t getRegisteredCount() const { return _count; }

    // This feeds the received data into the interpreter one byte at a time
    void byteInput(uint8_t byte);

    // This can be used to feed a string into the interpreter
    void stringInput(const char* str);

    // Output buffer — read exactly like Serial
    size_t available() const;
    int read();

    // All stop command, used by DSTOP
    void stopAll();

    // Sets the callback function used by TCode
	void setTCodeCallback(TCodeCallback f);

protected:
    TCodeCallback tcode_callback;

private:
    // Internal storage — fixed arrays of pointers, one per channel type
    Axis* _Lin[10] = {nullptr};
    Axis* _Rot[10] = {nullptr};
    Axis* _Vib[10] = {nullptr};
    Axis* _Aux[10] = {nullptr};
    uint8_t _count = 0;

    // Parser buffer
    char _cmdBuffer[48] = {0};
    uint8_t _cmdLen = 0;

    // Simple output buffer for D responses
    char _outputBuffer[64] = {0};
    uint8_t _outputLen = 0;
    uint8_t _outputPos = 0;

    // Command processing
    void processCommand(const char* cmd);
    void processAxisCommand(const char* cmd);
    void processDeviceCommand(const char* cmd);
    void processSaveCommand(const char* cmd);
    void executeAll();

    // 
    void queueResponse(const char* text);

    // Helper function that turns "L3" into letter='L' and index=3
    bool parseChannel(const char* channel, char& letter, uint8_t& index) const;

};

#endif