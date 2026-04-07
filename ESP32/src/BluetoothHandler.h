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

#include <BluetoothSerial.h>
// #include <BLEHandler.h>
#include "SettingsHandler.h"
#include "logging/LogHandler.h"
#include "TagHandler.h"
#include "esp_coexist.h"
#include "esp_bt.h"
#include "TCodeInterface.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

class BluetoothHandler : public TCodeInterface
{
public:
    bool setup()
    {
        LogHandler::info(_TAG, "Starting bluetooth serial: %s", "TCodeESP32");
        if (!SerialBT.begin("TCodeESP32"))
        {
            LogHandler::error(_TAG, "An error occurred initializing Bluetooth serial");
            return false;
        }
        LogHandler::info(_TAG, "Bluetooth started");
        _isConnected = true;
        return true;
    }

    // size_t read()
    // {
    //   return SerialBT.read();
    // }

    size_t available() override
    {
        if (!_isConnected)
        {
            return 0;
        }
        return SerialBT.available();
    }

    void send(const char *in) override
    {
        if (_isConnected)
        {
            SerialBT.println(in);
        }
    }

    size_t read(char *buf) override
    {
        if (!_isConnected)
        {
            buf[0] = {0};
            return 0;
        }
        size_t len = SerialBT.readBytesUntil('\n', buf, MAX_COMMAND);
        if (len < MAX_COMMAND)
        {
            buf[len] = '\n';
            return len + 1;
        }
        return len;
    }

    void stop()
    {
        _isConnected = false;
        SerialBT.disconnect();
        SerialBT.end();
    }

    // String readStringUntil(char terminator)
    // {
    //   return SerialBT.readStringUntil(terminator);
    // }
    // size_t readBytesUntil(char terminator, char *out, size_t len)
    // {
    //   return SerialBT.readBytesUntil(terminator, out, len);
    // }

    // void CommandCallback(const String &in)
    // { // This overwrites the callback for message return
    //   if (_isConnected)
    //     SerialBT.print(in);
    // }

    // void write(uint8_t message)
    // {
    //   // Serial.print("BTWrite: ");
    //   // Serial.println(message);
    //   SerialBT.write(message);
    // }
    // void println(const char *message)
    // {
    //   // Serial.print("BTWrite: ");
    //   // Serial.println(message);
    //   SerialBT.println(message);
    // }

    bool isConnected()
    {
        return _isConnected;
    }

    static void disable()
    {
        esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    }

private:
    const char *_TAG = TagHandler::BluetoothHandler;
    bool _isConnected = false;
    BluetoothSerial SerialBT;
};