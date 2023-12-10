#pragma once

#include "Arduino.h"

// #define LOG_LOCAL_LEVEL ESP_LOG_ERROR
// #include "esp_log.h"

#include <mutex>
#include <vector>
#include "utils.h"


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
    static void setLogLevel(LogLevel logLevel) {
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

    /// @brief Filters concurrent duplicates on the verbose log output
    /// @param enabled 
    static void setFilterDuplicates(bool enabled) {
        m_filterDuplicates = enabled;
    }
    
    static LogLevel getLogLevel() {
        return _currentLogLevel;
    }

    static void setIncludes(std::vector<String> tags) {
        clearIncludes();
		for(unsigned int i = 0; i < tags.size(); i++) {
           //addTag(tags[i]);
            m_tags.push_back(tags[i]);
		}
    }
    static std::vector<String> getIncludes() {
        return m_tags;
    }

    static void clearIncludes() {
        m_tags.clear();
    }

    static bool addInclude(const char* tag) {
        std::vector<String>::iterator position = std::find(m_tags.begin(), m_tags.end(), tag);
        if (position == m_tags.end()) // == myVector.end() means the element was not found
            m_tags.push_back(tag);
        else
            return false;
        return true;
    }

    static bool removeInclude(const char* tag) {
        std::vector<String>::iterator position = std::find(m_tags.begin(), m_tags.end(), tag);
        if (position != m_tags.end()) // == myVector.end() means the element was not found
            m_tags.erase(position);
        else
            return false;
        return true;
    }

    static void setExcludes(std::vector<String> tags) {
        clearExcludes();
		for(unsigned int i = 0; i < tags.size(); i++) {
           //addTag(tags[i]);
            m_filters.push_back(tags[i]);
		}
    }

    static std::vector<String> getExcludes() {
        return m_filters;
    }

    static void clearExcludes() {
        m_filters.clear();
    }

    static bool addExclude(const char* tag) {
        // Serial.print("Adding filter: ");
        // Serial.println(tag);
        // Serial.println(m_filters.size());
        std::vector<String>::iterator position = std::find(m_filters.begin(), m_filters.end(), tag);
        if (position == m_filters.end()) {// == myVector.end() means the element was not found
            m_filters.push_back(tag);
            // Serial.println(m_filters.size());
        } else {
            // Serial.println(m_filters.size());
            return false;
        }
        // Serial.println(m_filters.front());
        return true;
    }

    static bool removeExclude(const char* tag) {
        // Serial.print("Removing filter: ");
        // Serial.println(tag);
        // Serial.println(m_filters.size());
        std::vector<String>::iterator position = std::find(m_filters.begin(), m_filters.end(), tag);
        if (position != m_filters.end()) {// == myVector.end() means the element was not found
            m_filters.erase(position);
            // Serial.println(m_filters.size());
        } else {
            // Serial.println(m_filters.size());
            return false;
        }
        return true;
    }

    static void verbose(const char *tag, const char *format, ...) {
        if(_currentLogLevel == LogLevel::VERBOSE) {
            //Serial.println("verbose tryLock");
		    xSemaphoreTake(xMutex, portMAX_DELAY);
            if(isLogged(tag)) {
                if(m_filterDuplicates && strcmp(m_lastVerbose, format) == 0) {
                    xSemaphoreGive(xMutex);
                    return;
                }
                strcpy(m_lastVerbose, format);
                va_list vArgs;
                va_start(vArgs, format);
                parseMessage(format, "VERBOSE", tag, LogLevel::VERBOSE, vArgs);
                va_end(vArgs);
            }
            xSemaphoreGive(xMutex);
            //Serial.println("verbose exit");
        }
    }

    static void debug(const char *tag, const char *format, ...) {
        if(_currentLogLevel >= LogLevel::DEBUG) {
            //Serial.println("debug tryLock");
		    xSemaphoreTake(xMutex, portMAX_DELAY);
            if(isLogged(tag)) {
                if(m_filterDuplicates && strcmp(m_lastDebug, format) == 0) {
                    xSemaphoreGive(xMutex);
                    return;
                }
                strcpy(m_lastDebug, format);
                va_list vArgs;
                va_start(vArgs, format);
                parseMessage(format, "DEBUG", tag, LogLevel::DEBUG, vArgs);
                va_end(vArgs);
            }
            xSemaphoreGive(xMutex);
            //Serial.println("debug exit");
        }
    }

    static void info(const char *tag, const char *format, ...) {
        if(_currentLogLevel >= LogLevel::INFO) {
            //Serial.println("info tryLock");
		    xSemaphoreTake(xMutex, portMAX_DELAY);
            if(isLogged(tag)) {
                va_list vArgs;
                va_start(vArgs, format);
                parseMessage(format, "INFO", tag, LogLevel::INFO, vArgs);
                va_end(vArgs);
            }
            xSemaphoreGive(xMutex);
            //Serial.println("info exit");
        }
    }

    static void warning(const char *tag, const char *format, ...) {
        if(_currentLogLevel >= LogLevel::WARNING) {
            //Serial.println("warning tryLock");
		    xSemaphoreTake(xMutex, portMAX_DELAY);
            if(isLogged(tag)) {
                va_list vArgs;
                va_start(vArgs, format);
                parseMessage(format, "WARNING", tag, LogLevel::WARNING, vArgs);
                va_end(vArgs);
            }
            xSemaphoreGive(xMutex);
            //Serial.println("warning exit");
        }
    }

    static void error(const char *tag, const char *format, ...) {
        if(_currentLogLevel >= LogLevel::ERROR) {
            //Serial.println("error tryLock");
		    xSemaphoreTake(xMutex, portMAX_DELAY);
            if(isLogged(tag)) {
                va_list vArgs;
                va_start(vArgs, format);
                parseMessage(format, "ERROR", tag, LogLevel::ERROR, vArgs);
                va_end(vArgs);
            }
            xSemaphoreGive(xMutex);
        //Serial.println("error exit");
        }
    }

	static void setMessageCallback(LOG_FUNCTION_PTR_T f)
	{
		message_callback = f == nullptr ? 0 : f;
	}
private: 
    static LOG_FUNCTION_PTR_T message_callback;
    static LogLevel _currentLogLevel;
	static SemaphoreHandle_t xMutex;
    static std::vector<String> m_tags;
    static std::vector<String> m_filters;
    static char m_lastVerbose[1024];
    static char m_lastDebug[1024];
    static bool m_filterDuplicates;

    static void parseMessage(const char* valueFormat, const char* level, const char* tag, LogLevel logLevel, va_list vArgs) {
        try {
            if(strlen(valueFormat) > 1024) {
                Serial.println("Log value too big for buffer");
                return;
            }
            char temp[1024];
            int len = vsnprintf(temp, sizeof(temp) - 1, valueFormat, vArgs);
            temp[sizeof(temp) - 1] = 0;
            int i;

            for (i = len - 1; i >= 0; --i) {
                if (temp[i] != '\n' && temp[i] != '\r' && temp[i] != ' ') {
                    break;
                }
                temp[i] = 0;
            }
            if(i > 0) {
                Serial.printf("%s %s: %s\n", level, tag, temp);
                if(message_callback)
                    message_callback(temp, logLevel);
            }
        } catch (...) {
            Serial.print("Error processing log message.");
            Serial.println(valueFormat);
        }

        
            // char loc_buf[64];
            // char * temp = loc_buf;
            // //va_list arg;
            // va_list copy;
            // //va_start(arg, format);
            // va_copy(copy, arg);
            // int len = vsnprintf(temp, sizeof(loc_buf), valueFormat, copy);
            // va_end(copy);
            // if(len < 0) {
            //     va_end(arg);
            //     //return 0;
            // };
            // if(len >= (int)sizeof(loc_buf)){  // comparation of same sign type for the compiler
            //     temp = (char*) malloc(len+1);
            //     if(temp == NULL) {
            //         va_end(arg);
            //         //return 0;
            //     }
            //     len = vsnprintf(temp, len+1, valueFormat, arg);
            // }
            // va_end(arg);
            // len = Serial.write((uint8_t*)temp, len);
            // if(temp != loc_buf){
            //     free(temp);
            // }
    }
    static bool isTagged(const char* tag) {
        if(m_tags.empty())
            return true;//tag all by default
        std::vector<String>::iterator position = std::find(m_tags.begin(), m_tags.end(), tag);
        return position != m_tags.end();
    }
    static bool isFiltered(const char* tag) {
        if(m_filters.empty())
            return false;
        // Serial.print("isFiltered: ");
        // Serial.println(tag);
        // Serial.println(m_filters.size());
        // Serial.println(m_filters.front());
        std::vector<String>::iterator position = std::find(m_filters.begin(), m_filters.end(), tag);
        // Serial.print("position: ");
        // Serial.println(std::distance( m_filters.begin(), position ));
        return position != m_filters.end();
    }
    static bool isLogged(const char* tag) {
        bool tagged = isTagged(tag);
        bool filtered = isFiltered(tag);
        // Serial.print("issTagged: ");
        // Serial.println(tagged);
        // Serial.print("filtered: ");
        // Serial.println(filtered);
        return tagged && !filtered;
    }
};
SemaphoreHandle_t LogHandler::xMutex = xSemaphoreCreateMutex();
std::vector<String> LogHandler::m_tags;
std::vector<String> LogHandler::m_filters;
LogLevel LogHandler::_currentLogLevel = LogLevel::INFO;
LOG_FUNCTION_PTR_T LogHandler::message_callback = 0;
char LogHandler::m_lastVerbose[1024];
char LogHandler::m_lastDebug[1024];
bool LogHandler::m_filterDuplicates = false;