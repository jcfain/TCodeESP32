#pragma once

#include <SimpleFOC.h>
#include <encoders/mt6701/MagneticSensorMT6701SSI.h>
#include "SettingsHandler.h"
#include "LogHandler.h"
#include "Global.h"
#include "settingsFactory.h"

#define P_CONST 0.002             // Motor PID proportional constant
#define LOW_PASS 0.8              // Low pass filter factor for static noise reduction ( number < 1, 0 = none)

class BLDCTCodeMotor {

    float zero_electric_angle = NOT_SET;//!< absolute zero electric angle - if available
    Direction sensor_direction = Direction::UNKNOWN; //!< default is CW. if sensor_direction ==
public:
    BLDCTCodeMotor(
        const char* name,
        BLDCEncoderType encoderType, 
        double voltage,
        double supplyVoltage,
        double current,
        float angToPos, 
        float topStartOffset, 
        float endStopOffset,
        float zero_electric_angle = NOT_SET,
        Direction sensor_direction = Direction::UNKNOWN) : 
            m_name(name),
            angToPos(angToPos), 
            topStartOffset(topStartOffset), 
            endStopOffset(endStopOffset)
    { 
        PinMapSSR1* pinMap = PinMapSSR1::getInstance();
        if(encoderType == BLDCEncoderType::MT6701) {
            LogHandler::info(m_TAG, "Selected encoder: MT6701");
            if(pinMap->chipSelect() > -1) {
                LogHandler::info(m_TAG, "Setup BLDC motor on MT6701 chip select pin: %d", pinMap->chipSelect());
                m_sensor = new MagneticSensorMT6701SSI(pinMap->chipSelect());
            } else {
                LogHandler::error(m_TAG, "Invalid ChipSelect pin %d", pinMap->chipSelect());
                m_initFailed = true;
                return;
            }
        } else if(encoderType == BLDCEncoderType::PWM) {
            LogHandler::info(m_TAG, "Selected encoder: PWM");
            if(pinMap->encoder() > -1) {
                LogHandler::info(m_TAG, "Setup BLDC motor on PWM encoder pin: %d", pinMap->encoder());
                m_sensor = new MagneticSensorPWM(pinMap->encoder(), 5, 928);
            } else {
                LogHandler::error(m_TAG, "Invalid encoder pin %d", pinMap->encoder());
                m_initFailed = true;
                return;
            }
        } else {
            if(pinMap->chipSelect() > -1) {
                LogHandler::info(m_TAG, "Selected encoder: SPI");
                LogHandler::info(m_TAG, "Setup BLDC motor on SPI chip select pin: %d", pinMap->chipSelect());
                m_sensor = new MagneticSensorSPI(pinMap->chipSelect(), 14, 0x3FFF);
            } else {
                LogHandler::error(m_TAG, "Invalid ChipSelect pin %d", pinMap->chipSelect());
                m_initFailed = true;
                return;
            }
        }

        // BLDC motor & driver instance
        motor = new BLDCMotor(11,11.1);
        // BLDCDriver3PWM driver = BLDCDriver3PWM(pwmA, pwmB, pwmC, Enable(optional));
        LogHandler::info(m_TAG, "Setup BLDC PWM pins 1: %d, 2: %d, 3: %d, enable: %d", pinMap->pwmChannel1(), pinMap->pwmChannel2(), pinMap->pwmChannel3(), pinMap->enable());
        driver = new BLDCDriver3PWM(pinMap->pwmChannel1(), pinMap->pwmChannel2(), pinMap->pwmChannel3(), pinMap->enable());


        // initialise encoder hardware
        if(encoderType == BLDCEncoderType::MT6701) {
            //SPI.begin(pinMap->i2cScl(), pinMap->i2cSda(), 11, pinMap->chipSelect()); // Do we need MOSI custom?
            static_cast<MagneticSensorMT6701SSI*>(m_sensor)->init();
            LogHandler::debug(m_TAG, "init sensorMT6701");
        } else if (encoderType == BLDCEncoderType::PWM) { 
            static_cast<MagneticSensorPWM*>(m_sensor)->init(); 
            LogHandler::debug(m_TAG, "init sensorPWM");
        } else { 
            //SPI.begin(pinMap->i2cScl(), pinMap->i2cSda(), 11, pinMap->chipSelect()); // Do we need this custom?
            static_cast<MagneticSensorSPI*>(m_sensor)->init(); 
            LogHandler::debug(m_TAG, "init sensorSPI");
        }

        // driver config
        // Max DC voltage allowed - default voltage_limit
        LogHandler::debug(m_TAG, "Voltage: %f", voltage);
        driver->voltage_limit = voltage;
        // power supply voltage [V]
        double supplyAVoltage = BLDC_MOTORA_SUPPLY_DEFAULT;
        driver->voltage_power_supply = supplyVoltage;
        // driver init
        driver->init();

        // limiting motor movements
        LogHandler::debug(m_TAG, "Current: %f", current);
        motor->current_limit = current;   // [Amps] 

        // set control loop type to be used
        motor->torque_controller = TorqueControlType::voltage;
        motor->controller = MotionControlType::torque;

        // link the motor to the sensor
        motor->linkSensor(m_sensor); 
        // link the motor and the driver
        motor->linkDriver(driver);

        // initialize motor
        motor->init();
        motor->useMonitoring(Serial);

        // init current sense
        LogHandler::info(m_TAG, "Setting MotorA parameters: %f", zero_electric_angle);
        motor->sensor_direction = sensor_direction;
        motor->zero_electric_angle  = zero_electric_angle; // rad

        if (motor->initFOC())  {
            LogHandler::info(m_TAG, "FOC init success!");
        } else {
            LogHandler::error(m_TAG, "FOC init failed!");
            //return;
            m_initFailed = true;
        }
        LogHandler::info(m_TAG, "BLDC_MotorA_ZeroElecAngle %f", motor->zero_electric_angle);

        
        // link the motor to the sensor
        m_sensor->update();
        zeroAngle = m_sensor->getAngle();
        LogHandler::debug(m_TAG, "zeroAngle: %f", zeroAngle);
    }

    bool initialized()
    {
        return !m_initFailed;
    }

    bool update() 
    {
        if(!initialized()) {
            return false;
        }
        if(!startTime) {
            // Record start time
            startTime = millis();
            LogHandler::info(m_TAG, "startTime: %ld", startTime);
        }
        motor->loopFOC();
        return true;
    }

    bool move(int tcode)
    {
        if(!initialized()) {
            return false;
        }
        // Determine the linear position of the receiver in (0-10000)
        float xPosition = getPosition(); 
        //LogHandler::verbose(_TAG, "zeroAngle: %f", zeroAngle);

        // Control by motor voltage
        float motorVoltageNew;
        // Mode 0 is startup mode. 
        // Distance of travel is 12,000 (>10,000) just to make sure that the receiver reaches the top/bottom.
        if (bootmode) {
            // If using a hall sensor, roll upwards until the magnet triggers the hall effect sensor
            if (m_useHallSensor) {
                //LogHandler::verbose(_TAG, "Hall senso millis()-startTime: %ld", millis()-startTime);
                tcode  = map(millis()-startTime,0,2000,0,12000);
                if (!digitalRead(m_hallSensorPin)) {
                    LogHandler::debug(m_TAG, "Set bootmode false read hall");
                    bootmode = false;
                    zeroAngle = angle - topStartOffset;
                } else if (millis() > (startTime + 2000)) {
                    // Timeout after two seconds if sensor not triggered
                    bootmode = false;
                    LogHandler::debug(m_TAG, "Set bootmode false hall timeout");
                    zeroAngle = angle - topStartOffset - endStopOffset;
                }
                motorVoltageNew = P_CONST*(tcode - xPosition);
            } else {
                // Otherwise roll downwards for two seconds and press against bottom stop.
                // LogHandler::verbose(_TAG, "millis()-startTime: %ld", millis()-startTime);
                tcode  = map(millis()-startTime,0,2000,0,-12000);
                if (millis() > (startTime + 2000)) {
                    bootmode = false;
                    LogHandler::debug(m_TAG, "Set bootmode false NO HALL timeout");
                    zeroAngle = angle + endStopOffset;
                }
                motorVoltageNew = P_CONST*(tcode - xPosition);
                if (motorVoltageNew < -0.5) { motorVoltageNew = -0.5; }
            }
        // Otherwise set motor voltage based on position error     
        } else {
            motorVoltageNew = P_CONST*(tcode - xPosition);
        }
        // Low pass filter to reduce motor noise
        motorVoltage = LOW_PASS*motorVoltage + (1-LOW_PASS)*motorVoltageNew;  
        // Motion control function
        motor->move(motorVoltage);


        if(LogHandler::getLogLevel() == LogLevel::VERBOSE) {
            unsigned long currentMillis = millis();
            if (currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;
                LogHandler::verbose(m_TAG, "%s motor position: %f \t motorVoltage: %f \t bootmode: %ld \t xLin: %ld \t zeroAngle: %f \t angle: %f\n", m_name, xPosition, motorVoltage, bootmode, tcode, zeroAngle, angle);
                counter = 0;
            }
            counter++;
        }

        return true;
    }

    void useHallSensor(int pin)
    {
        m_useHallSensor = pin > -1;
        m_hallSensorPin = -1;
        
        if(m_useHallSensor) {
            LogHandler::info(m_TAG, "Using Hall Sensor");
            // Set pinmode for hall sensor
            pinMode(m_hallSensorPin, INPUT_PULLUP);
        } 
    }


private:
    const char* m_name;
    const char* m_TAG = TagHandler::MotorHandler;
    Sensor* m_sensor;
    BLDCMotor* motor;
    BLDCDriver3PWM* driver;
    bool m_initFailed = false;
    bool bootmode = true;
    float zeroAngle = 0.00;
    float xPosition = 0.00;
    unsigned long startTime = 0;
    float motorVoltage = 0.00;
    // The control code needs to know the angle of the motor relative to the encoder - "Zero elec. angle".
    // If a value is not entered it will perform a quick operation on startup to estimate this.
    // This will be displayed in the serial monitor each time the device starts up.
    // If the device is noticably faster in one direction the angle is out of alignment, try increasing or decreasing it by small increments (eg +/- 0.1).
    const Direction MotorA_SensorDirection = Direction::CW; // Do not change. If the motor is showing CCW rotate the motor connector 180 degrees to reverse the motor.
    float angToPos; // Number to convert a motor angle to a 0-10000 axis position
    float topStartOffset; // Angle turned by pulley for a full stroke
    float endStopOffset;  // Offset angle from bottom endstop on startup (rad)
    bool m_useHallSensor = false;
    int m_hallSensorPin = -1;

    // Logging limiter!
    unsigned long previousMillis = 0; // variable to store the time of the last report
    const long interval = 10; // interval at which to send reports (in ms)
    int counter = 0;

    float getPosition() {
        // Update sensor position
        m_sensor->update();
        float angle = m_sensor->getAngle();
        // Determine the linear position of the receiver in (0-10000)
        return (angle - zeroAngle)*angToPos; 
    }
};