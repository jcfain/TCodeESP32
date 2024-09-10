
#pragma once

#include <EEPROM.h>
#include "Axis.h"
#include "../TCodeBase.h"
#include "../../TagHandler.h"
// -----------------------------
// Class to manage Toy Comms
// -----------------------------
class TCode0_3 : public TCodeBase
{

public:
	// Setup function
	void setup(const char *firmware) override
	{
		firmwareID = firmware;

		// #ESP32# Enable EEPROM
		EEPROM.begin(320);

		// Vibe channels start at 0
		for (int i = 0; i < CHANNELS; i++)
		{
			Vibration[i].Set(0, ' ', 0);
		}
	}

	// Function to name and activate axis
	void RegisterAxis(const String &ID, const String &axisName)
	{
		char type = ID.charAt(0);
		int channel = ID.charAt(1) - '0';
		if ((0 <= channel && channel < CHANNELS))
		{
			switch (type)
			{
			// Axis commands
			case 'L':
			{
				Linear[channel].setName(axisName);
				break;
			}
			case 'R':
			{
				Rotation[channel].setName(axisName);
				break;
			}
			case 'V':
			{
				Vibration[channel].setName(axisName);
				break;
			}
			case 'A':
			{
				Auxiliary[channel].setName(axisName);
				break;
			}
			}
		}
	}

	// Function to read off individual bytes as input
	void read(byte inByte) override
	{
		bufferString += (char)inByte; // Add new character to string

		if (inByte == '\n')
		{								 // Execute string on newline
			bufferString.trim();		 // Remove spaces, etc, from buffer
			executeString(bufferString); // Execute string
			bufferString = "";			 // Clear input string
		}
	}

	// Function to read off whole strings as input
	void read(const String &inString) override
	{
		bufferString = inString;	 // Replace existing buffer with input string
		bufferString.trim();		 // Remove spaces, etc, from buffer
		executeString(bufferString); // Execute string
		bufferString = "";			 // Clear input string
	}

	// Function to set an axis
	void AxisInput(const String &ID, int magnitude, char extension, unsigned long extMagnitude)
	{
		char type = ID.charAt(0);
		int channel = ID.charAt(1) - '0';
		if ((0 <= channel && channel < CHANNELS))
		{
			switch (type)
			{
			// Axis commands
			case 'L':
				Linear[channel].Set(magnitude, extension, extMagnitude);
				break;
			case 'R':
				Rotation[channel].Set(magnitude, extension, extMagnitude);
				break;
			case 'V':
				Vibration[channel].Set(magnitude, extension, extMagnitude);
				break;
			case 'A':
				Auxiliary[channel].Set(magnitude, extension, extMagnitude);
				break;
			}
		}
	}

	// Function to read the current position of an axis
	float AxisRead(const String &ID)
	{
		int x = -1; // This is the return variable
		char type = ID.charAt(0);
		int channel = ID.charAt(1) - '0';
		if ((0 <= channel && channel < CHANNELS))
		{
			switch (type)
			{
			// Axis commands
			case 'L':
			{
				if (Linear[channel].isInitialized())
					x = Linear[channel].GetPosition();
				break;
			}
			case 'R':
			{
				if (Rotation[channel].isInitialized())
					x = Rotation[channel].GetPosition();
				break;
			}
			case 'V':
			{
				if (Vibration[channel].isInitialized())
					x = Vibration[channel].GetPosition();
				break;
			}
			case 'A':
			{
				if (Auxiliary[channel].isInitialized())
					x = Auxiliary[channel].GetPosition();
				break;
			}
			}
		}
		return x;
	}

	// bool IsInitialized(const String &ID)
	// {
	// 	char type = ID.charAt(0);
	// 	int channel = ID.charAt(1) - '0';
	// 	if ((0 <= channel && channel < CHANNELS))
	// 	{
	// 		switch (type)
	// 		{
	// 		// Axis commands
	// 		case 'L':
	// 		{
	// 			return Linear[channel].isInitialized();
	// 		}
	// 		case 'R':
	// 		{
	// 			return Rotation[channel].isInitialized();
	// 		}
	// 		case 'V':
	// 		{
	// 			return Vibration[channel].isInitialized();
	// 		}
	// 		case 'A':
	// 		{
	// 			return Auxiliary[channel].isInitialized();
	// 		}
	// 		}
	// 	}
	// 	return false;
	// }

	// Function to query when an axis was last commanded
	unsigned long AxisLast(const String &ID) 
	{
		unsigned long t = 0; // Return time
		char type = ID.charAt(0);
		int channel = ID.charAt(1) - '0';
		if ((0 <= channel && channel < CHANNELS))
		{
			switch (type)
			{
			// Axis commands
			case 'L':
				t = Linear[channel].lastT;
				break;
			case 'R':
				t = Rotation[channel].lastT;
				break;
			case 'V':
				t = Vibration[channel].lastT;
				break;
			case 'A':
				t = Auxiliary[channel].lastT;
				break;
			}
		}
		return t;
	}

	void getDeviceSettings(char *settings)
	{
		String deviceSettings = "";
		for (int i = 0; i < 10; i++)
		{
			deviceSettings += axisRow("L" + String(i), 8 * i, Linear[i].Name);
		}
		for (int i = 0; i < 10; i++)
		{
			deviceSettings += axisRow("R" + String(i), 8 * i + 80, Rotation[i].Name);
		}
		for (int i = 0; i < 10; i++)
		{
			deviceSettings += axisRow("V" + String(i), 8 * i + 160, Vibration[i].Name);
		}
		for (int i = 0; i < 10; i++)
		{
			deviceSettings += axisRow("A" + String(i), 8 * i + 240, Auxiliary[i].Name);
		}
		strcpy(settings, deviceSettings.c_str());
	}

private:
	const char *_TAG = TagHandler::TCodeHandler;
	// Strings
	const char *firmwareID;
	String bufferString; // String to hold incomming commands

	const static int CHANNELS = 11;

	// Declare axes
	Axis Linear[CHANNELS];
	Axis Rotation[CHANNELS];
	Axis Vibration[CHANNELS];
	Axis Auxiliary[CHANNELS];

	// Function to divide up and execute input string
	void executeString(String bufferString)
	{
		int index = bufferString.indexOf(' '); // Look for spaces in string
		while (index > 0)
		{
			readCmd(bufferString.substring(0, index));		  // Read off first command
			bufferString = bufferString.substring(index + 1); // Remove first command from string
			bufferString.trim();
			index = bufferString.indexOf(' '); // Look for next space
		}
		readCmd(bufferString); // Read off last command
	}

	// Function to process the individual commands
	void readCmd(String command)
	{

		// Switch between command types
		switch (command.charAt(0))
		{
		// Axis commands
		case 'L':
		case 'l':
		case 'R':
		case 'r':
		case 'V':
		case 'v':
		case 'A':
		case 'a':
			command.toUpperCase();
			axisCmd(command);
			break;

		// Device commands
		case 'D':
		case 'd':
			command.toUpperCase();
			deviceCmd(command);
			break;

		// Setup commands
		case '$':
		case '#':
			setupCmd(command);
			break;
		}
	}

	// Function to read and interpret axis commands
	void axisCmd(String command)
	{

		char type = command.charAt(0); // Type of command - LRVA
		bool valid = true;			   // Command validity flag, valid by default

		// Check for channel number
		int channel = command.charAt(1) - '0';
		if (channel < 0 || channel >= CHANNELS)
		{
			valid = false;
		}
		channel = constrain(channel, 0, CHANNELS);

		// Check for an extension
		char extension = ' ';
		int index = command.indexOf('S', 2);
		if (index > 0)
		{
			extension = 'S';
		}
		else
		{
			index = command.indexOf('I', 2);
			if (index > 0)
			{
				extension = 'I';
			}
		}
		if (index < 0)
		{
			index = command.length();
		}

		// Get command magnitude
		String magString = command.substring(2, index);
		magString = magString.substring(0, 4);
		while (magString.length() < 4)
		{
			magString += '0';
		}
		int magnitude = magString.toInt();
		if (magnitude == 0 && magString.charAt(0) != '0')
		{
			valid = false;
		} // Invalidate if zero returned, but not a number

		// Get extension magnitude
		long extMagnitude = 0;
		if (extension != ' ')
		{
			magString = command.substring(index + 1);
			magString = magString.substring(0, 8);
			extMagnitude = magString.toInt();
		}
		if (extMagnitude == 0)
		{
			extension = ' ';
		}

		// Switch between command types
		if (valid)
		{
			switch (type)
			{
			// Axis commands
			case 'L':
				Linear[channel].Set(magnitude, extension, extMagnitude);
				break;
			case 'R':
				Rotation[channel].Set(magnitude, extension, extMagnitude);
				break;
			case 'V':
				Vibration[channel].Set(magnitude, extension, extMagnitude);
				break;
			case 'A':
				Auxiliary[channel].Set(magnitude, extension, extMagnitude);
				break;
			}
		}
	}

	// Function to identify and execute device commands
	void deviceCmd(String command)
	{
		int i;
		// Remove "D"
		command = command.substring(1);

		// Look for device stop command
		if (command.substring(0, 4).equalsIgnoreCase("STOP"))
		{
			for (i = 0; i < 10; i++)
			{
				Linear[i].Stop();
			}
			for (i = 0; i < 10; i++)
			{
				Rotation[i].Stop();
			}
			for (i = 0; i < 10; i++)
			{
				Vibration[i].Set(0, ' ', 0);
			}
			for (i = 0; i < 10; i++)
			{
				Auxiliary[i].Stop();
			}
		}
		else
		{
			// Look for numbered device commands
			int commandNumber = command.toInt();
			if (commandNumber == 0 && command.charAt(0) != '0')
			{
				command = -1;
			}
			switch (commandNumber)
			{
			case 0:
			{
				char firmware[12] = "Firmware v";
				sendMessage(strcat(firmware, firmwareID));
			}
			break;

			case 1:
				sendMessage("TCode v0.3\n");
				break;

			case 2:
				char returnVal[255];
				getDeviceSettings(returnVal);
				sendMessage(returnVal);
				break;
			}
		}
	}

	// Function to modify axis preference values
	void setupCmd(String command)
	{
		int minVal = 0, maxVal = 0;
		String minValString, maxValString;
		bool valid;
		// Axis type
		char type = command.charAt(1);
		switch (type)
		{
		case 'L':
		case 'l':
		case 'R':
		case 'r':
		case 'V':
		case 'v':
		case 'A':
		case 'a':
			valid = true;
			break;

		default:
			type = ' ';
			valid = false;
			break;
		}
		// Axis channel number
		int channel = (command.substring(2, 3)).toInt();
		if (channel == 0 && command.charAt(2) != '0')
		{
			valid = false;
		}
		// Input numbers
		int index1 = command.indexOf('-');
		if (index1 != 3)
		{
			valid = false;
		}
		int index2 = command.indexOf('-', index1 + 1); // Look for spaces in string
		if (index2 <= 3)
		{
			valid = false;
		}
		if (valid)
		{
			// Min value
			minValString = command.substring(4, index2);
			minValString = minValString.substring(0, 4);
			while (minValString.length() < 4)
			{
				minValString += '0';
			}
			minVal = minValString.toInt();
			if (minVal == 0 && minValString.charAt(0) != '0')
			{
				valid = false;
			}
			// Max value
			maxValString = command.substring(index2 + 1);
			maxValString = maxValString.substring(0, 4);
			while (maxValString.length() < 4)
			{
				maxValString += '0';
			}
			maxVal = maxValString.toInt();
			if (maxVal == 0 && maxValString.charAt(0) != '0')
			{
				valid = false;
			}
		}
		// If a valid command, save axis preferences to EEPROM
		if (valid)
		{
			int memIndex = 0;
			switch (type)
			{
			case 'L':
			case 'l':
				memIndex = 0;
				break;
			case 'R':
			case 'r':
				memIndex = 80;
				break;
			case 'V':
			case 'v':
				memIndex = 160;
				break;
			case 'A':
			case 'a':
				memIndex = 240;
				break;
			}
			memIndex += 8 * channel;
			minVal = constrain(minVal, 0, 9999);
			EEPROM.put(memIndex, minVal - 1);
			minVal = constrain(maxVal, 0, 9999);
			EEPROM.put(memIndex + 4, maxVal - 10000);
			EEPROM.commit();
			// Output that axis changed successfully
			switch (type)
			{
			case 'L':
			case 'l':
				axisRow("L" + String(channel), memIndex, Linear[channel].Name);
				break;
			case 'R':
			case 'r':
				axisRow("R" + String(channel), memIndex, Rotation[channel].Name);
				break;
			case 'V':
			case 'v':
				axisRow("V" + String(channel), memIndex, Vibration[channel].Name);
				break;
			case 'A':
			case 'a':
				axisRow("A" + String(channel), memIndex, Auxiliary[channel].Name);
				break;
			}
		}
		else
		{
			sendMessage(command.c_str());
		}
	}

	// Function to print the details of an axis
	String axisRow(String axisID, int memIndex, String axisName)
	{
		int low, high;
		String line = "";
		if (axisName != "")
		{
			EEPROM.get(memIndex, low);
			low = constrain(low, -1, 9998);
			EEPROM.get(memIndex + 4, high);
			high = constrain(high, -10000, -1);
			line += axisID;
			line += " ";
			line += String(low + 1);
			line += " ";
			line += String(high + 10000);
			line += " ";
			line += axisName;
			line += "\n";
			// sendMessage(line.c_str());
			//  Serial.print(axisID);
			//  Serial.print(" ");
			//  Serial.print(low + 1);
			//  Serial.print(" ");
			//  Serial.print(high + 10000);
			//  Serial.print(" ");
			//  Serial.println(axisName);
		}
		return line;
	}
};
