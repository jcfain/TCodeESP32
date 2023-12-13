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


// Control constants
// (a.k.a. magic numbers for Eve)
#define RAIL_LENGTH 125           // Physical maximum travel of the receiver (mm)
#define STROKE_LENGTH 120         // Operating stroke length (mm)
#define P_CONST 0.002             // Motor PID proportional constant
#define LOW_PASS 0.8              // Low pass filter factor for static noise reduction ( number < 1, 0 = none)

// encoder position monitor variables
volatile int encoderPulseLength = 464;
volatile int encoderPulseCycle = 920;
volatile int encoderPulseStart = 0;
volatile int lastEncoderPulseCycle; 
// range is 5-928
volatile int longest = 500;
volatile int shortest = 500;



// Encoder Interrupt detector
// void IRAM_ATTR encoderChange() {
//     long currentMicros = esp_timer_get_time();
//     if(digitalRead(SettingsHandler::BLDC_Encoder_PIN) == HIGH)
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
        ANG_TO_POS = (10000*SettingsHandler::BLDC_Pulley_Circumference)/(2*3.14159*STROKE_LENGTH); // Number to convert a motor angle to a 0-10000 axis position
        LogHandler::debug(_TAG, "ANG_TO_POS: %ld", ANG_TO_POS);
        TOP_START_OFFSET = 2*3.14156*STROKE_LENGTH/SettingsHandler::BLDC_Pulley_Circumference; // Angle turned by pulley for a full stroke
        LogHandler::debug(_TAG, "TOP_START_OFFSET: %ld", TOP_START_OFFSET);
        ENDSTOP_START_OFFSET = 2*3.14159*(RAIL_LENGTH-STROKE_LENGTH)/(2*SettingsHandler::BLDC_Pulley_Circumference);  // Offset angle from bottom endstop on startup (rad)
        LogHandler::debug(_TAG, "ENDSTOP_START_OFFSET: %ld", ENDSTOP_START_OFFSET);

        // Begin tracking encoder
        if(SettingsHandler::BLDC_UseMT6701) {
            LogHandler::info(_TAG, "Setup BLDC motor on MT6701 chip select pin: %ld", SettingsHandler::BLDC_ChipSelect_PIN);
            sensorMT6701 = new MagneticSensorMT6701SSI(SettingsHandler::BLDC_ChipSelect_PIN);
        } else if(SettingsHandler::BLDC_UsePWM) {
            LogHandler::info(_TAG, "Setup BLDC motor on PWM encoder pin: %ld", SettingsHandler::BLDC_Encoder_PIN);
            sensorPWM = new MagneticSensorPWM(SettingsHandler::BLDC_Encoder_PIN, 5, 928);
        } else {
            LogHandler::info(_TAG, "Setup BLDC motor on SPI chip select pin: %ld", SettingsHandler::BLDC_ChipSelect_PIN);
            sensorSPI = new MagneticSensorSPI(SettingsHandler::BLDC_ChipSelect_PIN, 14, 0x3FFF);
        }
        // BLDC motor & driver instance
        motorA = new BLDCMotor(11,11.1);
        // BLDCDriver3PWM driver = BLDCDriver3PWM(pwmA, pwmB, pwmC, Enable(optional));
        driverA = new BLDCDriver3PWM(SettingsHandler::BLDC_PWMchannel1_PIN, SettingsHandler::BLDC_PWMchannel2_PIN, SettingsHandler::BLDC_PWMchannel3_PIN, SettingsHandler::BLDC_Enable_PIN);

        // Start serial connection and report status
        m_tcode->setup(SettingsHandler::ESP32Version, SettingsHandler::TCodeVersionName.c_str());

        m_tcode->StringInput("D0");
        m_tcode->StringInput("D1");

        // #ESP32# Enable EEPROM
        //EEPROM.begin(320); Done in TCode class

        // Register device axes
        m_tcode->RegisterAxis("L0", "Up");

        if(SettingsHandler::BLDC_UseHallSensor) {
            // Set pinmode for hall sensor
            pinMode(SettingsHandler::HallEffect_PIN, INPUT_PULLUP);
        }

        // Set Starting state
        zeroAngle = 0;
        mode = 0;
        
        // initialise encoder hardware
        if(sensorMT6701) {
            sensorMT6701->init();
        } else if (sensorPWM) { 
            sensorPWM->init(); 
        } else { 
            sensorSPI->init(); 
        }
        
        // driver config
        // power supply voltage [V]
        LogHandler::debug(_TAG, "Voltage: %f", SettingsHandler::BLDC_MotorA_Voltage);
        driverA->voltage_power_supply = SettingsHandler::BLDC_MotorA_Voltage;
        // Max DC voltage allowed - default voltage_power_supply
        driverA->voltage_limit = 20;
        // driver init
        driverA->init();

        // limiting motor movements
        LogHandler::debug(_TAG, "Current: %f", SettingsHandler::BLDC_MotorA_Current);
        motorA->current_limit = SettingsHandler::BLDC_MotorA_Current;   // [Amps]

        // set control loop type to be used
        motorA->torque_controller = TorqueControlType::voltage;
        motorA->controller = MotionControlType::torque;

        // link the motor to the sensor
        if(sensorMT6701) {
            motorA->linkSensor(sensorMT6701); 
        } else if (sensorPWM) { 
            motorA->linkSensor(sensorPWM); 
        } else { 
            motorA->linkSensor(sensorSPI); 
        }
        // link the motor and the driver
        motorA->linkDriver(driverA);

        // initialize motor
        motorA->init();
        motorA->useMonitoring(Serial);
        if(SettingsHandler::BLDC_MotorA_ParametersKnown) {
            motorA->initFOC(SettingsHandler::BLDC_MotorA_ZeroElecAngle, MotorA_SensorDirection);
        } else {
            motorA->initFOC();
        }
        
        // Set sensor angle and pre-set zero angle to current angle
        
        // link the motor to the sensor
        if(sensorMT6701) {
            sensorMT6701->update();
            zeroAngle = sensorMT6701->getAngle();
            LogHandler::verbose(_TAG, "MT6701 zeroAngle: %ld", zeroAngle);
        } else if (sensorPWM) { 
            sensorPWM->update(); 
            zeroAngle = sensorPWM->getAngle();
            LogHandler::verbose(_TAG, "PWM zeroAngle: %ld", zeroAngle);
        } else { 
            sensorSPI->update();
            zeroAngle = sensorSPI->getAngle();
            LogHandler::verbose(_TAG, "SPI zeroAngle: %ld", zeroAngle);
        }

        // Record start time
        startTime = millis();
        LogHandler::verbose(_TAG, "startTime: %ld", startTime);

        setupCommon();

        // Signal ready to start
        LogHandler::info(_TAG, "Ready!");
    }

    void read(byte inByte) override {
        m_tcode->ByteInput(inByte);
    }

    void read(String inString) override {
        m_tcode->StringInput(inString);
    }

    void setMessageCallback(TCODE_FUNCTION_PTR_T function) override {
        m_tcode->setMessageCallback(function);
    }


    void execute() override {

        // Run motor FOC loop
        motorA->loopFOC();

        // Collect inputs
        // These functions query the t-code object for the position/level at a specified time
        // Number recieved will be an integer, 0-9999
        int xLin = m_tcode->AxisRead("L0");
        LogHandler::verbose(_TAG, "xLin: %ld", xLin);


        // Update sensor position
        float angle;
        if(sensorMT6701) {
            sensorMT6701->update();
            angle = sensorMT6701->getAngle();
            LogHandler::verbose(_TAG, "update MT6701: %f", angle);
            Serial.printf("update MT6701: %f\n", angle);
        } else if (sensorPWM) { 
            sensorPWM->update();
            angle = sensorPWM->getAngle();
            LogHandler::verbose(_TAG, "update PWM: %f", angle);
        } else {
            sensorSPI->update();
            angle = sensorSPI->getAngle();
            LogHandler::verbose(_TAG, "update SPI: %f", angle);
        }
        // Determine the linear position of the receiver in (0-10000)
        xPosition = (angle - zeroAngle)*ANG_TO_POS; 
        LogHandler::verbose(_TAG, "zeroAngle: %f", zeroAngle);

        // Control by motor voltage
        float motorVoltageNew;
        // Mode 0 is startup mode. 
        // Distance of travel is 12,000 (>10,000) just to make sure that the receiver reaches the top/bottom.
        if (mode == 0) {
            // If using a hall sensor, roll upwards until the magnet triggers the hall effect sensor
            if (SettingsHandler::BLDC_UseHallSensor) {
                xLin  = map(millis()-startTime,0,2000,0,12000);
                if (!digitalRead(SettingsHandler::HallEffect_PIN)) {
                    mode = 1;
                    zeroAngle = angle - TOP_START_OFFSET;
                } else if (millis() > (startTime + 2000)) {
                    // Timeout after two seconds if sensor not triggered
                    mode = 1;
                    zeroAngle = angle - TOP_START_OFFSET - ENDSTOP_START_OFFSET;
                }
                motorVoltageNew = P_CONST*(xLin - xPosition);
            } else {
                // Otherwise roll downwards for two seconds and press against bottom stop.
                LogHandler::verbose(_TAG, "millis()-startTime: %ld", millis()-startTime);
                xLin  = map(millis()-startTime,0,2000,0,-12000);
                if (millis() > (startTime + 2000)) {
                    mode = 1;
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

        executeCommon(xLin);
        
        if(SettingsHandler::logLevel == LogLevel::VERBOSE) {
            unsigned long currentMillis = millis();
            if (currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;
                LogHandler::verbose(_TAG, "xPosition: %f \t motorVoltage: %f \t mode: %ld \t xLin: %ld \t zeroAngle: %f \t angle: %f\n", xPosition, motorVoltage, mode, xLin, zeroAngle, angle);
                counter = 0;
            }
            counter++;
        }
       
    }

private:

    const char* _TAG = TagHandler::BLDCHandler;
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
    float mode = 0.00;
    long startTime;
    float motorVoltage = 0.00;

    // IGNORE!
    unsigned long previousMillis = 0; // variable to store the time of the last report
    const long interval = 10; // interval at which to send reports (in ms)
    int counter = 0;

    // Derived constants
    double ANG_TO_POS; // Number to convert a motor angle to a 0-10000 axis position
    double TOP_START_OFFSET; // Angle turned by pulley for a full stroke
    double ENDSTOP_START_OFFSET;  // Offset angle from bottom endstop on startup (rad)
};
