#pragma once
// CONFIG_IDF_TARGET_ESP32E22
#error "this board is not supported at this time. I thought I had one but was mislead. I aim to add this at somepoint once I can get one"
#define MODULE_CURRENT ModuleType::E22
#define MAX_PWM_RESOLUTION 16
#define MAX_TIMERS 6
#define MAX_CHANNELS 6
#ifndef DEFAULT_BOARD
#define BOARD_TYPE_DEFAULT (uint8_t)BoardType::DEVKIT_E22
#endif
#define TASK_CPU_NUM PRO_CPU_NUM