#pragma once

#include "esp_log.h"

enum class LogLevel {
    ERROR,
    WARNING,
    INFO,
    VERBOSE
};

class LogHandler {
public:
    static void setLogLevel(LogLevel logLevel, const String tag = "*") {
        switch(logLevel) {
            case LogLevel::WARNING:
                esp_log_level_set(tag.c_str(), ESP_LOG_WARN);
            break;
            case LogLevel::INFO:
                esp_log_level_set(tag.c_str(), ESP_LOG_INFO);
            break;
            case LogLevel::VERBOSE:
                esp_log_level_set(tag.c_str(), ESP_LOG_VERBOSE);
            break;
            default:
                esp_log_level_set(tag.c_str(), ESP_LOG_ERROR);
        }
    }
    template<class... Ts>
    static void verbose(const char *tag, const char *format, Ts... args) {
        ESP_LOGV(tag, "%s", getFormatted(format, args...));
    }
    template<class... Ts>
    static void info(const char *tag, const char *format, Ts... args) {
        ESP_LOGI(tag, "%s", getFormatted(format, args...));
    }
    template<class... Ts>
    static void warning(const char *tag, const char *format, Ts... args) {
        ESP_LOGW(tag, "%s", getFormatted(format, args...));
    }
    template<class... Ts>
    static void error(const char *tag, const char *format, Ts... args) {
        ESP_LOGE(tag, "%s", getFormatted(format, args...));
    }

private:
    static char* getFormatted(const char *format, ...) {
        va_list vArgs;
        va_start(vArgs, format);
        char* temp = "";
        int len = vsnprintf(temp, sizeof(temp) - 1, format, vArgs);
        temp[sizeof(temp) - 1] = 0;
        // for (int i = len - 1; i >= 0; --i) {
        //     if (temp != '\n' && temp != '\r' && temp != ' ') {
        //         break;
        //     }
        //     temp = 0;
        // }
        va_end(vArgs);
        return temp;
    }
};