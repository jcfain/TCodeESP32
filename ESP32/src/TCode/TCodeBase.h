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
#include "LogHandler.h"
#include "TCodeRange.h"
#include "TCodeCommon.h"
#include "callback.h"


class TCodeBase {
public:
	virtual void setup(const char* firmware) = 0;
	virtual void read(byte inByte) = 0;
	virtual void read(const char* in) = 0;
    // Parses out the TCode range from a save comand
    virtual bool parseRange(TCodeRange& range, const char* cmd) = 0;

    // Formats the TCodeRange object into a tcode string to return to client
    virtual void formatRange(char *buf, const size_t len, const TCodeRange* ranges, const size_t rangesLen) = 0;
	// virtual void read(const String &input) = 0;
	virtual void setMessageCallback(TCodeCommandCallback f) // Sets the callback function used by TCode
	{
		if (f == nullptr)
		{
			message_callback = 0;
		}
		else
		{
			message_callback = f;
		}
	}
    void sendMessage(const char* input) {
		size_t inputLength = strlen(input);
		if(!inputLength)
			return;
		// // Commened out because commands  that arent external do not match the system command handler.
		// // This will probably need to be changed in the next tcode version.
		// if(!endsWith(input, "\n")) {
		// 	char buf[inputLength + 1];
		// 	strlcpy(buf, input, inputLength + 1);
		// 	strcat(buf, "\n");
        // 	message_callback(buf);
		// 	return;
		// }
		// LogHandler::debug("TcodeBase", "[sendMessage] %s", input);
		if(!message_callback)
		{
			LogHandler::warning("TcodeBase", "[sendMessage] callback not defined");
			message_callback = std::bind(&TCodeBase::defaultCallback, this, std::placeholders::_1, std::placeholders::_2);
		}

        message_callback(TCodeEvent::NOTIFY, input);
    }
protected: 
    TCodeCommandCallback message_callback = 0;
private: 
	void defaultCallback(TCodeEvent event, const char* input) // Default callback used by TCode uses serial communication
	{
		LogHandler::warning("Default TCode callback", "%s\n", input);
	}
};