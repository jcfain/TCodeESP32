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
#include "DFRobot_DF2301Q.h"
#include <Wire.h>
#include "LogHandler.h"
#include "TagHandler.h"

using VOICE_COMMAND_FUNCTION_PTR_T = void (*)(const char* tcodeCommand);

class VoiceHandler {
    
public:
    bool setup() {
        LogHandler::info(_TAG, "Setup voice");
		if(!SettingsHandler::waitForI2CDevices(DF2301Q_I2C_ADDR)) {
            return false;
        }
        int tries = 0;
        while (!asr.begin() && tries <= 3) {// Returns true no matter what right now.
            LogHandler::error(_TAG, "Could not connect, trying again.");
            if(tries >= 3){
                LogHandler::error(_TAG, "Communication with device failed, please check connection");
                return false;
            }
            tries++;
            delay(1000);
        }
        _isConnected = true;
        LogHandler::info(_TAG, "Begin ok!");
        SettingsFactory* settingsFactory = SettingsFactory::getInstance();
        if(settingsFactory->getVoiceVolume() > 0) {
            setVolume(settingsFactory->getVoiceVolume());
        }
        setMuteMode(settingsFactory->getVoiceMuted());
        setWakeTime(settingsFactory->getVoiceWakeTime());
        /**
             @brief Get wake-up duration
            @return The currently-set wake-up period
        */
        // uint8_t wakeTime = 0;
        // wakeTime = asr.getWakeTime();
        // LogHandler::info(_TAG, "wakeTime: %ld", wakeTime);
        
        return true;
    }
    bool isConnected() {
        return _isConnected;
    }

    /**
     * @brief Set voice volume
     * @param value - Volume value(1~7)
     */
    void setVolume(int value) {
        asr.setVolume(value);
    }
    /**
         @brief Set mute mode
        @param value - Mute mode; set value 1: mute, 0: unmute
    */
    void setMuteMode(bool value) {
        asr.setMuteMode(value);
    }
    /**
         @brief Set wake-up duration
        @param value - Wake-up duration (0-255)
    */
    void setWakeTime(int value) {
        asr.setWakeTime(value);
    }
    /**
         @brief Play the corresponding reply audio according to the ID
        @param CMDID - command word ID
    */
    void playByCMDID(int value) {
        asr.playByCMDID(value);
    }

	static void startLoop(void * parameter) {
		((VoiceHandler*)parameter)->loop();
	}

	void setMessageCallback(VOICE_COMMAND_FUNCTION_PTR_T f) {
		if (f == nullptr) {
			message_callback = 0;
		} else {
			message_callback = f;
		}
	}
    
private:
	const char* _TAG = TagHandler::VoiceHandler;

    //I2C communication
    DFRobot_DF2301Q_I2C asr;
	VOICE_COMMAND_FUNCTION_PTR_T message_callback = 0;
    bool _isRunning = false;
    bool _isConnected = false;

    void loop() { /**
        @brief Get the ID corresponding to the command word 
        @return Return the obtained command word ID, returning 0 means no valid ID is obtained
    */
		_isRunning = true;
		LogHandler::debug(_TAG, "Voice task cpu core: %u", xPortGetCoreID());
        TickType_t pxPreviousWakeTime = millis();
		while(_isRunning) {
            toTCode(asr.getCMDID());
            xTaskDelayUntil(&pxPreviousWakeTime, 1000/portTICK_PERIOD_MS);
        }
    }

    void toTCode(uint8_t voiceCommand) {

        switch (voiceCommand) {
            case 5:
            LogHandler::verbose(_TAG, "Custom Command: %ld", voiceCommand); 
            sendMessage("#motion-enable");
            char command[22];
            sprintf(command, "#motion-profile-set:%ld", SettingsHandler::getMotionDefaultProfileIndex() +1);
            sendMessage(command);
            break;
            case 6:
            LogHandler::verbose(_TAG, "Custom Command: %ld", voiceCommand); 
            sendMessage("#motion-enable");
            sendMessage("#motion-profile-set:1");
            break;
            case 7:
            LogHandler::verbose(_TAG, "Custom Command: %ld", voiceCommand); 
            sendMessage("#motion-enable");
            sendMessage("#motion-profile-set:2");
            break;
            case 8:
            LogHandler::verbose(_TAG, "Custom Command: %ld", voiceCommand); 
            sendMessage("#motion-enable");
            sendMessage("#motion-profile-set:3");
            break;
            case 9:
            LogHandler::verbose(_TAG, "Custom Command: %ld", voiceCommand); 
            sendMessage("#motion-enable");
            sendMessage("#motion-profile-set:4");
            break;
            case 10:
            LogHandler::verbose(_TAG, "Custom Command: %ld", voiceCommand); 
            sendMessage("#motion-enable");
            sendMessage("#motion-profile-set:5");
            break;
            case 11:
            LogHandler::verbose(_TAG, "Custom Command: %ld", voiceCommand); 
            sendMessage("#motion-disable");
            break;
        
            default:if (voiceCommand == 1 || voiceCommand == 2) {
                LogHandler::verbose(_TAG, "Wakup command: %ld", voiceCommand);
            }
             else if (voiceCommand != 0) {
                LogHandler::verbose(_TAG, "Command not used: %ld", voiceCommand);
            } 
        }
    }

    void sendMessage(const char* message) {
        if(message_callback)
            message_callback(message);
    }
};

