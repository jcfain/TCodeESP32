#pragma once

enum class TCodeVersion: int
{
    v0_2,
    v0_3,
    v0_5
};

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
    HTTPS
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