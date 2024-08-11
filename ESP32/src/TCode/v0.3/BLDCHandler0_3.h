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
#include <encoders/MT6701/MagneticSensorMT6701SSI.h>
#include "TCode0_3.h"
#include "../../SettingsHandler.h"
#include "../Global.h"
#include "../MotorHandler.h"
#include "../../TagHandler.h"
#include "../settingsFactory.h"


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

class BLDCHandler0_3 : public MotorHandler {

public:
    BLDCHandler0_3() : MotorHandler(new TCode0_3()) { }

    void setup() override {
        bootmode = true;
        m_settingsFactory = SettingsFactory::getInstance();
        PinMapInfo pinMapInfo = m_settingsFactory->getPins();
        PinMapSSR1* pinMap = pinMapInfo.pinMap<PinMapSSR1*>();
        int pullyCircumference = -1;
        m_settingsFactory->getValue(BLDC_PULLEY_CIRCUMFERENCE, pullyCircumference);
        int strokeLength = -1;
        m_settingsFactory->getValue(BLDC_STROKELENGTH, strokeLength);
        int railLength = -1;
        m_settingsFactory->getValue(BLDC_RAILLENGTH, railLength);
        ANG_TO_POS = (10000*pullyCircumference)/(2*3.14159*strokeLength); // Number to convert a motor angle to a 0-10000 axis position
        LogHandler::debug(_TAG, "ANG_TO_POS: %f", ANG_TO_POS);
        TOP_START_OFFSET = 2*3.14156*strokeLength/pullyCircumference; // Angle turned by pulley for a full stroke
        LogHandler::debug(_TAG, "TOP_START_OFFSET: %f", TOP_START_OFFSET);
        ENDSTOP_START_OFFSET = 2*3.14159*(railLength-strokeLength)/(2*pullyCircumference);  // Offset angle from bottom endstop on startup (rad)
        LogHandler::debug(_TAG, "ENDSTOP_START_OFFSET: %f", ENDSTOP_START_OFFSET);

        // Begin tracking encoder
        BLDCEncoderType encoderType = BLDCEncoderType::MT6701;
        m_settingsFactory->getValue(BLDC_ENCODER, encoderType);
        LogHandler::debug(_TAG, "Encoder type: %ld", encoderType);

        if(encoderType == BLDCEncoderType::MT6701) {
            LogHandler::info(_TAG, "Selected encoder: MT6701");
            m_settingsFactory->getValue(BLDC_ENCODER, encoderType);
            if(pinMap->chipSelect() > -1) {
                LogHandler::info(_TAG, "Setup BLDC motor on MT6701 chip select pin: %ld", pinMap->chipSelect());
                sensorMT6701 = new MagneticSensorMT6701SSI(pinMap->chipSelect());
            } else {
                LogHandler::error(_TAG, "Invalid ChipSelect pin %ld", pinMap->chipSelect());
                m_initFailed = true;
                return;
            }
        } else if(encoderType == BLDCEncoderType::PWM) {
            LogHandler::info(_TAG, "Selected encoder: PWM");
            if(pinMap->encoder() > -1) {
                LogHandler::info(_TAG, "Setup BLDC motor on PWM encoder pin: %ld", pinMap->encoder());
                sensorPWM = new MagneticSensorPWM(pinMap->encoder(), 5, 928);
            } else {
                LogHandler::error(_TAG, "Invalid encoder pin %ld", pinMap->encoder());
                m_initFailed = true;
                return;
            }
        } else {
            if(pinMap->chipSelect() > -1) {
                LogHandler::info(_TAG, "Selected encoder: SPI");
                LogHandler::info(_TAG, "Setup BLDC motor on SPI chip select pin: %ld", pinMap->chipSelect());
                sensorSPI = new MagneticSensorSPI(pinMap->chipSelect(), 14, 0x3FFF);
            } else {
                LogHandler::error(_TAG, "Invalid ChipSelect pin %ld", pinMap->chipSelect());
                m_initFailed = true;
                return;
            }
        }
        // BLDC motor & driver instance
        motorA = new BLDCMotor(11,11.1);
        // BLDCDriver3PWM driver = BLDCDriver3PWM(pwmA, pwmB, pwmC, Enable(optional));
        driverA = new BLDCDriver3PWM(pinMap->pwmChannel1(), pinMap->pwmChannel2(), pinMap->pwmChannel3(), pinMap->enable());

        // Start serial connection and report status
        m_tcode->setup(FIRMWARE_VERSION_NAME, m_settingsFactory->getTcodeVersionString());

        m_tcode->StringInput("D0");
        m_tcode->StringInput("D1");

        // #ESP32# Enable EEPROM
        //EEPROM.begin(320); Done in TCode class

        // Register device axes
        m_tcode->RegisterAxis("L0", "Up");
        m_settingsFactory->getValue(BLDC_USEHALLSENSOR, m_useHallSensor);
        m_hallSensorPin = pinMap->hallEffect();
        if(m_useHallSensor && m_hallSensorPin > -1) {
            LogHandler::info(_TAG, "Using Hall Sensor");
            // Set pinmode for hall sensor
            pinMode(m_hallSensorPin, INPUT_PULLUP);
        } else if(m_useHallSensor) {
            LogHandler::warning(_TAG, "Use hall sensor true but pin is invalid %ld. Reverting to no sensor.", pinMap->hallEffect());
            m_useHallSensor = false;
            m_settingsFactory->setValue(BLDC_USEHALLSENSOR, m_useHallSensor);
        }
        
        // initialise encoder hardware
        if(sensorMT6701) {
            sensorMT6701->init();
            LogHandler::debug(_TAG, "init sensorMT6701");
        } else if (sensorPWM) { 
            sensorPWM->init(); 
            LogHandler::debug(_TAG, "init sensorPWM");
        } else { 
            sensorSPI->init(); 
            LogHandler::debug(_TAG, "init sensorSPI");
        }
        
        // driver config
        // power supply voltage [V]
        double motorAVoltage = BLDC_MOTORA_VOLTAGE_DEFAULT;
        m_settingsFactory->getValue(BLDC_MOTORA_VOLTAGE, motorAVoltage);
        LogHandler::debug(_TAG, "Voltage: %f", motorAVoltage);
        driverA->voltage_power_supply = motorAVoltage;
        // Max DC voltage allowed - default voltage_power_supply
        driverA->voltage_limit = 20;
        // driver init
        driverA->init();

        // limiting motor movements
        double motorACurrent = BLDC_MOTORA_CURRENT_DEFAULT;
        m_settingsFactory->getValue(BLDC_MOTORA_CURRENT, motorACurrent);
        LogHandler::debug(_TAG, "Current: %f", motorACurrent);
        motorA->current_limit = motorACurrent;   // [Amps]
        motorA->voltage_limit = motorAVoltage;  

        // set control loop type to be used
        motorA->torque_controller = TorqueControlType::voltage;
        motorA->controller = MotionControlType::torque;

        // link the motor to the sensor
        if(sensorMT6701) {
            motorA->linkSensor(sensorMT6701); 
            LogHandler::debug(_TAG, "linkSensor sensorMT6701");
        } else if (sensorPWM) { 
            motorA->linkSensor(sensorPWM); 
            LogHandler::debug(_TAG, "linkSensor sensorPWM");
        } else { 
            motorA->linkSensor(sensorSPI); 
            LogHandler::debug(_TAG, "linkSensor sensorSPI");
        }
        // link the motor and the driver
        motorA->linkDriver(driverA);

        // initialize motor
        motorA->init();
        motorA->useMonitoring(Serial);

        // init current sense
        bool paramsKnown = BLDC_MOTORA_PARAMETERSKNOWN_DEFAULT;
        m_settingsFactory->getValue(BLDC_MOTORA_PARAMETERSKNOWN, paramsKnown);
        if(paramsKnown) {
            double zeroElecAngle = BLDC_MOTORA_ZEROELECANGLE_DEFAULT;
            m_settingsFactory->getValue(BLDC_MOTORA_ZEROELECANGLE, zeroElecAngle);
        // Set sensor angle and pre-set zero angle to current angle
            LogHandler::info(_TAG, "Setting MotorA parameters: %f", zeroElecAngle);
            motorA->sensor_direction = MotorA_SensorDirection;
            motorA->zero_electric_angle  = zeroElecAngle; // rad
        }

        if (motorA->initFOC())  {
            LogHandler::info(_TAG, "FOC init success!");
        } else {
            LogHandler::error(_TAG, "FOC init failed!");
            //return;
            m_initFailed = true;
        }
        LogHandler::info(_TAG, "BLDC_MotorA_ZeroElecAngle %f", motorA->zero_electric_angle);

        
        // link the motor to the sensor
        if(sensorMT6701) {
            sensorMT6701->update();
            zeroAngle = sensorMT6701->getAngle();
            LogHandler::debug(_TAG, "MT6701 zeroAngle: %ld", zeroAngle);
        } else if (sensorPWM) { 
            sensorPWM->update(); 
            zeroAngle = sensorPWM->getAngle();
            LogHandler::debug(_TAG, "PWM zeroAngle: %ld", zeroAngle);
        } else { 
            sensorSPI->update();
            zeroAngle = sensorSPI->getAngle();
            LogHandler::debug(_TAG, "SPI zeroAngle: %ld", zeroAngle);
        }


        setupCommon();

        // Signal ready to start
        if(m_initFailed)
            LogHandler::info(_TAG, "Error in setup");
        else
            LogHandler::info(_TAG, "Ready!");
    }

    void read(byte inByte) override {
        m_tcode->ByteInput(inByte);
    }

    void read(const String &input) override {
        m_tcode->StringInput(input);
    }
    
    void read(const char* &input, size_t len) override
    {
        for (int i = 0; i < len; i++) {
            read(input[i]);
        }
    }

    void setMessageCallback(TCODE_FUNCTION_PTR_T function) override {
        m_tcode->setMessageCallback(function);
    }


    void execute() override {

        if(m_initFailed) {
            return;
        }
        if(!startTime) {
            // Record start time
            startTime = millis();
            LogHandler::verbose(_TAG, "startTime: %ld", startTime);
        }
        // Run motor FOC loop
        motorA->loopFOC();

        // Collect inputs
        // These functions query the t-code object for the position/level at a specified time
        // Number recieved will be an integer, 0-9999
        int xLin = m_tcode->AxisRead("L0");
        //LogHandler::verbose(_TAG, "xLin: %ld", xLin);


        // Update sensor position
        float angle;
        if(sensorMT6701) {
            sensorMT6701->update();
            angle = sensorMT6701->getAngle();
            //LogHandler::verbose(_TAG, "update MT6701 angle: %f", angle);
        } else if (sensorPWM) { 
            sensorPWM->update();
            angle = sensorPWM->getAngle();
            //LogHandler::verbose(_TAG, "update PWM angle: %f", angle);
        } else {
            sensorSPI->update();
            angle = sensorSPI->getAngle();
            //LogHandler::verbose(_TAG, "update SPI angle: %f", angle);
        }
        // Determine the linear position of the receiver in (0-10000)
        xPosition = (angle - zeroAngle)*ANG_TO_POS; 
        //LogHandler::verbose(_TAG, "zeroAngle: %f", zeroAngle);

        // Control by motor voltage
        float motorVoltageNew;
        // Mode 0 is startup mode. 
        // Distance of travel is 12,000 (>10,000) just to make sure that the receiver reaches the top/bottom.
        if (bootmode) {
            // If using a hall sensor, roll upwards until the magnet triggers the hall effect sensor
            if (m_useHallSensor) {
                //LogHandler::verbose(_TAG, "Hall senso millis()-startTime: %ld", millis()-startTime);
                xLin  = map(millis()-startTime,0,2000,0,12000);
                if (!digitalRead(m_hallSensorPin)) {
                    LogHandler::debug(_TAG, "Set bootmode false read hall");
                    bootmode = false;
                    zeroAngle = angle - TOP_START_OFFSET;
                } else if (millis() > (startTime + 2000)) {
                    // Timeout after two seconds if sensor not triggered
                    bootmode = false;
                    LogHandler::debug(_TAG, "Set bootmode false hall timeout");
                    zeroAngle = angle - TOP_START_OFFSET - ENDSTOP_START_OFFSET;
                }
                motorVoltageNew = P_CONST*(xLin - xPosition);
            } else {
                // Otherwise roll downwards for two seconds and press against bottom stop.
                // LogHandler::verbose(_TAG, "millis()-startTime: %ld", millis()-startTime);
                xLin  = map(millis()-startTime,0,2000,0,-12000);
                if (millis() > (startTime + 2000)) {
                    bootmode = false;
                    LogHandler::debug(_TAG, "Set bootmode false NO HALL timeout");
                    zeroAngle = angle + ENDSTOP_START_OFFSET;
                }
                motorVoltageNew = P_CONST*(xLin - xPosition);
                if (motorVoltageNew < -0.5) { motorVoltageNew = -0.5; }
            }
        // Otherwise set motor voltage based on position error     
        } else {
            motorVoltageNew = P_CONST*(xLin - xPosition);
        }
        // Low pass filter to reduce motor noise
        motorVoltage = LOW_PASS*motorVoltage + (1-LOW_PASS)*motorVoltageNew;  
        // Motion control function
        motorA->move(motorVoltage);

        if(LogHandler::getLogLevel() == LogLevel::VERBOSE) {
            unsigned long currentMillis = millis();
            if (currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;
                LogHandler::verbose(_TAG, "xPosition: %f \t motorVoltage: %f \t bootmode: %ld \t xLin: %ld \t zeroAngle: %f \t angle: %f\n", xPosition, motorVoltage, bootmode, xLin, zeroAngle, angle);
                counter = 0;
            }
            counter++;
        }

        executeCommon(xLin);
       
    }

private:

    const char* _TAG = TagHandler::BLDCHandler;
    bool m_initFailed = false;
    SettingsFactory* m_settingsFactory;
    bool m_useHallSensor = false;
    int8_t m_hallSensorPin = -1;
    // Drive Parameters

    // The control code needs to know the angle of the motor relative to the encoder - "Zero elec. angle".
    // If a value is not entered it will perform a quick operation on startup to estimate this.
    // This will be displayed in the serial monitor each time the device starts up.
    // If the device is noticably faster in one direction the angle is out of alignment, try increasing or decreasing it by small increments (eg +/- 0.1).
    Direction MotorA_SensorDirection = Direction::CW; // Do not change. If the motor is showing CCW rotate the motor connector 180 degrees to reverse the motor.

    // BLDC motor & driver instance
    BLDCMotor* motorA;
    // BLDCDriver3PWM driver = BLDCDriver3PWM(pwmA, pwmB, pwmC, Enable(optional));
    BLDCDriver3PWM* driverA;
    // Declare a PWM and an SPI sensor. Only one will be used.
    MagneticSensorMT6701SSI* sensorMT6701 = 0;
    MagneticSensorPWM* sensorPWM = 0;
    MagneticSensorSPI* sensorSPI = 0;

    // Position variables
    float zeroAngle = 0.00;
    float xPosition = 0.00;
    bool bootmode = true;
    unsigned long startTime = 0;
    float motorVoltage = 0.00;

    // IGNORE!
    unsigned long previousMillis = 0; // variable to store the time of the last report
    const long interval = 10; // interval at which to send reports (in ms)
    int counter = 0;

    // Derived constants
    float ANG_TO_POS; // Number to convert a motor angle to a 0-10000 axis position
    float TOP_START_OFFSET; // Angle turned by pulley for a full stroke
    float ENDSTOP_START_OFFSET;  // Offset angle from bottom endstop on startup (rad)
};
