
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

#include "TCode0_3.h"
#include "../../SettingsHandler.h"
#include "../Global.h"
#include "../MotorHandler.h"
#include "../../TagHandler.h"

class ServoHandler0_3 : public MotorHandler {

public:
    ServoHandler0_3() : MotorHandler(new TCode0_3()) { }
    // Setup function
    // This is run once, when the arduino starts
    void setup() override {
        ms_per_rad = SettingsHandler::msPerRad;
        MainServo_Freq = SettingsHandler::servoFrequency;
        PitchServo_Freq = SettingsHandler::pitchFrequency;
// Servo Pulse intervals
        MainServo_Int = 1000000/MainServo_Freq;
        PitchServo_Int = 1000000/PitchServo_Freq;

        // Set SR6 arms to startup positions
        if (SettingsHandler::sr6Mode) { m_tcode->StringInput("R2750"); }

        // Register device axes
        m_tcode->RegisterAxis("L0", "Up");
        if (SettingsHandler::sr6Mode) {
            m_tcode->RegisterAxis("L1", "Forward");
            m_tcode->RegisterAxis("L2", "Left");
        }
        m_tcode->RegisterAxis("R0", "Twist");
        m_tcode->RegisterAxis("R1", "Roll");
        m_tcode->RegisterAxis("R2", "Pitch");
        // Setup Servo PWM channels
        // Lower Left Servo
        if(!DEBUG_BUILD) {// The default pins for these are used on the debugger board.
            LogHandler::verbose(_TAG, "Connecting left servo to pin: %u", SettingsHandler::LeftServo_PIN);
            ledcSetup(LowerLeftServo_PWM,MainServo_Freq,16);
            ledcAttachPin(SettingsHandler::LeftServo_PIN,LowerLeftServo_PWM);
            // Lower Right Servo
            LogHandler::verbose(_TAG, "Connecting right servo to pin: %u", SettingsHandler::RightServo_PIN);
            ledcSetup(LowerRightServo_PWM,MainServo_Freq,16);
            ledcAttachPin(SettingsHandler::RightServo_PIN,LowerRightServo_PWM);
        }
        if(SettingsHandler::sr6Mode)
        {
            // Upper Left Servo
            LogHandler::verbose(_TAG, "Connecting left upper servo to pin: %u", SettingsHandler::LeftUpperServo_PIN);
            ledcSetup(UpperLeftServo_PWM,MainServo_Freq,16);
            ledcAttachPin(SettingsHandler::LeftUpperServo_PIN,UpperLeftServo_PWM);
            if(!DEBUG_BUILD) {// The default pins for these are used on the debugger board.
                // Upper Right Servo
                LogHandler::verbose(_TAG, "Connecting right upper servo to pin: %u", SettingsHandler::RightUpperServo_PIN);
                ledcSetup(UpperRightServo_PWM,MainServo_Freq,16);
                ledcAttachPin(SettingsHandler::RightUpperServo_PIN,UpperRightServo_PWM);
                // Right Pitch Servo
                LogHandler::verbose(_TAG, "Connecting right pitch servo to pin: %u", SettingsHandler::PitchRightServo_PIN);
                ledcSetup(RightPitchServo_PWM,PitchServo_Freq,16);
                ledcAttachPin(SettingsHandler::PitchRightServo_PIN,RightPitchServo_PWM);
            }
        }
        // Left Pitch Servo
        LogHandler::verbose(_TAG, "Connecting pitch servo to pin: %u", SettingsHandler::PitchLeftServo_PIN);
        ledcSetup(LeftPitchServo_PWM,PitchServo_Freq,16);
        ledcAttachPin(SettingsHandler::PitchLeftServo_PIN,LeftPitchServo_PWM);

        setupCommon();
        
        // Signal done
        m_tcode->sendMessage("Ready!");
    }

    void setMessageCallback(TCODE_FUNCTION_PTR_T function) override {
        m_tcode->setMessageCallback(function);
    }

    void read(String input) override
    {
        m_tcode->StringInput(input);
    }

    void read(byte input) override 
    {
        m_tcode->ByteInput(input);
    }

    // String getDeviceSettings() {
    //     return m_tcode->getDeviceSettings();
    // }
// int testVar = -1;
// int testVar2 = -1;
    void execute() override {
        // Collect inputs
        // These functions query the t-code object for the position/level at a specified time
        // Number recieved will be an integer, 0-9999
        xLin = m_tcode->AxisRead("L0");
        if (SettingsHandler::sr6Mode) {
            yLin = m_tcode->AxisRead("L1");
            zLin = m_tcode->AxisRead("L2");
        }
        yRot = m_tcode->AxisRead("R1");
        zRot = m_tcode->AxisRead("R2");
        // If you want to mix your servos differently, enter your code below:

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
				// Serial.printf("Sending lowerLeftValue: %i\n", lowerLeftValue);
				// Serial.printf("Sending upperLeftValue: %i\n", upperLeftValue);
				// Serial.printf("Sending lowerRightValue: %i\n", lowerRightValue);
				// Serial.printf("Sending upperRightValue: %i\n", upperRightValue);
				// Serial.printf("Sending pitchLeftValue: %i\n", pitchLeftValue);
				// Serial.printf("Sending pitchRightValue: %i\n", pitchRightValue);
            // Set Servos
            ledcWrite(LowerLeftServo_PWM, map(SettingsHandler::LeftServo_ZERO - lowerLeftValue,0,MainServo_Int,0,65535));
            ledcWrite(UpperLeftServo_PWM, map(SettingsHandler::LeftUpperServo_ZERO + upperLeftValue,0,MainServo_Int,0,65535));
            ledcWrite(UpperRightServo_PWM, map(SettingsHandler::RightUpperServo_ZERO - upperRightValue,0,MainServo_Int,0,65535));
            ledcWrite(LowerRightServo_PWM, map(SettingsHandler::RightServo_ZERO + lowerRightValue,0,MainServo_Int,0,65535));
            ledcWrite(LeftPitchServo_PWM, map(constrain(SettingsHandler::PitchLeftServo_ZERO - pitchLeftValue, SettingsHandler::PitchLeftServo_ZERO - 600, SettingsHandler::PitchLeftServo_ZERO + 1000), 0, PitchServo_Int, 0, 65535));
            ledcWrite(RightPitchServo_PWM, map(constrain(SettingsHandler::PitchRightServo_ZERO + pitchRightValue, SettingsHandler::PitchRightServo_ZERO - 1000, SettingsHandler::PitchRightServo_ZERO + 600), 0, PitchServo_Int, 0, 65535));
        }

        executeCommon(xLin);
        // Done with servo channels

    }

private:
    const char* _TAG = TagHandler::ServoHandler3;
    int MainServo_Int;
    int PitchServo_Int;

    int MainServo_Freq;
    int PitchServo_Freq;

    // Declare classes
    // This uses the t-code object above
    // Declare operating variables
    // Position variables
    int xLin,yLin,zLin;
    // Rotation variables
    int yRot,zRot;

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
