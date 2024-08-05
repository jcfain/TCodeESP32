
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
#include "../settingsFactory.h"
#include "../PinMap.h"

class ServoHandler0_3 : public MotorHandler {

public:
    ServoHandler0_3() : MotorHandler(new TCode0_3()) { }
    // Setup function
    // This is run once, when the arduino starts
    void setup() override {
        m_settingsFactory = SettingsFactory::getInstance();
        m_settingsFactory->getValue(MS_PER_RAD, ms_per_rad);// SettingsHandler::getMsPerRad();
        m_settingsFactory->getValue(SERVO_FREQUENCY, MainServo_Freq);// = SettingsHandler::getServoFrequency();
        m_settingsFactory->getValue(PITCH_FREQUENCY, PitchServo_Freq);// = SettingsHandler::getPitchFrequency();
// Servo Pulse intervals
        MainServo_Int = 1000000/MainServo_Freq;
        PitchServo_Int = 1000000/PitchServo_Freq;

        m_deviceType = m_settingsFactory->getDeviceType();
        // Set SR6 arms to startup positions
        if (m_deviceType == DeviceType::SR6) 
        {
             m_tcode->StringInput("R2750"); 
        }

        // Register device axes
        m_tcode->RegisterAxis("L0", "Up");
        if (m_deviceType == DeviceType::SR6) 
        {
            m_tcode->RegisterAxis("L1", "Forward");
            m_tcode->RegisterAxis("L2", "Left");
        }
        m_tcode->RegisterAxis("R1", "Roll");
        m_tcode->RegisterAxis("R2", "Pitch");
        PinMap* pinMap;
        if (m_deviceType == DeviceType::SR6) 
        {
            pinMap = PinMapSR6::getInstance();
        } 
        else 
        {
            pinMap = PinMapOSR::getInstance();
        }
        int pin = -1;
        // Lower Left Servo
        if(!DEBUG_BUILD) {// The default pins for these are used on the debugger board.
            pin = ((PinMapOSR*)pinMap)->leftServo();
            if(pin > -1) {
                LogHandler::debug(_TAG, "Connecting left servo to pin: %ld", pin);
                #ifdef ESP_ARDUINO3
                ledcAttach(pin, MainServo_Freq, 16);
                #else
                ledcSetup(LowerLeftServo_PWM,MainServo_Freq,16);
                ledcAttachPin(pin,LowerLeftServo_PWM);
                #endif
            } else {
                LogHandler::error(_TAG, "Invalid left servo to pin: %ld", pin);
                m_initFailed = true;
            }
            pin = ((PinMapOSR*)pinMap)->rightServo();
            if(pin > -1) {
                // Lower Right Servo
                LogHandler::debug(_TAG, "Connecting right servo to pin: %ld", pin);
                #ifdef ESP_ARDUINO3
                ledcAttach(pin, MainServo_Freq, 16);
                #else
                ledcSetup(LowerRightServo_PWM,MainServo_Freq,16);
                ledcAttachPin(pin,LowerRightServo_PWM);
                #endif
            } else {
                LogHandler::error(_TAG, "Invalid right servo to pin: %ld", pin);
                m_initFailed = true;
            }
        }
        if(m_deviceType == DeviceType::SR6)
        {
            pin = ((PinMapSR6*)pinMap)->leftUpperServo();
            if(pin > -1) {
                // Upper Left Servo
                LogHandler::debug(_TAG, "Connecting left upper servo to pin: %ld", pin);
                #ifdef ESP_ARDUINO3
                ledcAttach(pin, MainServo_Freq, 16);
                #else
                ledcSetup(UpperLeftServo_PWM,MainServo_Freq,16);
                ledcAttachPin(pin,UpperLeftServo_PWM);
                #endif
            } else {
                LogHandler::error(_TAG, "Invalid left upper servo to pin: %ld", pin);
                m_initFailed = true;
            }
            if(!DEBUG_BUILD) {// The default pins for these are used on the debugger board.
                pin = ((PinMapSR6*)pinMap)->rightUpperServo();
                if(pin > -1) {
                    // Upper Right Servo
                    LogHandler::debug(_TAG, "Connecting right upper servo to pin: %ld", pin);
                    #ifdef ESP_ARDUINO3
                    ledcAttach(pin, MainServo_Freq, 16);
                    #else
                    ledcSetup(UpperRightServo_PWM,MainServo_Freq,16);
                    ledcAttachPin(pin,UpperRightServo_PWM);
                    #endif
                } else {
                    LogHandler::error(_TAG, "Invalid right upper servo to pin: %ld", pin);
                    m_initFailed = true;
                }
                pin = ((PinMapSR6*)pinMap)->pitchRight();
                if(pin> -1) {
                    // Right Pitch Servo
                    LogHandler::debug(_TAG, "Connecting right pitch servo to pin: %ld", pin);
                    #ifdef ESP_ARDUINO3
                    ledcAttach(pin, PitchServo_Freq, 16);
                    #else
                    ledcSetup(RightPitchServo_PWM,PitchServo_Freq,16);
                    ledcAttachPin(pin,RightPitchServo_PWM);
                    #endif
                } else {
                    LogHandler::error(_TAG, "Invalid right pitch servo to pin: %ld", pin);
                    m_initFailed = true;
                }
            }
        }
        pin = ((PinMapSR6*)pinMap)->pitchLeft();
        if(pin > -1) {
            // Left Pitch Servo
            LogHandler::debug(_TAG, "Connecting pitch servo to pin: %u", pin);
            #ifdef ESP_ARDUINO3
            ledcAttach(pin, PitchServo_Freq, 16);
            #else
            ledcSetup(LeftPitchServo_PWM,PitchServo_Freq,16);
            ledcAttachPin(pin,LeftPitchServo_PWM);
            #endif
        } else {
            LogHandler::error(_TAG, "Invalid pitch servo to pin: %u", pin);
            m_initFailed = true;
        }

        setupCommon();
        
        // Signal done
        if(m_initFailed)
            m_tcode->sendMessage("Init servos error!");
        else
            m_tcode->sendMessage("Ready!");
    }

    void setMessageCallback(TCODE_FUNCTION_PTR_T function) override {
        m_tcode->setMessageCallback(function);
    }

    void read(const String &input) override
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
        if(m_initFailed) {
            return;
        }
        // Collect inputs
        // These functions query the t-code object for the position/level at a specified time
        // Number recieved will be an integer, 0-9999
        xLin = m_tcode->AxisRead("L0");
        yRot = m_tcode->AxisRead("R1");
        zRot = m_tcode->AxisRead("R2");
        // If you want to mix your servos differently, enter your code below:

        // OSR2 Kinematics
        if(m_deviceType == DeviceType::OSR)
        {
            // Calculate arm angles
            // Linear scale inputs to servo appropriate numbers
            int stroke,roll,pitch;
            stroke = map(xLin,0,9999,-350,350);
            roll   = map(yRot,0,9999,-180,180);
            pitch  = map(zRot,0,9999,-350,350);
            if(m_settingsFactory->getInverseStroke()) 
            {
                ledcWrite(LowerLeftServo_PWM, map(m_settingsFactory->getLeftServo_ZERO() - stroke + roll,0,MainServo_Int,0,65535));
                ledcWrite(LowerRightServo_PWM, map(m_settingsFactory->getRightServo_ZERO() + stroke + roll,0,MainServo_Int,0,65535));
            }
            else
            {
                ledcWrite(LowerLeftServo_PWM, map(m_settingsFactory->getLeftServo_ZERO() + stroke + roll,0,MainServo_Int,0,65535));
                ledcWrite(LowerRightServo_PWM, map(m_settingsFactory->getRightServo_ZERO() - stroke + roll,0,MainServo_Int,0,65535));
            }
            
            if(m_settingsFactory->getInversePitch()) 
            {
                ledcWrite(LeftPitchServo_PWM, map(m_settingsFactory->getPitchLeftServo_ZERO() + pitch,0,PitchServo_Int,0,65535));

            }
            else
            {
                ledcWrite(LeftPitchServo_PWM, map(m_settingsFactory->getPitchLeftServo_ZERO() - pitch,0,PitchServo_Int,0,65535));
            }
        }
        else if(m_deviceType == DeviceType::SR6)
        {
            yLin = m_tcode->AxisRead("L1");
            zLin = m_tcode->AxisRead("L2");
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
            if(m_settingsFactory->getInverseStroke()) 
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
            ledcWrite(LowerLeftServo_PWM, map(m_settingsFactory->getLeftServo_ZERO() - lowerLeftValue,0,MainServo_Int,0,65535));
            ledcWrite(UpperLeftServo_PWM, map(m_settingsFactory->getLeftUpperServo_ZERO() + upperLeftValue,0,MainServo_Int,0,65535));
            ledcWrite(UpperRightServo_PWM, map(m_settingsFactory->getRightUpperServo_ZERO() - upperRightValue,0,MainServo_Int,0,65535));
            ledcWrite(LowerRightServo_PWM, map(m_settingsFactory->getRightServo_ZERO() + lowerRightValue,0,MainServo_Int,0,65535));
            uint16_t pitchLeftZero = m_settingsFactory->getPitchLeftServo_ZERO();
            ledcWrite(LeftPitchServo_PWM, map(constrain(pitchLeftZero - pitchLeftValue, pitchLeftZero - 600, pitchLeftZero + 1000), 0, PitchServo_Int, 0, 65535));
            uint16_t pitchRightZero = m_settingsFactory->getPitchRightServo_ZERO();
            ledcWrite(RightPitchServo_PWM, map(constrain(pitchRightZero + pitchRightValue, pitchRightZero - 1000, pitchRightZero + 600), 0, PitchServo_Int, 0, 65535));
        }

        executeCommon(xLin);
        // Done with servo channels

    }

private:
    const char* _TAG = TagHandler::ServoHandler3;
    SettingsFactory* m_settingsFactory;
    DeviceType m_deviceType;
    bool m_initFailed = false;
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
