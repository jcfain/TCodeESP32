#ifndef POWER_HANDLER_H
#define POWER_HANDLER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <math.h>
#include <memory>

#include "settings/SettingsHandler.h"
#include "logging/LogHandler.h"
#include "logging/TagHandler.h"
#include "tasks/TaskHandler.h"

using POWER_STATE_FUNCTION_PTR_T = void (*)(const char* payload);

class PowerHandler : public TaskHandler::Task {
public:
    PowerHandler() : Task(TaskHandler::Rates::SLOW) {}

    void setup() override {
        m_settingsFactory = SettingsFactory::getInstance();
        m_pinMap = m_settingsFactory->getPins();
        m_hasAnyMonitor = false;
        m_payloadJson.reserve(1400);
        loadCalibration();

        analogReadResolution(12);

        for (int i = 0; i < static_cast<int>(VoltageMonitors::MAX); ++i) {
            const VoltageMonitors source = static_cast<VoltageMonitors>(i);
            const int8_t pin = m_pinMap ? m_pinMap->voltageMonitor(source) : -1;
            if (pin >= 0) {
                pinMode(pin, INPUT);
#if defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S3)
                analogSetPinAttenuation(pin, ADC_11db);
#endif
                m_hasAnyMonitor = true;
            }
        }

        // Initialize servo voltage enable pin as OUTPUT
        const int8_t servoVoltageEnablePin = m_pinMap ? m_pinMap->servoVoltageEnable() : -1;
        if (servoVoltageEnablePin >= 0) {
            pinMode(servoVoltageEnablePin, OUTPUT);
            // Read saved state from settings, default to true
            bool enabledState = SERVO_VOLTAGE_ENABLE_STATE_DEFAULT;
            m_settingsFactory->getValue(SERVO_VOLTAGE_ENABLE_STATE, enabledState);
            setServoVoltageEnabled(enabledState);
            LogHandler::info(Tags::Power, "Servo voltage enable initialized on pin %d, state: %s", servoVoltageEnablePin, enabledState ? "enabled" : "disabled");
        }

        if (m_hasAnyMonitor) {
            LogHandler::info(Tags::Power, "Power monitor enabled");
        }
        else {
            LogHandler::info(Tags::Power, "Power monitor disabled (all monitor pins unset)");
        }
    }

    void setMessageCallback(POWER_STATE_FUNCTION_PTR_T callback) {
        message_callback = callback;
    }

    void setServoVoltageEnabled(bool enabled) {
        if (!m_pinMap) return;
        const int8_t pin = m_pinMap->servoVoltageEnable();
        if (pin < 0) return;
        digitalWrite(pin, enabled ? HIGH : LOW);
        m_servoVoltageEnabled = enabled;
        LogHandler::info(Tags::Power, "Servo voltage %s", enabled ? "enabled" : "disabled");
    }

    bool isServoVoltageEnabled() const {
        return m_servoVoltageEnabled;
    }

    void loop() override {
        if (!m_hasAnyMonitor || !m_pinMap || millis() < m_nextTickMs) {
            return;
        }

        m_nextTickMs = millis() + m_tickMs;

        JsonDocument doc;
        // Use a fixed-size stack buffer instead of repeated String::concat to
        // avoid churning the heap (each += on a String can realloc, which can
        // fragment the heap and crash inside the allocator under pressure).
        char logLine[256];
        int logPos = snprintf(logLine, sizeof(logLine), "Power monitor:");
        if (logPos < 0) logPos = 0;
        if (logPos > (int)sizeof(logLine)) logPos = sizeof(logLine);
        bool anyValue = false;

        for (int i = 0; i < static_cast<int>(VoltageMonitors::MAX); ++i) {
            const VoltageMonitors source = static_cast<VoltageMonitors>(i);
            const int8_t pin = m_pinMap->voltageMonitor(source);
            if (pin < 0) {
                continue;
            }

            const uint16_t raw = analogRead(pin);
            const float adcVoltage = (raw * 3.3f) / 4095.0f;
            const float railVoltage = convertToRailVoltage(source, adcVoltage);

            JsonObject sourceObj = doc[sourceName(source)].to<JsonObject>();
            sourceObj["pin"] = pin;
            sourceObj["raw"] = raw;
            sourceObj["adcVoltage"] = adcVoltage;
            sourceObj["railVoltage"] = railVoltage;
            sourceObj["dividerRatio"] = m_dividerRatio[indexOf(source)];
            sourceObj["offset"] = m_offset[indexOf(source)];

            const float nominal = nominalVoltageForSource(source);
            if (nominal > 0.0f) {
                sourceObj["nominalVoltage"] = nominal;
                sourceObj["percentage"] = (railVoltage / nominal) * 100.0f;
            }

            if (logPos < (int)sizeof(logLine)) {
                int written = snprintf(logLine + logPos, sizeof(logLine) - logPos,
                    " %s=%.3fV", sourceName(source), railVoltage);
                if (written > 0) {
                    logPos += written;
                    if (logPos > (int)sizeof(logLine)) logPos = sizeof(logLine);
                }
            }
            anyValue = true;
        }

        if (!anyValue) {
            return;
        }

        // Power monitor sample line is verbose by design — surfaces every
        // tick (~2 s) and is noisy at INFO. Promote to VERBOSE so callers
        // who care can opt in via log-level config; default consoles stay
        // quiet. JSON broadcast below still goes to the websocket.
        LogHandler::verbose(Tags::Power, "%s", logLine);

        // Add servo voltage enabled state to the JSON response
        doc["servoVoltageEnabled"] = m_servoVoltageEnabled;

        if (message_callback) {
            // Serialize into a single fixed-size heap allocation rather than
            // into a String. ArduinoJson's String writer calls String::concat
            // on every flush which calls realloc(); under heap fragmentation
            // that can panic inside the allocator. One malloc + one copy avoids
            // the realloc churn.
            const size_t jsonLen = measureJson(doc);
            std::unique_ptr<char[]> raw(new (std::nothrow) char[jsonLen + 1]);
            if (!raw) {
                LogHandler::error(Tags::Power, "PowerHandler: failed to allocate %u bytes", (unsigned)(jsonLen + 1));
            }
            else {
                const size_t written = serializeJson(doc, raw.get(), jsonLen + 1);
                raw[written] = '\0';
                message_callback(raw.get());
            }
        }
    }

private:
    static int indexOf(VoltageMonitors source) {
        return static_cast<int>(source);
    }

    static bool isDefaultRatio(float ratio) {
        return fabsf(ratio - 1.0f) < 0.0005f;
    }

    void loadCalibration() {
        // Global defaults for boards without monitor divider hardware.
        for (int i = 0; i < static_cast<int>(VoltageMonitors::MAX); ++i) {
            m_dividerRatio[i] = 1.0f;
            m_offset[i] = 0.0f;
        }

        m_settingsFactory->getValue(POWER_MONITOR_3V3_DIVIDER_RATIO, m_dividerRatio[indexOf(VoltageMonitors::VOLTAGE_3V3)]);
        m_settingsFactory->getValue(POWER_MONITOR_5V_DIVIDER_RATIO, m_dividerRatio[indexOf(VoltageMonitors::VOLTAGE_5V)]);
        m_settingsFactory->getValue(POWER_MONITOR_BATTERY_DIVIDER_RATIO, m_dividerRatio[indexOf(VoltageMonitors::VOLTAGE_BATTERY)]);
        m_settingsFactory->getValue(POWER_MONITOR_MOTOR_DIVIDER_RATIO, m_dividerRatio[indexOf(VoltageMonitors::VOLTAGE_MOTOR)]);
        m_settingsFactory->getValue(POWER_MONITOR_BUS_DIVIDER_RATIO, m_dividerRatio[indexOf(VoltageMonitors::VOLTAGE_BUS)]);

        m_settingsFactory->getValue(POWER_MONITOR_3V3_OFFSET, m_offset[indexOf(VoltageMonitors::VOLTAGE_3V3)]);
        m_settingsFactory->getValue(POWER_MONITOR_5V_OFFSET, m_offset[indexOf(VoltageMonitors::VOLTAGE_5V)]);
        m_settingsFactory->getValue(POWER_MONITOR_BATTERY_OFFSET, m_offset[indexOf(VoltageMonitors::VOLTAGE_BATTERY)]);
        m_settingsFactory->getValue(POWER_MONITOR_MOTOR_OFFSET, m_offset[indexOf(VoltageMonitors::VOLTAGE_MOTOR)]);
        m_settingsFactory->getValue(POWER_MONITOR_BUS_OFFSET, m_offset[indexOf(VoltageMonitors::VOLTAGE_BUS)]);

        m_settingsFactory->getValue(POWER_MONITOR_VBUS_NOMINAL, m_vbusNominal);
        m_settingsFactory->getValue(POWER_MONITOR_VMOTOR_NOMINAL, m_vmotorNominal);

        BoardType boardType = BoardType::DEVKIT;
        m_settingsFactory->getValue(BOARD_TYPE_SETTING, boardType);
        if (boardType == BoardType::SR6PCB) {
            // SR6PCB analog monitor divider network is 22k/222k => ratio ~= 0.099.
            // Calibrated to 0.106 based on measured accuracy (~6-11% error correction).
            for (int i = 0; i < static_cast<int>(VoltageMonitors::MAX); ++i) {
                if (isDefaultRatio(m_dividerRatio[i])) {
                    m_dividerRatio[i] = 0.106f;
                }
            }
        }
    }

    float convertToRailVoltage(VoltageMonitors source, float adcVoltage) const {
        const int index = indexOf(source);
        const float divider = m_dividerRatio[index];
        const float offset = m_offset[index];
        if (divider <= 0.0f) {
            return adcVoltage + offset;
        }
        return (adcVoltage / divider) + offset;
    }

    float nominalVoltageForSource(VoltageMonitors source) const {
        switch (source) {
        case VoltageMonitors::VOLTAGE_BUS:
            return m_vbusNominal;
        case VoltageMonitors::VOLTAGE_MOTOR:
            return m_vmotorNominal;
        default:
            return -1.0f;
        }
    }

    static const char* sourceName(VoltageMonitors source) {
        switch (source) {
        case VoltageMonitors::VOLTAGE_3V3:
            return "Voltage_3V3";
        case VoltageMonitors::VOLTAGE_5V:
            return "Voltage_5V";
        case VoltageMonitors::VOLTAGE_BATTERY:
            return "Voltage_Battery";
        case VoltageMonitors::VOLTAGE_MOTOR:
            return "Voltage_Motor";
        case VoltageMonitors::VOLTAGE_BUS:
            return "Voltage_Bus";
        default:
            return "Unknown";
        }
    }

    SettingsFactory* m_settingsFactory = nullptr;
    PinMap* m_pinMap = nullptr;
    POWER_STATE_FUNCTION_PTR_T message_callback = nullptr;
    bool m_servoVoltageEnabled = false;
    float m_dividerRatio[static_cast<int>(VoltageMonitors::MAX)] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
    float m_offset[static_cast<int>(VoltageMonitors::MAX)] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    float m_vbusNominal = 20.0f;
    float m_vmotorNominal =
#if MOTOR_TYPE == 1
        20.0f;
#else
        9.0f;
#endif
    bool m_hasAnyMonitor = false;
    unsigned long m_nextTickMs = 0;
    const unsigned long m_tickMs = 2000;
    String m_payloadJson;
};

#endif // POWER_HANDLER_H
