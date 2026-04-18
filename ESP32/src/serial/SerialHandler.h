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

    // Accept streamed TCode that is not newline-terminated by flushing after a
    // short idle gap between bytes.
    static constexpr uint32_t SERIAL_TCODE_IDLE_FLUSH_MS = 4;

    static void init()
    {
        LogHandler::info(Tags::Main, "Serial command handler initialized");
    }

    void setup() override
    {
        m_index = 0;
        m_buffer[0] = '\0';
        m_lastByteMs = 0;
        LogHandler::info(Tags::Main, "Serial command task started");
    }

    void loop() override
    {
        // Some TCode senders stream bytes without '\n'. If input pauses briefly,
        // flush the buffered command so it still reaches the motor parser.
        if (m_index > 0 && isTCodeCommand(m_buffer) && (millis() - m_lastByteMs) >= SERIAL_TCODE_IDLE_FLUSH_MS)
        {
            dispatchBufferedCommand();
        }

        while (Serial.available() > 0)
        {
            const char ch = static_cast<char>(Serial.read());
            m_lastByteMs = millis();

            if (ch == '\r')
            {
                continue;
            }

            if (ch == '\n' || ch == ';')
            {
                if (m_index > 0)
                {
                    dispatchBufferedCommand();
                }
                continue;
            }

            // If a new TCode axis token starts while a prior command is buffered,
            // treat that as a boundary for stream-based senders.
            if (m_index > 0 && isTCodeStartChar(ch) && isTCodeCommand(m_buffer))
            {
                dispatchBufferedCommand();
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
    void dispatchBufferedCommand()
    {
        m_buffer[m_index] = '\0';

        // Feed motor commands directly to motor handler (independent of WiFi)
        if (isTCodeCommand(m_buffer))
        {
            extern void feedMotorCommand(const char *cmd, size_t len);
            feedMotorCommand(m_buffer, m_index);
        }

        // Also process as system commands
        m_commandHandler.process(m_buffer);
        m_index = 0;
        m_buffer[0] = '\0';
    }

    bool isTCodeStartChar(char ch)
    {
        return ch == 'L' || ch == 'R' || ch == 'V' || ch == 'A' ||
               ch == 'l' || ch == 'r' || ch == 'v' || ch == 'a';
    }

    // Check if command is T-Code (e.g., L0123, R0456, etc.)
    bool isTCodeCommand(const char *cmd)
    {
        if (!cmd || !cmd[0])
            return false;
        // TCode commands start with L, R, V, A followed by digit
        char first = cmd[0];
        return (first == 'L' || first == 'R' || first == 'V' || first == 'A' ||
                first == 'l' || first == 'r' || first == 'v' || first == 'a') &&
               (cmd[1] >= '0' && cmd[1] <= '9');
    }

private:
    SystemCommandHandler m_commandHandler;
    char m_buffer[MAX_COMMAND] = {0};
    size_t m_index = 0;
    uint32_t m_lastByteMs = 0;
};

#endif // SERIAL_HANDLER_H_
