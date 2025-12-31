#pragma once

#include <SimpleFOC.h>
#include <encoders/mt6701/MagneticSensorMT6701SSI.h>
#include "SettingsHandler.h"
#include "LogHandler.h"
#include "Global.h"
#include "settingsFactory.h"

#define P_CONST 0.002             // Motor PID proportional constant
#define LOW_PASS 0.8              // Low pass filter factor for static noise reduction ( number < 1, 0 = none)

class BLDCTCodeMotor 
{
    // The control code needs to know the angle of the motor relative to the encoder - "Zero elec. angle".
    // If a value is not entered it will perform a quick operation on startup to estimate this.
    // This will be displayed in the serial monitor each time the device starts up.
    // If the device is noticably faster in one direction the angle is out of alignment, try increasing or decreasing it by small increments (eg +/- 0.1).
public:
    BLDCTCodeMotor(
        DeviceType deviceType,
        BLDCMotorChannel motorChannel,
        BLDCEncoderType encoderType, 
        int8_t chipSelectPin,
        int8_t encoderPin,
        int8_t pwm1Pin,
        int8_t pwm2Pin,
        int8_t pwm3Pin,
        int8_t enablePin,
        double voltage,
        double supplyVoltage,
        double current,
        float angToPos, 
        float topStartOffset, 
        float endStopOffset,
        bool parametersKnown,
        float zeroAngle = NOT_SET,
        Direction sensor_direction = Direction::CW) : 
            m_deviceType(deviceType),
            m_motorChannel(motorChannel),
            m_name(getName(motorChannel)),
            angToPos(angToPos), 
            topStartOffset(topStartOffset), 
            endStopOffset(endStopOffset),
            zeroAngle(zeroAngle)
    { 
        if(encoderType == BLDCEncoderType::MT6701) 
        {
            LogHandler::info(m_TAG, "Selected encoder for %s: MT6701", m_name);
            if(chipSelectPin > -1) 
            {
                LogHandler::info(m_TAG, "Setup %s BLDC motor on MT6701 chip select pin: %d", m_name, chipSelectPin);
                m_sensor = new MagneticSensorMT6701SSI(chipSelectPin);
            } 
            else 
            {
                LogHandler::error(m_TAG, "Invalid ChipSelect pin for %s: %d", m_name, chipSelectPin);
                m_initFailed = true;
                return;
            }
        } 
        else if(encoderType == BLDCEncoderType::PWM) 
        {
            LogHandler::info(m_TAG, "Selected encoder for %s: PWM", m_name);
            if(encoderPin > -1) 
            {
                LogHandler::info(m_TAG, "Setup %s BLDC motor on PWM encoder pin: %d", m_name, encoderPin);
                m_sensor = new MagneticSensorPWM(encoderPin, 5, 928);
            } 
            else 
            {
                LogHandler::error(m_TAG, "Invalid encoder pin for %s: %d", m_name, encoderPin);
                m_initFailed = true;
                return;
            }
        } 
        else 
        {
            if(chipSelectPin > -1) 
            {
                LogHandler::info(m_TAG, "Selected encoder for %s: SPI", m_name);
                LogHandler::info(m_TAG, "Setup %s BLDC motor on SPI chip select pin: %d", m_name, chipSelectPin);
                m_sensor = new MagneticSensorSPI(chipSelectPin, 14, 0x3FFF);
            } 
            else 
            {
                LogHandler::error(m_TAG, "Invalid ChipSelect pin for %s, %d", m_name, chipSelectPin);
                m_initFailed = true;
                return;
            }
        }

        // BLDC motor & driver instance
        motor = new BLDCMotor(11,11.1);
        //motor = new BLDCMotor(11,5.57); //? 
        
        LogHandler::info(m_TAG, "Setup %s BLDC pins: PWM1: %d, PWM2: %d, PWM3: %d, enable: %d", m_name, pwm1Pin, pwm2Pin, pwm3Pin, enablePin);
        driver = new BLDCDriver3PWM(pwm1Pin, pwm2Pin, pwm3Pin, enablePin);


        // initialise encoder hardware
        if(encoderType == BLDCEncoderType::MT6701) 
        {
            //SPI.begin(pinMap->i2cScl(), pinMap->i2cSda(), 11, chipSelectPin); // Do we need MOSI custom?
            static_cast<MagneticSensorMT6701SSI*>(m_sensor)->init();
            LogHandler::debug(m_TAG, "init %s MT6701 sensor", m_name);
        } 
        else if (encoderType == BLDCEncoderType::PWM) 
        { 
            static_cast<MagneticSensorPWM*>(m_sensor)->init(); 
            LogHandler::debug(m_TAG, "init %s PWM sensor", m_name);
        } 
        else 
        { 
            //SPI.begin(pinMap->i2cScl(), pinMap->i2cSda(), 11, chipSelectPin); // Do we need this custom?
            static_cast<MagneticSensorSPI*>(m_sensor)->init(); 
            LogHandler::debug(m_TAG, "init %s SPI sensor", m_name);
        }

        // driver config
        // Max DC voltage allowed - default voltage_limit
        LogHandler::debug(m_TAG, "%s Voltage limit: %f", m_name, voltage);
        driver->voltage_limit = voltage;
        // power supply voltage [V]
        LogHandler::debug(m_TAG, "%s Voltage power supply: %f", m_name, supplyVoltage);
        double supplyAVoltage = BLDC_MOTOR_SUPPLY_DEFAULT;
        driver->voltage_power_supply = supplyVoltage;
        // driver init
        driver->init();

        // limiting motor movements
        LogHandler::debug(m_TAG, "%s Current: %f", m_name, current);
        motor->current_limit = current;   // [Amps] 

        // set control loop type to be used
        motor->torque_controller = TorqueControlType::voltage;
        motor->controller = MotionControlType::torque;

        // link the motor to the sensor
        motor->linkSensor(m_sensor); 
        // link the motor and the driver
        motor->linkDriver(driver);

        // initialize motor
        if(!motor->init()) 
        {
            LogHandler::error(m_TAG, "%s motor init failed!", m_name);
            m_initFailed = true;
        }
        motor->useMonitoring(Serial);

        // init current sense
        if(parametersKnown) 
        {
            LogHandler::info(m_TAG, "Parameters known for motor %s", m_name);
            motor->sensor_direction = sensor_direction;
            motor->zero_electric_angle  = zeroAngle; // rad
            LogHandler::debug(m_TAG, "%s zero_electric_angle %f", m_name, motor->zero_electric_angle);
        }

        if (motor->initFOC())  
        {
            LogHandler::info(m_TAG, "%s FOC init success!", m_name);
        } 
        else 
        {
            LogHandler::error(m_TAG, "%s FOC init failed!", m_name);
            //return;
            m_initFailed = true;
        }
        LogHandler::debug(m_TAG, "%s sensor_direction %i", m_name, motor->sensor_direction);

        
        // link the motor to the sensor
        m_sensor->update();
        zeroAngle = m_sensor->getAngle();
        LogHandler::debug(m_TAG, "%s zeroAngle: %f", m_name, zeroAngle);
    }

    bool initialized()
    {
        return !m_initFailed;
    }

    void resetCalibration()
    {
        m_bootmode = true;
    }

    bool update() 
    {
        if(!initialized())
            return false;
        if(!startTime) 
        {
            // Record start time
            startTime = millis();
            LogHandler::info(m_TAG, "%s startTime: %ld", m_name, startTime);
        }
        motor->loopFOC();
        // Update sensor position
        m_sensor->update();
        sensorAngle = m_sensor->getAngle();
        // Determine the linear position of the receiver in (0-10000)
        motorPosition = (sensorAngle - zeroAngle)*angToPos; 
        //LogHandler::verbose(_TAG, "zeroAngle: %f", zeroAngle);
        return true;
    }

    bool bootCalibrate()
    {
        if(!initialized())
            return false;
        if(!m_bootmode)
            return true;

        // Control by motor voltage
        float motorVoltageNew;
        // Mode 0 is startup mode. 
        // Distance of travel is 12,000 (>10,000) just to make sure that the receiver reaches the top/bottom.
        if (m_bootmode) 
        {
            // If using a hall sensor, roll upwards until the magnet triggers the hall effect sensor
            // Only hall effect on stroke is supported currently.
            if (m_useHallSensor && m_motorChannel == BLDCMotorChannel::Stroke) 
            {
                //LogHandler::verbose(_TAG, "Hall senso millis()-startTime: %ld", millis()-startTime);
                m_targetMotorPosition = map(millis()-startTime,0,2000,0,12000);
                if (!digitalRead(m_hallSensorPin)) 
                {
                    LogHandler::debug(m_TAG, "Set %s bootmode false read hall", m_name);
                    m_bootmode = false;
                    if(m_motorChannel == BLDCMotorChannel::Stroke)
                        zeroAngle = sensorAngle - topStartOffset;
                    else
                        //zeroAngle = sensorAngle + topStartOffset;
                } 
                else if (millis() > (startTime + 2000)) 
                {
                    // Timeout after two seconds if sensor not triggered
                    m_bootmode = false;
                    LogHandler::debug(m_TAG, "Set %s bootmode false hall timeout", m_name);
                    // zeroAngle = sensorAngle - topStartOffset - endStopOffset;
                    if(m_motorChannel == BLDCMotorChannel::Stroke)
                    {
                        zeroAngle = sensorAngle - (topStartOffset - endStopOffset);
                    }
                    else
                    {
                        // Im not sure about this twist hall effect...
                        //zeroAngle = sensorAngle + (topStartOffset - endStopOffset);
                    }
                }
            } 
            else 
            {
                // Otherwise roll downwards for two seconds and press against bottom stop.
                // LogHandler::verbose(_TAG, "millis()-startTime: %ld", millis()-startTime);
                m_targetMotorPosition = map(millis()-startTime,0,2000,0,-12000);
                if (millis() > (startTime + 2000)) 
                {
                    m_bootmode = false;
                    LogHandler::debug(m_TAG, "Set %s bootmode false", m_name);
                    // zeroAngle = sensorAngle + endStopOffset;
                    if(m_motorChannel == BLDCMotorChannel::Stroke)
                    {
                        zeroAngle = sensorAngle + endStopOffset;
                    }
                    else
                    {
                        zeroAngle = sensorAngle - endStopOffset;
                    }
                }
            }   
        } 
        return true;
    }

    void process(int strokeTCode, int twistTCode = -1, int multiplier = 1) 
    {
        if(m_bootmode)
            return;
        if(m_deviceType == DeviceType::SSR1 && twistTCode == -1)
        {
            m_tcode = strokeTCode;
            m_targetMotorPosition = strokeTCode;
        }
        else if(m_deviceType == DeviceType::SSR2 && twistTCode > -1)
        {
            if(m_motorChannel == BLDCMotorChannel::Stroke)
            {
                m_tcode = strokeTCode;
                m_targetMotorPosition = strokeTCode + multiplier*(twistTCode-5000);
            }
            else
            {
                m_tcode = twistTCode;
                m_targetMotorPosition = strokeTCode - multiplier*(twistTCode-5000);
            }
        }
    }

    void move()
    {
        float motorVoltageNew = P_CONST*(m_targetMotorPosition - motorPosition);
        if (m_bootmode) { if (motorVoltageNew < -0.5) { motorVoltageNew = -0.5; } }
        // Low pass filter to reduce motor noise
        motorVoltage = LOW_PASS*motorVoltage + (1-LOW_PASS)*motorVoltageNew;  
        // Motion control function
        motor->move(motorVoltage);
        log();
    }

    void useHallSensor(int pin)
    {
        m_useHallSensor = pin > -1;
        m_hallSensorPin = -1;
        
        if(m_useHallSensor) 
        {
            LogHandler::info(m_TAG, "%s Using Hall Sensor", m_name);
            // Set pinmode for hall sensor
            pinMode(m_hallSensorPin, INPUT_PULLUP);
        } 
    }


private:
    DeviceType m_deviceType;
    const char* m_name;
    BLDCMotorChannel m_motorChannel;
    const char* m_TAG = TagHandler::BLDCMotor;
    Sensor* m_sensor;
    BLDCMotor* motor;
    BLDCDriver3PWM* driver;
    bool m_initFailed = false;
    bool m_bootmode = true;
    float zeroAngle = 0.00;
    float sensorAngle = 0.00;
    float motorPosition = 0.00;
    float m_targetMotorPosition = 0.00;
    int m_tcode = TCODE_MID;
    unsigned long startTime = 0;
    float motorVoltage = 0.00;
    float angToPos; // Number to convert a motor angle to a 0-10000 axis position
    float topStartOffset; // Angle turned by pulley for a full stroke
    float endStopOffset;  // Offset angle from bottom endstop on startup (rad)
    bool m_useHallSensor = false;
    int m_hallSensorPin = -1;

    // Logging limiter!
    unsigned long previousMillis = 0; // variable to store the time of the last report
    const long interval = 10; // interval at which to send reports (in ms)
    int counter = 0;

    const char* getName(BLDCMotorChannel motorChannel)
    {
        switch (motorChannel)
        {
            case BLDCMotorChannel::Twist:
                return "Twist";
                break;
            
            default:
                return "Stroke";
                break;
        }
    }

    void log() 
    {
        if(LogHandler::getLogLevel() == LogLevel::VERBOSE) 
        {
            unsigned long currentMillis = millis();
            if (currentMillis - previousMillis >= interval) 
            {
                previousMillis = currentMillis;
                LogHandler::verbose(m_TAG, "%s motor position: %f \t motorVoltage: %f \t bootmode: %ld \t tcode: %ld \t zeroAngle: %f \t angle: %f\n", m_name, motorPosition, motorVoltage, m_bootmode, m_tcode, zeroAngle, sensorAngle);
                counter = 0;
            }
            counter++;
        }
    }
};