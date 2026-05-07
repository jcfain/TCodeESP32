#pragma once
// CONFIG_IDF_TARGET_ESP32C5
#define MODULE_CURRENT ModuleType::C5
#define MAX_PWM_RESOLUTION 16
#define MAX_TIMERS 4
#define MAX_CHANNELS 6
#ifndef DEFAULT_BOARD
#define BOARD_TYPE_DEFAULT (uint8_t)BoardType::DEVKIT_C5
#endif
#define TASK_CPU_NUM PRO_CPU_NUM
// #define CONFIG_PREFIX ="C5"