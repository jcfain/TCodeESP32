
// OSR-Alpha3_ESP32
// by TempestMAx 9-7-21
// Please copy, share, learn, innovate, give attribution.
// Decodes T-code commands and uses them to control servos and vibration motors
// It can handle:
//   10x linear channels (L0, L1, L2... L9)
//   10x rotation channels (R0, R1, R2... L9)
//   10x vibration channels (V0, V1, V2... V9)
//   10x auxilliary channels (A0, A1, A2... A9)
// This code is for the ESP32 DevKit v1 and is designed to drive the SR6 stroker robot,
// but is also able to drive the OSR2. See below for servo pin assignments
// Have fun, play safe!
// History:
// Alpha3 - First ESP32 release, 9-7-2021

// ----------------------------
//   Settings
// ----------------------------

#pragma once

#include "TCode0_4.h"
#include "settings/SettingsHandler.h"
#include "Global.h"
#include "MotorHandler0_4.h"
#include "logging/TagHandler.h"
#include "settingsFactory.h"
#include "pinMap.h"

class ServoHandler0_4 : public MotorHandler0_4
{

public:
    ServoHandler0_4() : MotorHandler0_4(new TCode0_4()) {}
    // Setup function
    // This is run once, when the arduino starts
    void setup() override
    {
        LogHandler::debug(_TAG, "Setting up servo handler v4");
        m_settingsFactory = SettingsFactory::getInstance();
        m_settingsFactory->getValue(DEVICE_TYPE, m_deviceType);
        LogHandler::debug(_TAG, "DEVICE_TYPE: %d", m_deviceType);
        if(m_deviceType == DeviceType::NONE)
        {
            LogHandler::error(_TAG, "No device type selected. Visit the web config or use the command to set a device before starting the firmware.");
            m_initFailed = true;
            return;
        }
        if (m_deviceType == DeviceType::TVIBE)
        {
            LogHandler::info(_TAG, "Setting up motor for device type TVibe");
            setupCommon();
            m_tcode->sendMessage("Ready!");
            return;
        }
        LogHandler::debug(_TAG, "MS_PER_RAD: %d", ms_per_rad);

        // Set SR6 arms to startup positions
        if (m_deviceType == DeviceType::SR6)
        {
            read("R2750");
        }
        // Register device axes
        stroke_axis = new TCodeAxis("Stroke", {AxisType::Linear, 0}, 0.5f);
        m_tcode->RegisterAxis(stroke_axis);
        if (m_deviceType == DeviceType::SR6)
        {
            surge_axis = new TCodeAxis("Surge", {AxisType::Linear, 1}, 0.5f);
            m_tcode->RegisterAxis(surge_axis);
            sway_axis = new TCodeAxis("Sway", {AxisType::Linear, 2}, 0.5f);
            m_tcode->RegisterAxis(sway_axis);
        }
        roll_axis = new TCodeAxis("Roll", {AxisType::Rotation, 1}, 0.5f);
        m_tcode->RegisterAxis(roll_axis);
        pitch_axis = new TCodeAxis("Pitch", {AxisType::Rotation, 2}, 0.5f);
        m_tcode->RegisterAxis(pitch_axis);
        PinMap *pinMap;
        if (m_deviceType == DeviceType::SR6)
        {
            pinMap = PinMapSR6::getInstance();
        }
        else
        {
            pinMap = PinMapOSR::getInstance();
        }
// Lower Left Servo
#ifndef ESP_PROG // The default pins for these are used on the debugger board.
        m_leftServoPin = ((PinMapOSR *)pinMap)->leftServo();
        m_lowerLeftServoChannel = ((PinMapOSR *)pinMap)->leftServoChannel();
        if (m_leftServoPin > -1)
        {
            int freq = ((PinMapOSR *)pinMap)->getChannelFrequency(m_lowerLeftServoChannel);
            m_leftServo_Int = frequencyToMicroseconds(freq);
            attachServoPin("left servo", m_leftServoPin, freq, m_lowerLeftServoChannel, pinMap->getTimerDriverForChannel(m_lowerLeftServoChannel));
        }
        else
        {
            LogHandler::error(_TAG, "Invalid left servo pin: %d (channel: %d)", m_leftServoPin, m_lowerLeftServoChannel);
            m_initFailed = true;
        }
        m_rightServoPin = ((PinMapOSR *)pinMap)->rightServo();
        m_lowerRightServoChannel = ((PinMapOSR *)pinMap)->rightServoChannel();
        if (m_rightServoPin > -1)
        {
            int freq = ((PinMapOSR *)pinMap)->getChannelFrequency(m_lowerRightServoChannel);
            m_rightServo_Int = frequencyToMicroseconds(freq);
            attachServoPin("right servo", m_rightServoPin, freq, m_lowerRightServoChannel, pinMap->getTimerDriverForChannel(m_lowerRightServoChannel));
        }
        else
        {
            LogHandler::error(_TAG, "Invalid right servo pin: %d (channel: %d)", m_rightServoPin, m_lowerRightServoChannel);
            m_initFailed = true;
        }
#endif
        if (m_deviceType == DeviceType::SR6)
        {
            m_leftUpperServoPin = ((PinMapSR6 *)pinMap)->leftUpperServo();
            m_upperLeftServoChannel = ((PinMapSR6 *)pinMap)->leftUpperServoChannel();
            if (m_leftUpperServoPin > -1)
            {
                int freq = ((PinMapSR6 *)pinMap)->getChannelFrequency(m_upperLeftServoChannel);
                m_leftUpperServo_Int = frequencyToMicroseconds(freq);
                attachServoPin("left upper servo", m_leftUpperServoPin, freq, m_upperLeftServoChannel, pinMap->getTimerDriverForChannel(m_upperLeftServoChannel));
            }
            else
            {
                LogHandler::error(_TAG, "Invalid left upper servo pin: %d (channel: %d)", m_leftUpperServoPin, m_upperLeftServoChannel);
                m_initFailed = true;
            }
#ifndef ESP_PROG // The default pins for these are used on the debugger board. 12, 13, 14 & 15
            m_rightUpperServoPin = ((PinMapSR6 *)pinMap)->rightUpperServo();
            m_upperRightServoChannel = ((PinMapSR6 *)pinMap)->rightUpperServoChannel();
            if (m_rightUpperServoPin > -1)
            {
                int freq = ((PinMapSR6 *)pinMap)->getChannelFrequency(m_upperRightServoChannel);
                m_rightUpperServo_Int = frequencyToMicroseconds(freq);
                attachServoPin("right upper servo", m_rightUpperServoPin, freq, m_upperRightServoChannel, pinMap->getTimerDriverForChannel(m_upperRightServoChannel));
            }
            else
            {
                LogHandler::error(_TAG, "Invalid right upper servo pin: %d (channel: %d)", m_rightUpperServoPin, m_upperRightServoChannel);
                m_initFailed = true;
            }
            m_rightPitchServoPin = ((PinMapSR6 *)pinMap)->pitchRight();
            m_rightPitchServoChannel = ((PinMapSR6 *)pinMap)->pitchRightChannel();
            if (m_rightPitchServoPin > -1)
            {
                int freq = ((PinMapSR6 *)pinMap)->getChannelFrequency(m_rightPitchServoChannel);
                m_pitchRightServo_Int = frequencyToMicroseconds(freq);
                attachServoPin("right pitch servo", m_rightPitchServoPin, freq, m_rightPitchServoChannel, pinMap->getTimerDriverForChannel(m_rightPitchServoChannel));
            }
            else
            {
                LogHandler::error(_TAG, "Invalid right pitch servo pin: %d (channel: %d)", m_rightPitchServoPin, m_rightPitchServoChannel);
                m_initFailed = true;
            }
#endif
        }
        m_leftPitchServoPin = ((PinMapSR6 *)pinMap)->pitchLeft();
        m_leftPitchServoChannel = ((PinMapSR6 *)pinMap)->pitchLeftChannel();
        if (m_leftPitchServoPin > -1)
        {
            int freq = ((PinMapSR6 *)pinMap)->getChannelFrequency(m_leftPitchServoChannel);
            m_pitchLeftServo_Int = frequencyToMicroseconds(freq);
            attachServoPin("pitch servo", m_leftPitchServoPin, freq, m_leftPitchServoChannel, pinMap->getTimerDriverForChannel(m_leftPitchServoChannel));
        }
        else
        {
            LogHandler::error(_TAG, "Invalid pitch servo pin: %u (channel: %d)", m_leftPitchServoPin, m_leftPitchServoChannel);
            m_initFailed = true;
        }

        setupCommon();

        // Signal done
        if (m_initFailed)
        {
            m_tcode->sendMessage("Init servos error!");
        }
        else
        {
            m_tcode->sendMessage("Ready!");
        }
        return;
    }

    void setMessageCallback(TCodeCommandCallback function) override
    {
        m_tcode->setMessageCallback(function);
    }

    void read(const String &input) override
    {
        m_tcode->read(input);
    }

    void read(const char *input, size_t len) override
    {
        for (int i = 0; i < len; i++)
        {
            read(input[i]);
        }
    }

    void read(byte input) override
    {
        m_tcode->read(input);
    }

    // String getDeviceSettings() {
    //     return m_tcode->getDeviceSettings();
    // }
    // int testVar = -1;
    // int testVar2 = -1;
    int lastXLin = 0;
    void execute() override
    {
        if (m_initFailed)
        {
            return;
        }
        // Skip the normal PWM write loop while an Identify wiggle is in
        // progress — otherwise it overwrites the wiggle duty every ~1 ms
        // and the user sees no movement.
        if (isIdentifying())
        {
            return;
        }
        if (m_deviceType != DeviceType::TVIBE)
        {
            // Collect inputs
            // These functions query the t-code object for the position/level at a specified time
            // Number recieved will be an integer, 0-10000
            xLin = channelRead(stroke_axis);
            if (lastXLin != xLin)
            {
                Serial.println(xLin);
                lastXLin = xLin;
            }
            yRot = channelRead(roll_axis);
            zRot = channelRead(pitch_axis);

            if (m_deviceType == DeviceType::OSR)
            {
                executeOSR(xLin, yRot, zRot);
            }
            else if (m_deviceType == DeviceType::SR6)
            {
                executeSR6(xLin, yRot, zRot);
            }
        }

        executeCommon(xLin);

        // m_tcode->updateInterfaces();
    }

    /**
     * Wiggle a single physical servo for visual identification.
     * Uses a small ±100µs offset from ZERO to avoid mechanical damage on a
     * misconfigured device.  4 pulses over ~2 s then returns to centre.
     */
    void identifyServo(const char* servoName) override
    {
        int8_t pin = -1;
        int servoInt = -1;
        int zeroMicros = 1500;

        if (strcmp(servoName, "RightServo") == 0) {
            pin = m_rightServoPin;
            servoInt = m_rightServo_Int;
            zeroMicros = m_settingsFactory->getRightServo_ZERO();
        } else if (strcmp(servoName, "LeftServo") == 0) {
            pin = m_leftServoPin;
            servoInt = m_leftServo_Int;
            zeroMicros = m_settingsFactory->getLeftServo_ZERO();
        } else if (strcmp(servoName, "RightUpperServo") == 0) {
            pin = m_rightUpperServoPin;
            servoInt = m_rightUpperServo_Int;
            zeroMicros = m_settingsFactory->getRightUpperServo_ZERO();
        } else if (strcmp(servoName, "LeftUpperServo") == 0) {
            pin = m_leftUpperServoPin;
            servoInt = m_leftUpperServo_Int;
            zeroMicros = m_settingsFactory->getLeftUpperServo_ZERO();
        } else if (strcmp(servoName, "PitchServo") == 0) {
            pin = m_leftPitchServoPin;
            servoInt = m_pitchLeftServo_Int;
            zeroMicros = m_settingsFactory->getPitchLeftServo_ZERO();
        } else if (strcmp(servoName, "PitchRightServo") == 0) {
            pin = m_rightPitchServoPin;
            servoInt = m_pitchRightServo_Int;
            zeroMicros = m_settingsFactory->getPitchRightServo_ZERO();
        } else {
            // Valve, Twist, Squeeze — handled by parent class (has access to private members)
            MotorHandler0_4::identifyServo(servoName);
            return;
        }

        if (pin < 0 || servoInt < 0) {
            LogHandler::warning(_TAG, "identifyServo '%s' aborted: pin<0 or servoInt<0", servoName);
            return;
        }

        _startWiggleTask(pin, servoInt, zeroMicros);
    }

private:
    static constexpr Tags::tag_t _TAG = Tags::Servo;
    SettingsFactory *m_settingsFactory;
    DeviceType m_deviceType;
    int MainServo_Int;
    int PitchServo_Int;

    int8_t m_leftServoPin = -1;
    int8_t m_rightServoPin = -1;
    int8_t m_rightUpperServoPin = -1;
    int8_t m_leftUpperServoPin = -1;
    int8_t m_leftPitchServoPin = -1;
    int8_t m_rightPitchServoPin = -1;

    int8_t m_lowerLeftServoChannel = -1;
    int8_t m_lowerRightServoChannel = -1;
    int8_t m_upperLeftServoChannel = -1;
    int8_t m_upperRightServoChannel = -1;
    int8_t m_leftPitchServoChannel = -1;
    int8_t m_rightPitchServoChannel = -1;

    int m_leftServo_Int = -1;
    int m_rightServo_Int = -1;
    int m_leftUpperServo_Int = -1;
    int m_rightUpperServo_Int = -1;
    int m_pitchLeftServo_Int = -1;
    int m_pitchRightServo_Int = -1;

    TCodeAxis *stroke_axis = 0;
    TCodeAxis *surge_axis = 0;
    TCodeAxis *sway_axis = 0;
    TCodeAxis *roll_axis = 0;
    TCodeAxis *pitch_axis = 0;

    // Declare classes
    // This uses the t-code object above
    // Declare operating variables
    // Position variables
    int xLin, yLin, zLin;
    // Rotation variables
    int yRot, zRot;

    void executeOSR(int strokeTcode, int rollTcode, int pitchTcode)
    {
        // Calculate arm angles
        // Linear scale inputs to servo appropriate numbers
        int stroke, roll, pitch;
        stroke = map(strokeTcode, 0, 10000, -350, 350);
        roll = map(rollTcode, 0, 10000, -180, 180);
        pitch = map(pitchTcode, 0, 10000, -350, 350);
        int leftDuty;
        int rightDuty;
        int pitchDuty;
        if (m_settingsFactory->getInverseStroke())
        {
            leftDuty = map(m_settingsFactory->getLeftServo_ZERO() - stroke + roll, 0, m_leftServo_Int, 0, m_servoPWMMaxDuty);
            rightDuty = map(m_settingsFactory->getRightServo_ZERO() + stroke + roll, 0, m_rightServo_Int, 0, m_servoPWMMaxDuty);
        }
        else
        {
            leftDuty = map(m_settingsFactory->getLeftServo_ZERO() + stroke + roll, 0, m_leftServo_Int, 0, m_servoPWMMaxDuty);
            rightDuty = map(m_settingsFactory->getRightServo_ZERO() - stroke + roll, 0, m_rightServo_Int, 0, m_servoPWMMaxDuty);
        }
        if (m_settingsFactory->getInversePitch())
        {
            pitchDuty = map(m_settingsFactory->getPitchLeftServo_ZERO() + pitch, 0, m_pitchLeftServo_Int, 0, m_servoPWMMaxDuty);
        }
        else
        {
            pitchDuty = map(m_settingsFactory->getPitchLeftServo_ZERO() - pitch, 0, m_pitchLeftServo_Int, 0, m_servoPWMMaxDuty);
        }

#ifndef ESP_PROG
        writeServo(m_leftServoPin, leftDuty);
        writeServo(m_rightServoPin, rightDuty);
        writeServo(m_leftPitchServoPin, pitchDuty);
#endif
    }

    void executeSR6(int strokeTcode, int rollTcode, int pitchTcode)
    {
        yLin = channelRead(surge_axis);
        zLin = channelRead(sway_axis);
        // SR6 Kinematics
        // Calculate arm angles
        int roll, pitch, fwd, thrust, side;
        if (m_settingsFactory->getInverseStroke())
        {
            roll = map(rollTcode, TCODE_MIN, TCODE_MAX, 3000, -3000);
            pitch = map(pitchTcode, TCODE_MIN, TCODE_MAX, 2500, -2500);
            fwd = map(yLin, TCODE_MIN, TCODE_MAX, 3000, -3000);
            thrust = map(strokeTcode, TCODE_MIN, TCODE_MAX, 6000, -6000);
            side = map(zLin, TCODE_MIN, TCODE_MAX, 3000, -3000);
        }
        else
        {
            roll = map(rollTcode, TCODE_MIN, TCODE_MAX, -3000, 3000);
            pitch = map(pitchTcode, TCODE_MIN, TCODE_MAX, -2500, 2500);
            fwd = map(yLin, TCODE_MIN, TCODE_MAX, -3000, 3000);
            thrust = map(strokeTcode, TCODE_MIN, TCODE_MAX, -6000, 6000);
            side = map(zLin, TCODE_MIN, TCODE_MAX, -3000, 3000);
        }

        // Main arms
        int lowerLeftValue = SetMainServo(16248 - fwd, 1500 + thrust + roll);  // Lower left servo
        int lowerRightValue = SetMainServo(16248 - fwd, 1500 + thrust - roll); // Lower right servo
        int upperLeftValue = SetMainServo(16248 - fwd, 1500 - thrust - roll);  // Upper left servo
        int upperRightValue = SetMainServo(16248 - fwd, 1500 - thrust + roll); // Upper right servo
        int pitchLeftValue = SetPitchServo(16248 - fwd, 4500 - thrust, side - 1.5 * roll, -pitch);
        int pitchRightValue = SetPitchServo(16248 - fwd, 4500 - thrust, -side + 1.5 * roll, -pitch);

        int lowerLeftDuty = map(m_settingsFactory->getLeftServo_ZERO() - lowerLeftValue, 0, m_leftServo_Int, 0, m_servoPWMMaxDuty);
        int lowerRightDuty = map(m_settingsFactory->getRightServo_ZERO() + lowerRightValue, 0, m_rightServo_Int, 0, m_servoPWMMaxDuty);
        int upperLeftDuty = map(m_settingsFactory->getLeftUpperServo_ZERO() + upperLeftValue, 0, m_leftUpperServo_Int, 0, m_servoPWMMaxDuty);
        int upperRightDuty = map(m_settingsFactory->getRightUpperServo_ZERO() - upperRightValue, 0, m_rightUpperServo_Int, 0, m_servoPWMMaxDuty);
        uint16_t pitchLeftZero = m_settingsFactory->getPitchLeftServo_ZERO();
        uint16_t pitchRightZero = m_settingsFactory->getPitchRightServo_ZERO();
        int pitchLeftDuty = map(constrain(pitchLeftZero - pitchLeftValue, pitchLeftZero - 600, pitchLeftZero + 1000), 0, m_pitchLeftServo_Int, 0, m_servoPWMMaxDuty);
        int pitchRightDuty = map(constrain(pitchRightZero + pitchRightValue, pitchRightZero - 1000, pitchRightZero + 600), 0, m_pitchRightServo_Int, 0, m_servoPWMMaxDuty);
        // Set Servos
#if !ESP_PROG
        writeServo(m_leftServoPin, lowerLeftDuty);
        writeServo(m_rightServoPin, lowerRightDuty);
        writeServo(m_leftUpperServoPin, upperLeftDuty);
        writeServo(m_rightUpperServoPin, upperRightDuty);
        writeServo(m_leftPitchServoPin, pitchLeftDuty);
        writeServo(m_rightPitchServoPin, pitchRightDuty);
#endif
    }

    // -----------------------------------------------------------------------
    // Wiggle helper – spawns a short-lived FreeRTOS task so the WebSocket
    // handler is not blocked.
    // -----------------------------------------------------------------------
    struct WiggleParams {
        ServoHandler0_4 *self;
        int8_t pin;
        int    servoInt;
        int    zeroMicros;
    };

    static void _wiggleTask(void *arg)
    {
        WiggleParams *p = static_cast<WiggleParams *>(arg);
        // Block the motor loop's PWM writes for the duration of the wiggle
        // so executeOSR/executeSR6 doesn't overwrite our duty every ~1 ms.
        MotorHandler::setIdentifying(true);
        // ±100 µs offset → very small movement (~3°), safe regardless of config
        constexpr int OFFSET_US = 100;
        uint32_t hiDuty  = static_cast<uint32_t>(map(p->zeroMicros + OFFSET_US, 0, p->servoInt, 0, (int)p->self->m_servoPWMMaxDuty));
        uint32_t loDuty  = static_cast<uint32_t>(map(p->zeroMicros - OFFSET_US, 0, p->servoInt, 0, (int)p->self->m_servoPWMMaxDuty));
        uint32_t midDuty = static_cast<uint32_t>(map(p->zeroMicros,             0, p->servoInt, 0, (int)p->self->m_servoPWMMaxDuty));
        const uint32_t duties[4] = { hiDuty, loDuty, hiDuty, loDuty };
        for (int i = 0; i < 4; i++) {
            p->self->writeServo(p->pin, duties[i]);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        p->self->writeServo(p->pin, midDuty);
        MotorHandler::setIdentifying(false);
        delete p;
        vTaskDelete(nullptr);
    }

    void _startWiggleTask(int8_t pin, int servoInt, int zeroMicros)
    {
        WiggleParams *params = new WiggleParams{ this, pin, servoInt, zeroMicros };
        // 4 KiB stack: the wiggle path now calls LogHandler::info which
        // pulls newlib vsnprintf in (~2 KiB by itself). 2 KiB tripped the
        // FreeRTOS stack canary.
        xTaskCreate(_wiggleTask, "servoWiggle", 4096, params, 1, nullptr);
    }

    // Function to calculate the angle for the main arm servos
    // Inputs are target x,y coords of receiver pivot in 1/100 of a mm
    int SetMainServo(float x, float y)
    {
        x /= 100;
        y /= 100;                                                       // Convert to mm
        float gamma = atan2(x, y);                                      // Angle of line from servo pivot to receiver pivot
        float csq = sq(x) + sq(y);                                      // Square of distance between servo pivot and receiver pivot
        float c = sqrt(csq);                                            // Distance between servo pivot and receiver pivot
        float beta = acos(constrain((csq - 28125) / (100 * c), -1, 1)); // Angle between c-line and servo arm
        int out = ms_per_rad * (gamma + beta - 3.14159);                // Servo signal output, from neutral
        return out;
    }

    // Function to calculate the angle for the pitcher arm servos
    // Inputs are target x,y,z coords of receiver upper pivot in 1/100 of a mm
    // Also pitch in 1/100 of a degree
    int SetPitchServo(float x, float y, float z, float pitch)
    {
        pitch *= 0.0001745; // Convert to radians
        x += 5500 * sin(0.2618 + pitch);
        y -= 5500 * cos(0.2618 + pitch);
        x /= 100;
        y /= 100;
        z /= 100;                                                            // Convert to mm
        float bsq = 36250 - sq(75 + z);                                      // Equivalent arm length
        float gamma = atan2(x, y);                                           // Angle of line from servo pivot to receiver pivot
        float csq = sq(x) + sq(y);                                           // Square of distance between servo pivot and receiver pivot
        float c = sqrt(csq);                                                 // Distance between servo pivot and receiver pivot
        float beta = acos(constrain((csq + 5625 - bsq) / (150 * c), -1, 1)); // Angle between c-line and servo arm
        int out = ms_per_rad * (gamma + beta - 3.14159);                     // Servo signal output, from neutral
        return out;
    }
};
