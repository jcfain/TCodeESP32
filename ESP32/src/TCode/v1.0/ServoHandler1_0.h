
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

#include "../Global.h"
#include "../ServoHandler.h"
#include "../../SettingsHandler.h"
class ServoHandler1_0 : public ServoHandler {

public:
    ServoHandler1_0() : tcode(SettingsHandler::ESP32Version, SettingsHandler::TCodeVersionName) {}

    // Setup function
    // This is run once, when the arduino starts
    void setup(int servoFrequency, int pitchFrequency, int valveFrequency, int twistFrequency) override {
        //tcode = new TCode(SettingsHandler::ESP32Version, SettingsHandler::TCodeVersionName, btHandler);
        MainServo_Freq = servoFrequency;
        PitchServo_Freq = pitchFrequency;
        TwistServo_Freq = twistFrequency;
        ValveServo_Freq = valveFrequency;
// Servo Pulse intervals
        MainServo_Int = 1000000/MainServo_Freq;
        PitchServo_Int = 1000000/PitchServo_Freq;
        TwistServo_Int = 1000000/TwistServo_Freq;
        ValveServo_Int = 1000000/ValveServo_Freq;

        tcode.init();
        // report status
        tcode.inputString("D0");
        tcode.inputString("D1");

        // Set SR6 arms to startup positions
        if (SettingsHandler::sr6Mode) { tcode.inputString("R2750"); }

        // Register device axes
        tcode.axisRegister(Stroke, "L0");
        if (SettingsHandler::sr6Mode) {
            tcode.axisRegister(Sway, "L1");
            tcode.axisRegister(Surge, "L2");
        }
        tcode.axisRegister(Twist, "R0");//, "Twist"
        tcode.axisRegister(Roll, "R1");//, "Roll"
        tcode.axisRegister(Pitch, "R2");//, "Pitch"
        tcode.axisRegister(Vibe1, "V0");//, "Vibe1"
        if (!SettingsHandler::lubeEnabled) { tcode.axisRegister(Vibe2, "V1"); }//, "Vibe2")
        tcode.axisRegister(Valve, "A0");//, "Valve"
        tcode.axisRegister(Suck, "A1");//, "Suck"
        //tcode.axisRead("A1",VALVE_DEFAULT,'I',3000);
        if (SettingsHandler::lubeEnabled) {
            tcode.axisRegister(Lube, "A2");//, "Lube"
            //tcode.axisRead("A2",0,' ',0);
            pinMode(SettingsHandler::LubeButton_PIN,INPUT);
        }
        // Setup Servo PWM channels
        // Lower Left Servo
        if(!DEBUG_BUILD) {// The default pins for these are used on the debugger board.
            ledcSetup(LowerLeftServo_PWM,MainServo_Freq,16);
            ledcAttachPin(SettingsHandler::LeftServo_PIN,LowerLeftServo_PWM);
            // Lower Right Servo
            ledcSetup(LowerRightServo_PWM,MainServo_Freq,16);
            ledcAttachPin(SettingsHandler::RightServo_PIN,LowerRightServo_PWM);
        }
        if(SettingsHandler::sr6Mode)
        {
            // Upper Left Servo
            ledcSetup(UpperLeftServo_PWM,MainServo_Freq,16);
            ledcAttachPin(SettingsHandler::LeftUpperServo_PIN,UpperLeftServo_PWM);
            if(!DEBUG_BUILD) {// The default pins for these are used on the debugger board.
                // Upper Right Servo
                ledcSetup(UpperRightServo_PWM,MainServo_Freq,16);
                ledcAttachPin(SettingsHandler::RightUpperServo_PIN,UpperRightServo_PWM);
                // Right Pitch Servo
                ledcSetup(RightPitchServo_PWM,PitchServo_Freq,16);
                ledcAttachPin(SettingsHandler::PitchRightServo_PIN,RightPitchServo_PWM);
            }
        }
        // Left Pitch Servo
        ledcSetup(LeftPitchServo_PWM,PitchServo_Freq,16);
        ledcAttachPin(SettingsHandler::PitchLeftServo_PIN,LeftPitchServo_PWM);
        // Twist Servo
        ledcSetup(TwistServo_PWM,TwistServo_Freq,16);
        ledcAttachPin(SettingsHandler::TwistServo_PIN,TwistServo_PWM);
        // Valve Servo
        ledcSetup(ValveServo_PWM,ValveServo_Freq,16);
        ledcAttachPin(SettingsHandler::ValveServo_PIN,ValveServo_PWM);

        // Set vibration PWM pins
        // Vibe0 Pin
        ledcSetup(Vibe0_PWM,VibePWM_Freq,8);
        ledcAttachPin(SettingsHandler::Vibe0_PIN,Vibe0_PWM);
        // Vibe1 Pin
        ledcSetup(Vibe1_PWM,VibePWM_Freq,8);
        ledcAttachPin(SettingsHandler::Vibe1_PIN,Vibe1_PWM); 

        if(SettingsHandler::feedbackTwist)
        {
            // Initiate position tracking for twist
            pinMode(SettingsHandler::TwistFeedBack_PIN,INPUT);
            if(!SettingsHandler::analogTwist) 
            {
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
        
        // Signal done
        tcode.sendMessage("Ready!");
    }



    void read(String input) override
    {
        tcode.inputString(input);
    }

    void read(byte input) override 
    {
        tcode.inputByte(input);
    }

    void setMessageCallback(TCODE_FUNCTION_PTR_T function) override {
        tcode.setMessageCallback(function);
    }

// int testVar = -1;
// int testVar2 = -1;
    void execute() override {
        // Collect inputs
        // These functions query the t-code object for the position/level at a specified time
        // Number recieved will be an integer, 0-9999
        xLin = tcode.axisRead(Stroke);
        if (SettingsHandler::sr6Mode) {
            yLin = tcode.axisRead(Sway);
            zLin = tcode.axisRead(Surge);
        }
        xRot = tcode.axisRead(Twist);
        yRot = tcode.axisRead(Roll);
        zRot = tcode.axisRead(Pitch);
        vibe0 = tcode.axisRead(Vibe1);
        if (!SettingsHandler::lubeEnabled) { vibe1 = tcode.axisRead(Vibe2); }
        valveCmd = tcode.axisRead(Valve);
        suckCmd = tcode.axisRead(Suck);
        if (SettingsHandler::lubeEnabled) { lube = tcode.axisRead(Lube); }

        // If you want to mix your servos differently, enter your code below:

        if (SettingsHandler::feedbackTwist && !SettingsHandler::continuousTwist) 
        {
            float angPos;
            // Calculate twist position
            if (!SettingsHandler::analogTwist)
            {
                noInterrupts();
                float dutyCycle = twistPulseLength;
                dutyCycle = dutyCycle/lastTwistPulseCycle;
                interrupts();
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
        if (tcode.axisLastT(Suck) >= tcode.axisLastT(Valve)) {
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

        // OSR2 Kinematics
        if (!SettingsHandler::sr6Mode) {
            // Calculate arm angles
            // Linear scale inputs to servo appropriate numbers
            int stroke,roll,pitch;
            stroke = map(xLin,0,9999,-350,350);
            roll   = map(yRot,0,9999,-180,180);
            pitch  = map(zRot,0,9999,-350,350);
            if(SettingsHandler::inverseStroke) 
            {
                ledcWrite(LowerLeftServo_PWM, map(SettingsHandler::LeftServo_ZERO - stroke + roll,0,MainServo_Int,0,65535));
                ledcWrite(LowerRightServo_PWM, map(SettingsHandler::RightServo_ZERO + stroke + roll,0,MainServo_Int,0,65535));
            }
            else
            {
                ledcWrite(LowerLeftServo_PWM, map(SettingsHandler::LeftServo_ZERO + stroke + roll,0,MainServo_Int,0,65535));
                ledcWrite(LowerRightServo_PWM, map(SettingsHandler::RightServo_ZERO - stroke + roll,0,MainServo_Int,0,65535));
            }
            
            if(SettingsHandler::inversePitch) 
            {
                ledcWrite(LeftPitchServo_PWM, map(SettingsHandler::PitchLeftServo_ZERO + pitch,0,PitchServo_Int,0,65535));

            }
            else
            {
                ledcWrite(LeftPitchServo_PWM, map(SettingsHandler::PitchLeftServo_ZERO - pitch,0,PitchServo_Int,0,65535));
            }
        }
        else 
        {
            // SR6 Kinematics
            // Calculate arm angles
            int roll,pitch,fwd,thrust,side;
            roll = map(yRot,0,9999,-3000,3000);
            pitch = map(zRot,0,9999,-2500,2500);
            fwd = map(yLin,0,9999,-3000,3000);
            thrust = map(xLin,0,9999,-6000,6000);
            side = map(zLin,0,9999,-3000,3000);

            // Main arms
            int lowerLeftValue,upperLeftValue,pitchLeftValue,pitchRightValue,upperRightValue,lowerRightValue;
            if(SettingsHandler::inverseStroke) 
            {
                lowerLeftValue = SetMainServo(16248 - fwd, 1500 - thrust - roll); // Lower left servo
                upperLeftValue = SetMainServo(16248 - fwd, 1500 + thrust + roll); // Upper left servo
                upperRightValue = SetMainServo(16248 - fwd, 1500 + thrust - roll); // Upper right servo
                lowerRightValue = SetMainServo(16248 - fwd, 1500 - thrust + roll); // Lower right servo
                pitchLeftValue = SetPitchServo(16248 - fwd, 4500 + thrust, -side + 1.5*roll, -pitch);
                pitchRightValue = SetPitchServo(16248 - fwd, 4500 + thrust, side - 1.5*roll, -pitch);
            } 
            else 
            {
                lowerLeftValue = SetMainServo(16248 - fwd, 1500 + thrust + roll); // Lower left servo
                upperLeftValue = SetMainServo(16248 - fwd, 1500 - thrust - roll); // Upper left servo
                upperRightValue = SetMainServo(16248 - fwd, 1500 - thrust + roll); // Upper right servo
                lowerRightValue = SetMainServo(16248 - fwd, 1500 + thrust - roll); // Lower right servo
                pitchLeftValue = SetPitchServo(16248 - fwd, 4500 - thrust, side - 1.5*roll, -pitch);
                pitchRightValue = SetPitchServo(16248 - fwd, 4500 - thrust, -side + 1.5*roll, -pitch);
            }
            // Set Servos
            ledcWrite(LowerLeftServo_PWM, map(SettingsHandler::LeftServo_ZERO - lowerLeftValue,0,MainServo_Int,0,65535));
            ledcWrite(UpperLeftServo_PWM, map(SettingsHandler::LeftUpperServo_ZERO + upperLeftValue,0,MainServo_Int,0,65535));
            ledcWrite(UpperRightServo_PWM, map(SettingsHandler::RightUpperServo_ZERO - upperRightValue,0,MainServo_Int,0,65535));
            ledcWrite(LowerRightServo_PWM, map(SettingsHandler::RightServo_ZERO + lowerRightValue,0,MainServo_Int,0,65535));
            ledcWrite(LeftPitchServo_PWM, map(constrain(SettingsHandler::PitchLeftServo_ZERO - pitchLeftValue, SettingsHandler::PitchLeftServo_ZERO - 600, SettingsHandler::PitchLeftServo_ZERO + 1000), 0, PitchServo_Int, 0, 65535));
            ledcWrite(RightPitchServo_PWM, map(constrain(SettingsHandler::PitchRightServo_ZERO + pitchRightValue, SettingsHandler::PitchRightServo_ZERO - 1000, SettingsHandler::PitchRightServo_ZERO + 600), 0, PitchServo_Int, 0, 65535));
        }

        // Twist and valve
        int twist,valve;
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
        // Set Servos
        ledcWrite(TwistServo_PWM, map(SettingsHandler::TwistServo_ZERO + twist,0,TwistServo_Int,0,65535));
        ledcWrite(ValveServo_PWM, map(SettingsHandler::ValveServo_ZERO + valve,0,TwistServo_Int,0,65535));

        // Done with servo channels

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
        // Vibe timeout functions - shuts the vibne channels down if not commanded for a specified interval
        if (millis() - tcode.axisLastT(Vibe1) > VIBE_TIMEOUT) { tcode.axisRead(Vibe1); }
        if (!SettingsHandler::lubeEnabled && millis() - tcode.axisLastT(Vibe2) > VIBE_TIMEOUT) { tcode.axisRead(Vibe2); }
        
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
            if (millis() - tcode.axisLastT(Lube) > 500) { tcode.axisRead(Lube); } // Auto cutoff
        }
        // Done with lube
    }

private:
    int MainServo_Int;
    int PitchServo_Int;
    int TwistServo_Int;
    int ValveServo_Int;

    int MainServo_Freq;
    int PitchServo_Freq;
    int TwistServo_Freq;
    int ValveServo_Freq;

    // Declare classes
    // This uses the t-code object above
    TCode<10> tcode;

    TCode_ChannelID Stroke = {TCode_Channel_Type::Linear, 0, true};
    TCode_ChannelID Surge = {TCode_Channel_Type::Linear, 1, true};
    TCode_ChannelID Sway = {TCode_Channel_Type::Linear, 2, true};
    TCode_ChannelID Twist = {TCode_Channel_Type::Rotation, 0, true};
    TCode_ChannelID Roll = {TCode_Channel_Type::Rotation, 1, true};
    TCode_ChannelID Pitch = {TCode_Channel_Type::Rotation, 2, true};
    TCode_ChannelID Vibe1 = {TCode_Channel_Type::Vibration, 0, true};
    TCode_ChannelID Vibe2 = {TCode_Channel_Type::Vibration, 1, true};
    TCode_ChannelID Valve = {TCode_Channel_Type::Auxiliary, 0, true};
    TCode_ChannelID Suck = {TCode_Channel_Type::Auxiliary, 1, true};
    TCode_ChannelID Lube = {TCode_Channel_Type::Auxiliary, 2, true};

    // Declare operating variables
    // Position variables
    int xLin,yLin,zLin;
    // Rotation variables
    int xRot,yRot,zRot;
    // Vibration variables
    int vibe0,vibe1;
    // Lube variables
    int lube;
    // Valve variables
    int valveCmd,suckCmd;
    // Velocity tracker variables, for valve
    int xLast;
    unsigned long tLast;
    float upVel,valvePos;
    float twistServoAngPos = 0.5;
    int twistTurns = 0;
    float twistPos;

    // Function to calculate the angle for the main arm servos
    // Inputs are target x,y coords of receiver pivot in 1/100 of a mm
    int SetMainServo(float x, float y) {
        x /= 100; y /= 100;          // Convert to mm
        float gamma = atan2(x,y);    // Angle of line from servo pivot to receiver pivot
        float csq = sq(x) + sq(y);   // Square of distance between servo pivot and receiver pivot
        float c = sqrt(csq);         // Distance between servo pivot and receiver pivot
        float beta = acos((csq - 28125)/(100*c));  // Angle between c-line and servo arm
        int out = ms_per_rad*(gamma + beta - 3.14159); // Servo signal output, from neutral
        return out;
    }


    // Function to calculate the angle for the pitcher arm servos
    // Inputs are target x,y,z coords of receiver upper pivot in 1/100 of a mm
    // Also pitch in 1/100 of a degree
    int SetPitchServo(float x, float y, float z, float pitch) {
        pitch *= 0.0001745; // Convert to radians
        x += 5500*sin(0.2618 + pitch);
        y -= 5500*cos(0.2618 + pitch);
        x /= 100; y /= 100; z /= 100;   // Convert to mm
        float bsq = 36250 - sq(75 + z); // Equivalent arm length
        float gamma = atan2(x,y);       // Angle of line from servo pivot to receiver pivot
        float csq = sq(x) + sq(y);      // Square of distance between servo pivot and receiver pivot
        float c = sqrt(csq);            // Distance between servo pivot and receiver pivot
        float beta = acos((csq + 5625 - bsq)/(150*c)); // Angle between c-line and servo arm
        int out = ms_per_rad*(gamma + beta - 3.14159); // Servo signal output, from neutral
        return out;
    }
};
