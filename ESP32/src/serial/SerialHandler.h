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

#ifndef SERIAL_HANDLER_H_
#define SERIAL_HANDLER_H_

#include <Arduino.h>

#include "logging/LogHandler.h"
#include "logging/TagHandler.h"
#include "messages/SystemCommandHandler.h"
#include "tasks/TaskHandler.h"

class SerialHandler : public TaskHandler::Task
{
public:
    SerialHandler() : Task(TaskHandler::Rates::FAST) {}

    static void init()
    {
        LogHandler::info(Tags::Main, "Serial command handler initialized");
    }

    void setup() override
    {
        m_index = 0;
        m_buffer[0] = '\0';
        LogHandler::info(Tags::Main, "Serial command task started");
    }

    void loop() override
    {
        while (Serial.available() > 0)
        {
            const char ch = static_cast<char>(Serial.read());

            if (ch == '\r')
            {
                continue;
            }

            if (ch == '\n')
            {
                if (m_index > 0)
                {
                    m_buffer[m_index] = '\0';
                    m_commandHandler.process(m_buffer);
                    m_index = 0;
                    m_buffer[0] = '\0';
                }
                continue;
            }

            if (m_index >= (MAX_COMMAND - 1))
            {
                LogHandler::warning(Tags::Main, "Serial input exceeded max command length; dropping line");
                m_index = 0;
                m_buffer[0] = '\0';
                continue;
            }

            m_buffer[m_index++] = ch;
        }
    }

private:
    SystemCommandHandler m_commandHandler;
    char m_buffer[MAX_COMMAND] = { 0 };
    size_t m_index = 0;
};

#endif // SERIAL_HANDLER_H_
