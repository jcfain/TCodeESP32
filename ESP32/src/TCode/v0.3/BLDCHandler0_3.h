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
            LogHandler::error(_TAG, "No device type selected. Visit the web config or use the command to set a device before starting the firmware.");
            return false;
        }
        BLDCEncoderType encoderType = (BLDCEncoderType)BLDC_ENCODER_DEFAULT;
        m_settingsFactory->getValue(BLDC_ENCODER, encoderType);
        if(encoderType == BLDCEncoderType::NONE)
        {
            LogHandler::error(_TAG, "No encoder type selected. Visit the web config or use the command to set an encoder before starting the firmware.");
            return false;
        }

        float angToPos,topStartOffset,endstopOffset;
        if(m_deviceType == DeviceType::SSR1)
        {
            int pullyCircumference = -1;
            m_settingsFactory->getValue(BLDC_PULLEY_CIRCUMFERENCE, pullyCircumference);
            int strokeLength = -1;
            m_settingsFactory->getValue(BLDC_STROKELENGTH, strokeLength);
            int railLength = -1;
            m_settingsFactory->getValue(BLDC_RAILLENGTH, railLength);
            angToPos = (10000*pullyCircumference)/(2*3.14159*strokeLength); // Number to convert a motor angle to a 0-10000 axis position
            LogHandler::debug(_TAG, "angToPos: %f", angToPos);
            topStartOffset = 2*3.14156*strokeLength/pullyCircumference; // Angle turned by pulley for a full stroke
            LogHandler::debug(_TAG, "topStartOffset: %f", topStartOffset);
            endstopOffset = 2*3.14159*(railLength-strokeLength)/(2*pullyCircumference);  // Offset angle from bottom endstop on startup (rad)
            LogHandler::debug(_TAG, "endstopOffset: %f", endstopOffset);
        }
        else
        {
            angToPos = 530.516; // Number to convert a motor angle to a 0-10000 axis position
            LogHandler::debug(_TAG, "angToPos: %f", angToPos);
            topStartOffset = 2*3.14159*0.05; // Angle turned by pulley for a full stroke
            LogHandler::debug(_TAG, "topStartOffset: %f", topStartOffset);
            endstopOffset = 1;// Not used on SSR2 currently
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
            endstopOffset,
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
            // LogHandler::debug(_TAG, "leftAngToPos: %f", leftAngToPos);
            // float leftTopStartOffset = 2*3.14156*strokeLength/pullyCircumference; // Angle turned by pulley for a full stroke
            // LogHandler::debug(_TAG, "leftTopStartOffset: %f", leftTopStartOffset);
            // float leftEndstopOffset = 2*3.14159*(railLength-strokeLength)/(2*pullyCircumference);  // Offset angle from bottom endstop on startup (rad)
            // LogHandler::debug(_TAG, "leftEndstopOffset: %f", leftEndstopOffset);

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
                endstopOffset,
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

        bool useHallSensor = BLDC_USEHALLSENSOR_DEFAULT;
        m_settingsFactory->getValue(BLDC_USEHALLSENSOR, useHallSensor);
        int hallSensorPin = pinMap->hallEffect();
        if(motorA) 
        {
            m_tcode->RegisterAxis(TCODE_CHANNEL_STROKE, "Up");
            bool useHallSensor = BLDC_USEHALLSENSOR_DEFAULT;
            m_settingsFactory->getValue(BLDC_USEHALLSENSOR, useHallSensor);
            int hallSensorPin = pinMap->hallEffect();
            if(useHallSensor && hallSensorPin > -1) {
                motorA->useHallSensor(hallSensorPin);
            } else if(useHallSensor) {
                LogHandler::warning(_TAG, "Use hall sensor true but pin is invalid %d...ignoring", pinMap->hallEffect());
                // m_settingsFactory->setValue(BLDC_USEHALLSENSOR, m_useHallSensor);
            }
        }
        if(motorB)
        {
            if(useHallSensor && hallSensorPin > -1) {
                motorB->useHallSensor(hallSensorPin);
            }
            m_tcode->RegisterAxis(TCODE_CHANNEL_TWIST, "Twist");
        }
        
        setupCommon(motorB ? TCODE_CHANNEL_TWIST : "");// TODO make this better, im brainfog right now.

        // Signal ready to start
        if(m_initFailed)
            LogHandler::error(_TAG, "Error in setup");
        else
            LogHandler::info(_TAG, "Ready!");
        return true;
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

    void setMessageCallback(std::function<void(const char*)> function) override 
    {
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
            if(motorA && motorA->initialized())
            {
                motorA->bootCalibrate();
                motorA->update();
                motorA->process(stroke);
                motorA->move();
            }
        } 
        else if(m_deviceType == DeviceType::SSR2)
        {
            int twist = channelRead(TCODE_CHANNEL_TWIST);
            if (m_settingsFactory->getInverseTwist())
            {
                twist = TCODE_MAX - twist;
            }
            if(motorA && motorA->initialized())
            {
                motorA->bootCalibrate();
                motorA->update();
                motorA->process(stroke, twist, m_settingsFactory->getBLDCTwistMultiplier());
            }
            if(motorB && motorB->initialized())
            {
                motorB->bootCalibrate();
                motorB->update();
                motorB->process(stroke, twist, m_settingsFactory->getBLDCTwistMultiplier());
            }

            if(motorA && motorA->initialized())
                motorA->move();
            if(motorB && motorB->initialized())
                motorB->move();
        }

        executeCommon(stroke);
       
    }

private:

    const char* _TAG = TagHandler::BLDCHandler;
    bool m_initFailed = false;
    SettingsFactory* m_settingsFactory;

    BLDCTCodeMotor* motorA = 0;
    BLDCTCodeMotor* motorB = 0;

    DeviceType m_deviceType;
};
