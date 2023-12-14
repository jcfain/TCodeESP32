/* MIT License

Copyright (c) 2023 Jason C. Fain

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
#include "TCodeBase.h"
#include "../SettingsHandler.h"
#include "../TagHandler.h"

class MotorHandler {
public:
    MotorHandler() {}
    MotorHandler(TCodeBase* tcode) {
        m_tcode = tcode;
    }
    virtual void setup() = 0;
    virtual void read(byte inByte) = 0;
    virtual void read(String inString) = 0;
    virtual void execute() = 0;
    virtual void setMessageCallback(TCODE_FUNCTION_PTR_T function) = 0;

protected:
    // Servo microseconds per radian
    // (Standard: 637 μs/rad)
    // (LW-20: 700 μs/rad)
    // 270 2/3 of 637 = 424.666666667
    int ms_per_rad;  // (μs/rad)

    TCodeBase* m_tcode = 0;

    void setupCommon() {
        if(!m_tcode)
            return;
            
        ValveServo_Freq = SettingsHandler::valveFrequency;
        ValveServo_Int = 1000000/ValveServo_Freq;

        TwistServo_Freq = SettingsHandler::twistFrequency;
        TwistServo_Int = 1000000/TwistServo_Freq;

        SqueezeServo_Freq = SettingsHandler::squeezeFrequency;
        SqueezeServo_Int = 1000000/SqueezeServo_Freq;
        
        m_tcode->setup(SettingsHandler::ESP32Version, SettingsHandler::TCodeVersionName.c_str());
        // report status
        m_tcode->StringInput("D0");
        m_tcode->StringInput("D1");
        
        m_tcode->RegisterAxis("V0", "Vibe1");
        if (!SettingsHandler::lubeEnabled) { m_tcode->RegisterAxis("V1", "Vibe2"); }
        // m_tcode->RegisterAxis("V2", "Vibe3");
        // m_tcode->RegisterAxis("V3", "Vibe4");
        m_tcode->RegisterAxis("A0", "Valve");
        m_tcode->RegisterAxis("A1", "Suck");
        m_tcode->AxisInput("A1",VALVE_DEFAULT,'I',3000);
        if (SettingsHandler::lubeEnabled) {
            m_tcode->RegisterAxis("A2", "Lube");
            m_tcode->AxisInput("A2",0,' ',0);
            pinMode(SettingsHandler::LubeButton_PIN,INPUT);
        }
        m_tcode->RegisterAxis("A3", "Squeeze");

        // Valve Servo
        LogHandler::debug(_TAG, "Connecting valve servo to pin: %ld @ freq: %ld", SettingsHandler::ValveServo_PIN, SettingsHandler::valveFrequency);
        ledcSetup(ValveServo_PWM,ValveServo_Freq,16);
        ledcAttachPin(SettingsHandler::ValveServo_PIN,ValveServo_PWM);

         // Twist Servo
        LogHandler::verbose(_TAG, "Connecting twist servo to pin: %ld @ freq: %ld", SettingsHandler::TwistServo_PIN, SettingsHandler::twistFrequency);
        ledcSetup(TwistServo_PWM,TwistServo_Freq,16);
        ledcAttachPin(SettingsHandler::TwistServo_PIN,TwistServo_PWM);

        //Squeeze servo
        LogHandler::verbose(_TAG, "Connecting squeeze servo to pin: %ld @ freq: %ld", SettingsHandler::Squeeze_PIN, SettingsHandler::squeezeFrequency);
        ledcSetup(SqueezeServo_PWM,SqueezeServo_Freq,16);
        ledcAttachPin(SettingsHandler::Squeeze_PIN,SqueezeServo_PWM);

        // // Set vibration PWM pins
        // Vibe0 Pin
        LogHandler::debug(_TAG, "Connecting vib 1 to pin: %ld @ freq: %ld", SettingsHandler::Vibe0_PIN, VibePWM_Freq);
        ledcSetup(Vibe0_PWM,VibePWM_Freq,8);
        ledcAttachPin(SettingsHandler::Vibe0_PIN,Vibe0_PWM);
        // Vibe1 Pin
        LogHandler::debug(_TAG, "Connecting lube/vib 2 to pin: %ld @ freq: %ld", SettingsHandler::Vibe1_PIN, VibePWM_Freq);
        ledcSetup(Vibe1_PWM,VibePWM_Freq,8);
        ledcAttachPin(SettingsHandler::Vibe1_PIN,Vibe1_PWM); 

        if(SettingsHandler::Vibe2_PIN > 0) {
            LogHandler::debug(_TAG, "Connecting vib 3 to pin: %ld @ freq: %ld", SettingsHandler::Vibe2_PIN, VibePWM_Freq);
            ledcSetup(Vibe2_PWM,VibePWM_Freq,8);
            ledcAttachPin(SettingsHandler::Vibe2_PIN,Vibe2_PWM); 
        }
        if(SettingsHandler::Vibe3_PIN > 0) {
            LogHandler::debug(_TAG, "Connecting vib 4 to pin: %ld @ freq: %ld", SettingsHandler::Vibe3_PIN, VibePWM_Freq);
            ledcSetup(Vibe3_PWM,VibePWM_Freq,8);
            ledcAttachPin(SettingsHandler::Vibe3_PIN,Vibe3_PWM); 
        }

        if(SettingsHandler::feedbackTwist)
        {
            // Initiate position tracking for twist
            pinMode(SettingsHandler::TwistFeedBack_PIN,INPUT);
            if(!SettingsHandler::analogTwist) 
            {
                LogHandler::debug(_TAG, "Attaching interrupt for twist feedback to pin: %u", SettingsHandler::TwistFeedBack_PIN);
                attachInterrupt(SettingsHandler::TwistFeedBack_PIN, twistChange, CHANGE);
                //Serial.print("Setting digital twist "); 
                //Serial.println(SettingsHandler::TwistFeedBack_PIN);
            } 
            else
            {
                //Serial.print("Setting analog twist "); 
                //Serial.println(SettingsHandler::TwistFeedBack_PIN);
    /*             adcAttachPin(SettingsHandler::TwistFeedBack_PIN);
                analogReadResolution(11);
                analogSetAttenuation(ADC_6db); */
            }
        } 
    }

    void executeCommon(const int xLin) {
        if(!m_tcode)
            return;
        xRot = m_tcode->AxisRead("R0");
        vibe0 = m_tcode->AxisRead("V0");
        if (!SettingsHandler::lubeEnabled) { vibe1 = m_tcode->AxisRead("V1"); }
        vibe2 = m_tcode->AxisRead("V2");
        vibe3 = m_tcode->AxisRead("V3");
        valveCmd = m_tcode->AxisRead("A0");
        suckCmd = m_tcode->AxisRead("A1");
        if (SettingsHandler::lubeEnabled) { lube = m_tcode->AxisRead("A2"); }

        
        squeezeCmd = m_tcode->AxisRead("A3");


        if (SettingsHandler::feedbackTwist && !SettingsHandler::continuousTwist) 
        {
            float angPos;
            // Calculate twist position
            if (!SettingsHandler::analogTwist)
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
                int feedBackValue = analogRead(SettingsHandler::TwistFeedBack_PIN);
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
        if (SettingsHandler::feedbackTwist && !SettingsHandler::continuousTwist) 
        {
            twist  = (xRot - map(twistPos,-1500,1500,9999,0))/5;
            if(!SettingsHandler::analogTwist) 
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
                    // Serial.println(map(SettingsHandler::TwistServo_ZERO + twist,0,TwistServo_Int,0,65535));
                //}
            }
        } 
        else 
        {
            twist  = map(xRot,0,9999,1000,-1000);
        }
        int squeeze;
        squeeze = map(squeezeCmd,0,9999,1000,-1000);
        
        // Set Servos
        ledcWrite(TwistServo_PWM, map(SettingsHandler::TwistServo_ZERO + twist,0,TwistServo_Int,0,65535));
        ledcWrite(SqueezeServo_PWM, map(SettingsHandler::SqueezeServo_ZERO + squeeze,0,SqueezeServo_Int,0,65535));

        // Valce
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
        if (SettingsHandler::inverseValve) { valve = -valve; }
        if(SettingsHandler::valveServo90Degrees)
        {
            if (SettingsHandler::inverseValve) { 
                valve = map(valve,0,500,-500,500);
            } 
            else
            {
                valve = map(valve,-500,0,-500,500);
            }
        }
        ledcWrite(ValveServo_PWM, map(SettingsHandler::ValveServo_ZERO + valve,0,ValveServo_Int,0,65535));
        
        // Output vibration channels
        // These should drive PWM pins connected to vibration motors via MOSFETs or H-bridges.
        if (vibe0 > 0 && vibe0 <= 9999) {
            ledcWrite(Vibe0_PWM, map(vibe0,1,9999,31,255));
        } else {
            ledcWrite(Vibe0_PWM, 0);
        }
        if (!SettingsHandler::lubeEnabled && vibe1 > 0 && vibe1 <= 9999) {
            ledcWrite(Vibe1_PWM, map(vibe1,1,9999,31,255));
        } else {
            ledcWrite(Vibe1_PWM, 0);
        }
        if (vibe2 > 0 && vibe2 <= 9999) {
            ledcWrite(Vibe2_PWM, map(vibe2,1,9999,31,255));
        } else {
            ledcWrite(Vibe2_PWM, 0);
        }
        if (vibe3 > 0 && vibe3 <= 9999) {
            ledcWrite(Vibe3_PWM, map(vibe3,1,9999,31,255));
        } else {
            ledcWrite(Vibe3_PWM, 0);
        }
        // Vibe timeout functions - shuts the vibne channels down if not commanded for a specified interval
        if (millis() - m_tcode->AxisLast("V0") > VIBE_TIMEOUT) { m_tcode->AxisInput("V0",0,'I',500); }
        if (!SettingsHandler::lubeEnabled && millis() - m_tcode->AxisLast("V1") > VIBE_TIMEOUT) { m_tcode->AxisInput("V1",0,'I',500); }
        if (millis() - m_tcode->AxisLast("V2") > VIBE_TIMEOUT) { m_tcode->AxisInput("V2",0,'I',500); }
        if (millis() - m_tcode->AxisLast("V3") > VIBE_TIMEOUT) { m_tcode->AxisInput("V3",0,'I',500); }
        
        // Done with vibration channels

        // Lube functions
        if (SettingsHandler::lubeEnabled) {
            if (lube > 0 && lube <= 9999) {
                ledcWrite(Vibe1_PWM, map(lube,1,9999,127,255));
            } else if (digitalRead(SettingsHandler::LubeButton_PIN) == HIGH) {
                ledcWrite(Vibe1_PWM,SettingsHandler::lubeAmount);
            } else { 
                ledcWrite(Vibe1_PWM,0);
            }
            if (millis() - m_tcode->AxisLast("A2") > 500) { m_tcode->AxisInput("A2",0,' ',0); } // Auto cutoff
        }
        // Done with lube
    }
    
private:
    const char* _TAG = TagHandler::MotorHandler;

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
    int valveCmd,suckCmd;
    int vibe0,vibe1,vibe2,vibe3;
    float upVel,valvePos;
    unsigned long tLast;
    int xLast;
};