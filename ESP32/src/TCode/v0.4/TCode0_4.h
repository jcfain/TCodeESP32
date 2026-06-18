
#pragma once

#include "TCode.h"
#include "TagHandler.h"
#include "TCodeBase.h"

class TCode0_4 : public TCodeBase, public TCode
{
public:
	// Setup function
	void setup(const char *firmware) override
	{
		firmwareID = firmware;
	}

	void read(byte inByte) override
	{
		TCode::byteInput(inByte);
	}

	void read(const char* in) override
	{
		TCode::stringInput(in);
	}

	void setMessageCallback(TCodeCallback f) override
	{
		TCode::setTCodeCallback(f);
		TCodeBase::setMessageCallback(f);
	}

	// void getMessages() 
	// {
	// 	if(tcode_callback) 
	// 	{
	// 		// size_t len = 0;
	// 		// bool limitReached = false;
  	// 		// while (TCode::available() > 0) 
	// 		// { 
	// 		// 	outputBuffer[len++] = TCode::read();
	// 		// 	if(len == MAX_COMMAND - 1) 
	// 		// 	{
	// 		// 		limitReached = true;
	// 		// 	}
	// 		// 	if(limitReached || outputBuffer[len] == '\n') 
	// 		// 	{
	// 		// 		outputBuffer[len+1] = {0};
	// 		// 		if(message_callback) 
	// 		// 			message_callback(outputBuffer);
	// 		// 		// else
	// 		// 		// 	Serial.println(outputBuffer);
	// 		// 		outputBuffer[MAX_COMMAND] = {0};
	// 		// 	}
	// 		// 	if(limitReached)
	// 		// 		len = 0;
	// 		// }


	// 		// size_t length = TCode::available();
	// 		// if(!length)
	// 		// 	return;
	// 		// Serial.printf("TCode_4 getMessages length: %i\n", length);
	// 		// size_t index = 0;
	// 		// char outputBuffer[length] = {0};
	// 		// while (index < length) 
	// 		// {
	// 		// 	int c = TCode::read();
	// 		// 	outputBuffer[index] = (char)c;
	// 		// 	index++;
	// 		// 	if (c < 0 || (char)c == '\n') 
	// 		// 	{
	// 		// 		break;
	// 		// 	}
	// 		// 	// Serial.printf("TCode_4 getMessages index: %i\n", index);
	// 		// }
	// 		// outputBuffer[index] = {0};


	// 		// TCode::read(outputBuffer);
	// 		// if(!outputBuffer)
	// 		// 	return;
	// 		// Serial.printf("TCode_4 getMessages send: %s\n", outputBuffer);
	// 		// tcode_callback(outputBuffer);
	// 	}
	// }

private:
	const char *_TAG = TagHandler::TCodeHandler;
	const char *firmwareID;
};
