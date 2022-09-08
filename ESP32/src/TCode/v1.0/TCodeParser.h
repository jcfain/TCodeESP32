// TCode-Parser-Class-H v1.0,
// protocal by TempestMAx (https://www.patreon.com/tempestvr)
// implemented by Eve 26/06/2022
// Please copy, share, learn, innovate, give attribution.

#pragma once
#ifndef TCODE_PARSER_H
#define TCODE_PARSER_H
#include "TCodeConstants.h"
#include "TCodeEnums.h"
#include "Arduino.h"

class TCodeParser
{
private:
    /**
     * @brief Converts Lowecase Ascii to Uppercase
     * @param value is the value of the char which needs converting to uppercase
     * @return returns the uppercase representation of the inputted ascii char
     */
    static char toupper(const char value)
    {
        return ((value >= 'a') && (value <= 'z')) ? ('A' + (value - 'a')) : (value);
    }

    /**
     * @brief Checks if the inputed ascii char is a number
     * @param c is the char value which needs checking
     * @return returns true for if the inputted char is a number
     */
    static bool isnumber(const char c)
    {
        switch (c) // Check for numbers
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return true;
        default:
            return false;
        }
    }

    /**
     * @brief Checks if an inputted char is one used for an extention Command in the decoding of TCode
     * @param c is the char value which needs checking
     * @return returns true if the inputted char is used as an extention character
     */
    static bool isextention(const char c)
    {
        switch (toupper(c)) // checks if the inputted char is used as an extention character
        {
        case 'I':
        case 'S':
            return true;
        default:
            return false;
        }
    }

    /**
     * @brief In a string at the index pointed to by the index paramater gets the Integer contained at and after incrementing the index value
     * @param input the input string containing the integer
     * @param index the index pointing to a position in the string
     * @return returns a long the integer found in the string represented as base 10 unsigned if a integer string is found which is less than 4 characters long then it will be multiplied til it reaches that minimum e.g. "1" = 1000 ,"01" = 100
     */
    static long getNextInt(const String &input, int &index)
    {
        size_t count = 0;
        long accum = 0;
        while (isnumber(input.charAt(index))) // while there is a number at the index we are at in the string
        {
            accum *= 10;                                             // multiply the accumulator first to get the correct output value
            accum += static_cast<long>(toupper(input.charAt(index++)) - '0'); // get next int value '0' - '9' subtracting '0' gets the integer value of the next unit
            count++;                                                 // increase the indeces count to count the digits
        }

        if (count == 0) // no chars were found with 0 - 9
            return -1;

        while (count < 4) // if less than 4 digits were found make up for it by multiplying eg 1 = 1000, 90 = 9000, 001 = 10
        {
            accum *= 10;
            count++;
        }

        return accum;
    }

public:
    /**
     * @brief Checks if the inputted type and channel number are valid
     * @param type the TCode Channel Type
     * @param channel_int the Channel number
     * @returns returns true if the type and channel number are within accepted ranges and values 0-9 on channel number and a valid channel type
     */
    static bool idValid(const TCode_Channel_Type type, const int channel_int)
    {
        if ((channel_int >= 10) || (channel_int < 0)) // check if channel number is not out of range
            return false;

        switch (type) // check if the type is correct
        {
        case TCode_Channel_Type::Auxiliary:
        case TCode_Channel_Type::Linear:
        case TCode_Channel_Type::Rotation:
        case TCode_Channel_Type::Vibration:
            break;
        default:
            return false;
        }

        return true;
    }

    /**
     * @brief Checks if the inputted type and channel number are valid
     * @param id Is the combined form of TCode_Channel_Type and the channel number
     * @returns returns true if the type and channel number are within accepted ranges and values 0-9 on channel number and a valid channel type
     */
    static bool idValid(const TCode_ChannelID id)
    {
        return idValid(id.type, id.channel);
    }

    /**
     * @brief For a given ID returns the string representation of the ID
     * @param id Is the combined form of TCode_Channel_Type and the channel number
     * @returns the string representation of a TCode Channel ID
     */
    static String getStrfromID(const TCode_ChannelID id)
    {
        String out = "";
        switch (id.type) // gets the Channel type char and appends to the output string
        {
        case TCode_Channel_Type::Auxiliary:
            out += 'A';
            break;
        case TCode_Channel_Type::Linear:
            out += 'L';
            break;
        case TCode_Channel_Type::Rotation:
            out += 'R';
            break;
        case TCode_Channel_Type::Vibration:
            out += 'V';
            break;
        default:
            out += '?';
        }
        out += String(id.channel); // gets the string representation of the channel and appends to the output
        return out;
    }

    /**
     * @brief Takes a channel type and channel number and creates a Channel ID
     * @param type the TCode Channel Type
     * @param channel_int the Channel number
     * @returns returns a TCode_ChannelID from the channel type and channel number
     */
    static TCode_ChannelID constructID(const TCode_Channel_Type type, const int channel_int) // Constructs an ID from a channel type and channel number
    {
        bool valid = idValid(type, channel_int);
        return {type, channel_int, valid};
    }

    /**
     * @brief Gets the Extention type from a string at the index specified
     * @param input string to be processed
     * @param start_index starting index of the char to be processed
     * @returns returns a TCode Axis Extention Type used in the Axis to work out if an extention means Time or Speed
     */
    static TCode_Axis_Extention_Type getExtentionTypeFromStr(const String &input, const int start_index) // gets an ID from an index in a string pointed to by start index and returns a TCode ChannelID
    {
        char type_char = toupper(input.charAt(start_index));
        switch (type_char)
        {
        case 'I':
            return TCode_Axis_Extention_Type::Time;
        case 'S':
            return TCode_Axis_Extention_Type::Time;
        }
        return TCode_Axis_Extention_Type::None;
    }

    /**
     * @brief Gets the ID from an inputted string from a given index
     * @param input string to be processed
     * @param start_index starting index of the char to be processed
     * @returns returns a TCode Channel ID found at the location at the starting index
     */
    static TCode_ChannelID getIDFromStr(const String &input, const int start_index) // gets an ID from an index in a string pointed to by start index and returns a TCode ChannelID
    {
        char type_char = toupper(input.charAt(start_index));
        uint8_t channel_int = static_cast<uint8_t>(toupper(input.charAt(start_index + 1)) - '0'); // get channel number 0 - 9
        TCode_Channel_Type type = TCode_Channel_Type::None;
        switch (type_char) // get channel type
        {
        case 'L':
            type = TCode_Channel_Type::Linear;
            break;
        case 'R':
            type = TCode_Channel_Type::Rotation;
            break;
        case 'V':
            type = TCode_Channel_Type::Vibration;
            break;
        case 'A':
            type = TCode_Channel_Type::Auxiliary;
            break;
        }

        bool valid = idValid(type, channel_int); // check if valid
        return {type, channel_int, valid};
    }

    /**
     * @brief Returns the type of command provided by the input string
     * @param command command to be typed
     * @returns a TCode Command Type e.g. Axis, Device, Setup if it is not a valid command None is returned
     */
    static TCode_Command_Type getCommandType(const String &command) // for a given string finds what command it is related to
    {
        // Switch between command types
        switch (toupper(command.charAt(0)))
        {
        // Axis commands
        case 'L':
        case 'R':
        case 'V':
        case 'A':
            return TCode_Command_Type::Axis;
        // Device commands
        case 'D':
            return TCode_Command_Type::Device;
        // Setup commands
        case '$':
            return TCode_Command_Type::Setup;
        case '#':
            return TCode_Command_Type::External;
        }
        return TCode_Command_Type::None;
    }

    /**
     * @brief Parses an Axis Command
     * @param input string to be processed
     * @returns a TCode Axis command struct so that the command can be executed easier
     */
    static TCode_Axis_Command parseAxisCommand(const String &input) // Parses an Axis command
    {
        bool valid = true;
        int index = 0;
        TCode_ChannelID id = getIDFromStr(input, index); // Get the ID from the string
        int command_value = 0;
        long command_value_extention = 0;
        TCode_Axis_Extention_Type extention = TCode_Axis_Extention_Type::None;
        TCode_Axis_Ramp_Type rampType = TCode_Axis_Ramp_Type::Linear;

        index += 2;
        command_value = getNextInt(input, index); // Get the target value for the index
        if (command_value == -1)
            valid = false;

        command_value = constrain(command_value, 0, TCODE_MAX_AXIS); // constrain the value to the correct range

        if (isextention(toupper(input.charAt(index)))) // if the next char is an extention then find the value for the extention
        {
            switch (toupper(input.charAt(index++)))
            {
            case 'I':
                extention = TCode_Axis_Extention_Type::Time;
                break;
            case 'S':
                extention = TCode_Axis_Extention_Type::Speed;
                break;
            }
            command_value_extention = getNextInt(input, index); // get the extention value
            if (command_value_extention == -1)
                valid = false;
            command_value_extention = constrain(command_value_extention, 0, TCODE_MAX_AXIS_MAGNITUDE); // constrain the extention value to the correct range for an extention
        }

        if (extention != TCode_Axis_Extention_Type::None) // if there was no extention then skip else find the ramp type
        {
            char first = toupper(input.charAt(index++));
            char second = toupper(input.charAt(index));
            switch (first) // Decode what Ramp type it is
            {
            case '<':
                if (second == '>')
                {
                    rampType = TCode_Axis_Ramp_Type::EaseInOut;
                    index++;
                }
                else
                {
                    rampType = TCode_Axis_Ramp_Type::EaseIn;
                }
                break;
            case '>':
                rampType = TCode_Axis_Ramp_Type::EaseOut;
                break;
            case '=':
                rampType = TCode_Axis_Ramp_Type::Linear;
                break;
            }
        }

        if (toupper(input.charAt(index)) != '\0') // if the command has been processed and there are still characters left over the command has not been processed correctly/the command is incorrect
            valid = false;
        if (!id.valid) // make sure that the ID is valid if it isnt then the command is not valid
            valid = false;

        return {valid, id, extention, rampType, command_value, command_value_extention};
    };

    /**
     * @brief Parses a Device Command
     * @param input string to be processed
     * @returns a TCode Device Command which is a data representation of the device command to be processed easier
     */
    static TCode_Device_Command parseDeviceCommand(const String &input) // Parses a Device command
    {
        switch (toupper(input.charAt(1))) // looks at the first char and checks if it matches the values
        {
        case 'S':
            return {true, TCode_Device_Command_Type::StopDevice};
            break;
        case '0':
            return {true, TCode_Device_Command_Type::GetSoftwareVersion};
            break;
        case '1':
            return {true, TCode_Device_Command_Type::GetTCodeVersion};
            break;
        case '2':
            return {true, TCode_Device_Command_Type::GetAssignedAxisValues};
            break;
        }
        return {false, TCode_Device_Command_Type::None};
    };

    /**
     * @brief Parses a Setup Command
     * @param input string to be processed
     * @returns a TCode Setup command struct in a data representation to be processed easier
     */
    static TCode_Setup_Command parseSetupCommand(const String &input) // Parses a Setup command
    {
        bool valid = true;
        int index = 1;
        TCode_ChannelID id = getIDFromStr(input, index); // get the ID
        index += 2;
        if (toupper(input.charAt(index++)) != '-')
            valid = false;
        long min_value = getNextInt(input, index); // Get the first minimum value
        if (toupper(input.charAt(index++)) != '-')
            valid = false;

        long max_value = getNextInt(input, index); // Get the seccond maximum value
        if ((toupper(input.charAt(index)) != '\0') || (min_value == -1) || (max_value == -1) || (!id.valid))
            valid = false;
        min_value = constrain(min_value, 0, TCODE_MAX_AXIS); // constrain the values to the maximum axis constraint
        max_value = constrain(max_value, 0, TCODE_MAX_AXIS);
        if (min_value > max_value) // if the minimum is larger than the maximum the command is not valid
            valid = false;

        TCode_Save_Entry values = {min_value, max_value};
        return {valid, id, values};
    };

    static TCode_External_Command parseExternalCommand(const String &input) // Parses a Setup command
    {
        return {true, input};
    };

};
#endif