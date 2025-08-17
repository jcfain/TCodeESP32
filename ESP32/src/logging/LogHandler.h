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

#include <mutex>
#include <vector>

enum class LogLevel { NONE,
                      ERROR,
                      WARNING,
                      INFO,
                      DEBUG,
                      VERBOSE };
#define LOG_LEVEL_HELP "Sets system log level.\nValid values are: NONE=0, ERROR=1, WARNING=2, INFO=3, DEBUG=4, VERBOSE=5"

using LOG_FUNCTION_PTR_T = void (*)(const char *input, size_t length,
                                    LogLevel level);
class LogHandler {
public:
    static const int internal_buffer_length = 1024;

    static void setLogLevel(LogLevel logLevel) {
        if(logLevel != getInstance().m_currentLogLevel)
        {
			Serial.printf("Log level changed to: %d\n", (uint8_t)logLevel);
            getInstance().m_currentLogLevel = logLevel;
        }
    }

    static void setFilterDuplicates(bool enabled) {
        getInstance().m_filterDuplicates = enabled;
    }

    static LogLevel getLogLevel() { return getInstance().m_currentLogLevel; }

    static bool addInclude(const char *tag) {
        LogHandler &log = getInstance();
        std::vector<const char*>::iterator position = std::find_if(log.m_tags.begin(), log.m_tags.end(), [tag](const char* tagIn) {
            return !strcmp(tag, tagIn);
        });
        if (position == log.m_tags.end()) {
            log.m_tags.push_back(tag);
            Serial.printf("LogHandler: add include: %s\n", tag);
        } else
            return false;
        return true;
    }

    static void setIncludes(std::vector<const char*> tags) {
        clearIncludes();
        LogHandler &log = getInstance();
        for (size_t i = 0; i < tags.size(); i++) {
            log.m_tags.push_back(tags[i]);
        }
    }

    static const std::vector<const char*> getIncludes() {
        return getInstance().m_tags;
    }

    static bool removeInclude(const char *tag) {
        LogHandler &log = getInstance();
        std::vector<const char*>::iterator position = std::find_if(log.m_tags.begin(), log.m_tags.end(), [tag](const char* tagIn) {
            return !strcmp(tag, tagIn);
        });
        if (position != log.m_tags.end()) {
            log.m_tags.erase(position);
            Serial.printf("LogHandler: remove include: %s\n", tag);
        } else
            return false;
        return true;
    }

    static void clearIncludes() { getInstance().m_tags.clear(); }

    static bool addExclude(const char *tag) {
        LogHandler &log = getInstance();
        std::vector<const char*>::iterator position = std::find_if(log.m_filters.begin(), log.m_filters.end(), [tag](const char* tagIn) {
            return !strcmp(tag, tagIn);
        });
        if (position == log.m_filters.end()) {// == myVector.end() means the element was not found
            log.m_filters.push_back(tag);
            Serial.printf("LogHandler: add exclude: %s\n", tag);
            // Serial.println(m_filters.size());
        } else {
            // Serial.println(m_filters.size());
            return false;
        }
        // Serial.println(m_filters.front());
        return true;
    }

    static void setExcludes(std::vector<const char*> tags) {
        clearExcludes();
        for (size_t i = 0; i < tags.size(); i++) {
            getInstance().m_filters.push_back(tags[i]);
        }
    }

    static const std::vector<const char*> getExcludes() {
        return getInstance().m_filters;
    }

    static bool removeExclude(const char *tag) {
        LogHandler &log = getInstance();
        std::vector<const char*>::iterator position = std::find_if(log.m_filters.begin(), log.m_filters.end(), [tag](const char* tagIn) {
            return !strcmp(tag, tagIn);
        });
        if (position != log.m_filters.end()) {// == myVector.end() means the element was not found
            log.m_filters.erase(position);
            Serial.printf("LogHandler: remove exclude: %s\n", tag);
            // Serial.println(m_filters.size());
        } else {
            // Serial.println(m_filters.size());
            return false;
        }
        return true;
    }

    static void clearExcludes() { getInstance().m_filters.clear(); }

    static void info(const char *tag, const char *format, ...) {
        LogHandler &log = getInstance();
        if (log.m_currentLogLevel >= LogLevel::INFO) {
            xSemaphoreTake(log.m_xMutex, portMAX_DELAY);
            if (isLogged(tag)) {
                va_list vArgs;
                va_start(vArgs, format);
                parseMessage(format, "[INFO]", tag, LogLevel::INFO, vArgs);
                va_end(vArgs);
            }
            xSemaphoreGive(log.m_xMutex);
        }
    }

    static void warning(const char *tag, const char *format, ...) {
        LogHandler &log = getInstance();
        if (log.m_currentLogLevel >= LogLevel::WARNING) {
            xSemaphoreTake(log.m_xMutex, portMAX_DELAY);
            if (isLogged(tag)) {
                va_list vArgs;
                va_start(vArgs, format);
                parseMessage(format, "[WARNING]", tag, LogLevel::WARNING, vArgs);
                va_end(vArgs);
            }
            xSemaphoreGive(log.m_xMutex);
        }
    }

    static void error(const char *tag, const char *format, ...) {
        LogHandler &log = getInstance();
        if (log.m_currentLogLevel >= LogLevel::ERROR) {
            xSemaphoreTake(log.m_xMutex, portMAX_DELAY);
            if (isLogged(tag)) {
                va_list vArgs;
                va_start(vArgs, format);
                parseMessage(format, "[ERROR]", tag, LogLevel::ERROR, vArgs);
                va_end(vArgs);
            }
            xSemaphoreGive(log.m_xMutex);
        }
    }

    static void debug(const char *tag, const char *format, ...) {
        LogHandler &log = getInstance();
        if (log.m_currentLogLevel >= LogLevel::DEBUG) {
            xSemaphoreTake(log.m_xMutex, portMAX_DELAY);
            if (isLogged(tag)) {
                va_list vArgs;
                va_start(vArgs, format);
                parseMessage(format, "[DEBUG]", tag, LogLevel::DEBUG, vArgs);
                va_end(vArgs);
            }
            xSemaphoreGive(log.m_xMutex);
        }
    }

    static void verbose(const char *tag, const char *format, ...) {
        LogHandler &log = getInstance();
        if (log.m_currentLogLevel >= LogLevel::VERBOSE) {
            xSemaphoreTake(log.m_xMutex, portMAX_DELAY);
            if (isLogged(tag)) {
                va_list vArgs;
                va_start(vArgs, format);
                parseMessage(format, "[VERBOSE]", tag, LogLevel::VERBOSE, vArgs);
                va_end(vArgs);
            }
            xSemaphoreGive(log.m_xMutex);
        }
    }

    static const char *getLastError() { return getInstance().m_lastError; }

    static void setMessageCallback(LOG_FUNCTION_PTR_T f) {
        getInstance().m_message_callback = f == nullptr ? 0 : f;
    }

private:
    LogHandler() {}

    LogHandler(const LogHandler &) = delete;
    LogHandler &operator=(const LogHandler &) = delete;

    static LogHandler *logger_instance;
    static LogHandler &getInstance() {
        static LogHandler logger_instance;
        return logger_instance;
    }

    LOG_FUNCTION_PTR_T m_message_callback = 0;
    LogLevel m_currentLogLevel = LogLevel::INFO;
    SemaphoreHandle_t m_xMutex = xSemaphoreCreateMutex();
    std::vector<const char*> m_tags;
    std::vector<const char*> m_filters;
    char m_lastVerbose[internal_buffer_length];
    char m_lastDebug[internal_buffer_length];
    char m_lastError[internal_buffer_length];
    bool m_filterDuplicates = false;

    static void parseMessage(const char *valueFormat, const char *level,
                             const char *tag, LogLevel logLevel, va_list vArgs) {
        LogHandler &log = getInstance();
		if (strlen(valueFormat) > internal_buffer_length) {
			Serial.println("Log value too big for buffer");
			return;
		}
		char temp[internal_buffer_length] = {'\0'};
		int len = vsnprintf(temp, internal_buffer_length - 1, valueFormat, vArgs);

		if (len < 0) {
			Serial.println("Error printing vargs");
			return;
		}

		for (size_t i = internal_buffer_length - 1; i >= 0; --i) {
			if ((temp[i] != '\n') && (temp[i] != '\r') && (temp[i] != ' ') &&
				(i < len)) {
				break;
			}
			temp[i] = 0;
		}

		if (log.m_filterDuplicates) {
			switch (logLevel) {
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
		switch (logLevel) {
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

    static bool isTagged(const char *tag) {
        LogHandler &log = getInstance();
        if (log.m_tags.empty())
            return true; // tag all by default
        std::vector<const char*>::iterator position = std::find_if(log.m_tags.begin(), log.m_tags.end(), [tag](const char* tagIn) {
            return !strcmp(tag, tagIn);
        });
        return position != log.m_tags.end();
    }

    static bool isFiltered(const char *tag) {
        LogHandler &log = getInstance();
        if (log.m_filters.empty())
            return false;
        std::vector<const char*>::iterator position = std::find_if(log.m_filters.begin(), log.m_filters.end(), [tag](const char* tagIn) {
            return !strcmp(tag, tagIn);
        });
        return position != log.m_filters.end();
    }

    static bool isLogged(const char *tag) {
        bool tagged = isTagged(tag);
        bool filtered = isFiltered(tag);
        return tagged && !filtered;
    }
};