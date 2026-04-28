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
#include <HardwareSerial.h>
#include "SettingsHandler.h"
#include "logging/LogHandler.h"
#include "TagHandler.h"
#include "TCodeInterface.h"

#if ARDUINO_USB_CDC_ON_BOOT  //Serial used from Native_USB_CDC | HW_CDC_JTAG
#if ARDUINO_USB_MODE         // Hardware CDC mode
// Arduino Serial is the HW JTAG CDC device
#define SerialType HWCDC
#else  // !ARDUINO_USB_MODE -- Native USB Mode
// Arduino Serial is the Native USB CDC device
#define SerialType USBCDC
#endif  // ARDUINO_USB_MODE
#else   // !ARDUINO_USB_CDC_ON_BOOT -- Serial is used from UART0
// if not using CDC on Boot, Arduino Serial is the UART0 device
#define SerialType HardwareSerial
#endif  // ARDUINO_USB_CDC_ON_BOOT

class SerialHandler : public TCodeInterface
{
  public:
    bool setup(SerialType& serial = Serial, int baud = 115200, int rxpin = -1, int txpin = -1) 
    {
		LogHandler::info(m_TAG, "Starting Serial baud: %i txpin: %i rxpin: %i", baud, txpin, rxpin);
        m_serial = serial;
#if ARDUINO_USB_CDC_ON_BOOT  //Serial used from Native_USB_CDC | HW_CDC_JTAG
#if ARDUINO_USB_MODE         // Hardware CDC mode
// Arduino Serial is the HW JTAG CDC device
        m_serial.begin(baud);
#else  // !ARDUINO_USB_MODE -- Native USB Mode
        m_serial.begin(baud, 134217756UL, rxpin, txpin);
#endif  // ARDUINO_USB_MODE
#else   // !ARDUINO_USB_CDC_ON_BOOT -- Serial is used from UART0
        m_serial.begin(baud, 134217756UL, rxpin, txpin);
#endif  // ARDUINO_USB_CDC_ON_BOOT
		if(!m_serial.availableForWrite()) 
		{
        	LogHandler::error(m_TAG, "Serial not available");
			return false;
		}
        LogHandler::info(m_TAG, "Serial Listening");
    	// SettingsFactory* m_settingsFactory = SettingsFactory::getInstance();
        if(!m_serial)
            return false;
		initialized = true;
		return true;
    }

    size_t available() override
    {
		if(!initialized) 
        {
            return 0;
		}
        return m_serial.available();
    }

	void send(const char* in) override
	{
		if(initialized) 
        {
            LogHandler::verbose(m_TAG, "[send] %s", in);
            m_serial.println(in);
		}
	}

    size_t read(char* buf) override
    {
		if (!initialized) 
		{
			buf[0] = {0};
			return 0;
		}
        size_t len = m_serial.readBytesUntil('\n', buf, MAX_COMMAND);
        if(len < MAX_COMMAND) 
        {
            buf[len] = '\n';
            return len +1;
        }
        return len;
    }
private:
    bool initialized = false;
    const char* m_TAG = TagHandler::SerialHandler;
    SerialType& m_serial = Serial;
    HardwareSerial& getSerial(uint8_t index)
    {
        switch(index) 
        {
            case 0:
                return Serial0;
            case 1:
                return Serial1;
        }
        return Serial0;
    }
};