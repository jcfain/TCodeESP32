#pragma once
// CONFIG_IDF_TARGET_ESP32S3
#define MODULE_CURRENT ModuleType::S3
#define MAX_PWM_RESOLUTION 14
#define MAX_TIMERS 4
#define MAX_CHANNELS (MAX_TIMERS << 1)
#ifndef DEFAULT_BOARD
#ifdef S3_ZERO
    #define BOARD_TYPE_DEFAULT (uint8_t)BoardType::ZERO
#else
    #define BOARD_TYPE_DEFAULT (uint8_t)BoardType::N8R8
#endif
#endif

#define TASK_CPU_NUM APP_CPU_NUM
