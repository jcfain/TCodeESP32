
    
#pragma once

#include <Arduino.h>

void defaultCallback(const String &input) // Default callback used by TCode uses serial communication
{
    if (Serial)
    {
        Serial.println(input);
    }
}

class TCodeBase {
public:
	void setMessageCallback(TCODE_FUNCTION_PTR_T f) // Sets the callback function used by TCode
	{
		if (f == nullptr)
		{
			message_callback = &defaultCallback;
		}
		else
		{
			message_callback = f;
		}
	}
    void sendMessage(const String &input) {
        message_callback(input);
    }
protected: 
    TCODE_FUNCTION_PTR_T message_callback = &defaultCallback;
};