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
#include "setting.h"
enum class SaveRequired {
    NO,
    YES
};
struct CommandBase {
    public:
    const char* name;
    const char* command;
    const char* description;
    const SaveRequired isSaveRequired;
    const RestartRequired isRestartRequired;
    const SettingType valueType;
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
        CommandBase(commandbase_),
        callback(callback_) {}
    CommandValue(const Setting &setting, std::function<bool(T value)> callback_) :
        CommandBase({setting.friendlyName, setting.name, setting.description, SaveRequired::YES, setting.isRestartRequired, setting.type}),
        callback(callback_) {}
    std::function<bool(T value)> callback;
};