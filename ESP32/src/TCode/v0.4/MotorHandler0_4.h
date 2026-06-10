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
#include "SettingsHandler.h"
#include "TagHandler.h"
#include "LogHandler.h"
#include "MotorHandler.h"
#include "Axis.h"
#include "TCode0_4.h"

class MotorHandler0_4: public MotorHandler {
public:
    MotorHandler0_4() {}
    MotorHandler0_4(TCode0_4* tcode) : MotorHandler(), m_tcode(tcode) { }

protected:
    TCode0_4* m_tcode = 0;
    uint32_t m_servoPWMMaxDuty;
    // Servo microseconds per radian
    // (Standard: 637 μs/rad)
    // (LW-20: 700 μs/rad)
    // 270 2/3 of 637 = 424.666666667
// const servoDegreeValue180 = 637; 
// const servoDegreeValue270 = 425; 
    int ms_per_rad;  // (μs/rad)
    int maxServoRange;

    void setupCommon(const char* ignoredChannels = "") 
    {
        if(!m_tcode)
            return;
            
        m_settingsFactory = SettingsFactory::getInstance();

        PinMap* pinMap = m_settingsFactory->getPins();

        m_tcode->setup(FIRMWARE_VERSION_NAME);
        int vibeResolution, lubeResolution;
        m_settingsFactory->getValue(SERVO_RESOLUTION, servoResolution);
        m_settingsFactory->getValue(VIBE_RESOLUTION, vibeResolution);
        m_settingsFactory->getValue(LUBE_RESOLUTION, lubeResolution);

        m_servoPWMMaxDuty = static_cast<uint32_t>(pow(2, servoResolution) - 1);
        m_settingsFactory->getValue(MAX_SERVO_RANGE, maxServoRange);
        if(!maxServoRange)
        {
            LogHandler::error(_TAG, "Invalid, max Servo range. Setting to 180...");
            maxServoRange = 180;
        }
        ms_per_rad = 114592/maxServoRange;
        LogHandler::debug(_TAG, "MS_PER_RAD: %i", ms_per_rad);
        LogHandler::debug(_TAG, "Servo Resolution: %i", servoResolution);
        LogHandler::debug(_TAG, "Vibe Resolution: %i", vibeResolution);
        LogHandler::debug(_TAG, "Lube Resolution: %i", lubeResolution);
        
        m_valveServoPin = pinMap->valve();
        m_valveServoChannel = pinMap->valveChannel();
        if(m_valveServoPin > -1 && m_valveServoChannel > -1) 
        {
            suck = new Axis(TCODE_MID);
            m_tcode->addAxis("A1", *suck);
            valve = new Axis(TCODE_MID);
            m_tcode->addAxis("A0", *valve);
            int freq = pinMap->getChannelFrequency(m_valveServoChannel);
            attachPin("valve servo", m_valveServoPin, freq, m_valveServoChannel);
            m_valveServo_Int = frequencyToMicroseconds(freq);
        } 
        else 
        {
            m_valveServoPin = -1;
        }

        bool ignoreTwist = contains(ignoredChannels, TCODE_CHANNEL_TWIST);
        if(!ignoreTwist)
        {
            m_twistServoPin = pinMap->twist();
            m_twistServoChannel = pinMap->twistChannel();
            if(m_twistServoPin > -1 && m_twistServoChannel > -1) 
            {
                twist = new Axis(TCODE_MID);
                m_tcode->addAxis(TCODE_CHANNEL_TWIST, *twist);
                int freq = pinMap->getChannelFrequency(m_twistServoChannel);
                attachPin("twist servo", m_twistServoPin, freq, m_twistServoChannel);
                m_twistServo_Int = frequencyToMicroseconds(freq);
            } 
            else 
            {
                m_twistServoPin = -1;
            }
        }

        m_squeezeServoPin = pinMap->squeeze();
        m_squeezeServoChannel = pinMap->squeezeChannel();
        if(m_squeezeServoPin > -1 && m_squeezeServoChannel > -1) 
        {
            squeeze = new Axis(TCODE_MID);
            m_tcode->addAxis("A3", *squeeze);
            int freq = pinMap->getChannelFrequency(m_squeezeServoChannel);
            attachPin("aux servo", m_squeezeServoPin, freq, m_squeezeServoChannel);
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
            if(m_lubeButtonPin > -1 && m_vib1Pin > -1 && m_vib1Channel > -1) 
            {
                lube = new Axis(TCODE_MIN);
                m_tcode->addAxis("A2", *lube);
                pinMode(m_lubeButtonPin, INPUT);
                int freq = pinMap->getChannelFrequency(m_vib1Channel);
                attachPin("lube", m_vib1Pin, freq, m_vib1Channel, lubeResolution);
                // m_vib1_Int = frequencyToMicroseconds(freq);
                lubeRegistered = true;
            }
        }

        // Set vibration PWM pins
        m_vib0Pin = pinMap->vibe0();
        m_vib0Channel = pinMap->vibe0Channel();
        if(m_vib0Pin > -1 && m_vib0Channel > -1) 
        {
            vibration0 = new Axis(TCODE_MIN);
            m_tcode->addAxis("V0", *vibration0);
            int freq = pinMap->getChannelFrequency(m_vib0Channel);
            attachPin("vib 1", m_vib0Pin, freq, m_vib0Channel, vibeResolution);
            // m_vib0_Int = frequencyToMicroseconds(freq);
        } 
        else 
        {
            m_vib0Pin = -1;
        }

        if(!lubeRegistered) 
        {
            m_vib1Pin = pinMap->vibe1();
            m_vib1Channel = pinMap->vibe1Channel();
            if(m_vib1Pin > -1 && m_vib1Channel > -1) 
            {
                vibration1 = new Axis(TCODE_MIN);
                m_tcode->addAxis("V1", *vibration1);
                int freq = pinMap->getChannelFrequency(m_vib1Channel);
                attachPin("vib 2", m_vib1Pin, freq, m_vib1Channel, vibeResolution);
                // m_vib1_Int = frequencyToMicroseconds(freq);
            } 
            else 
            {
                m_vib1Pin = -1;
            }
        }
        m_vib2Pin = pinMap->vibe2();
        m_vib2Channel = pinMap->vibe2Channel();
        if(m_vib2Pin > -1 && m_vib2Channel > -1) 
        {
            vibration2 = new Axis(TCODE_MIN);
            m_tcode->addAxis("V2", *vibration2);
            int freq = pinMap->getChannelFrequency(m_vib2Channel);
            attachPin("vib 3", m_vib2Pin, freq, m_vib2Channel, vibeResolution);
            // m_vib2_Int = frequencyToMicroseconds(freq);
        } 
        else 
        {
            m_vib2Pin = -1;
        }
        m_vib3Pin = pinMap->vibe3();
        m_vib3Channel = pinMap->vibe3Channel();
        if(m_vib3Pin > -1 && m_vib3Channel > -1) 
        {
            vibration3 = new Axis(TCODE_MIN);
            m_tcode->addAxis("V3", *vibration3);
            int freq = pinMap->getChannelFrequency(m_vib3Channel);
            attachPin("vib 4", m_vib3Pin, freq, m_vib3Channel, vibeResolution);
            // m_vib3_Int = frequencyToMicroseconds(freq);
        } 
        else 
        {
            m_vib3Pin = -1;
        }

        m_settingsFactory->getValue(FEEDBACK_TWIST, m_isTwistFeedBack);
        if(m_isTwistFeedBack)
        {
            m_twistFeedBackPin = pinMap->twistFeedBack();
            if(m_twistFeedBackPin > -1) 
            {
                // Initiate position tracking for twist
                pinMode(m_twistFeedBackPin, INPUT);
                m_settingsFactory->getValue(ANALOG_TWIST, m_isAnalogTwist);
                if(!m_isAnalogTwist) 
                {
                    LogHandler::debug(_TAG, "Attaching interrupt for twist feedback to pin: %u", pinMap->twistFeedBack());
                    attachInterrupt(m_twistFeedBackPin, twistChange, CHANGE);
                    //Serial.print("Setting digital twist "); 
                    //Serial.println(SettingsHandler::getTwistFeedBack_PIN());
                } 
                else
                {
                    //Serial.print("Setting analog twist "); 
                    //Serial.println(SettingsHandler::getTwistFeedBack_PIN());
        /*             adcAttachPin(SettingsHandler::getTwistFeedBack_PIN());
                    analogReadResolution(11);
                    analogSetAttenuation(ADC_6db); */
                }
            }
        } 
        

        read("D0", 3);
        m_tcode->getMessages();
        read("D1", 3);
        m_tcode->getMessages();
    }

    void executeCommon(Axis* stroke) 
    {
        if(!m_tcode || m_initFailed)
            return;
        executeTwist();
        executeSqueeze();
        executeValve(stroke);
        executeVibe(0);
        if(!lubeRegistered)
            executeVibe(1);
        else
            executeLube();
        executeVibe(2);
        executeVibe(3);

        m_tcode->getMessages();
    }
    
protected:
    uint16_t channelRead(const char* name, Axis* channel) 
    {
        if(!channel)
            return TCODE_MID;
        uint16_t value = channel->getPosition();
        if(SettingsHandler::getChannelRangesEnabled()) 
        {
            Channel* channel = SettingsHandler::getChannel(name);
            if(channel && channel->rangeLimitEnabled)
            {
                return map(value, TCODE_MIN, TCODE_MAX, channel->userMin, channel->userMax);
            }
        }
        return value;
    }

private:
    const char* _TAG = TagHandler::MotorHandler;
    bool m_initFailed = false;
    SettingsFactory* m_settingsFactory;
    bool m_isAnalogTwist = false;
    bool m_isTwistFeedBack = false;

    Axis* twist = 0;
    Axis* vibration0 = 0;
    Axis* vibration1 = 0;
    Axis* vibration2 = 0;
    Axis* vibration3 = 0;
    Axis* valve = 0;
    Axis* suck = 0;
    Axis* lube = 0;
    Axis* squeeze = 0;

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

    // Not used/////////
    // int m_vib0_Int = -1;
    // int m_vib1_Int = -1;
    // int m_vib2_Int = -1;
    // int m_vib3_Int = -1;
    ////////////////////

    int xRot,squeezeCmd;
    // Velocity tracker variables, for valve
    float twistServoAngPos = 0.5;
    int twistTurns = 0;
    float twistPos;

    // int lube;
    bool lubeRegistered = false;
    int valveCmd,suckCmd;
    int vibe0,vibe1,vibe2,vibe3;
    float upVel,valvePos;
    unsigned long tLast;
    //int xLast;
    int strokeVel;

    void executeTwist() 
    {
        if(m_twistServoPin < 0) 
        {
            return;
        }
        xRot = channelRead(TCODE_CHANNEL_TWIST, twist);
        if(xRot > -1) 
        {
            if (m_isTwistFeedBack && !m_settingsFactory->getContinuousTwist()) 
            {
                float angPos;
                // Calculate twist position
                if (!m_isAnalogTwist)
                {  
                    //noInterrupts();
                    float dutyCycle = twistPulseLength;
                    dutyCycle = dutyCycle/lastTwistPulseCycle;
                    //interrupts();
                    angPos = (dutyCycle - 0.029)/0.942;
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
                angPos = constrain(angPos,0,1) - 0.5;
                if (angPos - twistServoAngPos < - 0.8) { twistTurns += 1; }
                if (angPos - twistServoAngPos > 0.8) { twistTurns -= 1; }
                twistServoAngPos = angPos;
                twistPos = 1000*(angPos + twistTurns);
            }

            // Twist
            int twist;
            if (m_isTwistFeedBack && !m_settingsFactory->getContinuousTwist()) 
            {
                twist  = (xRot - map(twistPos,-1500,1500, TCODE_MAX, TCODE_MIN))/5;
                if(!m_isAnalogTwist) 
                { 
                    twist  = constrain(twist, -750, 750);
                }
                else 
                {
                    int jitter = 1;
                    twist += jitter;
                    jitter *= -1;
                    if(m_settingsFactory->getInverseTwist())
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
                if(m_settingsFactory->getInverseTwist())
                    twist = map(xRot, TCODE_MIN, TCODE_MAX,-1000,1000);
                else
                    twist = map(xRot, TCODE_MIN, TCODE_MAX,1000,-1000);
            }
            #ifdef ESP_ARDUINO3
            ledcWrite(m_twistServoPin, map(m_settingsFactory->getTwistServo_ZERO() + twist,0,m_twistServo_Int,0,m_servoPWMMaxDuty));
            #else
            ledcWrite(m_twistServoChannel, map(m_settingsFactory->getTwistServo_ZERO() + twist,0,m_twistServo_Int,0,m_servoPWMMaxDuty));
            #endif
        }
    }

    void executeValve(Axis* stroke) {
        if(m_valveServoPin < 0) 
        {
            return;
        }
        valveCmd = channelRead("A0", valve);
        suckCmd = channelRead("A1", suck);
        // Use suck command if most recent
        bool suckMode;
        if (suck->getLast() >= valve->getLast())
        {
            suckMode = true;
            valveCmd = suckCmd;
        } 
        else 
        {
            suckMode = false;
        }
        // Set valve position
        if (suckMode) 
        {
            // Get receiver velocity
            strokeVel = stroke->getVelocity();
            if (strokeVel < -5) 
            {
                valveCmd = 0;  
            } else if ( strokeVel < 0 ) 
            {
                valveCmd = map(100*strokeVel,0,-500,suckCmd,0);
            if (valveCmd > 9999) 
                valveCmd = 9999; 
            if (valveCmd < 0) 
                valveCmd = 0;
            }
        }
        valvePos = (9*valvePos + map(valveCmd, TCODE_MIN, TCODE_MAX, 0, 1000))/10;

        int valve;
        valve  = valvePos - 500;
        valve  = constrain(valve, -500, 500);
        if (m_settingsFactory->getInverseValve()) 
        { 
            valve = -valve; 
        }
        if(m_settingsFactory->getValveServo90Degrees())
        {
            if (m_settingsFactory->getInverseValve()) 
            { 
                valve = map(valve,0,500,-500,500);
            } 
            else
            {
                valve = map(valve,-500,0,-500,500);
            }
        }
        #ifdef ESP_ARDUINO3
        ledcWrite(m_valveServoPin, map(m_settingsFactory->getValveServo_ZERO() + valve,0,m_valveServo_Int,0,m_servoPWMMaxDuty));
        #else
        ledcWrite(m_valveServoChannel, map(m_settingsFactory->getValveServo_ZERO() + valve,0,m_valveServo_Int,0,m_servoPWMMaxDuty));
        #endif
    }

    void executeVibe(int index) {
        // These should drive PWM pins connected to vibration motors via MOSFETs or H-bridges.
        const char* channel = "V0";
        Axis* axis = vibration0;
        #ifdef ESP_ARDUINO3
        int pwmChannel = m_vib0Pin;
        #else
        int pwmChannel = m_vib0Channel;
        #endif
        switch(index) 
        {
            case 0: 
            {
                channel = "V0";
                #ifdef ESP_ARDUINO3
                pwmChannel = m_vib0Pin;
                #else
                pwmChannel = m_vib0Channel;
                #endif
                break;
            }
            case 1: 
            {
                channel = "V1";
                axis = vibration1;
                #ifdef ESP_ARDUINO3
                pwmChannel = m_vib1Pin;
                #else
                pwmChannel = m_vib1Channel;
                #endif
                break;
            }
            case 2: 
            {
                channel = "V2";
                axis = vibration2;
                #ifdef ESP_ARDUINO3
                pwmChannel = m_vib2Pin;
                #else
                pwmChannel = m_vib2Channel;
                #endif
                break;
            }
            case 3: 
            {
                channel = "V3";
                axis = vibration3;
                #ifdef ESP_ARDUINO3
                pwmChannel = m_vib3Pin;
                #else
                pwmChannel = m_vib3Channel;
                #endif
                break;
            }
        }
        if(pwmChannel < 0) 
        {
            return;
        }
        int cmd = channelRead(channel, axis);
        if(cmd > -1) 
        {
            if (cmd > 0 && cmd <= TCODE_MAX) 
            {
                ledcWrite(pwmChannel, map(cmd,1,TCODE_MAX,31,255));
            } 
            else 
            {
                ledcWrite(pwmChannel, 0);
            }
            // Vibe timeout functions - shuts the vibe channels down if not commanded for a specified interval
            if(m_settingsFactory->getVibTimeoutEnabled())
            {
                if (millis() - axis->getLast() > m_settingsFactory->getVibTimeout()) 
                { 
                    axis->prepAxis(0,InputType::INTERVAL,500);
                    axis->setAxis();
                }
            }
        }
    }

    void executeLube() 
    {
        if(!lubeRegistered || m_vib1Pin < 0) 
        {
            return;
        }
        m_manualLubeOverride = digitalRead(m_lubeButtonPin) == HIGH;
        if (m_manualLubeOverride) 
        {
#ifdef ESP_ARDUINO3
            ledcWrite(m_vib1Pin,m_settingsFactory->getLubeAmount());
        } 
        else 
        { 
            ledcWrite(m_vib1Pin,0);
#else
            ledcWrite(m_vib1Channel,m_settingsFactory->getLubeAmount());
        } 
        else 
        { 
            ledcWrite(m_vib1Channel,0);
#endif
        }
        if(!m_manualLubeOverride)
        {
            int cmd = channelRead("A2", lube); 
            if (cmd > -1) 
            {
                if (cmd > 0 && cmd <= TCODE_MAX) 
                {
#ifdef ESP_ARDUINO3
                    ledcWrite(m_vib1Pin, map(cmd,1,TCODE_MAX,127,255));
#else
                    ledcWrite(m_vib1Channel, map(cmd,1,TCODE_MAX,127,255));
#endif
                } 
                if (millis() - lube->getLast() > 500) 
                { 
                    // Auto cutoff
                    lube->prepAxis(0,InputType::INTERVAL,100);
                    lube->setAxis();
                } 
            }
        }
    }

    void executeSqueeze() {
        if(m_squeezeServoPin < 0) 
        {
            return;
        }
        squeezeCmd = channelRead("A3", squeeze);
        if(squeezeCmd > -1) 
        {
            int squeeze = map(squeezeCmd,TCODE_MIN,TCODE_MAX,1000,-1000);
#ifdef ESP_ARDUINO3
            ledcWrite(m_squeezeServoPin, map(m_settingsFactory->getSqueezeServo_ZERO() + squeeze,0,m_squeezeServo_Int,0,m_servoPWMMaxDuty));
#else
            ledcWrite(m_squeezeServoChannel, map(m_settingsFactory->getSqueezeServo_ZERO() + squeeze,0,m_squeezeServo_Int,0,m_servoPWMMaxDuty));
#endif
        }
    }
};