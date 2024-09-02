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
#include "TCode0_3.h"
#include "../../SettingsHandler.h"
#include "../../TagHandler.h"
#include "logging/LogHandler.h"

class MotorHandler0_3: public MotorHandler {
public:
    MotorHandler0_3() {}
    MotorHandler0_3(TCode0_3* tcode) : MotorHandler(), m_tcode(tcode) { }

protected:
    TCode0_3* m_tcode = 0;
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
        PinMapInfo pinMapInfo = m_settingsFactory->getPins();
        PinMap* pinMap = pinMapInfo.pinMap<PinMap*>();
        m_settingsFactory->getValue(VALVE_FREQUENCY, ValveServo_Freq);
        //TIMER_CLK_FREQ
        ValveServo_Int = 1000000/ValveServo_Freq;

        m_settingsFactory->getValue(TWIST_FREQUENCY, TwistServo_Freq);
        TwistServo_Int = 1000000/TwistServo_Freq;

        m_settingsFactory->getValue(SQUEEZE_FREQUENCY, SqueezeServo_Freq);
        SqueezeServo_Int = 1000000/SqueezeServo_Freq;
        
        m_tcode->setup(FIRMWARE_VERSION_NAME, m_settingsFactory->getTcodeVersionString());

        m_servoPWMMaxDuty = static_cast<uint32_t>(pow(2, SERVO_PWM_RES) - 1);
        // report status
        m_tcode->read("D0");
        m_tcode->read("D1");
        
        m_valveServoPin = pinMap->valve();
        if(m_valveServoPin > -1) {
            m_tcode->AxisInput("A1",VALVE_DEFAULT,'I',3000);
            m_tcode->RegisterAxis("A1", "Suck");
            m_tcode->RegisterAxis("A0", "Valve");
            #ifdef ESP_ARDUINO3
            attachPin("valve servo", m_valveServoPin, ValveServo_Freq, ValveServo_PWM);
            #else
            attachPin("valve servo", m_valveServoPin, ValveServo_Freq, ValveServo_PWM);
            #endif
        }

        m_twistServoPin = pinMap->twist();
        if(m_twistServoPin > -1) {
            m_tcode->RegisterAxis("R0", "Twist");
            #ifdef ESP_ARDUINO3
            attachPin("twist servo", m_twistServoPin, TwistServo_Freq, TwistServo_PWM);
            #else
            attachPin("twist servo", m_twistServoPin, TwistServo_Freq, TwistServo_PWM);
            #endif
        }

        m_squeezeServoPin = pinMap->squeeze();
        if(m_squeezeServoPin > -1) {
            m_tcode->RegisterAxis("A3", "Squeeze");
            #ifdef ESP_ARDUINO3
            attachPin("aux servo", m_squeezeServoPin, SqueezeServo_Freq, SqueezeServo_PWM);
            #else
            attachPin("aux servo", m_squeezeServoPin, SqueezeServo_Freq, SqueezeServo_PWM);
            #endif
        }

        bool lubeEnabled = false; 
        m_settingsFactory->getValue(LUBE_ENABLED, lubeEnabled);
        if (lubeEnabled) {
            m_lubeButtonPin = pinMap->lubeButton();
            m_vib1Pin = pinMap->vibe1();
            if(m_lubeButtonPin > -1 && m_vib1Pin > -1) {
                m_tcode->RegisterAxis("A2", "Lube");
                m_tcode->AxisInput("A2",0,' ',0);
                pinMode(m_lubeButtonPin, INPUT);
                #ifdef ESP_ARDUINO3
                attachPin("lube", m_vib1Pin, VibePWM_Freq, Vibe1_PWM, 8);
                #else
                attachPin("lube", m_vib1Pin, VibePWM_Freq, Vibe1_PWM, 8);
                #endif
                lubeRegistered = true;
            }
        }

        // Set vibration PWM pins
        m_vib0Pin = pinMap->vibe0();
        if(m_vib0Pin > -1) {
            m_tcode->RegisterAxis("V0", "Vibe1");
            #ifdef ESP_ARDUINO3
            attachPin("vib 1", m_vib0Pin, VibePWM_Freq, Vibe0_PWM, 8);
            #else
            attachPin("vib 1", m_vib0Pin, VibePWM_Freq, Vibe0_PWM, 8);
            #endif
        }
        if(!lubeRegistered) {
            m_vib1Pin = pinMap->vibe1();
            if(m_vib0Pin > -1) {
                m_tcode->RegisterAxis("V1", "Vibe2");
                #ifdef ESP_ARDUINO3
                attachPin("vib 2", m_vib1Pin, VibePWM_Freq, Vibe1_PWM, 8);
                #else
                attachPin("vib 2", m_vib1Pin, VibePWM_Freq, Vibe1_PWM, 8);
                #endif
            }
        }
        m_vib2Pin = pinMap->vibe2();
        if(m_vib2Pin > -1) {
            m_tcode->RegisterAxis("V2", "Vibe3");
            #ifdef ESP_ARDUINO3
            attachPin("vib 3", m_vib2Pin, VibePWM_Freq, Vibe2_PWM, 8);
            #else
            attachPin("vib 3", m_vib2Pin, VibePWM_Freq, Vibe2_PWM, 8);
            #endif
        }
        m_vib3Pin = pinMap->vibe3();
        if(m_vib3Pin > -1) {
            m_tcode->RegisterAxis("V3", "Vibe4");
            #ifdef ESP_ARDUINO3
            attachPin("vib 4", m_vib3Pin, VibePWM_Freq, Vibe3_PWM, 8);
            #else
            attachPin("vib 4", m_vib3Pin, VibePWM_Freq, Vibe3_PWM, 8);
            #endif
        }

        m_settingsFactory->getValue(FEEDBACK_TWIST, m_isTwistFeedBack);
        if(m_isTwistFeedBack)
        {
            m_twistFeedBackPin = pinMap->twistFeedBack();
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
        if(!lubeRegistered)
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

    int ValveServo_Int;
    int ValveServo_Freq;
    int TwistServo_Int;
    int TwistServo_Freq;
    int SqueezeServo_Int;
    int SqueezeServo_Freq;

    int xRot,squeezeCmd;
    // Velocity tracker variables, for valve
    float twistServoAngPos = 0.5;
    int twistTurns = 0;
    float twistPos;

    int lube;
    bool lubeRegistered = false;
    int valveCmd,suckCmd;
    int vibe0,vibe1,vibe2,vibe3;
    float upVel,valvePos;
    unsigned long tLast;
    int xLast;

    void executeTwist() {
        if(m_twistServoPin < 0) {
            return;
        }
        xRot = m_tcode->AxisRead("R0");
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
                twist  = (xRot - map(twistPos,-1500,1500,9999,0))/5;
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
                    //     Serial.print("map(twistPos,-1500,1500,9999,0) "); 
                    //     Serial.println(map(twistPos,-1500,1500,9999,0));
                        // Serial.print("map "); 
                        // Serial.println(map(SettingsHandler::getTwistServo_ZERO() + twist,0,TwistServo_Int,0,m_servoPWMMaxDuty));
                    //}
                }
            } 
            else 
            {
                twist  = map(xRot,0,9999,1000,-1000);
            }
            #ifdef ESP_ARDUINO3
            ledcWrite(m_twistServoPin, map(m_settingsFactory->getTwistServo_ZERO() + twist,0,TwistServo_Int,0,m_servoPWMMaxDuty));
            #else
            ledcWrite(TwistServo_PWM, map(m_settingsFactory->getTwistServo_ZERO() + twist,0,TwistServo_Int,0,m_servoPWMMaxDuty));
            #endif
        }
    }

    void executeValve(int xLin) {
        if(m_valveServoPin < 0) {
            return;
        }
        valveCmd = m_tcode->AxisRead("A0");
        suckCmd = m_tcode->AxisRead("A1");
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
            if (m_tcode->AxisLast("A1") >= m_tcode->AxisLast("A0")) {
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
            valvePos = (9*valvePos + map(valveCmd, 0, 9999, 0, 1000))/10;

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
            ledcWrite(m_valveServoPin, map(m_settingsFactory->getValveServo_ZERO() + valve,0,ValveServo_Int,0,m_servoPWMMaxDuty));
            #else
            ledcWrite(ValveServo_PWM, map(m_settingsFactory->getValveServo_ZERO() + valve,0,ValveServo_Int,0,m_servoPWMMaxDuty));
            #endif
        }
    }

    void executeVibe(int index) {
        // These should drive PWM pins connected to vibration motors via MOSFETs or H-bridges.
        String channel = "V0";
        #ifdef ESP_ARDUINO3
        int pwmChannel = m_vib0Pin;
        #else
        int pwmChannel = Vibe0_PWM;
        #endif
        switch(index) {
            case 0: {
                channel = "V0";
                #ifdef ESP_ARDUINO3
                pwmChannel = m_vib0Pin;
                #else
                pwmChannel = Vibe0_PWM;
                #endif
                break;
            }
            case 1: {
                channel = "V1";
                #ifdef ESP_ARDUINO3
                pwmChannel = m_vib1Pin;
                #else
                pwmChannel = Vibe1_PWM;
                #endif
                break;
            }
            case 2: {
                channel = "V2";
                #ifdef ESP_ARDUINO3
                pwmChannel = m_vib2Pin;
                #else
                pwmChannel = Vibe2_PWM;
                #endif
                break;
            }
            case 3: {
                channel = "V3";
                #ifdef ESP_ARDUINO3
                pwmChannel = m_vib3Pin;
                #else
                pwmChannel = Vibe3_PWM;
                #endif
                break;
            }
        }
        if(pwmChannel < 0) {
            return;
        }
        int cmd = m_tcode->AxisRead(channel);
        if(cmd > -1) {
            if (cmd > 0 && cmd <= 9999) {
                ledcWrite(pwmChannel, map(cmd,1,9999,31,255));
            } else {
                ledcWrite(pwmChannel, 0);
            }
            // Vibe timeout functions - shuts the vibne channels down if not commanded for a specified interval
            if(m_settingsFactory->getVibTimeoutEnabled())
                if (millis() - m_tcode->AxisLast(channel) > m_settingsFactory->getVibTimeout()) { m_tcode->AxisInput(channel,0,'I',500); }
        }
    }

    void executeLube() {
        if(lubeRegistered) {
            if(m_vib1Pin < 0) {
                return;
            }
            int cmd = m_tcode->AxisRead("A2"); 
            if (cmd > -1) {
                if (cmd > 0 && cmd <= 9999) {
                    #ifdef ESP_ARDUINO3
                    ledcWrite(m_vib1Pin, map(cmd,1,9999,127,255));
                    #else
                    ledcWrite(Vibe1_PWM, map(cmd,1,9999,127,255));
                    #endif
                } else if (digitalRead(m_lubeButtonPin) == HIGH) {
                #ifdef ESP_ARDUINO3
                    ledcWrite(m_vib1Pin,m_settingsFactory->getLubeAmount());
                } else { 
                    ledcWrite(m_vib1Pin,0);
                #else
                    ledcWrite(Vibe1_PWM,m_settingsFactory->getLubeAmount());
                } else { 
                    ledcWrite(Vibe1_PWM,0);
                #endif
                }
                if (millis() - m_tcode->AxisLast("A2") > 500) { m_tcode->AxisInput("A2",0,' ',0); } // Auto cutoff
            }
        }
    }

    void executeSqueeze() {
        if(m_squeezeServoPin < 0) {
            return;
        }
        squeezeCmd = m_tcode->AxisRead("A3");
        if(squeezeCmd > -1) {
            int squeeze = map(squeezeCmd,0,9999,1000,-1000);
            #ifdef ESP_ARDUINO3
            ledcWrite(m_squeezeServoPin, map(m_settingsFactory->getSqueezeServo_ZERO() + squeeze,0,SqueezeServo_Int,0,m_servoPWMMaxDuty));
            #else
            ledcWrite(SqueezeServo_PWM, map(m_settingsFactory->getSqueezeServo_ZERO() + squeeze,0,SqueezeServo_Int,0,m_servoPWMMaxDuty));
            #endif
        }
    }
};