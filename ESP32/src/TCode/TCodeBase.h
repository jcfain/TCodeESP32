
    
#pragma once

#include <Arduino.h>

void defaultCallback(const char* input) // Default callback used by TCode uses serial communication
{
    if (Serial)
    {
        Serial.println(input);
    }
}

class TCodeBase {
public:
	virtual void setup(const char* firmware, const char* tcode) = 0;
	// V0.3+ only
	virtual void RegisterAxis(String ID, String axisName) = 0;
	virtual void ByteInput(byte inByte) = 0;
	virtual void StringInput(String inString) = 0;
	virtual void AxisInput(String ID, int magnitude, char extension, long extMagnitude) = 0;
	virtual int AxisRead(String ID) = 0;
	virtual unsigned long AxisLast(String ID) = 0;
	virtual void getDeviceSettings(char* settings) = 0;
	// V0.2+
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
    void sendMessage(const char* input) {
        message_callback(input);
    }
protected: 
    TCODE_FUNCTION_PTR_T message_callback = &defaultCallback;
};