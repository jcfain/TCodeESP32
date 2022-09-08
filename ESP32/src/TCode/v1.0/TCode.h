// TCode-Class-H v1.0,
// protocal by TempestMAx (https://www.patreon.com/tempestvr)
// implemented by Eve 05/02/2022
// usage of this class can be found at (https://github.com/Dreamer2345/Arduino_TCode_Parser)
// Please copy, share, learn, innovate, give attribution.
// Decodes T-code commands
// It can handle:
//   x linear channels (L0, L1, L2... L9)
//   x rotation channels (R0, R1, R2... L9)
//   x vibration channels (V0, V1, V2... V9)
//   x auxilliary channels (A0, A1, A2... A9)
// History:
//
#pragma once
#ifndef TCODE_H
#define TCODE_H
#include "Arduino.h"
#include "TCodeConstants.h"
#include "TCodeEnums.h"
#include "TCodeParser.h"
#include "TCodeAxis.h"
#include "TCodeBuffer.h"

#define TCODE_MAX_CHANNEL_COUNT 10
#define TCODE_CHANNEL_TYPES 4
#define CURRENT_TCODE_VERSION "TCode v0.4"

#ifndef TCODE_EEPROM_MEMORY_OFFSET
#define TCODE_EEPROM_MEMORY_OFFSET 0
#endif

#define TCODE_EEPROM_MEMORY_ID "TCODE"
#define TCODE_EEPROM_MEMORY_ID_LENGTH 5

#ifndef TCODE_USE_EEPROM
#define TCODE_USE_EEPROM true
#endif

#ifndef TCODE_COMMAND_BUFFER_LENGTH
#define TCODE_COMMAND_BUFFER_LENGTH 255
#endif

using TCODE_FUNCTION_PTR_T = void (*)(const String &input);


template <unsigned TCODE_CHANNEL_COUNT = 5>
class TCode
{
public:
    static_assert((TCODE_CHANNEL_COUNT > 0) && (TCODE_CHANNEL_COUNT <= TCODE_MAX_CHANNEL_COUNT), "TCode Channel Count must be larger than or equal to 1 but less than or equal to 10");
    static constexpr uintmax_t EEPROM_SIZE = TCODE_CHANNEL_COUNT * TCODE_CHANNEL_TYPES * sizeof(TCode_Save_Entry) + TCODE_EEPROM_MEMORY_ID_LENGTH;

    // Initalization
    TCode(const String &firmware);                              // Constructor for class using defined TCode Version number
    TCode(const String &firmware, const String &TCode_version); // Constructor for class using user defined TCode Version number
    void init();                                                // Initalizes the EEPROM and checks for the magic string

    // Input Functions
    void inputByte(byte input);            // Function to read off individual byte as input to the command buffer
    void inputChar(char input);            // Function to read off individual char as input to the command buffer
    void inputString(const String &input); // Function to take in a string as input to the command buffer
    void clearBuffer();

    // Axis Functions

    int axisRead(const TCode_ChannelID &id);                                                                    // Function to read the current position of an axis
    void axisWrite(const TCode_ChannelID &id, int magnitude, TCode_Axis_Extention_Type ext, long extMagnitude); // Function to set an axis
    unsigned long axisLastT(const TCode_ChannelID &id);                                                         // Function to query when an axis was last commanded
    void axisRegister(const TCode_ChannelID &id, String Name);                                                  // Function to name and activate axis
    bool axisChanged(const TCode_ChannelID &id);                                                                // Function to check if an axis has changed
    void axisRampType(const TCode_ChannelID &id, const TCode_Axis_Ramp_Type e);                                 // Function to set the ramp type of an axis

    // Repeated Axis Functions which only take type and channel number

    int axisRead(const TCode_Channel_Type &type, const int channel_int);
    void axisWrite(const TCode_Channel_Type &type, const int channel_int, int magnitude, TCode_Axis_Extention_Type ext, long extMagnitude);
    unsigned long axisLastT(const TCode_Channel_Type &type, const int channel_int);
    void axisRegister(const TCode_Channel_Type &type, const int channel_int, String Name);
    bool axisChanged(const TCode_Channel_Type &type, const int channel_int);
    void axisRampType(const TCode_Channel_Type &type, const int channel_int, const TCode_Axis_Ramp_Type e);

// Repeated Axis Functions depriciated but still here for backwards compatability if TCODE_USE_STRING_IDS is defined
#if defined(TCODE_USE_STRING_IDS)
    int axisRead(const String &id);
    void axisWrite(const String &id, int magnitude, const String &ext, long extMagnitude);
    unsigned long axisLastT(const String &id);
    void axisRegister(const String &id, String Name);
    bool axisChanged(const String &id);
    void axisRampType(const String &id, const TCode_Axis_Ramp_Type e);
#endif

    // Device Functions
    void stop(); // Function stops all outputs

    void setMessageCallback(TCODE_FUNCTION_PTR_T function); // Function to set the used message callback this can be used to change the method of message transmition (if nullptr is passed to this function the default callback will be used)
    void sendMessage(const String &s);                      // Function which calls the callback (the default callback for TCode is Serial communication)

    void setExternalCommandCallback(TCODE_FUNCTION_PTR_T function);
    void sendExternalCommand(const String &s);

private:
    String versionID;
    String firmwareID;

    TCodeBuffer<TCODE_COMMAND_BUFFER_LENGTH> buffer;

    TCodeAxis Linear[TCODE_CHANNEL_COUNT];
    TCodeAxis Rotation[TCODE_CHANNEL_COUNT];
    TCodeAxis Vibration[TCODE_CHANNEL_COUNT];
    TCodeAxis Auxiliary[TCODE_CHANNEL_COUNT];

    TCODE_FUNCTION_PTR_T message_callback;
    TCODE_FUNCTION_PTR_T external_command_callback;

    static void defaultCallback(const String &input); // Function which is the default callback for TCode this uses the serial communication if it is setup with Serial.begin() if it is not setup then nothing happens
    static void defaultExternalCallback(const String &input);

    void axisRow(const TCode_ChannelID &id, const String &name);  // Function to return the details of an axis stored in the EEPROM
    bool tryGetAxis(const TCode_ChannelID &id, TCodeAxis *&axis); // Function to get the axis specified by a decoded id if successfull returns true

    void executeNextBufferCommand();
    void readCommand(const String &command);                    // Function to process the individual commands
    void runAxisCommand(const TCode_Axis_Command &command);     // Function to read and interpret axis commands
    void runDeviceCommand(const TCode_Device_Command &command); // Function to identify and execute device commands
    void runSetupCommand(const TCode_Setup_Command &command);   // Function to modify axis preference values
    void runExternalCommand(const TCode_External_Command &command); //Passes the output of the parser to an external callback

    bool checkMemoryKey();                                                            // Function to check if a memory key "TCODE" has been placed in the EEPROM at the location TCODE_EEPROM_MEMORY_OFFSET
    void placeMemoryKey();                                                            // Function to place the memory key at location TCODE_EEPROM_MEMORY_OFFSET
    void resetMemory();                                                               // Function to reset the stored values area of the EEPROM
    int getHeaderEnd();                                                               // Function to get the memory location of the start of the stored values area
    int getMemoryLocation(const TCode_ChannelID &id);                                 // Function to get the memory location of an ID
    void updateSavedMemory(const TCode_ChannelID &id, const TCode_Save_Entry &entry); // Function to update the memory location of an id

    void commitEEPROMChanges();                 // Function abstracts the commit function for different board types;
    byte readEEPROM(const int idx);             // Function abstracts the EEPROM read command so that it can be redefined if need be for different board types
    void writeEEPROM(const int idx, byte byte); // Function abstracts the EEPROM write command so that it can be redefined if need be for different board types
    template <typename T>
    T &getEEPROM(const int idx, T &t); // Function abstracts the EEPROM get command so that it can be redefined if need be for different board types
    template <typename T>
    void putEEPROM(const int idx, T t); // Function abstracts the EEPROM put command so that it can be redefined if need be for different board types
};

template <unsigned TCODE_CHANNEL_COUNT>
TCode<TCODE_CHANNEL_COUNT>::TCode(const String &firmware)
{
    buffer.clear();
    firmwareID = firmware;
    versionID = CURRENT_TCODE_VERSION;
    stop();
    setMessageCallback(nullptr);
    setExternalCommandCallback(nullptr);
}

template <unsigned TCODE_CHANNEL_COUNT>
TCode<TCODE_CHANNEL_COUNT>::TCode(const String &firmware, const String &TCode_version)
{
    buffer.clear();
    firmwareID = firmware;
    versionID = TCode_version;
    stop();
    setMessageCallback(nullptr);
    setExternalCommandCallback(nullptr);
}

//=========================================================================================================
// Command Input

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::clearBuffer() // clears the buffer
{
    buffer.clear();
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::inputByte(byte input) // inputs a byte into the buffer
{
    inputChar(static_cast<char>(input));
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::inputChar(char input)
{
    bool success = buffer.push(input); // pushes the inputted char to the buffer
    if (!success && buffer.isFull())                               // if the buffer is full then execute the first command and then push the char to the buffer
    {
        executeNextBufferCommand();
        buffer.push(static_cast<char>(input));
    }

    if (input == '\n') // if a newline is encountered run all commands till the buffer is empty
    {
        while (!buffer.isEmpty())
            executeNextBufferCommand();
        buffer.clear();
    }
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::inputString(const String &input) // inputs a string into the buffer
{
    for (int i = 0; i < input.length(); i++)
    {
        inputChar(input.charAt(i));
    }
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::executeNextBufferCommand() // Reads the next command to be executed from the buffer and executes the command
{
    String subbuf = String("");
    int blevel = 0;
    while (!buffer.isEmpty())
    {
        if(buffer.peek() == '[')
            blevel += 1;
        if(buffer.peek() == ']')
            blevel -= 1;

        if(buffer.peek() == ' ' && (blevel == 0))
            break;
        if(buffer.peek() == '\n')
            break;
        if(blevel < 0)
            break;

        subbuf += buffer.pop();
    }

    if (!buffer.isEmpty()) // if the buffer is not empty if the next char is a space or newline remove it from the buffer
    {
        if (buffer.peek() == ' ')
        {
            buffer.pop();
        }
        else if (buffer.peek() == '\n')
        {
            buffer.pop();
        }
    }

    readCommand(subbuf);
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::readCommand(const String &command)
{
    TCode_Command_Type type = TCodeParser::getCommandType(command); // find what command was read
    // Switch between command types
    switch (type) // depending on the command type found parse the inputted command and execute the correct command function
    {
    case TCode_Command_Type::Axis:
    {
        TCode_Axis_Command result = TCodeParser::parseAxisCommand(command);
        runAxisCommand(result);
        break;
    }
    case TCode_Command_Type::Device:
    {
        TCode_Device_Command result = TCodeParser::parseDeviceCommand(command);
        runDeviceCommand(result);
        break;
    }
    case TCode_Command_Type::Setup:
    {
        TCode_Setup_Command result = TCodeParser::parseSetupCommand(command);
        runSetupCommand(result);
        break;
    }
    case TCode_Command_Type::External:
    {
        TCode_External_Command result = TCodeParser::parseExternalCommand(command);
        runExternalCommand(result);
        break;
    }
    default:
        break;
    }
}

//=========================================================================================================
// Axis Handling

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::stop() // Stop all channels
{
    for (int i = 0; i < TCODE_CHANNEL_COUNT; i++)
    {
        Linear[i].stop();
    }
    for (int i = 0; i < TCODE_CHANNEL_COUNT; i++)
    {
        Rotation[i].stop();
    }
    for (int i = 0; i < TCODE_CHANNEL_COUNT; i++)
    {
        Vibration[i].set(0, TCode_Axis_Extention_Type::None, 0);
    }
    for (int i = 0; i < TCODE_CHANNEL_COUNT; i++)
    {
        Auxiliary[i].stop();
    }
}

template <unsigned TCODE_CHANNEL_COUNT>
bool TCode<TCODE_CHANNEL_COUNT>::axisChanged(const TCode_ChannelID &id) // Get if the axis has changed
{
    TCodeAxis *axis = nullptr;
    if (tryGetAxis(id, axis))
    {
        return axis->changed();
    }
    return false;
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::axisRampType(const TCode_ChannelID &id, TCode_Axis_Ramp_Type e) // Set the ramp type of an axis
{
    TCodeAxis *axis = nullptr;
    if (tryGetAxis(id, axis))
    {
        axis->setRampType(e);
    }
}

template <unsigned TCODE_CHANNEL_COUNT>
int TCode<TCODE_CHANNEL_COUNT>::axisRead(const TCode_ChannelID &id) // Read the value of an axis
{
    int value = TCODE_DEFAULT_AXIS_RETURN_VALUE;
    TCodeAxis *axis = nullptr;
    if (tryGetAxis(id, axis))
    {
        value = axis->getPosition();
    }
    return value;
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::axisWrite(const TCode_ChannelID &inputID, int magnitude, TCode_Axis_Extention_Type extension, long extMagnitude) // set the parameters of an axis
{
    TCodeAxis *axis = nullptr;
    if (tryGetAxis(inputID, axis))
    {
        axis->set(magnitude, extension, extMagnitude);
    }
}

template <unsigned TCODE_CHANNEL_COUNT>
unsigned long TCode<TCODE_CHANNEL_COUNT>::axisLastT(const TCode_ChannelID &inputID) // get the last time the axis was updated in millis
{
    unsigned long t = 0; // Return time
    TCodeAxis *axis = nullptr;
    if (tryGetAxis(inputID, axis))
    {
        t = axis->lastT;
    }
    return t;
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::axisRegister(const TCode_ChannelID &inputID, String axisName) // register an axis with a string name
{
    TCodeAxis *axis = nullptr;
    if (tryGetAxis(inputID, axis))
    {
        axis->setName(axisName);
    }
}

// Duplicated functions to provide compatability.

template <unsigned TCODE_CHANNEL_COUNT>
int TCode<TCODE_CHANNEL_COUNT>::axisRead(const TCode_Channel_Type &type, const int channel_int)
{
    return axisRead(TCodeParser::constructID(type, channel_int));
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::axisWrite(const TCode_Channel_Type &type, const int channel_int, int magnitude, TCode_Axis_Extention_Type ext, long extMagnitude)
{
    axisWrite(TCodeParser::constructID(type, channel_int), magnitude, ext, extMagnitude);
}

template <unsigned TCODE_CHANNEL_COUNT>
unsigned long TCode<TCODE_CHANNEL_COUNT>::axisLastT(const TCode_Channel_Type &type, const int channel_int)
{
    return axisLastT(TCodeParser::constructID(type, channel_int));
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::axisRegister(const TCode_Channel_Type &type, const int channel_int, String Name)
{
    axisRegister(TCodeParser::constructID(type, channel_int), Name);
}

template <unsigned TCODE_CHANNEL_COUNT>
bool TCode<TCODE_CHANNEL_COUNT>::axisChanged(const TCode_Channel_Type &type, const int channel_int)
{
    return axisChanged(TCodeParser::constructID(type, channel_int));
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::axisRampType(const TCode_Channel_Type &type, const int channel_int, const TCode_Axis_Ramp_Type e)
{
    axisRampType(TCodeParser::constructID(type, channel_int), e);
}

// Depriciated

#if defined(TCODE_USE_STRING_IDS)
template <unsigned TCODE_CHANNEL_COUNT>
int TCode<TCODE_CHANNEL_COUNT>::axisRead(const String &id)
{
    return axisRead(TCodeParser::getIDFromStr(id, 0));
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::axisWrite(const String &id, int magnitude, const String &ext, long extMagnitude)
{
    axisWrite(TCodeParser::getIDFromStr(id, 0), magnitude, TCodeParser::getExtentionTypeFromStr(ext, 0), extMagnitude);
}

template <unsigned TCODE_CHANNEL_COUNT>
unsigned long TCode<TCODE_CHANNEL_COUNT>::axisLastT(const String &id)
{
    return axisLastT(TCodeParser::getIDFromStr(id, 0));
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::axisRegister(const String &id, String Name)
{
    axisRegister(TCodeParser::getIDFromStr(id, 0), Name);
}

template <unsigned TCODE_CHANNEL_COUNT>
bool TCode<TCODE_CHANNEL_COUNT>::axisChanged(const String &id)
{
    return axisChanged(TCodeParser::getIDFromStr(id, 0));
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::axisRampType(const String &id, const TCode_Axis_Ramp_Type e)
{
    axisRampType(TCodeParser::getIDFromStr(id, 0), e);
}
#endif

//=========================================================================================================
// EEPROM HANDLING

template <unsigned TCODE_CHANNEL_COUNT>
int TCode<TCODE_CHANNEL_COUNT>::getHeaderEnd()
{
    return TCODE_EEPROM_MEMORY_OFFSET + TCODE_EEPROM_MEMORY_ID_LENGTH;
}

template <unsigned TCODE_CHANNEL_COUNT>
int TCode<TCODE_CHANNEL_COUNT>::getMemoryLocation(const TCode_ChannelID &id) // Get the EEPROM memory location of the saved values
{
    int memloc = -1;
    if (TCodeParser::idValid(id))
    {
        int typeoffset = -1;
        switch (id.type)
        {
        case TCode_Channel_Type::Linear:
            typeoffset = 0;
            break;
        case TCode_Channel_Type::Rotation:
            typeoffset = 1;
            break;
        case TCode_Channel_Type::Vibration:
            typeoffset = 2;
            break;
        case TCode_Channel_Type::Auxiliary:
            typeoffset = 3;
            break;
        default:
            return -2;
        }
        int typebyteoffset = sizeof(TCode_Save_Entry) * TCODE_CHANNEL_COUNT * typeoffset;
        int entrybyteoffset = sizeof(TCode_Save_Entry) * id.channel;
        memloc = typebyteoffset + entrybyteoffset + getHeaderEnd();
    }
    return memloc;
}

template <unsigned TCODE_CHANNEL_COUNT>
bool TCode<TCODE_CHANNEL_COUNT>::checkMemoryKey() // Checks the EEPROM for the saved Magic Key which if not present means the EEPROM is not setup
{
    char b[TCODE_EEPROM_MEMORY_ID_LENGTH + 1];
    for (int i = 0; i < TCODE_EEPROM_MEMORY_ID_LENGTH; i++)
        b[i] = (char)readEEPROM(TCODE_EEPROM_MEMORY_OFFSET + i);
    return (String(b) == String(TCODE_EEPROM_MEMORY_ID));
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::placeMemoryKey() // Places the EEPROM Magic key into the EEPROM
{
    for (int i = 0; i < TCODE_EEPROM_MEMORY_ID_LENGTH; i++)
    {
        writeEEPROM(TCODE_EEPROM_MEMORY_OFFSET + i, TCODE_EEPROM_MEMORY_ID[i]);
    }
    commitEEPROMChanges();
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::resetMemory() // Resets the EEPROM stored values
{
    TCode_Save_Entry defaultSave = {0, TCODE_MAX_AXIS};
    int headerEnd = getHeaderEnd();
    for (int j = 0; j < TCODE_CHANNEL_TYPES; j++)
    {
        for (int i = 0; i < TCODE_CHANNEL_COUNT; i++)
        {
            int memloc = (sizeof(TCode_Save_Entry) * TCODE_CHANNEL_COUNT * j) + (sizeof(TCode_Save_Entry) * i) + headerEnd;
            putEEPROM(memloc, defaultSave);
        }
    }
    commitEEPROMChanges();
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::updateSavedMemory(const TCode_ChannelID &id, const TCode_Save_Entry &entry) // updates the saved memory values for a specific channel
{
    if (!TCodeParser::idValid(id))
        return;

    int memloc = getMemoryLocation(id);
    if (memloc >= 0)
    {
        putEEPROM(memloc, entry);
        commitEEPROMChanges();
    }
}
//=========================================================================================================
// Command Execution

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::runAxisCommand(const TCode_Axis_Command &command)
{
    if (!command.valid)
        return;

    if (!TCodeParser::idValid(command.ID))
        return;

    if (command.ID.channel > TCODE_CHANNEL_COUNT) // Check if the ID is out of bounds for the given number of internaly stored channels
        return;

    TCodeAxis *axis = nullptr;
    if (tryGetAxis(command.ID, axis))
    {
        axis->set(command.command_value, command.extention_type, command.command_value_extention);
        if (command.ramp_type != TCode_Axis_Ramp_Type::None)
            axis->setRampType(command.ramp_type);
    }
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::runDeviceCommand(const TCode_Device_Command &command)
{
    if (!command.valid)
        return;

    switch (command.type)
    {
    case TCode_Device_Command_Type::StopDevice:
        stop();
        break;
    case TCode_Device_Command_Type::GetTCodeVersion:
        sendMessage(firmwareID + '\n');
        break;
    case TCode_Device_Command_Type::GetSoftwareVersion:
        sendMessage(versionID + '\n');
        break;
    case TCode_Device_Command_Type::GetAssignedAxisValues:
        for (int i = 0; i < TCODE_CHANNEL_COUNT; i++)
        {
            axisRow(TCodeParser::constructID(TCode_Channel_Type::Linear, i), Linear[i].getName());
        }
        for (int i = 0; i < TCODE_CHANNEL_COUNT; i++)
        {
            axisRow(TCodeParser::constructID(TCode_Channel_Type::Rotation, i), Rotation[i].getName());
        }
        for (int i = 0; i < TCODE_CHANNEL_COUNT; i++)
        {
            axisRow(TCodeParser::constructID(TCode_Channel_Type::Vibration, i), Vibration[i].getName());
        }
        for (int i = 0; i < TCODE_CHANNEL_COUNT; i++)
        {
            axisRow(TCodeParser::constructID(TCode_Channel_Type::Auxiliary, i), Auxiliary[i].getName());
        }
        break;
    }
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::runSetupCommand(const TCode_Setup_Command &command)
{
    if (!command.valid)
        return;

    if (!TCodeParser::idValid(command.ID))
        return;

    if (command.ID.channel > TCODE_CHANNEL_COUNT) // Check if the ID is out of bounds for the given number of internaly stored channels
        return;

    if (!TCODE_USE_EEPROM)
        sendMessage(F("EEPROM NOT IN USE\n"));

    updateSavedMemory(command.ID, command.Save);
    switch (command.ID.type)
    {
    case TCode_Channel_Type::Linear:
        axisRow(command.ID, Linear[command.ID.channel].getName());
        break;
    case TCode_Channel_Type::Rotation:
        axisRow(command.ID, Rotation[command.ID.channel].getName());
        break;
    case TCode_Channel_Type::Vibration:
        axisRow(command.ID, Vibration[command.ID.channel].getName());
        break;
    case TCode_Channel_Type::Auxiliary:
        axisRow(command.ID, Auxiliary[command.ID.channel].getName());
        break;
    }
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::runExternalCommand(const TCode_External_Command &command)
{
    if (!command.valid)
        return;

    sendExternalCommand(command.command + '\n');
}

//=========================================================================================================
// Internal Axis Commands

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::axisRow(const TCode_ChannelID &id, const String &name) // Sends to the controler the settings for an axis
{
    if (name == "")
        return;

    if (!TCodeParser::idValid(id))
        return;

    int memloc = getMemoryLocation(id);
    if (memloc >= 0)
    {
        TCode_Save_Entry save;
        getEEPROM(memloc, save);
        sendMessage(TCodeParser::getStrfromID(id) + ' ' + String(save.min) + ' ' + String(save.max) + " " + name + '\n');
    }
}

template <unsigned TCODE_CHANNEL_COUNT>
bool TCode<TCODE_CHANNEL_COUNT>::tryGetAxis(const TCode_ChannelID &id, TCodeAxis *&axis) // Trys to get the axis for the ID given
{
    axis = nullptr;

    if (!TCodeParser::idValid(id)) // Check if the ID is valid
        return false;

    if (id.channel > TCODE_CHANNEL_COUNT) // Check if the ID is out of bounds for the given number of internaly stored channels
        return false;

    switch (id.type)
    {
    case TCode_Channel_Type::Linear:
        axis = &Linear[id.channel];
        return true;
    case TCode_Channel_Type::Rotation:
        axis = &Rotation[id.channel];
        return true;
    case TCode_Channel_Type::Vibration:
        axis = &Vibration[id.channel];
        return true;
    case TCode_Channel_Type::Auxiliary:
        axis = &Auxiliary[id.channel];
        return true;
    }

    return false;
}

//=========================================================================================================
// Message Callbacks

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::defaultCallback(const String &input) // Default callback used by TCode uses serial communication
{
    if (Serial)
    {
        Serial.print(input);
    }
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::setMessageCallback(TCODE_FUNCTION_PTR_T f) // Sets the callback function used by TCode
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

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::sendMessage(const String &s) // Used to communicate with the controling device
{
    if (message_callback != nullptr)
        message_callback(s);
}

//
//External Command Callback

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::defaultExternalCallback(const String &input) // Default callback used by TCode uses serial communication
{
    if (Serial)
    {
        Serial.print(input);
    }
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::setExternalCommandCallback(TCODE_FUNCTION_PTR_T f) // Sets the callback function used by TCode
{
    if (f == nullptr)
    {
        external_command_callback = &defaultExternalCallback;
    }
    else
    {
        external_command_callback = f;
    }
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::sendExternalCommand(const String &s) // Used to communicate with the controling device
{
    if (external_command_callback != nullptr)
        external_command_callback(s);
}


//=========================================================================================================
// PER BOARD CODE AREA
#if defined(ARDUINO_ESP32_DEV)
#include <EEPROM.h>
template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::init()
{
    if (!EEPROM.begin(EEPROM_SIZE))
    {
        sendMessage(F("EEPROM failed to initialise\n"));
    }
    else
    {
        sendMessage(F("EEPROM initialised\n"));
    }

    if (!checkMemoryKey() && TCODE_USE_EEPROM)
    {
        placeMemoryKey();
        resetMemory();
        commitEEPROMChanges();
    }
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::commitEEPROMChanges()
{
    if (EEPROM.commit())
    {
        sendMessage(F("EEPROM successfully committed!\n"));
    }
    else
    {
        sendMessage(F("ERROR! EEPROM commit failed!\n"));
    }
}

template <unsigned TCODE_CHANNEL_COUNT>
byte TCode<TCODE_CHANNEL_COUNT>::readEEPROM(int idx)
{
    return EEPROM.read(idx);
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::writeEEPROM(int idx, byte b)
{
    EEPROM.write(idx, b);
}

template <unsigned TCODE_CHANNEL_COUNT>
template <typename T>
T &TCode<TCODE_CHANNEL_COUNT>::getEEPROM(int idx, T &t)
{
    return EEPROM.get(idx, t);
}

template <unsigned TCODE_CHANNEL_COUNT>
template <typename T>
void TCode<TCODE_CHANNEL_COUNT>::putEEPROM(int idx, T t)
{
    EEPROM.put(idx, t);
}

#elif defined(ARDUINO_SAMD_NANO_33_IOT) // Nano 33 IOT
#include <FlashAsEEPROM.h>
template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::init()
{
    if (!EEPROM.begin())
    {
        sendMessage(F("EEPROM failed to initialise\n"));
    }
    else
    {
        sendMessage(F("EEPROM initialised\n"));
    }

    if (!checkMemoryKey() && TCODE_USE_EEPROM)
    {
        placeMemoryKey();
        resetMemory();
        commitEEPROMChanges();
    }
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::commitEEPROMChanges()
{
    if (EEPROM.commit())
    {
        sendMessage(F("EEPROM successfully committed!\n"));
    }
    else
    {
        sendMessage(F("ERROR! EEPROM commit failed!\n"));
    }
}

template <unsigned TCODE_CHANNEL_COUNT>
byte TCode<TCODE_CHANNEL_COUNT>::readEEPROM(int idx)
{
    return EEPROM.read(idx);
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::writeEEPROM(int idx, byte b)
{
    EEPROM.write(idx, b);
}

template <unsigned TCODE_CHANNEL_COUNT>
template <typename T>
T &TCode<TCODE_CHANNEL_COUNT>::getEEPROM(int idx, T &t)
{
    uint8_t *ptr = (uint8_t *)&t;
    for (int count = sizeof(T); count; --count)
        *ptr++ = EEPROM.read(idx);
    return t;
}

template <unsigned TCODE_CHANNEL_COUNT>
template <typename T>
void TCode<TCODE_CHANNEL_COUNT>::putEEPROM(int idx, T t)
{
    const uint8_t *ptr = (const uint8_t *)&t;
    size_t e = 0;
    for (int count = sizeof(T); count; --count, e++)
        EEPROM.update(idx + e, *ptr++);
}
#else // Uses the default arduino methods for setting EEPROM
#include <EEPROM.h>
template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::init()
{
    if (!checkMemoryKey() && TCODE_USE_EEPROM)
    {
        placeMemoryKey();
        resetMemory();
        commitEEPROMChanges();
    }
    sendMessage(F("EEPROM initialised\n"));
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::commitEEPROMChanges()
{
}

template <unsigned TCODE_CHANNEL_COUNT>
byte TCode<TCODE_CHANNEL_COUNT>::readEEPROM(int idx)
{
    return EEPROM.read(idx);
}

template <unsigned TCODE_CHANNEL_COUNT>
void TCode<TCODE_CHANNEL_COUNT>::writeEEPROM(int idx, byte b)
{
    EEPROM.write(idx, b);
}

template <unsigned TCODE_CHANNEL_COUNT>
template <typename T>
T &TCode<TCODE_CHANNEL_COUNT>::getEEPROM(int idx, T &t)
{
    return EEPROM.get(idx, t);
}

template <unsigned TCODE_CHANNEL_COUNT>
template <typename T>
void TCode<TCODE_CHANNEL_COUNT>::putEEPROM(int idx, T t)
{
    EEPROM.put(idx, t);
}
#endif

#endif