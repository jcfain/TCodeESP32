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
    DA,
    DISPLAY_,
    TEMP,
    HAS_TCODE_V2,
    HTTPS,
    COEXIST_FEATURE
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
    SSR1
};

enum class BLDCEncoderType: int {
    MT6701,
    SPI,
    PWM
};

enum class BLEDeviceType {
    TCODE,
    LOVE,
    HC
};

enum class BLELoveDeviceType {
    EDGE
    
};

