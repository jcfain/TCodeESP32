/* MIT License

Copyright (c) 2024 Jason C. Fain

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
enum class CommandValueType {
    NONE,
    NUMBER,
    STRING
};
enum class SaveRequired {
    NO,
    YES
};
enum class RestartRequired {
    NO,
    YES
};
struct CommandBase {
    public:
    // CommandBase(const char* name_, const char* command_, const char* description_, const SaveRequired isSaveRequired_, const RestartRequired isRestartRequired_, const CommandValueType valueType_) :  
    //     name(name_), 
    //     command(command_), 
    //     description(description_), 
    //     isSaveRequired(isSaveRequired_), 
    //     isRestartRequired(isRestartRequired_), 
    //     valueType(valueType_) {}
    const char* name;
    const char* command;
    const char* description;
    const SaveRequired isSaveRequired;
    const RestartRequired isRestartRequired;
    const CommandValueType valueType;
};
struct Command : CommandBase {
    Command(CommandBase commandbase_, std::function<bool()> callback_) :  
    CommandBase(commandbase_),
    callback(callback_) {}
    std::function<bool()> callback;
};
template<typename T>
struct CommandValue: CommandBase {
    CommandValue(CommandBase commandbase_, std::function<bool(T value)> callback_) :
    // CommandValue(const char* name_, const char* command_, const char* description_, const SaveRequired isSaveRequired_, const RestartRequired isRestartRequired_, const CommandValueType valueType_, std::function<bool(T value)> callback_) :  
    // CommandBase(name_, 
    // command_, 
    // description_, 
    // isSaveRequired_, 
    // isRestartRequired_, 
    // valueType_),
    CommandBase(commandbase_),
    callback(callback_) {}
    std::function<bool(T value)> callback;
};