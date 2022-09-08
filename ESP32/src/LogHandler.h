// #pragma once

// #include "esp_log.h"

// enum class LogLevel {
//     Error,
//     Warning,
//     Info,
//     Verbose
// };

// class LogHandler {
// public:
//     static void SetLogLevel(LogLevel logLevel) {
//         switch(logLevel) {
//             case LogLevel::Warning:
//                 esp_log_level_set("*", ESP_LOG_WARN);
//             break;
//             case LogLevel::Info:
//                 esp_log_level_set("*", ESP_LOG_INFO);
//             break;
//             case LogLevel::Verbose:
//                 esp_log_level_set("*", ESP_LOG_VERBOSE);
//             break;
//             default:
//                 esp_log_level_set("*", ESP_LOG_ERROR);
//         }
//     }
//     static void Verbose(const char *tag, const char *format, ... ) {
//         ESP_LOGV(tag, format, ...);
//     }
//     static void Info(const char *tag, const char *format, ... ) {
//         ESP_LOGI(tag, format, ...);
//     }
//     static void Warning(const char *tag, const char *format, ... ) {
//         ESP_LOGW(tag, format, ...);
//     }
//     static void Error(const char *tag, const char *format, ... ) {
//         ESP_LOGE(tag, format, var ...);
//     }
// };

// #include <functional>

// template<typename Func, typename... Args>
// struct nest {
//     std::function<void()> callBack;

//     void setup(Func func1, Args... args) {
//         callBack = [func1, args...]()
//         {
//             (func1)(args...);
//         };
//     }

//     unsigned process() {
//         callBack();
//         return 0;
//     }
// };

// template<typename Func, typename... Args>
// void handleFunc(Func func, Args&&... args) {
//     nest<Func, Args...> myNest;
//     myNest.setup(func, args...);
// }