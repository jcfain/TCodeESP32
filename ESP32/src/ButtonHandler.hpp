
#pragma once

#include <Arduino.h>
#include "LogHandler.h"
#include "TagHandler.h"

#define BUTTON_ANALOG_TOL 20
#define MAX_BUTTON_INDEX 4
//enum class
struct ButtonModel {
    uint8_t index;
    int8_t pin = -1;
    gpio_pull_mode_t pullMode = gpio_pull_mode_t::GPIO_PULLDOWN_ONLY;
    char command[MAX_COMMAND];
};


class ButtonHandler {


public:
ButtonHandler() {
    buttonIndexMap[0] = 245;
    buttonIndexMap[1] = 331;
    buttonIndexMap[2] = 501;
    buttonIndexMap[3] = 1013;
}

void init(char bootButtonCommand[MAX_COMMAND], ButtonModel buttons[MAX_BUTTONS]) {
    if(m_initialized) {
        return;
    }
    m_buttonQueue = xQueueCreate(MAX_BUTTONS, sizeof(char[MAX_COMMAND]));
    if(m_buttonQueue == NULL) {
        LogHandler::error(_TAG, "Error creating the debug queue");
    }
    initBootbutton(bootButtonCommand);
    initAnalogButtons(buttons);
    m_initialized = true;
}

void initAnalogButtons(ButtonModel buttons[MAX_BUTTONS]) {
    for(int i = 0; i < MAX_BUTTONS; i++) {
        if(buttons[i].pin > -1 && buttons[i].index < MAX_BUTTON_INDEX) {
            ButtonModel button = buttons[i];
            m_buttons[i] = ButtonModel(button);
            if(button.pullMode == gpio_pull_mode_t::GPIO_PULLDOWN_ONLY) {
                pinMode(button.pin, INPUT_PULLDOWN);
                attachInterrupt(digitalPinToInterrupt(button.pin), buttonInterrupt, RISING);
            } else {
                pinMode(button.pin, INPUT_PULLUP);
                attachInterrupt(digitalPinToInterrupt(button.pin), buttonInterrupt, FALLING);
            }
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

void read(char buf[MAX_COMMAND]) {
    //String command;
    if(xQueueReceive(m_buttonQueue, buf, 0)) {
        LogHandler::debug(_TAG, "Recieve command: %s", buf);
    } else {
        buf[0] = {0};
    }
}

static void bootButtonInterrupt() {
	xSemaphoreTakeFromISR(xMutex, NULL);
    xQueueSendFromISR(m_buttonQueue, m_bootButtonCommand, NULL);
    xSemaphoreGiveFromISR(xMutex, NULL);
}

static void buttonInterrupt() {
    uint8_t index = 0;
    for(int i = 0; i < MAX_BUTTONS; i++) {
        if(m_buttons[i].pin > -1 && m_buttons[i].index < MAX_BUTTON_INDEX) {
            int value = analogRead(m_buttons[i].pin);
            LogHandler::debug(_TAG, "Recieve button analog value: %ld", value);
            if(value > buttonIndexMap[m_buttons[i].index] && value < buttonIndexMap[m_buttons[i].index] + BUTTON_ANALOG_TOL)
                xQueueSendFromISR(m_buttonQueue, m_buttons[i].command, NULL);
        }
    }
}

private: 
    static const char* _TAG;
    bool m_initialized = false;

	static SemaphoreHandle_t xMutex;
    static char m_bootButtonCommand[MAX_COMMAND];
    static uint16_t buttonIndexMap[MAX_BUTTON_INDEX];
    static ButtonModel m_buttons[MAX_BUTTONS];
    static QueueHandle_t m_buttonQueue;
};
const char*  ButtonHandler::_TAG = TagHandler::ButtonHandler;
QueueHandle_t ButtonHandler::m_buttonQueue;
ButtonModel ButtonHandler::m_buttons[MAX_BUTTONS];
char ButtonHandler::m_bootButtonCommand[MAX_COMMAND];
uint16_t ButtonHandler::buttonIndexMap[MAX_BUTTON_INDEX];
SemaphoreHandle_t ButtonHandler::xMutex = xSemaphoreCreateMutex();