#pragma once

enum class TCodeVersion: int
{
    v0_2,
    v0_3,
    v0_5
};
void convertFromJson(JsonVariantConst src, TCodeVersion& dst) {
    static_cast<uint8_t>(dst);
}
bool convertToJson(const TCodeVersion& src, JsonVariant dst) {
  return dst.set(static_cast<uint8_t>(src));
}

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
void convertFromJson(JsonVariantConst src, BuildFeature& dst) {
    static_cast<uint8_t>(dst);
}
bool convertToJson(const BuildFeature& src, JsonVariant dst) {
  return dst.set(static_cast<uint8_t>(src));
}

enum class BoardType: int
{
    DEVKIT,
    CRIMZZON,
    ISAAC
};
void convertFromJson(JsonVariantConst src, BoardType& dst) {
    static_cast<uint8_t>(dst);
}
bool convertToJson(const BoardType& src, JsonVariant dst) {
  return dst.set(static_cast<uint8_t>(src));
}


enum class MotorType: int
{
    Servo,
    BLDC
};

void convertFromJson(JsonVariantConst src, MotorType& dst) {
    static_cast<uint8_t>(dst);
}
bool convertToJson(const MotorType& src, JsonVariant dst) {
  return dst.set(static_cast<uint8_t>(src));
}


enum class DeviceType: int
{
    OSR,
    SR6,
    SSR1
};
void convertFromJson(JsonVariantConst src, DeviceType& dst) {
    static_cast<uint8_t>(dst);
}
bool convertToJson(const DeviceType& src, JsonVariant dst) {
  return dst.set(static_cast<uint8_t>(src));
}


enum class BLDCEncoderType: int {
    MT6701,
    SPI,
    PWM
};

void convertFromJson(JsonVariantConst src, BLDCEncoderType& dst) {
    static_cast<uint8_t>(dst);
}
bool convertToJson(const BLDCEncoderType& src, JsonVariant dst) {
  return dst.set(static_cast<uint8_t>(src));
}
