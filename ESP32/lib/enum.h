#pragma once

enum class TCodeVersion: int
{
    //v0_2,
    v0_3,
    v0_4
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
    S3
};

enum class BoardType: int
{
    DEVKIT,
    CRIMZZON,
    ISAAC
};

enum class MotorType: int
{
    Servo,
    BLDC
};

enum class DeviceType: int
{
    OSR,
    SR6,
    SSR1,
    TVIBE
};

enum class BLDCEncoderType: int {
    MT6701,
    SPI,
    PWM
};

enum class BLEDeviceType: int {
    TCODE,
    LOVE,
    HC
};

enum class BLELoveDeviceType: int {
    EDGE
    
};

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