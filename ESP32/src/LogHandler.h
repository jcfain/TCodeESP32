#pragma once

#include "Arduino.h"

// #define LOG_LOCAL_LEVEL ESP_LOG_ERROR
// #include "esp_log.h"

#include <mutex>


enum class LogLevel {
    ERROR,
    WARNING,
    INFO,
    DEBUG,
    VERBOSE
};

using LOG_FUNCTION_PTR_T = void (*)(const char* input, LogLevel level);
class LogHandler {
public:
    // Need to port to espidf framework instead of arduino to be able to change this at runtime.
    static void setLogLevel(LogLevel logLevel, const String tag = "*") {
        _currentLogLevel = logLevel;
        // // Chant change this at runtime...
        // switch(logLevel) {
        //     case LogLevel::WARNING:
        //         esp_log_level_set(tag.c_str(), ESP_LOG_WARN);
        //     break;
        //     case LogLevel::INFO:
        //         esp_log_level_set(tag.c_str(), ESP_LOG_INFO);
        //     break;
        //     case LogLevel::DEBUG:
        //         esp_log_level_set(tag.c_str(), ESP_LOG_DEBUG);
        //     break;
        //     case LogLevel::VERBOSE:
        //         esp_log_level_set(tag.c_str(), ESP_LOG_VERBOSE);
        //     break;
        //     default:
        //         esp_log_level_set(tag.c_str(), ESP_LOG_ERROR);
        // }
    }
    static LogLevel getLogLevel() {
        return _currentLogLevel;
    }

    static void verbose(const char *tag, const char *format, ...) {
        if(_currentLogLevel == LogLevel::VERBOSE) {
            if (serial_mtx.try_lock()) {
                std::lock_guard<std::mutex> lck(serial_mtx, std::adopt_lock);
                va_list vArgs;
                va_start(vArgs, format);
                parseMessage(format, "VERBOSE", LogLevel::VERBOSE, vArgs);
                va_end(vArgs);
            }
        }
    }

    static void debug(const char *tag, const char *format, ...) {
        if(_currentLogLevel >= LogLevel::DEBUG) {
            if (serial_mtx.try_lock()) {
                std::lock_guard<std::mutex> lck(serial_mtx, std::adopt_lock);
                va_list vArgs;
                va_start(vArgs, format);
                parseMessage(format, "DEBUG", LogLevel::DEBUG, vArgs);
                va_end(vArgs);
            }
        }
    }

    static void info(const char *tag, const char *format, ...) {
        if(_currentLogLevel >= LogLevel::INFO) {
            if (serial_mtx.try_lock()) {
                std::lock_guard<std::mutex> lck(serial_mtx, std::adopt_lock);
                va_list vArgs;
                va_start(vArgs, format);
                parseMessage(format, "INFO", LogLevel::INFO, vArgs);
                va_end(vArgs);
            }
        }
    }

    static void warning(const char *tag, const char *format, ...) {
        if(_currentLogLevel >= LogLevel::WARNING) {
            if (serial_mtx.try_lock()) {
                std::lock_guard<std::mutex> lck(serial_mtx, std::adopt_lock);
                va_list vArgs;
                va_start(vArgs, format);
                parseMessage(format, "WARNING", LogLevel::WARNING, vArgs);
                va_end(vArgs);
            }
        }
    }

    static void error(const char *tag, const char *format, ...) {
        if(_currentLogLevel >= LogLevel::ERROR) {
            if (serial_mtx.try_lock()) {
                std::lock_guard<std::mutex> lck(serial_mtx, std::adopt_lock);
                va_list vArgs;
                va_start(vArgs, format);
                parseMessage(format, "ERROR", LogLevel::ERROR, vArgs);
                va_end(vArgs);
            }
        }
    }

	static void setMessageCallback(LOG_FUNCTION_PTR_T f)
	{
		message_callback = f == nullptr ? 0 : f;
	}
private: 
    static LOG_FUNCTION_PTR_T message_callback;
    static LogLevel _currentLogLevel;
    static std::mutex serial_mtx;

    static void parseMessage(const char* format, const char* level, LogLevel logLevel, va_list vArgs) {
        char temp[1024];
        int len = vsnprintf(temp, sizeof(temp) - 1, format, vArgs);
        temp[sizeof(temp) - 1] = 0;
        int i;

        for (i = len - 1; i >= 0; --i) {
            if (temp[i] != '\n' && temp[i] != '\r' && temp[i] != ' ') {
                break;
            }
            temp[i] = 0;
        }
        if(i > 0) {
            Serial.printf("%s: %s%s", level, temp, "\n");
            if(message_callback)
                message_callback(temp, logLevel);
        }
    }
};
std::mutex  LogHandler::serial_mtx;
LogLevel LogHandler::_currentLogLevel = LogLevel::INFO;
LOG_FUNCTION_PTR_T LogHandler::message_callback = 0;