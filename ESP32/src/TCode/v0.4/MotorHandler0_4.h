/* MIT License

Copyright (c) 2026 Jason C. Fain

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#pragma once

#include <Arduino.h>
#include "Global.h"
#include "MotorHandler.h"
#include "TCode0_4.h"
#include "settings/SettingsHandler.h"
#include "logging/TagHandler.h"

class MotorHandler0_4 : public MotorHandler
{
public:
    MotorHandler0_4() {}
    MotorHandler0_4(TCode0_4 *tcode) : MotorHandler(), m_tcode(tcode) {}

protected:
    TCode0_4 *m_tcode = 0;
    uint32_t m_servoPWMMaxDuty;
    // Servo microseconds per radian
    // (Standard: 637 μs/rad)
    // (LW-20: 700 μs/rad)
    // 270 2/3 of 637 = 424.666666667
    int ms_per_rad; // (μs/rad)
    uint8_t maxServoRange;

    void setupCommon()
    {
        if (!m_tcode)
            return;

        m_settingsFactory = SettingsFactory::getInstance();

        PinMap *pinMap = m_settingsFactory->getPins();

        m_tcode->setup(FIRMWARE_VERSION_NAME);

        // See MotorHandler0_3::setupCommon for rationale: default these to the
        // compile-time resolution so a missing settings key cannot leave
        // m_servoPWMMaxDuty as UINT32_MAX.
        int servoResolution = SERVO_PWM_RES;
        int vibeResolution = SERVO_PWM_RES;
        int lubeResolution = SERVO_PWM_RES;
        m_settingsFactory->getValue(SERVO_RESOLUTION, servoResolution);
        m_settingsFactory->getValue(VIBE_RESOLUTION, vibeResolution);
        m_settingsFactory->getValue(LUBE_RESOLUTION, lubeResolution);
        if (servoResolution <= 0 || servoResolution > 16) servoResolution = SERVO_PWM_RES;
        if (vibeResolution  <= 0 || vibeResolution  > 16) vibeResolution  = SERVO_PWM_RES;
        if (lubeResolution  <= 0 || lubeResolution  > 16) lubeResolution  = SERVO_PWM_RES;

        m_servoPWMMaxDuty = static_cast<uint32_t>((1UL << servoResolution) - 1);
        m_settingsFactory->getValue(MAX_SERVO_RANGE, maxServoRange);
        if (!maxServoRange)
        {
            LogHandler::error(Tags::Motor, "Invalid, max Servo range. Setting to 180...");
            maxServoRange = 180;
        }
        ms_per_rad = 114592 / maxServoRange;
        LogHandler::debug(Tags::Motor, "MS_PER_RAD: %d", ms_per_rad);
        LogHandler::debug(Tags::Motor, "Servo Resolution: %d", servoResolution);
        LogHandler::debug(Tags::Motor, "Vibe Resolution: %d", vibeResolution);
        LogHandler::debug(Tags::Motor, "Lube Resolution: %d", lubeResolution);


        m_valveServoPin = pinMap->valve();
        m_valveServoChannel = pinMap->valveChannel();
        if (m_valveServoPin > -1)
        {
            valve_channel = new TCodeAxis("Valve", {AxisType::Auxiliary, 0}, 0.0f);
            m_tcode->RegisterAxis(valve_channel);
            m_tcode->setAxisData(valve_channel, 0.5, AxisExtentionType::Time, 3000);
            suck_channel = new TCodeAxis("Valve", {AxisType::Auxiliary, 1}, 0.0f);
            m_tcode->RegisterAxis(suck_channel);
            int freq = pinMap->getChannelFrequency(m_valveServoChannel);
            attachServoPin("valve servo", m_valveServoPin, freq, m_valveServoChannel, pinMap->getTimerDriverForChannel(m_valveServoChannel));
            m_valveServo_Int = frequencyToMicroseconds(freq);
        }
        else
        {
            m_valveServoPin = -1;
        }

        m_twistServoPin = pinMap->twist();
        m_twistServoChannel = pinMap->twistChannel();
        if (m_twistServoPin > -1)
        {
            twist_channel = new TCodeAxis("Twist", {AxisType::Rotation, 0}, 0.5f);
            m_tcode->RegisterAxis(twist_channel);
            int freq = pinMap->getChannelFrequency(m_twistServoChannel);
            attachServoPin("twist servo", m_twistServoPin, freq, m_twistServoChannel, pinMap->getTimerDriverForChannel(m_twistServoChannel));
            m_twistServo_Int = frequencyToMicroseconds(freq);
        }
        else
        {
            m_twistServoPin = -1;
        }

        m_squeezeServoPin = pinMap->squeeze();
        m_squeezeServoChannel = pinMap->squeezeChannel();
        if (m_squeezeServoPin > -1)
        {
            squeeze_channel = new TCodeAxis("Squeeze", {AxisType::Auxiliary, 3}, 0.5f);
            m_tcode->RegisterAxis(squeeze_channel);
            int freq = pinMap->getChannelFrequency(m_squeezeServoChannel);
            attachServoPin("aux servo", m_squeezeServoPin, freq, m_squeezeServoChannel, pinMap->getTimerDriverForChannel(m_squeezeServoChannel));
            m_squeezeServo_Int = frequencyToMicroseconds(freq);
        }
        else
        {
            m_squeezeServoPin = -1;
        }

        bool lubeEnabled = false;
        m_settingsFactory->getValue(LUBE_ENABLED, lubeEnabled);
        if (lubeEnabled)
        {
            m_lubeButtonPin = pinMap->lubeButton();
            m_vib1Pin = pinMap->vibe1();
            m_vib1Channel = pinMap->vibe1Channel();
            if (m_lubeButtonPin > -1 && m_vib1Pin > -1)
            {
                lube_channel = new TCodeAxis("Lube", {AxisType::Auxiliary, 2}, 0.0f);
                m_tcode->RegisterAxis(lube_channel);
                // m_tcode->AxisInput("A2",0,' ',0);
                m_tcode->setAxisData(lube_channel, 0, AxisExtentionType::Time, 0);
                pinMode(m_lubeButtonPin, m_settingsFactory->getLubeButtonPinMode());
                int freq = pinMap->getChannelFrequency(m_vib1Channel);
                attachLedcPin("lube", m_vib1Pin, freq, m_vib1Channel);
                // m_vib1_Int = frequencyToMicroseconds(freq);
                lubeRegistered = true;
            }
        }

        // Set vibration PWM pins
        m_vib0Pin = pinMap->vibe0();
        m_vib0Channel = pinMap->vibe0Channel();
        if (m_vib0Pin > -1)
        {
            vibe0_channel = new TCodeAxis("Vibe 1", {AxisType::Vibration, 0}, 0.0f);
            m_tcode->RegisterAxis(vibe0_channel);
            int freq = pinMap->getChannelFrequency(m_vib0Channel);
            attachLedcPin("vib 1", m_vib0Pin, freq, m_vib0Channel);
            // m_vib0_Int = frequencyToMicroseconds(freq);
        }
        else
        {
            m_vib0Pin = -1;
        }

        if (!lubeRegistered)
        {
            m_vib1Pin = pinMap->vibe1();
            m_vib1Channel = pinMap->vibe1Channel();
            if (m_vib1Pin > -1)
            {
                vibe1_channel = new TCodeAxis("Vibe 2", {AxisType::Vibration, 1}, 0.0f);
                m_tcode->RegisterAxis(vibe1_channel);
                int freq = pinMap->getChannelFrequency(m_vib1Channel);
                attachLedcPin("vib 2", m_vib1Pin, freq, m_vib1Channel);
                // m_vib1_Int = frequencyToMicroseconds(freq);
            }
            else
            {
                m_vib1Pin = -1;
            }
        }
        m_vib2Pin = pinMap->vibe2();
        m_vib2Channel = pinMap->vibe2Channel();
        if (m_vib2Pin > -1)
        {
            vibe2_channel = new TCodeAxis("Vibe 3", {AxisType::Vibration, 2}, 0.0f);
            m_tcode->RegisterAxis(vibe2_channel);
            int freq = pinMap->getChannelFrequency(m_vib2Channel);
            attachLedcPin("vib 3", m_vib2Pin, freq, m_vib2Channel);
            // m_vib2_Int = frequencyToMicroseconds(freq);
        }
        else
        {
            m_vib2Pin = -1;
        }
        m_vib3Pin = pinMap->vibe3();
        m_vib3Channel = pinMap->vibe3Channel();
        if (m_vib3Pin > -1)
        {
            vibe3_channel = new TCodeAxis("Vibe 4", {AxisType::Vibration, 3}, 0.0f);
            m_tcode->RegisterAxis(vibe3_channel);
            int freq = pinMap->getChannelFrequency(m_vib3Channel);
            attachLedcPin("vib 4", m_vib3Pin, freq, m_vib3Channel);
            // m_vib3_Int = frequencyToMicroseconds(freq);
        }
        else
        {
            m_vib3Pin = -1;
        }

        m_settingsFactory->getValue(FEEDBACK_TWIST, m_isTwistFeedBack);
        if (m_isTwistFeedBack)
        {
            m_twistFeedBackPin = pinMap->twistFeedBack();
            if (m_twistFeedBackPin > -1)
            {
                // Initiate position tracking for twist
                pinMode(m_twistFeedBackPin, INPUT);
                m_settingsFactory->getValue(ANALOG_TWIST, m_isAnalogTwist);
                if (!m_isAnalogTwist)
                {
                    LogHandler::debug(Tags::Motor, "Attaching interrupt for twist feedback to pin: %u", pinMap->twistFeedBack());
                    attachInterrupt(m_twistFeedBackPin, twistChange, CHANGE);
                    // Serial.print("Setting digital twist ");
                    // Serial.println(SettingsHandler::getTwistFeedBack_PIN());
                }
                else
                {
                    // Serial.print("Setting analog twist ");
                    // Serial.println(SettingsHandler::getTwistFeedBack_PIN());
                    /*             adcAttachPin(SettingsHandler::getTwistFeedBack_PIN());
                                analogReadResolution(11);
                                analogSetAttenuation(ADC_6db); */
                }
            }
        }
        // report status
        m_tcode->read("D0\n");
        m_tcode->read("D1\n");
    }

    bool m_initFailed = false;

    void executeCommon(const int xLin)
    {
        if (!m_tcode || m_initFailed)
            return;
        executeTwist();
        executeSqueeze();
        executeValve(xLin);
        executeVibe(0);
        if (!lubeRegistered)
            executeVibe(1);
        else
            executeLube();
        executeVibe(2);
        executeVibe(3);
    }

protected:
    uint16_t channelRead(TCode::Axis::TCodeAxis *channel)
    {
        uint16_t value = m_tcode->getChannelPosition(channel);
        if (SettingsHandler::getChannelRangesEnabled())
        {
            Channel *channel = SettingsHandler::getChannel(channel->Name);
            if (channel && channel->rangeLimitEnabled)
            {
                return map(value, TCODE_MIN, TCODE_MAX, channel->userMin, channel->userMax);
            }
        }
        return value;
    }

    void identifyServo(const char* servoName) override
    {
        int8_t pin = -1;
        int servoInt = -1;
        int zeroMicros = 1500;
        if (strcmp(servoName, "ValveServo") == 0) {
            pin = m_valveServoPin;
            servoInt = m_valveServo_Int;
            zeroMicros = m_settingsFactory->getValveServo_ZERO();
        } else if (strcmp(servoName, "TwistServo") == 0) {
            pin = m_twistServoPin;
            servoInt = m_twistServo_Int;
            zeroMicros = m_settingsFactory->getTwistServo_ZERO();
        } else if (strcmp(servoName, "SqueezeServo") == 0) {
            pin = m_squeezeServoPin;
            servoInt = m_squeezeServo_Int;
            zeroMicros = m_settingsFactory->getSqueezeServo_ZERO();
        }
        if (pin < 0 || servoInt < 0)
            return;
        struct WiggleParams {
            MotorHandler0_4 *self;
            int8_t pin;
            int    servoInt;
            int    zeroMicros;
        };
        auto* params = new WiggleParams{ this, pin, servoInt, zeroMicros };
        xTaskCreate([](void* arg) {
            auto* p = static_cast<WiggleParams*>(arg);
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
            delete p;
            vTaskDelete(nullptr);
        }, "servoWiggle", 2048, params, 1, nullptr);
    }

private:
    SettingsFactory *m_settingsFactory;
    bool m_isAnalogTwist = false;
    bool m_isTwistFeedBack = false;
    int8_t m_twistFeedBackPin = -1;
    int8_t m_lubeButtonPin = -1;
    // Servo pin cache
    int8_t m_twistServoPin = -1;
    int8_t m_squeezeServoPin = -1;
    int8_t m_valveServoPin = -1;
    int8_t m_vib0Pin = -1;
    int8_t m_vib1Pin = -1;
    int8_t m_vib2Pin = -1;
    int8_t m_vib3Pin = -1;

    int8_t m_twistServoChannel = -1;
    int8_t m_squeezeServoChannel = -1;
    int8_t m_valveServoChannel = -1;
    int8_t m_vib0Channel = -1;
    int8_t m_vib1Channel = -1;
    int8_t m_vib2Channel = -1;
    int8_t m_vib3Channel = -1;

    int m_twistServo_Int = -1;
    int m_squeezeServo_Int = -1;
    int m_valveServo_Int = -1;

    bool m_manualLubeOverride = false;
    // Last duty written to the lube output. Used for debug-log edge
    // detection so we don't spam the log every loop iteration.
    int m_lastLubeDuty = -1;

    TCodeAxis *twist_channel = 0;
    TCodeAxis *squeeze_channel = 0;
    TCodeAxis *vibe0_channel = 0;
    TCodeAxis *vibe1_channel = 0;
    TCodeAxis *vibe2_channel = 0;
    TCodeAxis *vibe3_channel = 0;
    TCodeAxis *valve_channel = 0;
    TCodeAxis *suck_channel = 0;
    TCodeAxis *lube_channel = 0;

    int xRot, squeezeCmd;
    // Velocity tracker variables, for valve
    float twistServoAngPos = 0.5;
    int twistTurns = 0;
    float twistPos;

    int lube;
    bool lubeRegistered = false;
    int valveCmd, suckCmd;
    int vibe0, vibe1, vibe2, vibe3;
    float upVel, valvePos;
    unsigned long tLast;
    int xLast;

    void executeTwist()
    {
        if (!twist_channel)
            return;
        xRot = channelRead(twist_channel);
        if (xRot > -1)
        {
            if (m_isTwistFeedBack && !m_settingsFactory->getContinuousTwist())
            {
                float angPos;
                // Calculate twist position
                if (!m_isAnalogTwist)
                {
                    // noInterrupts();
                    float dutyCycle = twistPulseLength;
                    dutyCycle = dutyCycle / lastTwistPulseCycle;
                    // interrupts();
                    angPos = (dutyCycle - 0.029) / 0.942;
                    //  Serial.print("angPos ");
                    //  Serial.println(angPos);
                }
                else
                {
                    int feedBackValue = analogRead(m_twistFeedBackPin);
                    angPos = feedBackValue / 675.0;
                    // if(feedBackValue != testVar) {
                    //     testVar = feedBackValue;
                    //     Serial.print("feedBackValue: ");
                    //     Serial.println(feedBackValue);
                    //     Serial.print("angPos: ");
                    //     Serial.println(angPos);
                    // }
                }
                angPos = constrain(angPos, 0, 1) - 0.5;
                if (angPos - twistServoAngPos < -0.8)
                {
                    twistTurns += 1;
                }
                if (angPos - twistServoAngPos > 0.8)
                {
                    twistTurns -= 1;
                }
                twistServoAngPos = angPos;
                twistPos = 1000 * (angPos + twistTurns);
            }

            // Twist
            int twist;
            if (m_isTwistFeedBack && !m_settingsFactory->getContinuousTwist())
            {
                twist = (xRot - map(twistPos, -1500, 1500, TCODE_MAX, TCODE_MIN)) / 5;
                if (!m_isAnalogTwist)
                {
                    twist = constrain(twist, -750, 750);
                }
                else
                {
                    int jitter = 1;
                    twist += jitter;
                    jitter *= -1;
                    if (m_settingsFactory->getInverseTwist())
                        twist = -constrain(twist, 500, -500);
                    else
                        twist = -constrain(twist, -500, 500);
                    // if(twist != testVar2) {
                    //     testVar2 = twist;
                    //     Serial.print("twist: ");
                    //     Serial.println(1500 + twist);
                    //     Serial.print("map(twistPos,-1500,1500,TCODE_MAX,TCODE_MIN) ");
                    //     Serial.println(map(twistPos,-1500,1500,TCODE_MAX,TCODE_MIN));
                    // Serial.print("map ");
                    // Serial.println(map(SettingsHandler::getTwistServo_ZERO() + twist,0,TwistServo_Int,0,m_servoPWMMaxDuty));
                    //}
                }
            }
            else
            {
                if (m_settingsFactory->getInverseTwist())
                    twist = map(xRot, TCODE_MIN, TCODE_MAX, -1000, 1000);
                else
                    twist = map(xRot, TCODE_MIN, TCODE_MAX, 1000, -1000);
            }
            writeServo(m_twistServoPin, map(m_settingsFactory->getTwistServo_ZERO() + twist, 0, m_twistServo_Int, 0, m_servoPWMMaxDuty));
        }
    }

    void executeValve(int xLin)
    {
        if (!valve_channel && !suck_channel)
            return;
        if (valve_channel)
            valveCmd = channelRead(valve_channel);
        if (suck_channel)
            suckCmd = channelRead(suck_channel);
        if (valveCmd > -1 || suckCmd > -1)
        {
            // Valve
            // Calculate valve position
            // Track receiver velocity
            unsigned long t = millis();
            float upVelNow;
            if (t > tLast)
            {
                upVelNow = xLin - xLast;
                upVelNow /= t - tLast;
                upVel = (upVelNow + 9 * upVel) / 10;
            }
            tLast = t;
            xLast = xLin;
            // Use suck command if most recent
            bool suck;
            if (m_tcode->getAxisLastCommandTime(suck_channel) >= m_tcode->getAxisLastCommandTime(valve_channel))
            {
                suck = true;
                valveCmd = suckCmd;
            }
            else
            {
                suck = false;
            }
            // Set valve position
            if (suck)
            {
                if (upVel < -5)
                {
                    valveCmd = 0;
                }
                else if (upVel < 0)
                {
                    valveCmd = map(100 * upVel, 0, -500, suckCmd, 0);
                }
            }
            valvePos = (9 * valvePos + map(valveCmd, TCODE_MIN, TCODE_MAX, 0, 1000)) / 10;

            int valve;
            valve = valvePos - 500;
            valve = constrain(valve, -500, 500);
            if (m_settingsFactory->getInverseValve())
            {
                valve = -valve;
            }
            if (m_settingsFactory->getValveServo90Degrees())
            {
                if (m_settingsFactory->getInverseValve())
                {
                    valve = map(valve, 0, 500, -500, 500);
                }
                else
                {
                    valve = map(valve, -500, 0, -500, 500);
                }
            }
            writeServo(m_valveServoPin, map(m_settingsFactory->getValveServo_ZERO() + valve, 0, m_valveServo_Int, 0, m_servoPWMMaxDuty));
        }
    }

    void executeVibe(int index)
    {
// These should drive PWM pins connected to vibration motors via MOSFETs or H-bridges.
#ifdef ESP_ARDUINO3
        int pwmChannel = m_vib0Pin;
#else
        int pwmChannel = m_vib0Channel;
#endif
        TCodeAxis *vibChannel = 0;
        switch (index)
        {
        case 0:
        {
#ifdef ESP_ARDUINO3
            pwmChannel = m_vib0Pin;
#else
            pwmChannel = m_vib0Channel;
#endif
            vibChannel = vibe0_channel;
            break;
        }
        case 1:
        {
#ifdef ESP_ARDUINO3
            pwmChannel = m_vib1Pin;
#else
            pwmChannel = m_vib1Channel;
#endif
            vibChannel = vibe1_channel;
            break;
        }
        case 2:
        {
#ifdef ESP_ARDUINO3
            pwmChannel = m_vib2Pin;
#else
            pwmChannel = m_vib2Channel;
#endif
            vibChannel = vibe2_channel;
            break;
        }
        case 3:
        {
#ifdef ESP_ARDUINO3
            pwmChannel = m_vib3Pin;
#else
            pwmChannel = m_vib3Channel;
#endif
            vibChannel = vibe3_channel;
            break;
        }
        }
        if (!vibChannel)
            return;
        int cmd = channelRead(vibChannel);
        if (cmd > -1)
        {
            if (cmd > 0 && cmd <= TCODE_MAX)
            {
                writeVibe8((uint8_t)pwmChannel, (uint8_t)map(cmd, 1, TCODE_MAX, 31, 255));
            }
            else
            {
                writeVibe8((uint8_t)pwmChannel, 0);
            }
            // Vibe timeout functions - shuts the vibne channels down if not commanded for a specified interval
            if (m_settingsFactory->getVibTimeoutEnabled())
                if (millis() - m_tcode->getAxisLastCommandTime(vibChannel) > m_settingsFactory->getVibTimeout())
                {
                    m_tcode->setAxisData(vibChannel, 0.0, AxisExtentionType::Time, 500);
                }
        }
    }

    void executeLube()
    {
        if (!lubeRegistered || m_vib1Pin < 0)
        {
            return;
        }
        const bool prevPressed = m_manualLubeOverride;
        switch (m_settingsFactory->getLubeButtonPinMode())
        {
        case INPUT_PULLDOWN:
            m_manualLubeOverride = digitalRead(m_lubeButtonPin) == HIGH;
            break;
        case INPUT:
            m_manualLubeOverride = digitalRead(m_lubeButtonPin) == HIGH;
            break;
        case INPUT_PULLUP:
        default:
            m_manualLubeOverride = digitalRead(m_lubeButtonPin) == LOW;
            break;
        }
        if (m_manualLubeOverride != prevPressed)
        {
            // Edge-only debug logs to keep the loop quiet.
            LogHandler::debug(Tags::Motor, "Lube button %s (pin %d)",
                m_manualLubeOverride ? "pressed" : "released", (int)m_lubeButtonPin);
        }
        if (m_manualLubeOverride)
        {
            const int amount = m_settingsFactory->getLubeAmount();
            if (m_manualLubeOverride != prevPressed || amount != m_lastLubeDuty)
            {
                LogHandler::debug(Tags::Motor, "Lube PWM (manual) -> pin %d duty %d",
                    (int)m_vib1Pin, amount);
                m_lastLubeDuty = amount;
            }
#ifdef ESP_ARDUINO3
            writeVibe8((uint8_t)m_vib1Pin, (uint8_t)amount);
        }
        else
        {
            if (prevPressed && m_lastLubeDuty != 0)
            {
                LogHandler::debug(Tags::Motor, "Lube PWM (manual off) -> pin %d duty 0", (int)m_vib1Pin);
                m_lastLubeDuty = 0;
            }
            writeVibe8((uint8_t)m_vib1Pin, 0);
#else
            ledcWrite(m_vib1Channel, amount);
        }
        else
        {
            ledcWrite(m_vib1Channel, 0);
#endif
        }
        if (!m_manualLubeOverride)
        {
            int cmd = channelRead(lube_channel);
            if (cmd > -1)
            {
                if (cmd > 0 && cmd <= TCODE_MAX)
                {
                    const int duty = map(cmd, 1, TCODE_MAX, 127, 255);
                    if (duty != m_lastLubeDuty)
                    {
                        LogHandler::debug(Tags::Motor, "Lube PWM (axis A2) -> pin %d duty %d (cmd %d)",
                            (int)m_vib1Pin, duty, cmd);
                        m_lastLubeDuty = duty;
                    }
#ifdef ESP_ARDUINO3
                    writeVibe8((uint8_t)m_vib1Pin, (uint8_t)duty);
#else
                    ledcWrite(m_vib1Channel, duty);
#endif
                }
                if (millis() - m_tcode->getAxisLastCommandTime(lube_channel) > 500)
                {
                    m_tcode->setAxisData(lube_channel, 0.0, AxisExtentionType::Time, 0);
                } // Auto cutoff
            }
        }
    }

    void executeSqueeze()
    {
        if (!squeeze_channel)
            return;
        squeezeCmd = channelRead(squeeze_channel);
        if (squeezeCmd > -1)
        {
            int squeeze = map(squeezeCmd, TCODE_MIN, TCODE_MAX, 1000, -1000);
            writeServo(m_squeezeServoPin, map(m_settingsFactory->getSqueezeServo_ZERO() + squeeze, 0, m_squeezeServo_Int, 0, m_servoPWMMaxDuty));
        }
    }
};