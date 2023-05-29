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
#include "TCode0_3.h"
#include "../../SettingsHandler.h"
#include "../Global.h"
#include "../MotorHandler.h"
#include "../../TagHandler.h"


// Control constants
// (a.k.a. magic numbers for Eve)
#define RAIL_LENGTH 125           // Physical maximum travel of the receiver (mm)
#define STROKE_LENGTH 120         // Operating stroke length (mm)
#define PULLEY_CIRCUMFERENCE 40   // Drive pulley circumference (mm)
#define P_CONST 0.002             // Motor PID proportional constant
#define LOW_PASS 0.8              // Low pass filter factor for static noise reduction ( number < 1, 0 = none)
// Derived constants
#define ANG_TO_POS (10000*PULLEY_CIRCUMFERENCE)/(2*3.14159*STROKE_LENGTH) // Number to convert a motor angle to a 0-10000 axis position
#define START_OFFSET 2*3.14159*(RAIL_LENGTH-STROKE_LENGTH)/(2*PULLEY_CIRCUMFERENCE)  // Offset angle from endstop on startup (rad)

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

    static void initEncoder(){
    // do the init
    }

    static float readEncoder(){
        // read my sensor
        // return the angle value in radians in between 0 and 2PI
        int min = 5;
        int max = 928;
        if (encoderPulseLength > max) {encoderPulseLength = max; } //Serial.println("Bip!");}
        if (encoderPulseLength < min) {encoderPulseLength = min; } //Serial.println("Bop!");}
        float angle;  
        angle = encoderPulseLength - 5;
        angle *= 0.00680735;
        return angle;
    }

    void setup() override {
        // Begin tracking encoder
        LogHandler::debug(_TAG, "Setup BLDC motor on pin: %ld", SettingsHandler::BLDC_Encoder_PIN);
        // attachInterrupt(SettingsHandler::BLDC_Encoder_PIN, encoderChange, CHANGE);
        if(SettingsHandler::BLDC_UsePWM) {
            sensorPWM = new MagneticSensorPWM(SettingsHandler::BLDC_Encoder_PIN, 5, 928);
        } else {
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

        // Set Starting state
        zeroAngle = 0;
        mode = 0;
        
        // initialise encoder hardware
        if (sensorPWM) { sensorPWM->init(); } else { sensorSPI->init(); }
        
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
        if (sensorPWM) { motorA->linkSensor(sensorPWM); } else { motorA->linkSensor(sensorSPI); }
        // link the motor and the driver
        motorA->linkDriver(driverA);

        // initialize motor
        motorA->init();
        if(MotorA_ParametersKnown) {
            motorA->initFOC(MotorA_ZeroElecAngle, MotorA_SensorDirection);
        } else {
            motorA->useMonitoring(Serial);
            motorA->initFOC();
        }
        
        // Set sensor angle and pre-set zero angle to current angle
        if (sensorPWM) { 
            sensorPWM->update(); 
            zeroAngle = sensorPWM->getAngle();
        } else { 
            sensorSPI->update();
            zeroAngle = sensorSPI->getAngle();
        }

        // Record start time
        startTime = millis();

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
        xLin = m_tcode->AxisRead("L0");


        // Update sensor position
        float angle;
        if (sensorPWM) { 
            sensorPWM->update();
            angle = sensorPWM->getAngle();
        } else {
            sensorSPI->update();
            angle = sensorSPI->getAngle();
        }
        // Determine the linear position of the receiver in (0-10000)
        xPosition = (angle - zeroAngle)*ANG_TO_POS; 

        // Control by motor voltage
        float motorVoltageNew;
        // If the device is in startup mode roll downwards for two seconds and press against bottom stop.
        // Distance of travel is 12,000 (>10,000) just to make sure that the receiver reaches the bottom.
        if (mode == 0) {
            xLin  = map(millis()-startTime,0,2000,0,-12000);
            if (millis() > (startTime + 2000)) {
            mode = 1;
            zeroAngle = angle + START_OFFSET;
            }
            motorVoltageNew = P_CONST*(xLin - xPosition);
            if (motorVoltageNew < -0.5) { motorVoltageNew = -0.5; }
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
                //LogHandler::verbose(_TAG, "xPosition: %f \t motorVoltage: %f \t mode: %f", xPosition, motorVoltage, mode);
                counter = 0;
            }
            counter++;
        }
       
    }

private:

    const char* _TAG = TagHandler::BLDCHandler;
    long startTime;
    // Drive Parameters

    // The control code needs to know the angle of the motor relative to the encoder - "Zero elec. angle".
    // If a value is not entered it will perform a quick operation on startup to estimate this.
    // This will be displayed in the serial monitor each time the device starts up.
    // If the device is noticably faster in one direction the angle is out of alignment, try increasing or decreasing it by small increments (eg +/- 0.1).
    bool MotorA_ParametersKnown = false;        // Once you know the zero elec angle for the motor enter it below and set this flag to true.
    float MotorA_ZeroElecAngle = 0.00;                 // This number is the zero angle (in radians) for the motor relative to the encoder.
    Direction MotorA_SensorDirection = Direction::CW; // Do not change. If the motor is showing CCW rotate the motor connector 180 degrees to reverse the motor.

    // BLDC motor & driver instance
    BLDCMotor* motorA;
    // BLDCDriver3PWM driver = BLDCDriver3PWM(pwmA, pwmB, pwmC, Enable(optional));
    BLDCDriver3PWM* driverA;
    // Declare a PWM and an SPI sensor. Only one will be used.
    MagneticSensorPWM* sensorPWM = 0;
    MagneticSensorSPI* sensorSPI = 0;

    // Position variables
    int xLin;
    float zeroAngle;
    float xPosition;
    float mode;
    float motorVoltage;

    // IGNORE!
    unsigned long previousMillis = 0; // variable to store the time of the last report
    const long interval = 10; // interval at which to send reports (in ms)
    int counter = 0;
};
