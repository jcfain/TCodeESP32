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
#include <vector>
#include <ArduinoJson.h>

#include "enum.h"
#include "callback.h"
#include "constants.h"
#include "utils.h"

#define LOG_LEVEL_HELP "Sets system log level.\nValid values are: NONE=0, ERROR=1, WARNING=2, INFO=3, DEBUG=4, VERBOSE=5"

struct LogMessage {
    LogLevel level;
    size_t len;
    char message[MAX_LOG_STORE];
};

class LogHandler {
public:
    static const int internal_buffer_length = 1024;

    static void init() 
    {
        Serial.println("[LogHandler::init]");
        // if(m_logQueue) 
        // {
        //     return;
        //     // vQueueDelete(m_logQueue);
        //     // m_logQueue = 0;
        // }
        // LogLevel level = getInstance().getLogLevel();
        // if(level > LogLevel::NONE)
        // {
        //     int queueSize = 25;
        //     if(level == LogLevel::VERBOSE) {
        //         queueSize = 200;
        //     } else if(level == LogLevel::DEBUG) {
        //         queueSize = 100;
        //     }
        // }
    }

    static void setLogStore(bool enabled) 
    {
        m_logStored = enabled;
        if(m_logStored)
        {
            m_logQueue = xQueueCreate(25, sizeof(LogMessage));
            if(m_logQueue == NULL) 
            {
                Serial.println("[LogHandler::setLogStore] Error creating the log queue");
            }
        }
        else if(m_logQueue)
        {
            vQueueDelete(m_logQueue);
            m_logQueue = 0;
        }
    }

    static bool getLog(LogMessage* log) {
        if(m_logQueue) 
        {
            if(xQueueReceive(m_logQueue, log, 0) == pdTRUE) 
            {
                // if(getLogLevel() == LogLevel::VERBOSE)
                //     Serial.printf("[LogHandler::getLog] read from q: %s\n", log->message);
                return true;
            }
        }
        return false;
    }

    static void setLogLevel(LogLevel logLevel) {
        if(logLevel != getInstance().m_currentLogLevel)
        {
			Serial.printf("[LogHandler::setLogLevel] Log level changed to: %d\n", (uint8_t)logLevel);
            getInstance().m_currentLogLevel = logLevel;
            //init();
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
            Serial.printf("[LogHandler::addInclude] add include: %s\n", tag);
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
            Serial.printf("[LogHandler::removeInclude] remove include: %s\n", tag);
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
            Serial.printf("[LogHandler::addExclude] add exclude: %s\n", tag);
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
            Serial.printf("[LogHandler::removeExclude] remove exclude: %s\n", tag);
            // Serial.println(m_filters.size());
        } else {
            // Serial.println(m_filters.size());
            return false;
        }
        return true;
    }

    static void clearExcludes() { getInstance().m_filters.clear(); }

    static size_t info(const char *tag, const char *format, ...) {
        LogHandler &log = getInstance();
        size_t len = 0;
        if (log.m_currentLogLevel >= LogLevel::INFO) {
            xSemaphoreTake(log.m_xMutex, portMAX_DELAY);
            if (isLogged(tag)) {
                va_list vArgs;
                va_start(vArgs, format);
                len = parseMessage(format, "[INFO]", tag, LogLevel::INFO, vArgs);
                va_end(vArgs);
            }
            xSemaphoreGive(log.m_xMutex);
        }
        return len;
    }

    static size_t warning(const char *tag, const char *format, ...) {
        LogHandler &log = getInstance();
        size_t len = 0;
        if (log.m_currentLogLevel >= LogLevel::WARNING) {
            xSemaphoreTake(log.m_xMutex, portMAX_DELAY);
            if (isLogged(tag)) {
                va_list vArgs;
                va_start(vArgs, format);
                len = parseMessage(format, "[WARNING]", tag, LogLevel::WARNING, vArgs);
                va_end(vArgs);
            }
            xSemaphoreGive(log.m_xMutex);
        }
        return len;
    }

    static size_t error(const char *tag, const char *format, ...) {
        LogHandler &log = getInstance();
        size_t len = 0;
        if (log.m_currentLogLevel >= LogLevel::ERROR) {
            xSemaphoreTake(log.m_xMutex, portMAX_DELAY);
            if (isLogged(tag)) {
                va_list vArgs;
                va_start(vArgs, format);
                len = parseMessage(format, "[ERROR]", tag, LogLevel::ERROR, vArgs);
                va_end(vArgs);
            }
            xSemaphoreGive(log.m_xMutex);
        }
        return len;
    }

    static size_t debug(const char *tag, const char *format, ...) {
        LogHandler &log = getInstance();
        size_t len = 0;
        if (log.m_currentLogLevel >= LogLevel::DEBUG) {
            xSemaphoreTake(log.m_xMutex, portMAX_DELAY);
            if (isLogged(tag)) {
                va_list vArgs;
                va_start(vArgs, format);
                len = parseMessage(format, "[DEBUG]", tag, LogLevel::DEBUG, vArgs);
                va_end(vArgs);
            }
            xSemaphoreGive(log.m_xMutex);
        }
        return len;
    }

    static size_t verbose(const char *tag, const char *format, ...) {
        LogHandler &log = getInstance();
        size_t len = 0;
        if (log.m_currentLogLevel >= LogLevel::VERBOSE) {
            xSemaphoreTake(log.m_xMutex, portMAX_DELAY);
            if (isLogged(tag)) {
                va_list vArgs;
                va_start(vArgs, format);
                len = parseMessage(format, "[VERBOSE]", tag, LogLevel::VERBOSE, vArgs);
                va_end(vArgs);
            }
            xSemaphoreGive(log.m_xMutex);
        }
        return len;
    }

    /// @brief Logs a message without any formatting
    /// @param format 
    /// @param  
    /// @return 
    static size_t raw(const char *format = "", ...) {
        LogHandler &log = getInstance();
        size_t len = 0;
        xSemaphoreTake(log.m_xMutex, portMAX_DELAY);
        va_list vArgs;
        va_start(vArgs, format);
        len = parseMessage(format, "", "", LogLevel::NONE, vArgs);
        va_end(vArgs);
        xSemaphoreGive(log.m_xMutex);
        return len;
    }

    static const char *getLastError() { return getInstance().m_lastError; }

    static void setMessageCallback(LogCallback f) {
        getInstance().m_message_callback = f == nullptr ? 0 : f;
    }

    static void printWebAddress(const char* tag, const char* hostAddress, const int& port) 
    {
        char webServerportString[7] = {0};
        snprintf(webServerportString, 7, ":%i", port);
        LogHandler::info(tag, "Web address: http://%s%s", hostAddress, port == 80 ? "" : webServerportString);
    }

    static void printFree(bool forcePrint = false) {
        if(forcePrint || LogHandler::getLogLevel() == LogLevel::DEBUG)
        {
            uint32_t freeHEap = ESP.getFreeHeap();
            uint32_t heapSize = ESP.getHeapSize();
            //https://esp32.com/viewtopic.php?t=27780
            //https://github.com/espressif/esp-idf/blob/master/components/heap/include/esp_heap_caps.h#L20-L37
            //esp_get_free_internal_heap_size
            LogHandler::raw("Used heap INTERNAL: %u/%u Free: %u\n", heapSize - freeHEap, heapSize, freeHEap);
            LogHandler::raw("Free psram: %u\n", ESP.getFreePsram());
            LogHandler::raw("Total Psram: %u\n", ESP.getPsramSize());
            LogHandler::raw("LittleFS used: %i\n", LittleFS.usedBytes());
            LogHandler::raw("LittleFS total: %i\n", LittleFS.totalBytes());
            //LogHandler::debug(_TAG, "Used Psram: %u/%u", ESP.getPsramSize() - ESP.getFreePsram(), ESP.getPsramSize());
            LogHandler::raw("Sketch size: %u\n", ESP.getSketchSize());
            LogHandler::raw("Sketch free space: %u\n", ESP.getFreeSketchSpace());
            LogHandler::raw("DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
            LogHandler::raw("IRAM %u\n", heap_caps_get_free_size(MALLOC_CAP_32BIT));
            LogHandler::raw("FREE_HEAP Default %u\n", esp_get_free_heap_size());
            LogHandler::raw("MIN_FREE_HEAP %u\n", esp_get_minimum_free_heap_size() );
            //uxTaskGetStackHighWaterMark
        }
    }

    static bool logDeserializationError(const char* tag, DeserializationError error, const char* fileName, DeserializationError::Code& code) 
    {
        if (error)
        {
            LogHandler::error(tag, "Error deserializing json: %s", fileName);
            code = error.code();
            switch (code)
            {
                case DeserializationError::Code::Ok:
                    LogHandler::error(tag, "Code: Ok");
                    break;
                case DeserializationError::Code::EmptyInput:
                    LogHandler::error(tag, "Code: EmptyInput");
                    break;
                case DeserializationError::Code::IncompleteInput:
                    LogHandler::error(tag, "Code: IncompleteInput");
                    break;
                case DeserializationError::Code::InvalidInput:
                    LogHandler::error(tag, "Code: InvalidInput");
                    break;
                case DeserializationError::Code::NoMemory:
                    LogHandler::error(tag, "Code: NoMemory");
                    break;
                case DeserializationError::Code::TooDeep:
                    LogHandler::error(tag, "Code: TooDeep");
                    break;
            }
            return true;
        }
        return false;
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
    static inline QueueHandle_t m_logQueue = 0;
    static inline bool m_logStored = false;

    LogCallback m_message_callback = 0;
    LogLevel m_currentLogLevel = LogLevel::INFO;
    SemaphoreHandle_t m_xMutex = xSemaphoreCreateMutex();
    std::vector<const char*> m_tags;
    std::vector<const char*> m_filters;
    char m_lastVerbose[internal_buffer_length];
    char m_lastDebug[internal_buffer_length];
    char m_lastError[internal_buffer_length];
    bool m_filterDuplicates = false;

    static size_t parseMessage(const char *valueFormat, const char *level,
                             const char *tag, LogLevel logLevel, va_list vArgs) 
    {
        LogHandler &log = getInstance();
		if (strlen(valueFormat) > internal_buffer_length) 
        {
			Serial.println("[LogHandler::parseMessage] Error Log value too big for buffer");
			return 0;
		}
		char temp[internal_buffer_length] = {'\0'};
		int len = vsnprintf(temp, internal_buffer_length - 1, valueFormat, vArgs);

		if (len < 0) 
        {
			Serial.println("[LogHandler::parseMessage] Error printing vargs");
			return 0;
		}
		// for (size_t i = internal_buffer_length - 1; i >= 0; --i) 
        // {
		// 	if ((temp[i] != '\n') && (temp[i] != '\r') && (temp[i] != ' ') && (i < len)) 
        //     {
		// 		break;
		// 	}
		// 	temp[i] = 0;
		// }
		char temp2[internal_buffer_length] = {'\0'};
        if(logLevel == LogLevel::NONE)
            len = snprintf(temp2, internal_buffer_length, "%s", temp);
        else
            len = snprintf(temp2, internal_buffer_length, "%s %s: %s", level, tag, temp);

		if (len < 0) 
        {
			Serial.println("[LogHandler::parseMessage] Error printing with tag");
			return 0;
		}

		if (log.m_filterDuplicates) 
        {
			switch (logLevel) {
            case LogLevel::NONE:
            case LogLevel::INFO:
            case LogLevel::WARNING:
                    break;
			case LogLevel::ERROR:
				if (strcmp(log.m_lastError, temp2) == 0)
					return 0;
				break;
			case LogLevel::VERBOSE:
				if (strcmp(log.m_lastVerbose, temp2) == 0)
					return 0;
				break;
			case LogLevel::DEBUG:
				if (strcmp(log.m_lastDebug, temp2) == 0)
					return 0;
				break;
			}
		}
		Serial.printf("%s\n", temp2);
		switch (logLevel) 
        {
            case LogLevel::NONE:
            case LogLevel::INFO:
            case LogLevel::WARNING:
                break;
            case LogLevel::ERROR:
                strncpy(log.m_lastError, temp2, internal_buffer_length);
                break;
            case LogLevel::VERBOSE:
                strncpy(log.m_lastVerbose, temp2, internal_buffer_length);
                break;
            case LogLevel::DEBUG:
                strncpy(log.m_lastDebug, temp2, internal_buffer_length);
                break;
		}

        storeLog(temp2, len, logLevel);
		if (log.m_message_callback)
			log.m_message_callback(temp2, len, logLevel);
        return len;
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

    static void storeLog(const char* message, const size_t& len, const LogLevel& level)
    {
        if (!m_logQueue) 
            return;
        if(level > LogLevel::DEBUG)// Ignore verbose. Seems to cause issues for now.
            return;
        if (!uxQueueSpacesAvailable(m_logQueue)) 
            return;
        
        // LogMessage logMessage;
        // logMessage.level = level;
        // Chunker chunker(message, len, MAX_LOG_STORE);
        // size_t chunkLen = chunker(logMessage.message);
        // Serial.printf("[LogHandler::storeLog] chunkLen: %u\n", chunkLen);
        // while(chunkLen > 0) 
        // {
        //     logMessage.len = chunkLen;
        //     if(xQueueSend(m_logQueue, &logMessage, 0) != pdTRUE) 
        //     {
        //         Serial.printf("[LogHandler::storeLog] Error storing log message: %s, len: %u\n", logMessage.message, logMessage.len);
        //     }
        //     chunkLen = chunker(logMessage.message);
        //     Serial.printf("[LogHandler::storeLog] chunkLen: %u\n", chunkLen);
        // }


        int maxLen = MAX_LOG_STORE;
        LogMessage logMessage;
        logMessage.level = level;
        if(len > maxLen) 
        {
            int mod = len % maxLen;
            int chunkTotal = len - mod;
            int sendChunks = chunkTotal / maxLen;
            int sent = 0;
            int amountToSend = chunkTotal / sendChunks;
            if(getLogLevel() >= LogLevel::DEBUG) 
            {
                Serial.printf("[LogHandler::storeLog] len: %i, sendChunks: %i, amountToSend: %i\n", len, sendChunks, amountToSend);
                Serial.printf("[LogHandler::storeLog] sent: %i\n", sent);
            }
            for(int i = 0; i < sendChunks; i++)
            {
                // char messageToSend[amountToSend];
                strncpy(logMessage.message, message + sent, amountToSend);
                logMessage.message[amountToSend] = '\0';
                logMessage.len = amountToSend;
                if(getLogLevel() >= LogLevel::DEBUG) 
                    Serial.printf("[LogHandler::storeLog] truncated: %s\n", logMessage.message);
                if(xQueueSend(m_logQueue, &logMessage, 0) != pdTRUE) 
                {
                    Serial.printf("[LogHandler::storeLog] Error storing log message trancate: %s\n", logMessage.message);
                }
                sent += amountToSend;
                if(getLogLevel() >= LogLevel::DEBUG) 
                    Serial.printf("[LogHandler::storeLog] sent: %i\n", sent);
            }
            if(mod)
            {
                strncpy(logMessage.message, message + sent, mod);
                logMessage.message[mod] = '\0';
                strncat(logMessage.message, "\n", 5);
                logMessage.len = mod +1;
                if(getLogLevel() >= LogLevel::DEBUG) 
                    Serial.printf("[LogHandler::storeLog] truncated mod: %i, message: %s\n", mod, logMessage.message);
                if(xQueueSend(m_logQueue, &logMessage, 0) != pdTRUE) 
                {
                    Serial.printf("[LogHandler::storeLog] Error storing log message trancate mod: %s\n", logMessage.message);
                }
                sent += mod;
                if(getLogLevel() >= LogLevel::DEBUG) 
                    Serial.printf("[LogHandler::storeLog] sent: %i\n", sent);
            }
        } 
        else 
        {
            strncpy(logMessage.message, message, len);
            logMessage.message[len] = '\0';
            strncat(logMessage.message, "\n", 5);
            logMessage.len = len +1;
            if(xQueueSend(m_logQueue, &logMessage, 0) != pdTRUE) 
            {
                Serial.printf("[LogHandler::storeLog] Error storing log message\n");
            }
        }
    }
};