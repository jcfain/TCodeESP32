
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
#include "MotorHandler0_3.h"
#include "../../TagHandler.h"
#include "../settingsFactory.h"
#include "../PinMap.h"

class ServoHandler0_3 : public MotorHandler0_3 {

public:
    ServoHandler0_3() : MotorHandler0_3(new TCode0_3()) { }
    // Setup function
    // This is run once, when the arduino starts
    void setup() override {
        LogHandler::debug(_TAG, "Setting up servo handler v3");
        m_settingsFactory = SettingsFactory::getInstance();
        
        m_settingsFactory->getValue(DEVICE_TYPE, m_deviceType);
        m_settingsFactory->getValue(MS_PER_RAD, ms_per_rad);
        LogHandler::debug(_TAG, "DEVICE_TYPE: %d", m_deviceType);
        LogHandler::debug(_TAG, "MS_PER_RAD: %d", ms_per_rad);

        // Set SR6 arms to startup positions
        if (m_deviceType == DeviceType::SR6) 
        {
             m_tcode->read("R2750"); 
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
        // Lower Left Servo
        #ifndef ESP_PROG// The default pins for these are used on the debugger board.
        m_leftServoPin = ((PinMapOSR*)pinMap)->leftServo();
        if(m_leftServoPin > -1) {
            m_lowerLeftServoChannel = ((PinMapOSR*)pinMap)->leftServoChannel();
            int freq = ((PinMapOSR*)pinMap)->getChannelFrequency(m_lowerLeftServoChannel);
            m_leftServo_Int = calcInt(freq);
            attachPin("left servo", m_leftServoPin, freq, m_lowerLeftServoChannel);
        } else {
            LogHandler::error(_TAG, "Invalid left servo to pin: %d", m_leftServoPin);
            m_initFailed = true;
        }
        m_rightServoPin = ((PinMapOSR*)pinMap)->rightServo();
        if(m_rightServoPin > -1) {
            m_lowerRightServoChannel = ((PinMapOSR*)pinMap)->rightServoChannel();
            int freq = ((PinMapOSR*)pinMap)->getChannelFrequency(m_lowerRightServoChannel);
            m_rightServo_Int = calcInt(freq);
            attachPin("right servo", m_rightServoPin, freq, m_lowerRightServoChannel);
        } else {
            LogHandler::error(_TAG, "Invalid right servo to pin: %d", m_rightServoPin);
            m_initFailed = true;
        }
        #endif
        if(m_deviceType == DeviceType::SR6)
        {
            m_leftUpperServoPin = ((PinMapSR6*)pinMap)->leftUpperServo();
            if(m_leftUpperServoPin > -1) {
                m_upperLeftServoChannel = ((PinMapSR6*)pinMap)->leftUpperServoChannel();
                int freq = ((PinMapSR6*)pinMap)->getChannelFrequency(m_upperLeftServoChannel);
                m_leftUpperServo_Int = calcInt(freq);
                attachPin("left upper servo", m_leftUpperServoPin, freq, m_upperLeftServoChannel);
            } else {
                LogHandler::error(_TAG, "Invalid left upper servo to pin: %d", m_leftUpperServoPin);
                m_initFailed = true;
            }
            #ifndef ESP_PROG// The default pins for these are used on the debugger board. 12, 13, 14 & 15
                m_rightUpperServoPin = ((PinMapSR6*)pinMap)->rightUpperServo();
                if(m_rightUpperServoPin > -1) {
                    m_upperRightServoChannel = ((PinMapSR6*)pinMap)->rightUpperServoChannel();
                    int freq = ((PinMapSR6*)pinMap)->getChannelFrequency(m_upperRightServoChannel);
                    m_rightUpperServo_Int = calcInt(freq);
                    attachPin("right upper servo", m_rightUpperServoPin, freq, m_upperRightServoChannel);
                } else {
                    LogHandler::error(_TAG, "Invalid right upper servo to pin: %d", m_rightUpperServoPin);
                    m_initFailed = true;
                }
                m_rightPitchServoPin = ((PinMapSR6*)pinMap)->pitchRight();
                if(m_rightPitchServoPin > -1) {
                    m_rightPitchServoChannel = ((PinMapSR6*)pinMap)->pitchRightChannel();
                    int freq = ((PinMapSR6*)pinMap)->getChannelFrequency(m_rightPitchServoChannel);
                    m_pitchRightServo_Int = calcInt(freq);
                    attachPin("right pitch servo", m_rightPitchServoPin, freq, m_rightPitchServoChannel);
                } else {
                    LogHandler::error(_TAG, "Invalid right pitch servo to pin: %d", m_rightPitchServoPin);
                    m_initFailed = true;
                }
            #endif
        }
        m_leftPitchServoPin = ((PinMapSR6*)pinMap)->pitchLeft();
        if(m_leftPitchServoPin > -1) {
            m_leftPitchServoChannel = ((PinMapSR6*)pinMap)->pitchLeftChannel();
            int freq = ((PinMapSR6*)pinMap)->getChannelFrequency(m_leftPitchServoChannel);
            m_pitchLeftServo_Int = calcInt(freq);
            attachPin("pitch servo", m_leftPitchServoPin, freq, m_leftPitchServoChannel);
        } else {
            LogHandler::error(_TAG, "Invalid pitch servo to pin: %u", m_leftPitchServoPin);
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
        m_tcode->read(input);
    }
    
    void read(const char* input, size_t len) override
    {
        for (int i = 0; i < len; i++) {
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

        if(m_deviceType == DeviceType::OSR)
        {
            executeOSR(xLin, yRot, zRot);
        }
        else if(m_deviceType == DeviceType::SR6)
        {
            executeSR6(xLin, yRot, zRot);
        }

        executeCommon(xLin);
        // Done with servo channels

    }

private:
    const char* _TAG = TagHandler::ServoHandler;
    SettingsFactory* m_settingsFactory;
    DeviceType m_deviceType;
    bool m_initFailed = false;

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

    // Declare classes
    // This uses the t-code object above
    // Declare operating variables
    // Position variables
    int xLin,yLin,zLin;
    // Rotation variables
    int yRot,zRot;

    void executeOSR(int strokeTcode, int rollTcode, int pitchTcode) {
        // Calculate arm angles
        // Linear scale inputs to servo appropriate numbers
        int stroke,roll,pitch;
        stroke = map(strokeTcode,0,9999,-350,350);
        roll   = map(rollTcode,0,9999,-180,180);
        pitch  = map(pitchTcode,0,9999,-350,350);
        int leftDuty;
        int rightDuty;
        int pitchDuty;
        if(m_settingsFactory->getInverseStroke()) 
        {
            leftDuty = map(m_settingsFactory->getLeftServo_ZERO() - stroke + roll,0,m_leftServo_Int,0,m_servoPWMMaxDuty);
            rightDuty = map(m_settingsFactory->getRightServo_ZERO() + stroke + roll,0,m_rightServo_Int,0,m_servoPWMMaxDuty);
        }
        else
        {
            leftDuty = map(m_settingsFactory->getLeftServo_ZERO() + stroke + roll,0,m_leftServo_Int,0,m_servoPWMMaxDuty);
            rightDuty = map(m_settingsFactory->getRightServo_ZERO() - stroke + roll,0,m_rightServo_Int,0,m_servoPWMMaxDuty);
        }
        if(m_settingsFactory->getInversePitch()) 
        {
            pitchDuty = map(m_settingsFactory->getPitchLeftServo_ZERO() + pitch,0,m_pitchLeftServo_Int,0,m_servoPWMMaxDuty);
        }
        else
        {
            pitchDuty = map(m_settingsFactory->getPitchLeftServo_ZERO() - pitch,0,m_pitchLeftServo_Int,0,m_servoPWMMaxDuty);
            // Serial.println(pitch);
            // Serial.println(m_settingsFactory->getPitchLeftServo_ZERO());
            // Serial.println(PitchServo_Int);
            // Serial.println(pitchDuty);
            // Serial.println(m_servoPWMMaxDuty);
        }

#ifndef ESP_PROG
        #ifdef ESP_ARDUINO3
        ledcWrite(m_leftServoPin, leftDuty);
        ledcWrite(m_rightServoPin, rightDuty);
        ledcWrite(m_leftPitchServoPin, pitchDuty);
        #else
        ledcWrite(m_lowerLeftServoChannel, leftDuty);
        ledcWrite(m_lowerRightServoChannel, rightDuty);
        ledcWrite(m_leftPitchServoChannel, pitchDuty);
        #endif
#endif
}

    void executeSR6(int strokeTcode, int rollTcode, int pitchTcode) 
    {
        yLin = m_tcode->AxisRead("L1");
        zLin = m_tcode->AxisRead("L2");
        // SR6 Kinematics
        // Calculate arm angles
        int roll,pitch,fwd,thrust,side;
        roll = map(rollTcode,0,9999,-3000,3000);
        pitch = map(pitchTcode,0,9999,-2500,2500);
        fwd = map(yLin,0,9999,-3000,3000);
        thrust = map(strokeTcode,0,9999,-6000,6000);
        side = map(zLin,0,9999,-3000,3000);

        // Main arms
        int lowerLeftValue,upperLeftValue,pitchLeftValue,pitchRightValue,upperRightValue,lowerRightValue;
        if(m_settingsFactory->getInverseStroke()) 
        {
            lowerLeftValue = SetMainServo(16248 - fwd, 1500 - thrust - roll); // Lower left servo
            lowerRightValue = SetMainServo(16248 - fwd, 1500 - thrust + roll); // Lower right servo
            upperLeftValue = SetMainServo(16248 - fwd, 1500 + thrust + roll); // Upper left servo
            upperRightValue = SetMainServo(16248 - fwd, 1500 + thrust - roll); // Upper right servo
            pitchLeftValue = SetPitchServo(16248 - fwd, 4500 + thrust, -side + 1.5*roll, -pitch);
            pitchRightValue = SetPitchServo(16248 - fwd, 4500 + thrust, side - 1.5*roll, -pitch);
        } 
        else 
        {
            lowerLeftValue = SetMainServo(16248 - fwd, 1500 + thrust + roll); // Lower left servo
            lowerRightValue = SetMainServo(16248 - fwd, 1500 + thrust - roll); // Lower right servo
            upperLeftValue = SetMainServo(16248 - fwd, 1500 - thrust - roll); // Upper left servo
            upperRightValue = SetMainServo(16248 - fwd, 1500 - thrust + roll); // Upper right servo
            pitchLeftValue = SetPitchServo(16248 - fwd, 4500 - thrust, side - 1.5*roll, -pitch);
            pitchRightValue = SetPitchServo(16248 - fwd, 4500 - thrust, -side + 1.5*roll, -pitch);
        }
        int lowerLeftDuty = map(m_settingsFactory->getLeftServo_ZERO() - lowerLeftValue,0,m_leftServo_Int,0,m_servoPWMMaxDuty);
        int lowerRightDuty = map(m_settingsFactory->getRightServo_ZERO() + lowerRightValue,0,m_rightServo_Int,0,m_servoPWMMaxDuty);
        int upperLeftDuty = map(m_settingsFactory->getLeftUpperServo_ZERO() + upperLeftValue,0,m_leftUpperServo_Int,0,m_servoPWMMaxDuty);
        int upperRightDuty = map(m_settingsFactory->getRightUpperServo_ZERO() - upperRightValue,0,m_rightUpperServo_Int,0,m_servoPWMMaxDuty);
        uint16_t pitchLeftZero = m_settingsFactory->getPitchLeftServo_ZERO();
        uint16_t pitchRightZero = m_settingsFactory->getPitchRightServo_ZERO();
        int pitchLeftDuty = map(constrain(pitchLeftZero - pitchLeftValue, pitchLeftZero - 600, pitchLeftZero + 1000), 0, m_pitchLeftServo_Int, 0, m_servoPWMMaxDuty);
        int pitchRightDuty = map(constrain(pitchRightZero + pitchRightValue, pitchRightZero - 1000, pitchRightZero + 600), 0, m_pitchRightServo_Int, 0, m_servoPWMMaxDuty);
        // Set Servos
#if !ESP_PROG
        #ifdef ESP_ARDUINO3
        ledcWrite(m_leftServoPin, lowerLeftDuty);
        ledcWrite(m_rightServoPin, lowerRightDuty);
        ledcWrite(m_leftUpperServoPin, upperLeftDuty);
        ledcWrite(m_rightUpperServoPin, upperRightDuty);
        ledcWrite(m_leftPitchServoPin, pitchLeftDuty);
        ledcWrite(m_rightPitchServoPin, pitchRightDuty);
        #else
        ledcWrite(m_lowerLeftServoChannel, lowerLeftDuty);
        ledcWrite(m_lowerRightServoChannel, lowerRightDuty);
        ledcWrite(m_upperLeftServoChannel, upperLeftDuty);
        ledcWrite(m_upperRightServoChannel, upperRightDuty);
        ledcWrite(m_leftPitchServoChannel, pitchLeftDuty);
        ledcWrite(m_rightPitchServoChannel, pitchRightDuty);
        #endif
#endif
    }

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
