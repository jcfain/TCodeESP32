
#pragma once

// #include <EEPROM.h>
#include "TCode.h"
#include "TagHandler.h"
#include "TCodeBase.h"

class TCode0_4 : public TCodeBase, public TCode
{
	// TCode0_4() {}

public:
	// Setup function
	void setup(const char *firmware) override
	{
		firmwareID = firmware;

		// #ESP32# Enable EEPROM
		// EEPROM.begin(320);

		// // Vibe channels start at 0
		// for (int i = 0; i < CHANNELS; i++)
		// {
		// 	Vibration[i].Set(0, ' ', 0);
		// }
	}

	// Function to read off individual bytes as input
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
	}

	void getMessages() {
		if(tcode_callback) 
		{
			// size_t len = 0;
			// bool limitReached = false;
  			// while (TCode::available() > 0) 
			// { 
			// 	outputBuffer[len++] = TCode::read();
			// 	if(len == MAX_COMMAND - 1) 
			// 	{
			// 		limitReached = true;
			// 	}
			// 	if(limitReached || outputBuffer[len] == '\n') 
			// 	{
			// 		outputBuffer[len+1] = {0};
			// 		if(message_callback) 
			// 			message_callback(outputBuffer);
			// 		// else
			// 		// 	Serial.println(outputBuffer);
			// 		outputBuffer[MAX_COMMAND] = {0};
			// 	}
			// 	if(limitReached)
			// 		len = 0;
			// }
			size_t length = TCode::available();
			if(!length)
				return;
			Serial.printf("TCode_4 getMessages length: %i\n", length);
			size_t index = 0;
			char outputBuffer[length] = {0};
			while (index < length) 
			{
				int c = TCode::read();
				outputBuffer[index] = (char)c;
				index++;
				if (c < 0 || (char)c == '\n') 
				{
					break;
				}
				Serial.printf("TCode_4 getMessages index: %i\n", index);
			}
			outputBuffer[index] = {0};
			Serial.printf("TCode_4 getMessages send: %s\n", outputBuffer);
			tcode_callback(outputBuffer);
		}
	}
	// // Function to read off whole strings as input
	// void read(const String &inString) override
	// {
	// 	bufferString = inString;	 // Replace existing buffer with input string
	// 	bufferString.trim();		 // Remove spaces, etc, from buffer
	// 	executeString(bufferString); // Execute string
	// 	bufferString = "";			 // Clear input string
	// }

private:
	const char *_TAG = TagHandler::TCodeHandler;
	// Strings
	const char *firmwareID;
	// char outputBuffer[MAX_COMMAND] = {0};
	// String bufferString; // String to hold incomming commands

	// const static int CHANNELS = 11;

	// TCode m_tcode;


	// // Function to divide up and execute input string
	// void executeString(String bufferString)
	// {
	// 	int index = bufferString.indexOf(' '); // Look for spaces in string
	// 	while (index > 0)
	// 	{
	// 		readCmd(bufferString.substring(0, index));		  // Read off first command
	// 		bufferString = bufferString.substring(index + 1); // Remove first command from string
	// 		bufferString.trim();
	// 		index = bufferString.indexOf(' '); // Look for next space
	// 	}
	// 	readCmd(bufferString); // Read off last command
	// }

	// // Function to process the individual commands
	// void readCmd(String command)
	// {

	// 	// Switch between command types
	// 	switch (command.charAt(0))
	// 	{
	// 	// TCodeAxis0_3 commands
	// 	case 'L':
	// 	case 'l':
	// 	case 'R':
	// 	case 'r':
	// 	case 'V':
	// 	case 'v':
	// 	case 'A':
	// 	case 'a':
	// 		command.toUpperCase();
	// 		axisCmd(command);
	// 		break;

	// 	// Device commands
	// 	case 'D':
	// 	case 'd':
	// 		command.toUpperCase();
	// 		deviceCmd(command);
	// 		break;

	// 	// Setup commands
	// 	case '$':
	// 	case '#':
	// 		setupCmd(command);
	// 		break;
	// 	}
	// }

	// // Function to read and interpret axis commands
	// void axisCmd(String command)
	// {
	// }

	// // Function to identify and execute device commands
	// void deviceCmd(String command)
	// {
	// 	int i;
	// 	// Remove "D"
	// 	command = command.substring(1);

	// 	// Look for device stop command
	// 	if (command.substring(0, 4).equalsIgnoreCase("STOP"))
	// 	{
	// 		for (i = 0; i < 10; i++)
	// 		{
	// 			Linear[i].Stop();
	// 		}
	// 		for (i = 0; i < 10; i++)
	// 		{
	// 			Rotation[i].Stop();
	// 		}
	// 		for (i = 0; i < 10; i++)
	// 		{
	// 			Vibration[i].Set(0, ' ', 0);
	// 		}
	// 		for (i = 0; i < 10; i++)
	// 		{
	// 			Auxiliary[i].Stop();
	// 		}
	// 	}
	// 	else
	// 	{
	// 		// Look for numbered device commands
	// 		int commandNumber = command.toInt();
	// 		if (commandNumber == 0 && command.charAt(0) != '0')
	// 		{
	// 			command = -1;
	// 		}
	// 		switch (commandNumber)
	// 		{
	// 		case 0:
	// 		{
	// 			char firmware[12] = "Firmware v";
	// 			sendMessage(strcat(firmware, firmwareID));
	// 		}
	// 		break;

	// 		case 1:
	// 			sendMessage("TCode v0.3\n");
	// 			break;

	// 		case 2:
	// 			char returnVal[255];
	// 			getDeviceSettings(returnVal);
	// 			sendMessage(returnVal);
	// 			break;
	// 		}
	// 	}
	// }

	// // Function to modify axis preference values
	// void setupCmd(String command)
	// {
	// 	int minVal = 0, maxVal = 0;
	// 	String minValString, maxValString;
	// 	bool valid;
	// 	// If a valid command, save axis preferences to EEPROM
	// 	if (valid)
	// 	{
	// 	}
	// 	else
	// 	{
	// 		sendMessage(command.c_str());
	// 	}
	// }
};
