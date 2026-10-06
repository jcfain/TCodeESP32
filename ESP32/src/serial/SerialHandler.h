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
        // Discard any boot-loader / log garbage sitting in the RX FIFO
        while (Serial.available() > 0)
        {
            Serial.read();
        }
        LogHandler::info(Tags::Main, "Serial command task started");
    }

    void loop() override
    {
        while (Serial.available() > 0)
        {
            const char ch = static_cast<char>(Serial.read());

            // \r, \n, and ; all act as line terminators.
            // \r\n works: \r dispatches the command, \n finds an empty buffer.
            if (ch == '\r' || ch == '\n' || ch == ';')
            {
                if (m_index > 0)
                {
                    dispatchBufferedCommand();
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
            m_buffer[m_index] = '\0';
        }
    }

private:
    void dispatchBufferedCommand()
    {
        m_buffer[m_index] = '\0';

        // Feed motor commands directly to motor handler (independent of WiFi).
        // The TCode library requires a '\n' terminator to trigger command parsing,
        // so append one before dispatching.
        if (isTCodeCommand(m_buffer))
        {
            extern void feedMotorCommand(const char *cmd, size_t len);
            if (m_index < MAX_COMMAND - 1)
            {
                m_buffer[m_index] = '\n';
                m_buffer[m_index + 1] = '\0';
                feedMotorCommand(m_buffer, m_index + 1);
                m_buffer[m_index] = '\0';
            }
            else
            {
                feedMotorCommand(m_buffer, m_index);
            }
        }

        // Also process as system commands.
        // Some system commands (e.g. #device-home, #motion-disable) generate
        // internal TCode (like DSTOP) that must be forwarded to the motor.
        m_commandHandler.process(m_buffer);
        drainSystemTCode();
        m_index = 0;
        m_buffer[0] = '\0';
    }

    void drainSystemTCode()
    {
        extern void feedMotorCommand(const char *cmd, size_t len);
        char buf[MAX_COMMAND];
        while (m_commandHandler.getTCode(buf))
        {
            size_t len = strlen(buf);
            if (len > 0)
            {
                feedMotorCommand(buf, len);
            }
        }
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
};

#endif // SERIAL_HANDLER_H_
