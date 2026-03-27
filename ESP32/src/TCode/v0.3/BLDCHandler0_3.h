// SSR1-P_TCode_ESP32_Alpha2
// by TempestMAx 23-2-2023
// Please copy, share, learn, innovate, give attribution.
// Decodes T-code commands and uses them to control servos a single brushless motor
// It can handle:
//   3x linear channels (L0, L1, L2)
//   3x rotation channels (R0, R1, R2) 
//   3x vibration channels (V0, V1, V2)
//   3x auxilliary channels (A0, A1, A2)
// This code is designed to drive the SSR1 stroker robot, but is also intended to be
// used as a template to be adapted to run other t-code controlled arduino projects
// Have fun, play safe!
// History:
// Alpha1 - First release. 2-2-2023
// Alpha2 - Encoder moved to PIN33, End switch pin removed and start sequence changed. 23-2-2023
// Modifications by Khrull (Fain)


#pragma once

#include <SimpleFOC.h>
#include <SimpleFOCDrivers.h>
#include <encoders/mt6701/MagneticSensorMT6701SSI.h>
#include "TCode0_3.h"
#include "SettingsHandler.h"
#include "Global.h"
#include "MotorHandler0_3.h"
#include "TagHandler.h"
#include "settingsFactory.h"
#include "BLDCTCodeMotor.h"


// Control constants
// (a.k.a. magic numbers for Eve)
#define P_CONST 0.002             // Motor PID proportional constant
#define LOW_PASS 0.8              // Low pass filter factor for static noise reduction ( number < 1, 0 = none)

// // encoder position monitor variables
// volatile int encoderPulseLength = 464;
// volatile int encoderPulseCycle = 920;
// volatile int encoderPulseStart = 0;
// volatile int lastEncoderPulseCycle; 
// // range is 5-928
// volatile int longest = 500;
// volatile int shortest = 500;



// Encoder Interrupt detector
// void IRAM_ATTR encoderChange() {
//     long currentMicros = esp_timer_get_time();
//     if(digitalRead(SettingsHandler::getBLDC_Encoder_PIN()) == HIGH)
//     {
//         encoderPulseCycle = currentMicros-encoderPulseStart;
//         encoderPulseStart = currentMicros;
//     }
//     else
//     {
//         encoderPulseLength = currentMicros-encoderPulseStart;
//     }
// }

class BLDCHandler0_3 : public MotorHandler0_3 {

public:
    BLDCHandler0_3() : MotorHandler0_3(new TCode0_3()) { }

    bool setup() override {
        m_settingsFactory = SettingsFactory::getInstance();
        //PinMapInfo pinMapInfo = m_settingsFactory->getPins();
        m_deviceType = DeviceType::NONE;
        m_settingsFactory->getValue(DEVICE_TYPE, m_deviceType);
        if(m_deviceType == DeviceType::NONE)
        {
            LogHandler::error(m_TAG, "No device type selected. Visit the web config or use the command to set a device before starting the firmware.");
            m_initFailed = true;
            return false;
        }
        BLDCEncoderType encoderType = (BLDCEncoderType)BLDC_ENCODER_DEFAULT;
        m_settingsFactory->getValue(BLDC_ENCODER, encoderType);
        if(encoderType == BLDCEncoderType::NONE)
        {
            LogHandler::error(m_TAG, "No encoder type selected. Visit the web config or use the command to set an encoder before starting the firmware.");
            m_initFailed = true;
            return false;
        }

        if(m_deviceType == DeviceType::SSR1)
        {
            int pullyCircumference = -1;
            m_settingsFactory->getValue(BLDC_PULLEY_CIRCUMFERENCE, pullyCircumference);
            int strokeLength = -1;
            m_settingsFactory->getValue(BLDC_STROKELENGTH, strokeLength);
            int railLength = -1;
            m_settingsFactory->getValue(BLDC_RAILLENGTH, railLength);
            angToPos = (10000*pullyCircumference)/(2*3.14159*strokeLength); // Number to convert a motor angle to a 0-10000 axis position
            LogHandler::debug(m_TAG, "angToPos: %f", angToPos);
            topStartOffset = 2*3.14156*strokeLength/pullyCircumference; // Angle turned by pulley for a full stroke
            LogHandler::debug(m_TAG, "topStartOffset: %f", topStartOffset);
            endStopOffset = 2*3.14159*(railLength-strokeLength)/(2*pullyCircumference);  // Offset angle from bottom endstop on startup (rad)
            LogHandler::debug(m_TAG, "endStopOffset: %f", endStopOffset);
        }
        else
        {
            angToPos = 530.516; // Number to convert a motor angle to a 0-10000 axis position
            LogHandler::debug(m_TAG, "angToPos: %f", angToPos);
            topStartOffset = 2*3.14159*0.05; // Angle turned by pulley for a full stroke
            LogHandler::debug(m_TAG, "topStartOffset: %f", topStartOffset);
            endStopOffset = 1;// Not used on SSR2 currently
        }

        
        PinMapSSR* pinMap = PinMapSSR::getInstance();

        double rightMotorVoltage = BLDC_MOTOR_VOLTAGE_DEFAULT;
        m_settingsFactory->getValue(BLDC_MOTOR_VOLTAGE, rightMotorVoltage);
        // power supply voltage [V]
        double rightSupplyVoltage = BLDC_MOTOR_SUPPLY_DEFAULT;
        m_settingsFactory->getValue(BLDC_MOTOR_SUPPLY, rightSupplyVoltage);
        // limiting motor movements
        double rightMotorCurrent = BLDC_MOTOR_CURRENT_DEFAULT;
        m_settingsFactory->getValue(BLDC_MOTOR_CURRENT, rightMotorCurrent);

        // init current sense
        double zeroElecAngle = BLDC_MOTOR_ZEROELECANGLE_DEFAULT;
        bool paramsKnown = BLDC_MOTOR_PARAMETERSKNOWN_DEFAULT;
        m_settingsFactory->getValue(BLDC_MOTOR_PARAMETERSKNOWN, paramsKnown);
        if(paramsKnown) 
        {
            m_settingsFactory->getValue(BLDC_MOTOR_ZEROELECANGLE, zeroElecAngle);
        }

        motorA = new BLDCTCodeMotor(
            m_deviceType,
            BLDCMotorPosition::A,
            encoderType, 
            pinMap->chipSelect(),
            pinMap->encoder(),
            pinMap->pwmChannel1(),
            pinMap->pwmChannel2(),
            pinMap->pwmChannel3(),
            pinMap->enable(),
            rightMotorVoltage,
            rightSupplyVoltage,
            rightMotorCurrent,
            angToPos, 
            topStartOffset, 
            endStopOffset,
            paramsKnown,
            zeroElecAngle);
                
        if(!motorA->initialized()) 
        {
            m_initFailed = true;
            delete motorA;
            motorA = 0;
            return false;
        }


        if(m_deviceType == DeviceType::SSR2)
        {
            // float leftAngToPos = (10000*pullyCircumference)/(2*3.14159*strokeLength); // Number to convert a motor angle to a 0-10000 axis position
            // LogHandler::debug(m_TAG, "leftAngToPos: %f", leftAngToPos);
            // float leftTopStartOffset = 2*3.14156*strokeLength/pullyCircumference; // Angle turned by pulley for a full stroke
            // LogHandler::debug(m_TAG, "leftTopStartOffset: %f", leftTopStartOffset);
            // float leftEndstopOffset = 2*3.14159*(railLength-strokeLength)/(2*pullyCircumference);  // Offset angle from bottom endstop on startup (rad)
            // LogHandler::debug(m_TAG, "leftEndstopOffset: %f", leftEndstopOffset);

            // Begin tracking encoder
            m_settingsFactory->getValue(BLDC_B_ENCODER, encoderType);
            double leftMotorVoltage = BLDC_B_MOTOR_VOLTAGE_DEFAULT;
            m_settingsFactory->getValue(BLDC_B_MOTOR_VOLTAGE, leftMotorVoltage);
            // power supply voltage [V]
            double leftSupplyVoltage = BLDC_B_MOTOR_SUPPLY_DEFAULT;
            m_settingsFactory->getValue(BLDC_B_MOTOR_SUPPLY, leftSupplyVoltage);
            // limiting motor movements
            double leftMotorCurrent = BLDC_B_MOTOR_CURRENT_DEFAULT;
            m_settingsFactory->getValue(BLDC_B_MOTOR_CURRENT, leftMotorCurrent);

            // init current sense
            paramsKnown = BLDC_B_MOTOR_PARAMETERSKNOWN_DEFAULT;
            zeroElecAngle = BLDC_B_MOTOR_ZEROELECANGLE_DEFAULT;
            m_settingsFactory->getValue(BLDC_B_MOTOR_PARAMETERSKNOWN, paramsKnown);
            if(paramsKnown) 
            {
                m_settingsFactory->getValue(BLDC_B_MOTOR_ZEROELECANGLE, zeroElecAngle);
            }
            motorB = new BLDCTCodeMotor(
                m_deviceType,
                BLDCMotorPosition::B,
                encoderType,
                pinMap->motorBChipSelect(),
                pinMap->motorBEncoder(),
                pinMap->motorBPwmChannel1(),
                pinMap->motorBPwmChannel2(),
                pinMap->motorBPwmChannel3(),
                pinMap->motorBEnable(),
                leftMotorVoltage,
                leftSupplyVoltage,
                leftMotorCurrent,
                angToPos, 
                topStartOffset, 
                endStopOffset,
                paramsKnown,
                zeroElecAngle);

            if(!motorB->initialized()) {
                m_initFailed = true;
                delete motorB;
                motorB = 0;
                return false;
            }
        }

        // Start serial connection and report status
        m_tcode->setup(FIRMWARE_VERSION_NAME);

        // Register device axes

        m_useHallSensor = BLDC_USEHALLSENSOR_DEFAULT;
        m_settingsFactory->getValue(BLDC_USEHALLSENSOR, m_useHallSensor);
        m_hallSensorPin = pinMap->hallEffect();
        if(m_useHallSensor && m_hallSensorPin > -1) {
            pinMode(m_hallSensorPin,INPUT_PULLUP);
        } else if(m_useHallSensor) {
            LogHandler::warning(m_TAG, "Use hall sensor true but pin is invalid %d...ignoring", pinMap->hallEffect());
            // m_settingsFactory->setValue(BLDC_USEHALLSENSOR, m_useHallSensor);
        }
        if(motorA) 
        {
            m_tcode->RegisterAxis(TCODE_CHANNEL_STROKE, "Up");
            bool useHallSensor = BLDC_USEHALLSENSOR_DEFAULT;
            m_settingsFactory->getValue(BLDC_USEHALLSENSOR, useHallSensor);
            int hallSensorPin = pinMap->hallEffect();
        }
        if(motorB)
        {
            m_tcode->RegisterAxis(TCODE_CHANNEL_TWIST, "Twist");
        }
        
        setupCommon(motorB ? TCODE_CHANNEL_TWIST : "");// TODO make this better, im brainfog right now.

        // Signal ready to start
        if(m_initFailed)
            LogHandler::error(m_TAG, "Error in setup");
        else
            LogHandler::info(m_TAG, "Ready!");
        return true;
    }

    void read(byte inByte) override 
    {
        m_tcode->read(inByte);
    }

    void read(const String &input) override 
    {
        m_tcode->read(input);
    }
    
    void read(const char* input, size_t len) override
    {
        for (int i = 0; i < len; i++) 
        {
            read(input[i]);
        }
    }

    void setMessageCallback(std::function<void(const char*)> function) override 
    {
        m_tcode->setMessageCallback(function);
    }


    void execute() override {
        // executeCommon(stroke);
        if(m_initFailed)
            return;
        // Collect inputs
        // These functions query the t-code object for the position/level at a specified time
        // Number recieved will be an integer, 0-9999
        int strokeTCode = channelRead(TCODE_CHANNEL_STROKE);
        if (m_settingsFactory->getInverseStroke())
        {
            strokeTCode = 9999 - strokeTCode;
        }
        // LogHandler::verbose(m_TAG, "strokeTCode: %ld", strokeTCode);

        if(m_deviceType == DeviceType::SSR1)
        {
            executeSSR1(strokeTCode);
        }
        else if(m_deviceType == DeviceType::SSR2)
        {
            int twistTCode = channelRead(TCODE_CHANNEL_TWIST);
            if (m_settingsFactory->getInverseTwist())
            {
                twistTCode = 9999 - twistTCode;
            }
            // LogHandler::verbose(m_TAG, "twistTCode: %ld", twistTCode);
            executeSSR2(strokeTCode, twistTCode);
        }

        executeCommon(strokeTCode);
       
    }

private:

    const char* m_TAG = TagHandler::BLDCHandler;
    bool m_initFailed = false;
    SettingsFactory* m_settingsFactory;

    BLDCTCodeMotor* motorA = 0;
    float zeroAngleA = 0.00;
    float sensorAngleA = 0.00;
    float m_motorAnglePositionA = 0.00;
    float m_targetMotorPositionA = 0.00;
    float motorVoltageA = 0.00;
    BLDCTCodeMotor* motorB = 0;
    float zeroAngleB = 0.00;
    float sensorAngleB = 0.00;
    float m_motorAnglePositionB = 0.00;
    float m_targetMotorPositionB = 0.00;
    float motorVoltageB = 0.00;

    ///SSR2 only
        float strokePosition;
        float twistPosition;
        int startPoint;
    ///

    DeviceType m_deviceType;
    unsigned long startTime = 0;
    bool m_useHallSensor = false;
    int8_t m_hallSensorPin = -1;
    bool m_bootmode = true;
    float angToPos; // Number to convert a motor angle to a 0-10000 axis position
    float topStartOffset; // Angle turned by pulley for a full stroke
    float endStopOffset;  // Offset angle from bottom endstop on startup (rad)

    // Logging limiter!
    unsigned long previousMillis = 0; // variable to store the time of the last report
    const long interval = 10; // interval at which to send reports (in ms)
    int counter = 0;
    bool m_homingMode = false;

    // Until we get these SSR1 and SSR2 laid out lets keep them separate.
    void executeSSR1(int strokeTCode)
    {
        if(m_initFailed)
            return;
        if(!startTime) {
            // Record start time
            startTime = millis();
            LogHandler::verbose(m_TAG, "startTime: %ld", startTime);
        }
        // Run motor FOC loop
        motorA->loopFOC();

        // Update sensor position
        motorA->sensor()->update();
        float angle = motorA->sensor()->getAngle();
        // Determine the linear position of the receiver in (0-10000)
        m_motorAnglePositionA = (angle - zeroAngleA)*angToPos; 
        //LogHandler::verbose(m_TAG, "zeroAngle: %f", zeroAngle);

        // Control by motor voltage
        float motorVoltageNew;
        // Mode 0 is startup mode. 
        // Distance of travel is 12,000 (>10,000) just to make sure that the receiver reaches the top/bottom.
        if (m_bootmode) {
            // If using a hall sensor, roll upwards until the magnet triggers the hall effect sensor
            if (m_useHallSensor) {
                //LogHandler::verbose(m_TAG, "Hall senso millis()-startTime: %ld", millis()-startTime);
                strokeTCode  = map(millis()-startTime,0,2000,0,12000);
                if (!digitalRead(m_hallSensorPin)) {
                    LogHandler::debug(m_TAG, "Set bootmode false read hall");
                    m_bootmode = false;
                    zeroAngleA = angle - topStartOffset;
                } else if (millis() > (startTime + 2000)) {
                    // Timeout after two seconds if sensor not triggered
                    m_bootmode = false;
                    LogHandler::debug(m_TAG, "Set bootmode false hall timeout");
                    zeroAngleA = angle - topStartOffset - endStopOffset;
                }
                motorVoltageNew = P_CONST*(strokeTCode - m_motorAnglePositionA);
            } else {
                // Otherwise roll downwards for two seconds and press against bottom stop.
                // LogHandler::verbose(m_TAG, "millis()-startTime: %ld", millis()-startTime);
                strokeTCode  = map(millis()-startTime,0,2000,0,-12000);
                if (millis() > (startTime + 2000)) {
                    m_bootmode = false;
                    LogHandler::debug(m_TAG, "Set bootmode false NO HALL timeout");
                    zeroAngleA = angle + endStopOffset;
                }
                motorVoltageNew = P_CONST*(strokeTCode - m_motorAnglePositionA);
                if (motorVoltageNew < -0.5) { motorVoltageNew = -0.5; }
            }
        // Otherwise set motor voltage based on position error     
        } else {
            motorVoltageNew = P_CONST*(strokeTCode - m_motorAnglePositionA);
        }
        // Low pass filter to reduce motor noise
        motorVoltageA = LOW_PASS*motorVoltageA + (1-LOW_PASS)*motorVoltageNew;  
        // Motion control function
        motorA->move(motorVoltageA);

        log();
    }

    void executeSSR2(int strokeTCode, int twistTCode)
    { 
        if(m_initFailed)
            return;
        // Run motor FOC loop
        motorA->loopFOC();
        motorB->loopFOC();

        float twistMultiplier = m_settingsFactory->getBLDCTwistMultiplier();
        float twistLimit = 0.5f; //TODO make this a setting

        // Update sensor position
        motorA->sensor()->update();
        float angleA = motorA->sensor()->getAngle();
        motorB->sensor()->update();
        float angleB = motorB->sensor()->getAngle();
        // Determine the linear position of the receiver in (0-10000)
        m_motorAnglePositionA = -(angleA - zeroAngleA)*angToPos;
        m_motorAnglePositionB = (angleB - zeroAngleB)*angToPos;
        strokePosition = 0.5*(m_motorAnglePositionA + m_motorAnglePositionB);
        twistPosition = m_motorAnglePositionB - m_motorAnglePositionA;

        // Position control
        float strokeTargetPosition, twistTargetPosition;
        if (m_bootmode) 
        {
            if(m_useHallSensor)
            {
                strokeTargetPosition = map(millis()-startTime,0,2000,0,12000);
                twistTargetPosition = 0;
                if (!digitalRead(m_hallSensorPin)) 
                {
                    m_bootmode = false;
                    m_homingMode = true;
                    zeroAngleA = angleA + 10000/angToPos;
                    zeroAngleB = angleB - 10000/angToPos;
                    startPoint = 9999;
                    startTime = millis();
                    Serial.println("Ready!");
                } 
                else if (millis() > (startTime + 2000)) 
                {
                    m_bootmode = false;
                    m_homingMode = true;
                    zeroAngleA = angleA + 10000/angToPos + topStartOffset;
                    zeroAngleB = angleB - 10000/angToPos - topStartOffset;
                    startPoint = 9999;
                    startTime = millis();
                    Serial.println("Ready!");
                }
            }
            else
            {
                strokeTargetPosition = map(millis()-startTime,0,2000,0,-12000);
                twistTargetPosition = 0;
                if (millis() > (startTime + 2000)) 
                {
                    m_bootmode = false;
                    m_homingMode = true;
                    zeroAngleA = angleA - topStartOffset;
                    zeroAngleB = angleB + topStartOffset;
                    startPoint = 0;
                    startTime = millis();
                    Serial.println("Ready!");
                }
            }
        } 
        else if (m_homingMode) 
        {
            strokeTargetPosition = map(millis()-startTime,0,2000,startPoint,strokeTCode);
            twistTargetPosition = 0;
            if (millis() > (startTime + 2000)) 
            {
                m_homingMode = false;
            }
        } 
        else 
        {
            // Otherwise set motor voltage based on position error   
            strokeTargetPosition = strokeTCode;
            twistTargetPosition = twistMultiplier*(5000 - twistTCode);
        }

        float strokeVoltage, twistVoltage;
        strokeVoltage = P_CONST*(strokeTargetPosition - strokePosition);
        twistVoltage = P_CONST*(twistTargetPosition - twistPosition);
        if (twistVoltage > twistLimit) {twistVoltage = twistLimit;}
        if (twistVoltage < -twistLimit) {twistVoltage = -twistLimit;}

        // Control by motor voltage
        float motorVoltageNewA;
        motorVoltageNewA = -strokeVoltage + twistVoltage;
        if (m_bootmode) 
        {
            if (motorVoltageNewA < -0.5) { motorVoltageNewA = -0.5; }
            if (motorVoltageNewA > 0.5) { motorVoltageNewA = 0.5; } 
        }

        // Low pass filter to reduce motor noise
        motorVoltageA = 0.8*motorVoltageA + 0.2*motorVoltageNewA;  
        // Motion control function
        motorA->move(motorVoltageA);

            // Control by motor voltage
        float motorVoltageNewB;
        motorVoltageNewB = strokeVoltage + twistVoltage;
        if (m_bootmode) 
        {
            if (motorVoltageNewB < -0.5) { motorVoltageNewB = -0.5; }
            if (motorVoltageNewB > 0.5) { motorVoltageNewB = 0.5; }
        }

        // Low pass filter to reduce motor noise
        motorVoltageB = 0.8*motorVoltageB + 0.2*motorVoltageNewB;  
        // Motion control function
        motorB->move(motorVoltageB);
    }

    void log() 
    {
        if(m_initFailed)
            return;
        if(LogHandler::getLogLevel() == LogLevel::VERBOSE) 
        {
            unsigned long currentMillis = millis();
            if (currentMillis - previousMillis >= interval) 
            {
                previousMillis = currentMillis;
                LogHandler::verbose(m_TAG, "%s motor position: %f \t motorVoltage: %f \t bootmode: %ld \t tcode: %ld \t zeroAngle: %f \t angle: %f\n", motorA->name(), m_motorAnglePositionA, motorVoltageA, m_bootmode, zeroAngleA, sensorAngleA);
                if(motorB)
                    LogHandler::verbose(m_TAG, "%s motor position: %f \t motorVoltage: %f \t bootmode: %ld \t tcode: %ld \t zeroAngle: %f \t angle: %f\n", motorB->name(), m_motorAnglePositionB, motorVoltageB, m_bootmode, zeroAngleB, sensorAngleB);
                counter = 0;
            }
            counter++;
        }
    }
};
