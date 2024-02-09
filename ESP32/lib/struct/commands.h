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
#include "command.hpp"

class Commands {
public:
    const char DELEMITER = '#';
    const char DELEMITER_SAVE = '$'; 
    const char DELEMITER_VALUE = ':'; 
    const Command HELP{"Help", "#help", "Print the help screen", CommandValueType::NONE, []() -> bool{
		return execute([]() -> bool {
			printCommandHelp();
			return true;
		});
	}};
    const Command SAVE{"Help", "$save", "Saves all settings", CommandValueType::NONE, []() -> bool{return true;}};
    const Command DEFAULT_ALL{"Default all", "$defaultAll", "Saves all settings to default", CommandValueType::NONE, []() -> bool{return true;}};
    const Command RESTART{"Restart", "#restart", "Restart the system", CommandValueType::NONE, []() -> bool{return true;}};
    const Command CLEAR_LOGS_INCLUDE{"Clear log include", "#clear-log-include", "Clears all the log included tags", CommandValueType::NONE, []() -> bool{return true;}};
    const Command CLEAR_LOGS_EXCLUDE{"Clear log exclude", "#clear-log-exclude", "Clears all the log excluded tags", CommandValueType::NONE, []() -> bool{return true;}};
    const Command MOTION_ENABLE{"Motion enable", "#motion-enable", "Enables the motion generator", CommandValueType::NONE, []() -> bool{return true;}};
    const Command MOTION_DISABLE{"Motion disable", "#motion-disable", "Disables the motion generator", CommandValueType::NONE, []() -> bool{return true;}};
    const Command MOTION_TOGGLE{"Motion toggle", "#motion-toggle", "Toggles the motion generator", CommandValueType::NONE, []() -> bool{return true;}};
    const Command MOTION_PROFILE_CYCLE{"Motion profile cycle", "#motion-profile-cycle", "Cycles the motion generator profiles stopping after last profile", CommandValueType::NONE, []() -> bool{return true;}};
    //const Command PAUSE_TOGGLE{"Wifi ssid", "#pause-toggle", "Pauses all motion of the device", CommandValueType::NONE, []() -> bool{}};
    const CommandValue<const char*>WIFI_SSID{"Wifi ssid", "#wifi-ssid", "Sets the ssid of the wifi AP", CommandValueType::STRING, [](const char* value) -> bool{
		return validateMaxLength("Wifi SSID", value, sizeof(SettingsHandler::ssid), false, [](const char* value) -> bool {
			strcpy(SettingsHandler::ssid, value);
			return true;
		}, true, true); 
	}};
    const CommandValue<const char*>WIFI_PASS{"Wifi pass", "#wifi-pass", "Sets the password of the wifi AP", CommandValueType::STRING, [](const char* value) -> bool{return true;}};
    const CommandValue<const int>LOG_LEVEL{"Log level", "#log-level", "Sets system log level", CommandValueType::NUMBER, [](const int value) -> bool{return true;}};
    const CommandValue<const char*>ADD_LOG_INCLUDE{"Add log include", "#add-log-include", "Adds a tag to the log includes", CommandValueType::STRING, [](const char* value) -> bool{return true;}};
    const CommandValue<const char*>REMOVE_LOG_INCLUDE{"Remove log include", "#remove-log-include", "Removes a tag from the log includes", CommandValueType::STRING, [](const char* value) -> bool{return true;}};
    const CommandValue<const char*>ADD_LOG_EXCLUDE{"Add log exclude", "#add-log-exclude", "Adds a tag to the log excludes", CommandValueType::STRING, [](const char* value) -> bool{return true;}};
    const CommandValue<const char*>REMOVE_LOG_EXLUDE{"Remove log exclude", "#remove-log-exclude", "Removes a tag from the log excludes", CommandValueType::STRING, [](const char* value) -> bool{return true;}};
    const CommandValue<const char*>MOTION_PROFILE_NAME{"Motion profile set by name", "#motion-profile-name", "Sets the current running profile by name", CommandValueType::STRING, [](const char* value) -> bool{return true;}};
    const CommandValue<const int>MOTION_PROFILE_SET{"Motion profile set by number", "#motion-profile-set", "Sets the current running profile by number", CommandValueType::NUMBER, [](const int value) -> bool{return true;}};
    const Command EDGE{"Edge", "#edge", "Outputs the edge command to external application", CommandValueType::NONE, []() -> bool{return true;}};
    const Command LEFT{"Left", "#left", "Outputs the left command to external application", CommandValueType::NONE, []() -> bool{return true;}};
    const Command RIGHT{"Right", "#right", "Outputs the right command to external application", CommandValueType::NONE, []() -> bool{return true;}};
    const Command OK{"Ok", "#ok", "Outputs the ok command to external application", CommandValueType::NONE, []() -> bool{return true;}};

    Command commands[14] = {
        HELP,
        SAVE,
        DEFAULT_ALL,
        RESTART,
        CLEAR_LOGS_INCLUDE,
        CLEAR_LOGS_EXCLUDE,
        MOTION_ENABLE,
        MOTION_DISABLE,
        MOTION_TOGGLE,
        MOTION_PROFILE_CYCLE,
        //PAUSE_TOGGLE,
        EDGE,
        LEFT,
        RIGHT,
        OK
    };
    CommandValue<const int> commandNumberValues[2] = {
        LOG_LEVEL,
        MOTION_PROFILE_SET
    };
    CommandValue<const char*> commandCharValues[7] = {
        WIFI_SSID,
        WIFI_PASS,
        ADD_LOG_INCLUDE,
        REMOVE_LOG_INCLUDE,
        ADD_LOG_EXCLUDE,
        REMOVE_LOG_INCLUDE,
        MOTION_PROFILE_NAME
    };
};