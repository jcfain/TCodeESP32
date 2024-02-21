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
#include "LogHandler.h"
#include "TagHandler.h"
#include "../lib/constants.h"
#include "../lib/struct/buttonSet.h"


class ButtonHandler {


public:
    ButtonHandler() {
        buttonIndexMap[0] = 900;
        buttonIndexMap[1] = 1200;
        buttonIndexMap[2] = 2000;
        buttonIndexMap[3] = 4096;
    }

    void init(uint16_t analogDebounce, char bootButtonCommand[MAX_COMMAND], ButtonSet buttonSets[MAX_BUTTON_SETS]) {
        if(m_initialized) {
            return;
        }
        buttonAnalogDebounce = analogDebounce;
        m_buttonQueue = xQueueCreate(MAX_BUTTON_SETS * MAX_BUTTONS, sizeof(struct ButtonModel*));
        if(m_buttonQueue == NULL) {
            LogHandler::error(_TAG, "Error creating the debug queue");
        }
        initBootbutton(bootButtonCommand);
        initAnalogButtons(buttonSets);
        m_initialized = true;
    }

    void initAnalogButtons(ButtonSet buttonSets[MAX_BUTTON_SETS]) {
        for(int i = 0; i < MAX_BUTTON_SETS; i++) {
            m_buttonSets[i] = ButtonSet(buttonSets[i]);
            auto buttonSet = m_buttonSets[i];
            auto buttonPin = buttonSet.pin;
            if(buttonPin > -1) {
                auto pin = digitalPinToInterrupt(buttonPin);
                if(pin == -1) {
                    LogHandler::error(_TAG, "Invalid interupt button pin: %ld", buttonPin);
                    continue;
                };    
                xTaskCreate(&analog_button_task, "buttonTask", 3096, this, 5, &buttonTask);
                return;
                // LogHandler::debug(_TAG, "Checking button set: %s, pin: %ld", buttonSet.name, buttonPin);
                // if(buttonSet.pullMode == gpio_pull_mode_t::GPIO_PULLDOWN_ONLY) {
                //     LogHandler::info(_TAG, "Setting up pull down button set %u on pin: %ld", i +1, buttonPin);
                //     pinMode(buttonPin, INPUT_PULLDOWN);
                //     attachInterrupt(buttonPin, buttonInterrupt, RISING);
                // } else {
                //     LogHandler::info(_TAG, "Setting up pull up button set %u on pin: %ld", i +1, buttonPin);
                //     pinMode(buttonPin, INPUT_PULLUP);
                //     attachInterrupt(buttonPin, buttonInterrupt, FALLING);
                // }
            }
        }
    }

    void initBootbutton(char command[MAX_COMMAND]) {
        pinMode(0, INPUT_PULLDOWN);
        attachInterrupt(digitalPinToInterrupt(0), bootButtonInterrupt, RISING);
        updateBootButtonCommand(command);
    }

    void updateBootButtonCommand(char bootButtonCommand[MAX_COMMAND]) {
        xSemaphoreTake(xMutex, portMAX_DELAY);
        if(strlen(bootButtonCommand) > 0) {
            strcpy(m_bootButtonCommand, bootButtonCommand);
        } else {
            m_bootButtonCommand[0] = {0};
        }
        xSemaphoreGive(xMutex);
    }

    void updateAnalogButtonCommands(ButtonSet buttonSets[MAX_BUTTON_SETS]) {
        xSemaphoreTake(xMutex, portMAX_DELAY);
        for(int i = 0; i < MAX_BUTTON_SETS; i++) {
            m_buttonSets[i] = ButtonSet(buttonSets[i]);
        }
        xSemaphoreGive(xMutex);
    }

    void updateAnalogDebounce(uint16_t debounce) {
        buttonAnalogDebounce = debounce;
    }

    void read(ButtonModel* &buf) {
        // if(m_enterInterupt) {
        //     LogHandler::info(_TAG, "Enter interupt");
        //     readButtons();
        //     m_lastDebounce = millis();
        //     m_enterInterupt = false;
        // }
        //String command;
        void* recieve;
        if(xQueueReceive(m_buttonQueue, &(recieve), 0)) {
            buf = (ButtonModel*)recieve;
            LogHandler::debug(_TAG, "Recieve command: %s:%s", buf->name, buf->command);
        } else {
            buf = 0;
        }
    }

    static void bootButtonInterrupt() {
        xSemaphoreTakeFromISR(xMutex, NULL);
        xQueueSendFromISR(m_buttonQueue, m_bootButtonCommand, NULL);
        xSemaphoreGiveFromISR(xMutex, NULL);
    }

    // static void buttonInterrupt() {
    //     if(!m_enterInterupt && millis() - m_lastDebounce > buttonAnalogDebounce) {
    //         m_enterInterupt = true;    
    //     }
    // }

private: 
    static const char* _TAG;
    bool m_initialized = false;


    static long m_lastDebounce;
    static volatile bool m_enterInterupt;
	static SemaphoreHandle_t xMutex;
    static char m_bootButtonCommand[MAX_COMMAND];
    static uint16_t buttonIndexMap[MAX_BUTTONS];
    static ButtonSet m_buttonSets[MAX_BUTTON_SETS];
    static QueueHandle_t m_buttonQueue;
    static uint8_t buttonAnalogTolorance;
    static uint16_t buttonAnalogDebounce;

    TaskHandle_t buttonTask = NULL;
    static void analog_button_task(void* arg) {
        for(;;) {
            if(millis() - m_lastDebounce > buttonAnalogDebounce) {
                m_lastDebounce = millis();
                readButtons();
            }
            vTaskDelay(10/portTICK_PERIOD_MS);
        }
    }
    static bool readButtons() {
        uint8_t index = 0;
        for(int i = 0; i < MAX_BUTTON_SETS; i++) {
            if(m_buttonSets[i].pin > -1) {
                for(int j = 0; j < MAX_BUTTONS; j++) {
                    auto value = analogRead(m_buttonSets[i].pin);
                    auto index = m_buttonSets[i].buttons[j].index;
                    LogHandler::verbose(_TAG, "readButtons value: %ld, index: %ld, index value: %ld", value, index, buttonIndexMap[index]);
                    bool isPressedValue = value >= buttonIndexMap[index] - buttonAnalogTolorance && value <= buttonIndexMap[index] + buttonAnalogTolorance;
                    if(!m_buttonSets[i].buttons[j].isPressed() && isPressedValue) {
                        LogHandler::debug(_TAG, "Button '%s' pressed: %u, set index: %u button index: %u", m_buttonSets[i].buttons[j].name, value, i, j);
                        // xSemaphoreTake(xMutex, portMAX_DELAY);
                        m_buttonSets[i].buttons[j].press();
                        struct ButtonModel *pxMessage = &(m_buttonSets[i].buttons[j]);// Why did I have to do all this!?
                        xQueueSend(m_buttonQueue, ( void * ) &pxMessage, portMAX_DELAY);
                        // xSemaphoreGive(xMutex);
                        return true;
                    } else if(m_buttonSets[i].buttons[j].isPressed() && !isPressedValue) {
                        LogHandler::debug(_TAG, "Button '%s' released", m_buttonSets[i].buttons[j].name);
                        m_buttonSets[i].buttons[j].release();
                        struct ButtonModel *pxMessage = &(m_buttonSets[i].buttons[j]);// Why did I have to do all this!?
                        xQueueSend(m_buttonQueue, ( void * ) &pxMessage, portMAX_DELAY);
                    }
                }
            }
        }
        return false;
    }
};
const char*  ButtonHandler::_TAG = TagHandler::ButtonHandler;
uint8_t ButtonHandler::buttonAnalogTolorance = 150;
uint16_t ButtonHandler::buttonAnalogDebounce = 150;
QueueHandle_t ButtonHandler::m_buttonQueue;
ButtonSet ButtonHandler::m_buttonSets[MAX_BUTTON_SETS];
char ButtonHandler::m_bootButtonCommand[MAX_COMMAND];
uint16_t ButtonHandler::buttonIndexMap[MAX_BUTTONS];
SemaphoreHandle_t ButtonHandler::xMutex = xSemaphoreCreateMutex();
volatile bool ButtonHandler::m_enterInterupt = false;
long ButtonHandler::m_lastDebounce = millis();