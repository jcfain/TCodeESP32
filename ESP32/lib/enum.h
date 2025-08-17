#pragma once
#include <cstdint>

enum class TCodeVersion: int
{
    //v0_2,
    v0_3,
    v0_4,
    MAX
};

// enum class LogLevel {
//     ERROR,
//     WARNING,
//     INFO,
//     DEBUG,
//     VERBOSE
// };

enum class BuildFeature: int
{
    NONE,
    DEBUG,
    WIFI,
    BLUETOOTH,
    BLE,
    DA,
    DISPLAY_,
    TEMP,
    HTTPS,
    COEXIST_FEATURE,
    MAX_FEATURES
};

enum class ModuleType: int
{
    WROOM32,
    S3,
    MAX
};
#define MODULE_TYPES_HELP "WROOM32=0, S3=1"

enum class BoardType: int
{
    DEVKIT,
    ZERO,
    N8R8,
    CRIMZZON,
    ISAAC,
    SSR1PCB,
    MAX
};
#if MOTOR_TYPE == 1
#define BOARD_TYPES_HELP "Sets system board type and changes the default pinout.\nValid values are: DEVKIT=0, ZERO=1, N8R8=2, SSR1PCB=5"
#else
#define BOARD_TYPES_HELP "Sets system board type and changes the default pinout.\nValid values are: DEVKIT=0, ZERO=1, N8R8=2, SR6MB=3, InControl=4"
#endif

enum class MotorType: int
{
    Servo,
    BLDC,
    MAX
};

#define MOTOR_TYPES_HELP "Servo=0, BLDC=1"

enum class DeviceType: int
{
    OSR,
    SR6,
    SSR1,
    TVIBE,
    MAX
};

#if MOTOR_TYPE == 1
#define DEVICE_TYPES_HELP "Sets the system device type and resets the pinout\nValid values are: SSR1=2"
#else
#define DEVICE_TYPES_HELP "Sets the system device type and resets the pinout\nValid values are: OSR=0, SR6=1, TVIBE=3"
#endif

enum class BLDCEncoderType: int {
    MT6701,
    SPI,
    PWM,
    MAX
};
#define BLDC_ENCODER_TYPES_HELP "MT6701=0, SPI=1, PWM=2"

enum class BLEDeviceType: int {
    TCODE,
    LOVE,
    HC,
    MAX
};
#define BLDC_DEVICE_TYPES_HELP "TCODE=0, LOVE=1, HC=2"

enum class BLELoveDeviceType: int {
    EDGE,
    MAX
};
#define BLDC_LOVE_DEVICE_TYPES_HELP "EDGE=0"

enum class ESPTimerChannelNum: int8_t {
    NONE = -1,
#if CONFIG_IDF_TARGET_ESP32
    HIGH0_CH0,
    HIGH0_CH1,
    HIGH1_CH2,
    HIGH1_CH3,
    HIGH2_CH4,
    HIGH2_CH5,
    HIGH3_CH6,
    HIGH3_CH7,
#endif
    LOW0_CH0,
    LOW0_CH1,
    LOW1_CH2,
    LOW1_CH3,
    LOW2_CH4,
    LOW2_CH5,
    LOW3_CH6,
    LOW3_CH7,
    MAX
};