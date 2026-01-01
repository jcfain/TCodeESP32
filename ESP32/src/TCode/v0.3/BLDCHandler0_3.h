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

    void setup() override {
        m_settingsFactory = SettingsFactory::getInstance();
        //PinMapInfo pinMapInfo = m_settingsFactory->getPins();
        int pullyCircumference = -1;
        m_settingsFactory->getValue(BLDC_PULLEY_CIRCUMFERENCE, pullyCircumference);
        int strokeLength = -1;
        m_settingsFactory->getValue(BLDC_STROKELENGTH, strokeLength);
        int railLength = -1;
        m_settingsFactory->getValue(BLDC_RAILLENGTH, railLength);
        float strokeAngToPos = (10000*pullyCircumference)/(2*3.14159*strokeLength); // Number to convert a motor angle to a 0-10000 axis position
        LogHandler::debug(_TAG, "strokeAngToPos: %f", strokeAngToPos);
        float strokeTopStartOffset = 2*3.14156*strokeLength/pullyCircumference; // Angle turned by pulley for a full stroke
        LogHandler::debug(_TAG, "strokeTopStartOffset: %f", strokeTopStartOffset);
        float strokeEndstopOffset = 2*3.14159*(railLength-strokeLength)/(2*pullyCircumference);  // Offset angle from bottom endstop on startup (rad)
        LogHandler::debug(_TAG, "strokeEndstopOffset: %f", strokeEndstopOffset);

        m_deviceType = DeviceType::SSR1;
        m_settingsFactory->getValue(DEVICE_TYPE, m_deviceType);
        
        PinMapSSR* pinMap = PinMapSSR::getInstance();

        BLDCEncoderType encoderType = (BLDCEncoderType)BLDC_ENCODER_DEFAULT;
        m_settingsFactory->getValue(BLDC_ENCODER, encoderType);
        double strokeMotorAVoltage = BLDC_MOTOR_VOLTAGE_DEFAULT;
        m_settingsFactory->getValue(BLDC_MOTOR_VOLTAGE, strokeMotorAVoltage);
        // power supply voltage [V]
        double strokeSupplyAVoltage = BLDC_MOTOR_SUPPLY_DEFAULT;
        m_settingsFactory->getValue(BLDC_MOTOR_SUPPLY, strokeSupplyAVoltage);
        // limiting motor movements
        double strokeMotorACurrent = BLDC_MOTOR_CURRENT_DEFAULT;
        m_settingsFactory->getValue(BLDC_MOTOR_CURRENT, strokeMotorACurrent);

        // init current sense
        double zeroElecAngle = BLDC_MOTOR_ZEROELECANGLE_DEFAULT;
        bool paramsKnown = BLDC_MOTOR_PARAMETERSKNOWN_DEFAULT;
        m_settingsFactory->getValue(BLDC_MOTOR_PARAMETERSKNOWN, paramsKnown);
        if(paramsKnown) {
            m_settingsFactory->getValue(BLDC_MOTOR_ZEROELECANGLE, zeroElecAngle);
        }

        rightMotor = new BLDCTCodeMotor(
            m_deviceType,
            BLDCMotorPosition::Right,
            encoderType, 
            pinMap->chipSelect(),
            pinMap->encoder(),
            pinMap->pwmChannel1(),
            pinMap->pwmChannel2(),
            pinMap->pwmChannel3(),
            pinMap->enable(),
            strokeMotorAVoltage,
            strokeSupplyAVoltage,
            strokeMotorACurrent,
            strokeAngToPos, 
            strokeTopStartOffset, 
            strokeEndstopOffset,
            paramsKnown,
            zeroElecAngle);
                
        if(!rightMotor->initialized()) {
            m_initFailed = true;
            delete rightMotor;
            rightMotor = 0;
        }


        if(m_deviceType == DeviceType::SSR2)
        {
            float leftAngToPos = (10000*pullyCircumference)/(2*3.14159*strokeLength); // Number to convert a motor angle to a 0-10000 axis position
            LogHandler::debug(_TAG, "leftAngToPos: %f", strokeAngToPos);
            float leftTopStartOffset = 2*3.14156*strokeLength/pullyCircumference; // Angle turned by pulley for a full stroke
            LogHandler::debug(_TAG, "leftTopStartOffset: %f", strokeTopStartOffset);
            float leftEndstopOffset = 2*3.14159*(railLength-strokeLength)/(2*pullyCircumference);  // Offset angle from bottom endstop on startup (rad)
            LogHandler::debug(_TAG, "leftEndstopOffset: %f", strokeEndstopOffset);

            // Begin tracking encoder
            m_settingsFactory->getValue(BLDC_LEFT_ENCODER, encoderType);
            double leftMotorAVoltage = BLDC_LEFT_MOTOR_VOLTAGE_DEFAULT;
            m_settingsFactory->getValue(BLDC_LEFT_MOTOR_VOLTAGE, leftMotorAVoltage);
            // power supply voltage [V]
            double leftSupplyAVoltage = BLDC_LEFT_MOTOR_SUPPLY_DEFAULT;
            m_settingsFactory->getValue(BLDC_LEFT_MOTOR_SUPPLY, leftSupplyAVoltage);
            // limiting motor movements
            double leftMotorACurrent = BLDC_LEFT_MOTOR_CURRENT_DEFAULT;
            m_settingsFactory->getValue(BLDC_LEFT_MOTOR_CURRENT, leftMotorACurrent);

            // init current sense
            paramsKnown = BLDC_LEFT_MOTOR_PARAMETERSKNOWN_DEFAULT;
            zeroElecAngle = BLDC_LEFT_MOTOR_ZEROELECANGLE_DEFAULT;
            m_settingsFactory->getValue(BLDC_LEFT_MOTOR_PARAMETERSKNOWN, paramsKnown);
            if(paramsKnown) {
                m_settingsFactory->getValue(BLDC_LEFT_MOTOR_ZEROELECANGLE, zeroElecAngle);
            }
            leftMotor = new BLDCTCodeMotor(
                m_deviceType,
                BLDCMotorPosition::Left,
                encoderType,
                pinMap->leftChipSelect(),
                pinMap->leftEncoder(),
                pinMap->leftPwmChannel1(),
                pinMap->leftPwmChannel2(),
                pinMap->leftPwmChannel3(),
                pinMap->leftEnable(),
                leftMotorAVoltage,
                leftSupplyAVoltage,
                leftMotorACurrent,
                leftAngToPos, 
                leftTopStartOffset, 
                leftEndstopOffset,
                paramsKnown,
                zeroElecAngle);

            if(!leftMotor->initialized()) {
                m_initFailed = true;
                delete leftMotor;
                leftMotor = 0;
            }
        }

        // Start serial connection and report status
        m_tcode->setup(FIRMWARE_VERSION_NAME);

        // Register device axes
        if(rightMotor)
        {
            m_tcode->RegisterAxis(TCODE_CHANNEL_STROKE, "Up");
            bool useHallSensor = BLDC_USEHALLSENSOR_DEFAULT;
            m_settingsFactory->getValue(BLDC_USEHALLSENSOR, useHallSensor);
            int hallSensorPin = pinMap->hallEffect();
            if(useHallSensor && hallSensorPin > -1) {
                rightMotor->useHallSensor(hallSensorPin);
            } else if(useHallSensor) {
                LogHandler::warning(_TAG, "Use hall sensor true but pin is invalid %d...ignoring", pinMap->hallEffect());
                // m_settingsFactory->setValue(BLDC_USEHALLSENSOR, m_useHallSensor);
            }
        }

        if(leftMotor) 
        {
            m_tcode->RegisterAxis(TCODE_CHANNEL_TWIST, "Twist");
            bool useHallSensor = BLDC_USEHALLSENSOR_DEFAULT;
            m_settingsFactory->getValue(BLDC_LEFT_USEHALLSENSOR, useHallSensor);
            int hallSensorPin = pinMap->leftHallEffect();
            if(useHallSensor && hallSensorPin > -1) {
                leftMotor->useHallSensor(hallSensorPin);
            } else if(useHallSensor) {
                LogHandler::warning(_TAG, "Use hall sensor true but pin is invalid %d...ignoring", pinMap->hallEffect());
                // m_settingsFactory->setValue(BLDC_USEHALLSENSOR, m_useHallSensor);
            }
        }
        
        setupCommon(leftMotor ? TCODE_CHANNEL_TWIST : "");// TODO make this better, im brainfog right now.

        // Signal ready to start
        if(m_initFailed)
            LogHandler::error(_TAG, "Error in setup");
        else
            LogHandler::info(_TAG, "Ready!");
    }

    void read(byte inByte) override {
        m_tcode->read(inByte);
    }

    void read(const String &input) override {
        m_tcode->read(input);
    }
    
    void read(const char* input, size_t len) override
    {
        for (int i = 0; i < len; i++) {
            read(input[i]);
        }
    }

    void setMessageCallback(TCODE_FUNCTION_PTR_T function) override {
        m_tcode->setMessageCallback(function);
    }


    void execute() override {
        // Collect inputs
        // These functions query the t-code object for the position/level at a specified time
        // Number recieved will be an integer, {{TCODE_MIN}}-{{TCODE_MAX}}
        int stroke = channelRead(TCODE_CHANNEL_STROKE);
        if (m_settingsFactory->getInverseStroke())
        {
            stroke = TCODE_MAX - stroke;
        }
        //LogHandler::verbose(_TAG, "stroke: %ld", stroke);


        if(m_deviceType == DeviceType::SSR1)
        {
            if(rightMotor && rightMotor->initialized())
            {
                rightMotor->bootCalibrate();
                rightMotor->update();
                rightMotor->process(stroke);
                rightMotor->move();
            }
        } 
        else if(m_deviceType == DeviceType::SSR2)
        {
            int twist = channelRead(TCODE_CHANNEL_TWIST);
            if (m_settingsFactory->getInverseTwist())
            {
                twist = TCODE_MAX - twist;
            }
            if(leftMotor && leftMotor->initialized())
            {
                leftMotor->bootCalibrate();
                leftMotor->update();
                leftMotor->process(stroke, twist, m_settingsFactory->getBLDCTwistMultiplier());
            }
            if(rightMotor && rightMotor->initialized())
            {
                rightMotor->bootCalibrate();
                rightMotor->update();
                rightMotor->process(stroke, twist, m_settingsFactory->getBLDCTwistMultiplier());
            }

            if(rightMotor && rightMotor->initialized())
                rightMotor->move();
            if(leftMotor && leftMotor->initialized())
                leftMotor->move();
        }

        executeCommon(stroke);
       
    }

private:

    const char* _TAG = TagHandler::BLDCHandler;
    bool m_initFailed = false;
    SettingsFactory* m_settingsFactory;

    BLDCTCodeMotor* rightMotor = 0;
    BLDCTCodeMotor* leftMotor = 0;

    DeviceType m_deviceType;
};
