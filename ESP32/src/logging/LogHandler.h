/* MIT License

Copyright (c) 2026 Jason C. Fain

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
#include <mutex>
#include <string>
#include "logging/TagHandler.h"
#include "enum.h"

#define LOG_LEVEL_HELP "Sets system log level.\nValid values are: NONE=0, ERROR=1, WARNING=2, INFO=3, DEBUG=4, VERBOSE=5"

using LOG_FUNCTION_PTR_T = void (*)(const char *input, size_t length,
    LogLevel level);

class LogHandler
{
public:
    static const int internal_buffer_length = 1024;

    static void setLogLevel(LogLevel logLevel)
    {
        if (logLevel != getInstance().m_currentLogLevel)
        {
            Serial.printf("Log level changed to: %d\n", (uint8_t)logLevel);
            getInstance().m_currentLogLevel = logLevel;
        }
    }

    static void setFilterDuplicates(bool enabled)
    {
        getInstance().m_filterDuplicates = enabled;
    }

    static LogLevel getLogLevel() { return getInstance().m_currentLogLevel; }

    // --- Bitmask-based include/exclude API ---
    static void addInclude(Tags::tag_t tag)
    {
        Tags::set_tags(getInstance().m_includedTags, tag);
        Serial.printf("LogHandler: add include: %s\n", Tags::TAG_STRINGS[tag]);
    }

    static void addIncludes(uint32_t tag_masks)
    {
        getInstance().m_includedTags |= tag_masks;
    }

    static void removeInclude(Tags::tag_t tag)
    {
        Tags::unset_tags(getInstance().m_includedTags, tag);
        Serial.printf("LogHandler: remove include: %s\n", Tags::TAG_STRINGS[tag]);
    }

    static void removeIncludes(uint32_t tag_masks)
    {
        getInstance().m_includedTags &= ~tag_masks;
    }

    static void setIncludes(uint32_t tag_masks)
    {
        getInstance().m_includedTags = tag_masks;
    }

    static uint32_t getIncludes()
    {
        return getInstance().m_includedTags;
    }

    static void clearIncludes() { getInstance().m_includedTags = 0; }

    static void addExclude(Tags::tag_t tag)
    {
        Tags::set_tags(getInstance().m_excludedTags, tag);
        Serial.printf("LogHandler: add exclude: %s\n", Tags::TAG_STRINGS[tag]);
    }

    static void addExcludes(uint32_t tag_masks)
    {
        getInstance().m_excludedTags |= tag_masks;
    }

    static void removeExclude(Tags::tag_t tag)
    {
        Tags::unset_tags(getInstance().m_excludedTags, tag);
        Serial.printf("LogHandler: remove exclude: %s\n", Tags::TAG_STRINGS[tag]);
    }

    static void removeExcludes(uint32_t tag_masks)
    {
        getInstance().m_excludedTags &= ~tag_masks;
    }

    static void setExcludes(uint32_t tag_masks)
    {
        getInstance().m_excludedTags = tag_masks;
    }

    static uint32_t getExcludes()
    {
        return getInstance().m_excludedTags;
    }

    static void clearExcludes() { getInstance().m_excludedTags = 0; }

    // String conversions for serialization
    static std::string getIncludesAsString()
    {
        return Tags::as_str(getInstance().m_includedTags);
    }

    static std::string getExcludesAsString()
    {
        return Tags::as_str(getInstance().m_excludedTags);
    }

    // --- Logging methods (tag_t only) ---
    static void info(Tags::tag_t tag, const char *format, ...)
    {
        va_list vArgs;
        va_start(vArgs, format);
        vlog(LogLevel::INFO, "[INFO]", tag, format, vArgs);
        va_end(vArgs);
    }

    static void warning(Tags::tag_t tag, const char *format, ...)
    {
        va_list vArgs;
        va_start(vArgs, format);
        vlog(LogLevel::WARNING, "[WARNING]", tag, format, vArgs);
        va_end(vArgs);
    }

    static void error(Tags::tag_t tag, const char *format, ...)
    {
        va_list vArgs;
        va_start(vArgs, format);
        vlog(LogLevel::ERROR, "[ERROR]", tag, format, vArgs);
        va_end(vArgs);
    }

    static void debug(Tags::tag_t tag, const char *format, ...)
    {
        va_list vArgs;
        va_start(vArgs, format);
        vlog(LogLevel::DEBUG, "[DEBUG]", tag, format, vArgs);
        va_end(vArgs);
    }

    static void verbose(Tags::tag_t tag, const char *format, ...)
    {
        va_list vArgs;
        va_start(vArgs, format);
        vlog(LogLevel::VERBOSE, "[VERBOSE]", tag, format, vArgs);
        va_end(vArgs);
    }

    static const char *getLastError() { return getInstance().m_lastError; }

    static void setMessageCallback(LOG_FUNCTION_PTR_T f)
    {
        getInstance().m_message_callback = f == nullptr ? 0 : f;
    }

private:
    LogHandler() {}

    LogHandler(const LogHandler &) = delete;
    LogHandler &operator=(const LogHandler &) = delete;

    static LogHandler *logger_instance;
    static LogHandler &getInstance()
    {
        static LogHandler logger_instance;
        return logger_instance;
    }

    LOG_FUNCTION_PTR_T m_message_callback = 0;
    LogLevel m_currentLogLevel = LogLevel::INFO;
    SemaphoreHandle_t m_xMutex = xSemaphoreCreateMutex();
    uint32_t m_includedTags = 0; // 0 means include all
    uint32_t m_excludedTags = 0;
    char m_lastVerbose[internal_buffer_length];
    char m_lastDebug[internal_buffer_length];
    char m_lastError[internal_buffer_length];
    bool m_filterDuplicates = false;

    static void parseMessage(const char *valueFormat, const char *level,
                             const char *tag, LogLevel logLevel, va_list vArgs)
    {
        LogHandler &log = getInstance();
        if (strlen(valueFormat) > internal_buffer_length)
        {
            Serial.println("Log value too big for buffer");
            return;
        }
        char temp[internal_buffer_length] = {'\0'};
        int len = vsnprintf(temp, internal_buffer_length - 1, valueFormat, vArgs);

        if (len < 0)
        {
            Serial.println("Error printing vargs");
            return;
        }

        for (size_t i = internal_buffer_length - 1; i >= 0; --i)
        {
            if ((temp[i] != '\n') && (temp[i] != '\r') && (temp[i] != ' ') &&
                (i < len))
            {
                break;
            }
            temp[i] = 0;
        }

        if (log.m_filterDuplicates)
        {
            switch (logLevel)
            {
            case LogLevel::NONE:
            case LogLevel::INFO:
            case LogLevel::WARNING:
                break;
            case LogLevel::ERROR:
                if (strcmp(log.m_lastError, temp) == 0)
                    return;
                break;
            case LogLevel::VERBOSE:
                if (strcmp(log.m_lastVerbose, temp) == 0)
                    return;
                break;
            case LogLevel::DEBUG:
                if (strcmp(log.m_lastDebug, temp) == 0)
                    return;
                break;
            }
        }

        Serial.printf("%s %s: %s\n", level, tag, temp);
        switch (logLevel)
        {
        case LogLevel::NONE:
        case LogLevel::INFO:
        case LogLevel::WARNING:
            break;
        case LogLevel::ERROR:
            strncpy(log.m_lastError, temp, internal_buffer_length);
            break;
        case LogLevel::VERBOSE:
            strncpy(log.m_lastVerbose, temp, internal_buffer_length);
            break;
        case LogLevel::DEBUG:
            strncpy(log.m_lastDebug, temp, internal_buffer_length);
            break;
        }

        if (log.m_message_callback)
            log.m_message_callback(temp, len, logLevel);
    }

    static bool isLogged(Tags::tag_t tag)
    {
        LogHandler &log = getInstance();
        // If no includes set, include all; otherwise check if tag is included
        bool included = (log.m_includedTags == 0) || Tags::is_set(log.m_includedTags, tag);
        bool excluded = Tags::is_set(log.m_excludedTags, tag);
        return included && !excluded;
    }

    static void vlog(LogLevel level, const char *levelStr, Tags::tag_t tag, const char *format, va_list vArgs)
    {
        LogHandler &log = getInstance();
        if (log.m_currentLogLevel >= level)
        {
            xSemaphoreTake(log.m_xMutex, portMAX_DELAY);
            if (isLogged(tag))
            {
                const char *tagStr = (tag < Tags::LAST) ? Tags::TAG_STRINGS[tag] : "unknown";
                parseMessage(format, levelStr, tagStr, level, vArgs);
            }
            xSemaphoreGive(log.m_xMutex);
        }
    }
};
