#pragma once

enum class TCodeVersion
{
    v0_2,
    v0_3,
    v0_5
};

enum class BuildFeature
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

enum class BoardType
{
    DEVKIT,
    CRIMZZON,
    ISAAC
};

enum class MotorType
{
    Servo,
    BLDC
};