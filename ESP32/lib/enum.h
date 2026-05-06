#pragma once
#include <cstdint>

enum class LogLevel
{
    NONE,
    ERROR,
    WARNING,
    INFO,
    DEBUG,
    VERBOSE
};

enum class SettingType
{
    NONE,
    Boolean,
    Number,
    String,
    Float,
    Double,
    ArrayString,
    ArrayInt,
    MAX
};

enum class SettingProfile
{
    System,
    Wireless,
    Wifi,
    Button,
    MotionProfile,
    Temperature,
    Display,
    Servo,
    Pin,
    Timer,
    Bldc,
    Battery,
    Voice,
    PWM,
    Analog,
    Bluetooth,
    Ble,
    ChannelRanges,
    Vib,
    Disabled,
    Readonly,
    MAX
};

enum class RestartRequired {
    NO,
    YES
};

enum class TCodeVersion: int
{
    //v0_2,
    v0_3,
    v0_4,
    MAX
};

#ifdef MOTOR_TYPE_BLDC
enum class BLDCMotorPosition
{
    A,
    B
};
enum class BLDCBootMode {
    CALIBRATE,
    HOMING,
    NORMAL
};
#endif

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
    C5,
    C6,
    E22,
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
    SR6PCB,
    DEVKIT_C5,
    DEVKIT_C6,
    DEVKIT_E22,
    MAX
};
#ifdef MOTOR_TYPE_BLDC
#define BOARD_TYPES_HELP "Sets system board type and changes the default pinout.\nValid values are: DEVKIT=0, ZERO=1, N8R8=2, SSR1PCB=5, DEVKITC6=6, DEVKITC61=7"
#else
#define BOARD_TYPES_HELP "Sets system board type and changes the default pinout.\nValid values are: DEVKIT=0, ZERO=1, N8R8=2, SR6MB=3, InControl=4, SR6PCB=6, DEVKITC6=7, DEVKITC61=8"
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
    NONE,
    OSR,
    SR6,
    SSR1,
    SSR2,
    TVIBE,
    MAX
};

#ifdef MOTOR_TYPE_BLDC
#define DEVICE_TYPES_HELP "Sets the system device type and resets the pinout\nValid values are: SSR1=2, SSR2=3"
#else
#define DEVICE_TYPES_HELP "Sets the system device type and resets the pinout\nValid values are: OSR=0, SR6=1, TVIBE=5"
#endif

enum class BLDCEncoderType: int {
    NONE,
    MT6701,
    SPI,
    PWM,
    MAX
};
#define BLDC_ENCODER_TYPES_HELP "MT6701=0, SPI=1, PWM=2"

enum class LubeButtonPinMode : int {
    PULL_UP,
    PULL_DOWN,
    FLOAT,
    MAX
};
#define LUBE_BUTTON_PIN_MODE_HELP "PULL_UP=0, PULL_DOWN=1, FLOAT=2"

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

/** PWM peripheral driver selection for a timer group.
 *  MCPWM = 0: use MCPWM hardware (servo-grade precision, limited to 12 outputs).
 *  LEDC  = 1: use LEDC hardware (general purpose, up to 16 channels).
 */
enum class PwmDriver : int8_t {
    MCPWM = 0,
    LEDC = 1
};
#define PWM_DRIVER_HELP "MCPWM=0, LEDC=1"

enum class WifiBand {
    AUTO,
    MODE24ghz,
    MODE5ghz,
    MODE6ghz,
    MAX
};
#if SOC_WIFI_SUPPORT_5G
#define WIFI_MODE_HELP "AUTO=0, 2.4ghz=1, 5ghz=2"
#elif SOC_WIFI_SUPPORT_6G
#define WIFI_MODE_HELP "AUTO=0, 2.4ghz=1, 5ghz=2, 6ghz=3"
#else
#define WIFI_MODE_HELP "AUTO=0, 2.4ghz=1"
#endif


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
    LOW0_CH0,
    LOW0_CH1,
    LOW1_CH2,
    LOW1_CH3,
    LOW2_CH4,
    LOW2_CH5,
    LOW3_CH6,
    LOW3_CH7,
#elif CONFIG_IDF_TARGET_ESP32S3
    // ESP32-S3: keep LOW (LEDC) channels at 0..7 so they map to valid LEDC
    // channel numbers; HIGH (MCPWM) channels follow as 8..15 — MCPWM ignores
    // the raw channel number at attach time so any unique value works.
    LOW0_CH0,
    LOW0_CH1,
    LOW1_CH2,
    LOW1_CH3,
    LOW2_CH4,
    LOW2_CH5,
    LOW3_CH6,
    LOW3_CH7,
    HIGH0_CH0,
    HIGH0_CH1,
    HIGH1_CH2,
    HIGH1_CH3,
    HIGH2_CH4,
    HIGH2_CH5,
    HIGH3_CH6,
    HIGH3_CH7,
#endif
    MAX
};

enum class VoltageMonitors : int8_t {
    VOLTAGE_3V3,
    VOLTAGE_5V,
    VOLTAGE_BATTERY,
    VOLTAGE_MOTOR,
    VOLTAGE_BUS,
    MAX
};

enum class TemperatureType {
	INTERNAL,
	SLEEVE
};
