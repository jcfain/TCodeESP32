/* MIT License

Copyright (c) 2024 Jason C. Fain

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
#include "SettingsHandler.h"
#include "TagHandler.h"

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
    int ms_per_rad;  // (μs/rad)

    void setupCommon() {
        if(!m_tcode)
            return;
            
        m_settingsFactory = SettingsFactory::getInstance();
        PinMap* pinMap = m_settingsFactory->getPins();

        m_servoPWMMaxDuty = static_cast<uint32_t>(pow(2, SERVO_PWM_RES) - 1);
        
        m_tcode->setup(FIRMWARE_VERSION_NAME);
        
        m_valveServoPin = pinMap->valve();
        m_valveServoChannel = pinMap->valveChannel();
        if(m_valveServoPin > -1 && m_valveServoChannel > -1) {
            valve_axis = new TCodeAxis("Valve", {AxisType::Auxiliary, 0}, 0.0f);
            m_tcode->RegisterAxis(valve_axis);
            m_tcode->setAxisData(valve_axis, 0.5, AxisExtentionType::Time, 3000);
            suck_axis = new TCodeAxis("Valve", {AxisType::Auxiliary, 1}, 0.0f);
            m_tcode->RegisterAxis(suck_axis);
            int freq = pinMap->getChannelFrequency(m_valveServoChannel);
            attachPin("valve servo", m_valveServoPin, freq, m_valveServoChannel);
            m_valveServo_Int = frequencyToMicroseconds(freq);
        }

        m_twistServoPin = pinMap->twist();
        m_twistServoChannel = pinMap->twistChannel();
        if(m_twistServoPin > -1 && m_twistServoChannel > -1) {
            twist_axis = new TCodeAxis("Twist", {AxisType::Rotation, 0}, 0.5f);
            m_tcode->RegisterAxis(twist_axis);
            int freq = pinMap->getChannelFrequency(m_twistServoChannel);
            attachPin("twist servo", m_twistServoPin, freq, m_twistServoChannel);
            m_twistServo_Int = frequencyToMicroseconds(freq);
        }

        m_squeezeServoPin = pinMap->squeeze();
        m_squeezeServoChannel = pinMap->squeezeChannel();
        if(m_squeezeServoPin > -1 && m_squeezeServoChannel > -1) {
            squeeze_axis = new TCodeAxis("Squeeze", {AxisType::Auxiliary, 3}, 0.5f);
            m_tcode->RegisterAxis(squeeze_axis);
            int freq = pinMap->getChannelFrequency(m_squeezeServoChannel);
            attachPin("aux servo", m_squeezeServoPin, freq, m_squeezeServoChannel);
            m_squeezeServo_Int = frequencyToMicroseconds(freq);
        }

        bool lubeEnabled = false; 
        m_settingsFactory->getValue(LUBE_ENABLED, lubeEnabled);
        if (lubeEnabled) {
            m_lubeButtonPin = pinMap->lubeButton();
            m_vib1Pin = pinMap->vibe1();
            m_vib1Channel = pinMap->vibe1Channel();
            if(m_lubeButtonPin > -1 && m_vib1Pin > -1 && m_vib1Channel > -1) {
                lube_axis = new TCodeAxis("Lube", {AxisType::Auxiliary, 2}, 0.0f);
                m_tcode->RegisterAxis(lube_axis);
                //m_tcode->AxisInput("A2",0,' ',0);
                m_tcode->setAxisData(lube_axis, 0, AxisExtentionType::Time, 0);
                pinMode(m_lubeButtonPin, INPUT);
                int freq = pinMap->getChannelFrequency(m_vib1Channel);
                attachPin("lube", m_vib1Pin, freq, m_vib1Channel, 8);
            }
        }

        // Set vibration PWM pins
        m_vib0Pin = pinMap->vibe0();
        m_vib0Channel = pinMap->vibe0Channel();
        if(m_vib0Pin > -1 && m_vib0Channel > -1) {
            vibe0_axis = new TCodeAxis("Vibe 1", {AxisType::Vibration, 0}, 0.0f);
            m_tcode->RegisterAxis(vibe0_axis);
            int freq = pinMap->getChannelFrequency(m_vib0Channel);
            attachPin("vib 1", m_vib0Pin, freq, m_vib0Channel, 8);
        }
        if(!lube_axis) {
            m_vib1Pin = pinMap->vibe1();
            m_vib1Channel = pinMap->vibe1Channel();
            if(m_vib1Pin > -1 && m_vib1Channel > -1) {
                vibe1_axis = new TCodeAxis("Vibe 2", {AxisType::Vibration, 1}, 0.0f);
                m_tcode->RegisterAxis(vibe1_axis);
                int freq = pinMap->getChannelFrequency(m_vib1Channel);
                attachPin("vib 2", m_vib1Pin, freq, m_vib1Channel, 8);
            }
        }
        m_vib2Pin = pinMap->vibe2();
        m_vib2Channel = pinMap->vibe2Channel();
        if(m_vib2Pin > -1 && m_vib2Channel > -1) {
            vibe2_axis = new TCodeAxis("Vibe 3", {AxisType::Vibration, 2}, 0.0f);
            m_tcode->RegisterAxis(vibe2_axis);
            int freq = pinMap->getChannelFrequency(m_vib2Channel);
            attachPin("vib 3", m_vib2Pin, freq, m_vib2Channel, 8);
        }
        m_vib3Pin = pinMap->vibe3();
        m_vib3Channel = pinMap->vibe3Channel();
        if(m_vib3Pin > -1 && m_vib3Channel > -1) {
            vibe3_axis = new TCodeAxis("Vibe 4", {AxisType::Vibration, 3}, 0.0f);
            m_tcode->RegisterAxis(vibe3_axis);
            int freq = pinMap->getChannelFrequency(m_vib3Channel);
            attachPin("vib 4", m_vib3Pin, freq, m_vib3Channel, 8);
        }

        m_settingsFactory->getValue(FEEDBACK_TWIST, m_isTwistFeedBack);
        m_twistFeedBackPin = pinMap->twistFeedBack();
        if(m_isTwistFeedBack)
        {
            if(m_twistFeedBackPin > -1) {
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
        // report status
        m_tcode->read("D0\n");
        m_tcode->read("D1\n");
    }

    void executeCommon(const int xLin) {
        if(!m_tcode || m_initFailed)
            return;
        executeTwist();
        executeSqueeze();
        executeValve(xLin);
        executeVibe(0);
        if(!lube_axis)
            executeVibe(1);
        else
            executeLube();
        executeVibe(2);
        executeVibe(3);

    }
    
private:
    const char* _TAG = TagHandler::MotorHandler;
    bool m_initFailed = false;
    SettingsFactory* m_settingsFactory;
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
    
	TCodeAxis* twist_axis = 0;
	TCodeAxis* squeeze_axis = 0;
	TCodeAxis* vibe0_axis = 0;
	TCodeAxis* vibe1_axis = 0;
	TCodeAxis* vibe2_axis = 0;
	TCodeAxis* vibe3_axis = 0;
	TCodeAxis* valve_axis = 0;
	TCodeAxis* suck_axis = 0;
	TCodeAxis* lube_axis = 0;

    int xRot,squeezeCmd;
    // Velocity tracker variables, for valve
    float twistServoAngPos = 0.5;
    int twistTurns = 0;
    float twistPos;

    int lube;
    int valveCmd,suckCmd;
    int vibe0,vibe1,vibe2,vibe3;
    float upVel,valvePos;
    unsigned long tLast;
    int xLast;

    void executeTwist() {
        if(!twist_axis)
            return;
        xRot = m_tcode->getAxisPosition(twist_axis);
        if(xRot > -1) {
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
                twist  = (xRot - map(twistPos,-1500,1500,10000,0))/5;
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
                    //     Serial.print("map(twistPos,-1500,1500,10000,0) "); 
                    //     Serial.println(map(twistPos,-1500,1500,10000,0));
                        // Serial.print("map "); 
                        // Serial.println(map(SettingsHandler::getTwistServo_ZERO() + twist,0,TwistServo_Int,0,65535));
                    //}
                }
            } 
            else 
            {
                if(m_settingsFactory->getInverseTwist())
                    twist = map(xRot,0,10000,-1000,1000);
                else
                    twist = map(xRot,0,10000,1000,-1000);
            }
            #ifdef ESP_ARDUINO3
            ledcWrite(m_twistServoPin, map(m_settingsFactory->getTwistServo_ZERO() + twist,0,m_twistServo_Int,0,m_servoPWMMaxDuty));
            #else
            ledcWrite(m_twistServoChannel, map(m_settingsFactory->getTwistServo_ZERO() + twist,0,m_twistServo_Int,0,m_servoPWMMaxDuty));
            #endif
        }
    }

    void executeValve(int xLin) {
        if(!valve_axis && !suck_axis)
            return;
        if(valve_axis)
            valveCmd = m_tcode->getAxisPosition(valve_axis);
        if(suck_axis)
            suckCmd = m_tcode->getAxisPosition(suck_axis);
        if(valveCmd > -1 || suckCmd > -1) {
            // Valve
            // Calculate valve position
            // Track receiver velocity
            unsigned long t = millis();
            float upVelNow;
            if (t > tLast) {
                upVelNow = xLin - xLast;
                upVelNow /= t - tLast;
                upVel = (upVelNow + 9*upVel)/10;
            }
            tLast = t;
            xLast = xLin;
            // Use suck command if most recent
            bool suck;
            if (m_tcode->getAxisLastCommandTime(suck_axis) >= m_tcode->getAxisLastCommandTime(valve_axis)) {
                suck = true;
                valveCmd = suckCmd;
            } else {
                suck = false;
            }
            // Set valve position
            if (suck) {
                if (upVel < -5) {
                    valveCmd = 0;  
                } else if ( upVel < 0 ) {
                    valveCmd = map(100*upVel, 0, -500, suckCmd, 0);
                }
            }
            valvePos = (9*valvePos + map(valveCmd, 0, 10000, 0, 1000))/10;

            int valve;
            valve  = valvePos - 500;
            valve  = constrain(valve, -500, 500);
            if (m_settingsFactory->getInverseValve()) { valve = -valve; }
            if(m_settingsFactory->getValveServo90Degrees())
            {
                if (m_settingsFactory->getInverseValve()) { 
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
    }

    void executeVibe(int index) {
        // These should drive PWM pins connected to vibration motors via MOSFETs or H-bridges.
        #ifdef ESP_ARDUINO3
        int pwmChannel = m_vib0Pin;
        #else
        int pwmChannel = m_vib0Channel;
        #endif
        TCodeAxis* vibChannel = 0;
        switch(index) {
            case 0: {
                #ifdef ESP_ARDUINO3
                pwmChannel = m_vib0Pin;
                #else
                pwmChannel = m_vib0Channel;
                #endif
                vibChannel = vibe0_axis;
                break;
            }
            case 1: {
                #ifdef ESP_ARDUINO3
                pwmChannel = m_vib1Pin;
                #else
                pwmChannel = m_vib1Channel;
                #endif
                vibChannel = vibe1_axis;
                break;
            }
            case 2: {
                #ifdef ESP_ARDUINO3
                pwmChannel = m_vib2Pin;
                #else
                pwmChannel = m_vib2Channel;
                #endif
                vibChannel = vibe2_axis;
                break;
            }
            case 3: {
                #ifdef ESP_ARDUINO3
                pwmChannel = m_vib3Pin;
                #else
                pwmChannel = m_vib3Channel;
                #endif
                vibChannel = vibe3_axis;
                break;
            }
        }
        if(!vibChannel)
            return;
        int cmd = m_tcode->getAxisPosition(vibChannel);
        if(cmd > -1) {
            if (cmd > 0 && cmd <= 10000) {
                ledcWrite(pwmChannel, map(cmd,1,10000,31,255));
            } else {
                ledcWrite(pwmChannel, 0);
            }
            // Vibe timeout functions - shuts the vibne channels down if not commanded for a specified interval
            if(m_settingsFactory->getVibTimeoutEnabled())
                if (millis() - m_tcode->getAxisLastCommandTime(vibChannel) > m_settingsFactory->getVibTimeout()) { m_tcode->setAxisData(vibChannel, 0.0, AxisExtentionType::Time, 500); }
        }
    }

    void executeLube() {
        if(!lube_axis)
            return;
        int cmd = m_tcode->getAxisPosition(lube_axis); 
        if (cmd > -1) {
            if (cmd > 0 && cmd <= 10000) {
                #ifdef ESP_ARDUINO3
                ledcWrite(m_vib1Pin, map(cmd,1,10000,127,255));
                #else
                ledcWrite(m_vib1Channel, map(cmd,1,10000,127,255));
                #endif
            } else if (digitalRead(m_lubeButtonPin) == HIGH) {
            #ifdef ESP_ARDUINO3
                ledcWrite(m_vib1Pin,m_settingsFactory->getLubeAmount());
            } else { 
                ledcWrite(m_vib1Pin,0);
            #else
                ledcWrite(m_vib1Channel,m_settingsFactory->getLubeAmount());
            } else { 
                ledcWrite(m_vib1Channel,0);
            #endif
            }
            if (millis() - m_tcode->getAxisLastCommandTime(lube_axis) > 500) { m_tcode->setAxisData(lube_axis, 0.0, AxisExtentionType::Time, 0); } // Auto cutoff
        }
    }

    void executeSqueeze() {
        if(!squeeze_axis)
            return;
        squeezeCmd = m_tcode->getAxisPosition(squeeze_axis);
        if(squeezeCmd > -1) {
            int squeeze = map(squeezeCmd,0,10000,1000,-1000);
            #ifdef ESP_ARDUINO3
            ledcWrite(m_squeezeServoPin, map(m_settingsFactory->getSqueezeServo_ZERO() + squeeze,0,m_squeezeServo_Int,0,m_servoPWMMaxDuty));
            #else
            ledcWrite(m_squeezeServoChannel, map(m_settingsFactory->getSqueezeServo_ZERO() + squeeze,0,m_squeezeServo_Int,0,m_servoPWMMaxDuty));
            #endif
        }
    }
};