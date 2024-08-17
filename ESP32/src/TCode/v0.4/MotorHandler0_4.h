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
#include "../Global.h"
#include "../MotorHandler.h"
#include "TCodeBaseV4.h"
#include "../../SettingsHandler.h"
#include "../../TagHandler.h"

class MotorHandler0_4: public MotorHandler {
public:
    MotorHandler0_4() {}
    MotorHandler0_4(TCode0_4* tcode) : MotorHandler(), m_tcode(tcode) { }

protected:
    TCode0_4* m_tcode = 0;
    // Servo microseconds per radian
    // (Standard: 637 μs/rad)
    // (LW-20: 700 μs/rad)
    // 270 2/3 of 637 = 424.666666667
    int ms_per_rad;  // (μs/rad)

    void setupCommon() {
        if(!m_tcode)
            return;
            
        m_settingsFactory = SettingsFactory::getInstance();
        PinMapInfo pinMapInfo = m_settingsFactory->getPins();
        PinMap* pinMap = pinMapInfo.pinMap<PinMap*>();
        m_settingsFactory->getValue(VALVE_FREQUENCY, ValveServo_Freq);
        ValveServo_Int = 1000000/ValveServo_Freq;

        m_settingsFactory->getValue(TWIST_FREQUENCY, TwistServo_Freq);
        TwistServo_Int = 1000000/TwistServo_Freq;

        m_settingsFactory->getValue(SQUEEZE_FREQUENCY, SqueezeServo_Freq);
        SqueezeServo_Int = 1000000/SqueezeServo_Freq;
        
        m_tcode->setup(FIRMWARE_VERSION_NAME, m_settingsFactory->getTcodeVersionString());
        // report status
        m_tcode->read("D0");
        m_tcode->read("D1");
        
        if(pinMap->valve() > -1) {
            valve_axis = new TCodeAxis("Valve", {AxisType::Auxiliary, 0}, 0.0f);
            m_tcode->RegisterAxis(valve_axis);
            m_tcode->setAxisData(valve_axis, 0.5, AxisExtentionType::Time, 3000);
            suck_axis = new TCodeAxis("Valve", {AxisType::Auxiliary, 1}, 0.0f);
            m_tcode->RegisterAxis(suck_axis);
            LogHandler::debug(_TAG, "Connecting valve servo to pin: %ld @ freq: %ld", pinMap->valve(), ValveServo_Freq);
            #ifdef ESP_ARDUINO3
            ledcAttach(pinMap->valve(), ValveServo_Freq, 16);
            #else
            ledcSetup(ValveServo_PWM,ValveServo_Freq,16);
            ledcAttachPin(pinMap->valve(),ValveServo_PWM);
            #endif
        }

        if(pinMap->twist() > -1) {
            twist_axis = new TCodeAxis("Twist", {AxisType::Rotation, 0}, 0.5f);
            m_tcode->RegisterAxis(twist_axis);
            LogHandler::debug(_TAG, "Connecting twist servo to pin: %ld @ freq: %ld", pinMap->twist(), TwistServo_Freq);
            #ifdef ESP_ARDUINO3
            ledcAttach(pinMap->twist(), TwistServo_Freq, 16);
            #else
            ledcSetup(TwistServo_PWM,TwistServo_Freq,16);
            ledcAttachPin(pinMap->twist(),TwistServo_PWM);
            #endif
        }

        if(pinMap->squeeze() > -1) {
            squeeze_axis = new TCodeAxis("Squeeze", {AxisType::Auxiliary, 3}, 0.5f);
            m_tcode->RegisterAxis(squeeze_axis);
            LogHandler::debug(_TAG, "Connecting squeeze servo to pin: %ld @ freq: %ld", pinMap->squeeze(), SqueezeServo_Freq);
            #ifdef ESP_ARDUINO3
            ledcAttach(pinMap->squeeze(), SqueezeServo_Freq, 16);
            #else
            ledcSetup(SqueezeServo_PWM,SqueezeServo_Freq,16);
            ledcAttachPin(pinMap->squeeze(),SqueezeServo_PWM);
            #endif
        }

        bool lubeEnabled = false; 
        m_settingsFactory->getValue(LUBE_ENABLED, lubeEnabled);
        m_lubeButtonPin = pinMap->lubeButton();
        if (lubeEnabled && m_lubeButtonPin > -1 && pinMap->vibe1() > -1) {
            lube_axis = new TCodeAxis("Lube", {AxisType::Auxiliary, 2}, 0.0f);
            m_tcode->RegisterAxis(lube_axis);
            //m_tcode->AxisInput("A2",0,' ',0);
            m_tcode->setAxisData(lube_axis, 0, AxisExtentionType::Time, 0);
            pinMode(m_lubeButtonPin, INPUT);
            #ifdef ESP_ARDUINO3
            ledcAttach(pinMap->vibe1() ,VibePWM_Freq,8);
            #else
            ledcSetup(Vibe1_PWM, VibePWM_Freq,8);
            ledcAttachPin(pinMap->vibe1() ,Vibe1_PWM); 
            #endif
        }

        // Set vibration PWM pins
        if(pinMap->vibe0() > -1) {
            vibe0_axis = new TCodeAxis("Vibe 1", {AxisType::Vibration, 0}, 0.0f);
            m_tcode->RegisterAxis(vibe0_axis);
            LogHandler::debug(_TAG, "Connecting vib 1 to pin: %ld @ freq: %ld", pinMap->vibe0(), VibePWM_Freq);
            #ifdef ESP_ARDUINO3
            ledcAttach(pinMap->vibe0(),VibePWM_Freq,8);
            #else
            ledcSetup(Vibe0_PWM,VibePWM_Freq,8);
            ledcAttachPin(pinMap->vibe0(),Vibe0_PWM);
            #endif
        }
        if(!lube_axis && pinMap->vibe1() > -1) {
            vibe1_axis = new TCodeAxis("Vibe 2", {AxisType::Vibration, 1}, 0.0f);
            m_tcode->RegisterAxis(vibe1_axis);
            LogHandler::debug(_TAG, "Connecting lube/vib 2 to pin: %ld @ freq: %ld", pinMap->vibe1(), VibePWM_Freq);
            #ifdef ESP_ARDUINO3
            ledcAttach(pinMap->vibe1(),VibePWM_Freq,8);
            #else
            ledcSetup(Vibe1_PWM,VibePWM_Freq,8);
            ledcAttachPin(pinMap->vibe1(),Vibe1_PWM); 
            #endif
        }
        if(pinMap->vibe2() > -1) {
            vibe2_axis = new TCodeAxis("Vibe 3", {AxisType::Vibration, 2}, 0.0f);
            m_tcode->RegisterAxis(vibe2_axis);
            LogHandler::debug(_TAG, "Connecting vib 3 to pin: %ld @ freq: %ld", pinMap->vibe2(), VibePWM_Freq);
            #ifdef ESP_ARDUINO3
            ledcAttach(pinMap->vibe2(),VibePWM_Freq,8);
            #else
            ledcSetup(Vibe2_PWM,VibePWM_Freq,8);
            ledcAttachPin(pinMap->vibe2() ,Vibe2_PWM); 
            #endif
        }
        if(pinMap->vibe3() > -1) {
            vibe3_axis = new TCodeAxis("Vibe 4", {AxisType::Vibration, 3}, 0.0f);
            m_tcode->RegisterAxis(vibe3_axis);
            LogHandler::debug(_TAG, "Connecting vib 4 to pin: %ld @ freq: %ld", pinMap->vibe3(), VibePWM_Freq);
            #ifdef ESP_ARDUINO3
            ledcAttach(pinMap->vibe3() ,VibePWM_Freq,8);
            #else
            ledcSetup(Vibe3_PWM,VibePWM_Freq,8);
            ledcAttachPin(pinMap->vibe3() ,Vibe3_PWM); 
            #endif
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

    int ValveServo_Int;
    int ValveServo_Freq;
    int TwistServo_Int;
    int TwistServo_Freq;
    int SqueezeServo_Int;
    int SqueezeServo_Freq;
    
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
                twist  = map(xRot,0,10000,1000,-1000);
            }
            ledcWrite(TwistServo_PWM, map(m_settingsFactory->getTwistServo_ZERO() + twist,0,TwistServo_Int,0,65535));
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
            ledcWrite(ValveServo_PWM, map(m_settingsFactory->getValveServo_ZERO() + valve,0,ValveServo_Int,0,65535));
        }
    }

    void executeVibe(int index) {
        // These should drive PWM pins connected to vibration motors via MOSFETs or H-bridges.
        int pwmChannel = Vibe0_PWM;
        TCodeAxis* vibChannel = 0;
        switch(index) {
            case 0: {
                pwmChannel = Vibe0_PWM;
                vibChannel = vibe0_axis;
                break;
            }
            case 1: {
                pwmChannel = Vibe1_PWM;
                vibChannel = vibe1_axis;
                break;
            }
            case 2: {
                pwmChannel = Vibe2_PWM;
                vibChannel = vibe2_axis;
                break;
            }
            case 3: {
                pwmChannel = Vibe3_PWM;
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
                ledcWrite(Vibe1_PWM, map(cmd,1,10000,127,255));
            } else if (digitalRead(m_lubeButtonPin) == HIGH) {
                ledcWrite(Vibe1_PWM,m_settingsFactory->getLubeAmount());
            } else { 
                ledcWrite(Vibe1_PWM,0);
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
            ledcWrite(SqueezeServo_PWM, map(m_settingsFactory->getSqueezeServo_ZERO() + squeeze,0,SqueezeServo_Int,0,65535));
        }
    }
};