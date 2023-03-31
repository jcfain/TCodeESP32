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

#define CHANNELS 3

#include <SimpleFOC.h>
#include "TCode0_3.h"
#include "../../SettingsHandler.h"
#include "../Global.h"
#include "../MotorHandler.h"


// encoder position monitor variables
volatile int encoderPulseLength = 464;
volatile int encoderPulseCycle = 920;
volatile int encoderPulseStart = 0;
volatile int lastEncoderPulseCycle; 
// range is 5-928
volatile int longest = 500;
volatile int shortest = 500;



// Encoder Interrupt detector
void IRAM_ATTR encoderChange() {
    long currentMicros = esp_timer_get_time();
    if(digitalRead(SettingsHandler::BLDC_Encoder_PIN) == HIGH)
    {
        encoderPulseCycle = currentMicros-encoderPulseStart;
        encoderPulseStart = currentMicros;
    }
    else
    {
        encoderPulseLength = currentMicros-encoderPulseStart;
    }
}

class BLDCHandler0_3 : public MotorHandler {

public:
    BLDCHandler0_3() : MotorHandler(new TCode0_3()) { }
    //Encoder Interrupt detector
    static void IRAM_ATTR encoderRising() {
        attachInterrupt(SettingsHandler::BLDC_Encoder_PIN, encoderFalling, FALLING);
        encoderPulseCycle = micros()-encoderPulseStart;
        encoderPulseStart = micros();
    }
    static void IRAM_ATTR encoderFalling() {
        attachInterrupt(SettingsHandler::BLDC_Encoder_PIN, encoderRising, RISING);
        encoderPulseLength = micros()-encoderPulseStart;
    }
    // static void IRAM_ATTR encoderChange() 
    // {
    //     if(digitalRead(SettingsHandler::TwistFeedBack_PIN) == HIGH)
    //     {
    //         encoderPulseCycle = micros()-encoderPulseStart;
    //         encoderPulseStart = micros();
    //     }
    //     else if (lastEncoderPulseCycle != encoderPulseCycle) 
    //     {
    //         int currentPulseLength = micros()-encoderPulseStart;
    //         if (currentPulseLength <= 1000000/50) {
    //             encoderPulseLength = currentPulseLength;
    //             lastEncoderPulseCycle = encoderPulseCycle;
    //         }
    //     }
    // }

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
            
            // Begin tracking encoder
            attachInterrupt(SettingsHandler::BLDC_Encoder_PIN, encoderChange, CHANGE);
            //attachInterrupt(SettingsHandler::BLDC_Encoder_PIN, encoderRising, RISING);

            // initialise encoder hardware
            sensorA.init();
            
            // driver config
            // power supply voltage [V]
            driverA.voltage_power_supply = SettingsHandler::BLDC_MotorA_Voltage;
            // Max DC voltage allowed - default voltage_power_supply
            driverA.voltage_limit = 20;
            // driver init
            driverA.init();

            // limiting motor movements
            motorA.current_limit = SettingsHandler::BLDC_MotorA_Current;   // [Amps]

            // set control loop type to be used
            motorA.torque_controller = TorqueControlType::voltage;
            motorA.controller = MotionControlType::torque;

            // link the motor to the sensor
            motorA.linkSensor(&sensorA);
            // link the motor and the driver
            motorA.linkDriver(&driverA);
            // comment out if not needed
            motorA.useMonitoring(Serial);

            // initialize motor
            motorA.init();
            if(MotorA_ParametersKnown) {
                motorA.initFOC(MotorA_ZeroElecAngle, MotorA_SensorDirection);
            } else {
                motorA.initFOC();
            }
            
            // Set sensor angle and pre-set zero angle to current angle
            sensorA.update();
            zeroAngle = sensorA.getAngle();  

            // Record start time
            startTime = millis();

            setupCommon();

            // Signal ready to start
            Serial.println("Ready!");
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
        motorA.loopFOC();

        // Collect inputs
        // These functions query the t-code object for the position/level at a specified time
        // Number recieved will be an integer, 0-9999
        xLin = m_tcode->AxisRead("L0");


        // Update sensor position
        sensorA.update();
        float angle = sensorA.getAngle();
        // Determine the linear position of the receiver in (0-10000)
        xPosition = (angle - zeroAngle)*588; // *10000/17

        // Control by motor voltage
        float motorVoltageNew;
        // If the device is in startup mode roll downwards for two seconds
        // and press against bottom stop.   
        if (mode == 0) {
            xLin  = map(millis()-startTime,0,2000,0,-10000);
            if (millis() > (startTime + 2000)) {
            mode = 1;
            zeroAngle = angle + 0.5;
            }
            motorVoltageNew = 0.002*(xLin - xPosition);
            if (motorVoltageNew < -0.5) { motorVoltageNew = -0.5; }
        // Otherwise set motor voltage based on position error     
        } else {
            motorVoltageNew = 0.002*(xLin - xPosition);
        }
        // Low pass filter to reduce motor noise
        motorVoltage = 0.8*motorVoltage + 0.2*motorVoltageNew;  
        // Motion control function
        motorA.move(motorVoltage);

        executeCommon(xLin);
        
        /* IGNORE! 
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;
            Serial.print(xPosition);
            Serial.print("\t");
            Serial.print(motorVoltage);
            Serial.print("\t");
            Serial.println(mode);
            counter = 0;
        }
        counter++;
        */
       
    }

private:

    long startTime;
    // Drive Parameters

    // The control code needs to know the angle of the motor relative to the encoder - "Zero elec. angle".
    // If a value is not entered it will perform a quick operation on startup to estimate this.
    // This will be displayed in the serial monitor each time the device starts up.
    // If the device is noticably faster in one direction the angle is out of alignment, try increasing or decreasing it by small increments (eg +/- 0.1).
    bool MotorA_ParametersKnown = false;        // Once you know the zero elec angle for the motor enter it below and set this flag to true.
    float MotorA_ZeroElecAngle = 1.45;                 // This number is the zero angle (in radians) for the motor relative to the encoder.
    Direction MotorA_SensorDirection = Direction::CW; // Do not change. If the motor is showing CCW rotate the motor connector 180 degrees to reverse the motor.

    GenericSensor sensorA = GenericSensor(readEncoder, initEncoder);
    // BLDC motor & driver instance
    // BLDCMotor motor = BLDCMotor(pole pair number, phase resistance (optional) );
    BLDCMotor motorA = BLDCMotor(11,11.1);
    // BLDCDriver3PWM driver = BLDCDriver3PWM(pwmA, pwmB, pwmC, Enable(optional));
    BLDCDriver3PWM driverA = BLDCDriver3PWM(SettingsHandler::BLDC_PWMchannel1_PIN, SettingsHandler::BLDC_PWMchannel2_PIN, SettingsHandler::BLDC_PWMchannel3_PIN, SettingsHandler::BLDC_Enable_PIN);

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
