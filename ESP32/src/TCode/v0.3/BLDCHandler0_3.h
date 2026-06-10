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
// #include <SimpleFOCDebug.h>
#include <encoders/mt6701/MagneticSensorMT6701SSI.h>
#include "Global.h"
#include "SettingsHandler.h"
#include "TagHandler.h"
#include "settingsFactory.h"
#include "MotorHandler0_3.h"
#include "BLDCTCodeSensorSPI.h"
#include "printX.h"


class BLDCHandler0_3 : public MotorHandler0_3 
{
public:
    BLDCHandler0_3() : MotorHandler0_3(new TCode0_3()) { }

    bool setup() override 
    {
        m_settingsFactory = SettingsFactory::getInstance();
        m_settingsFactory->getValue(DEVICE_TYPE, m_deviceType);
        LogHandler::debug(_TAG, "BLDC Device type type: %d", m_deviceType);
        if(m_deviceType == DeviceType::NONE)
        {
            SettingsHandler::addPersistentError(initDeviceTypeError);
            LogHandler::error(_TAG, initDeviceTypeError);
            return false;
        }
        m_settingsFactory->getValue(BLDC_MOTORA_ENCODER, encoderAType);
        LogHandler::debug(_TAG, "Motor A Encoder type: %d", encoderAType);
        if(encoderAType == BLDCEncoderType::NONE)
        {
            SettingsHandler::addPersistentError(initEncoderTypeError);
            LogHandler::error(_TAG, initEncoderTypeError);
            return false;
        }
        if(m_deviceType == DeviceType::SSR2)
        {
            m_settingsFactory->getValue(BLDC_MOTORB_ENCODER, encoderBType);
            LogHandler::debug(_TAG, "Motor B Encoder type: %d", encoderBType);
            if(encoderBType == BLDCEncoderType::NONE)
            {
                SettingsHandler::addPersistentError(initEncoderTypeError);
                LogHandler::error(_TAG, initEncoderTypeError);
                return false;
            }
        }
        //PinMapInfo pinMapInfo = m_settingsFactory->getPins();
        PinMapSSR* pinMap = PinMapSSR::getInstance();

        InitEncoders initEncoder(pinMap->chipSelect(), pinMap->motorBChipSelect(), &SPI);
        if(m_deviceType == DeviceType::SSR1)
        {
            int pullyCircumference = -1;
            m_settingsFactory->getValue(BLDC_MOTORA_PULLEY_CIRCUMFERENCE, pullyCircumference);
            int strokeLength = -1;
            m_settingsFactory->getValue(BLDC_STROKELENGTH, strokeLength);
            int railLength = -1;
            m_settingsFactory->getValue(BLDC_RAILLENGTH, railLength);
            angToPos = (10000*pullyCircumference)/(2*3.14159*strokeLength); // Number to convert a motor angle to a 0-10000 axis position
            LogHandler::debug(_TAG, "angToPos: %f", angToPos);
            topStartOffset = 2*3.14156*strokeLength/pullyCircumference; // Angle turned by pulley for a full stroke
            LogHandler::debug(_TAG, "topStartOffset: %f", topStartOffset);
            endStopOffset = 2*3.14159*(railLength-strokeLength)/(2*pullyCircumference);  // Offset angle from bottom endstop on startup (rad)
            LogHandler::debug(_TAG, "endStopOffset: %f", endStopOffset);
        }
        else
        {
            LogHandler::debug(_TAG, "[setup] SSR2");
            angToPos = 530.516; // Number to convert a motor angle to a 0-10000 axis position
            LogHandler::debug(_TAG, "angToPos: %f", angToPos);
            topStartOffset = 2*3.14159*0.05; // Angle turned by pulley for a full stroke
            LogHandler::debug(_TAG, "topStartOffset: %f", topStartOffset);
            endStopOffset = 1;// Not used on SSR2 currently
            //initEncoder = new InitEncoders(pinMap->chipSelect(), pinMap->motorBChipSelect(), &SPI);
            LogHandler::info(_TAG, "TwistMultiplier %f", m_settingsFactory->getBLDCTwistMultiplier());
        }

        // Begin tracking encoder
        if(encoderAType == BLDCEncoderType::MT6701) 
        {
            LogHandler::info(_TAG, "Selected Motor A encoder: MT6701");
            if(pinMap->chipSelect() > -1) 
            {
                LogHandler::info(_TAG, "Setup BLDC motor A on MT6701 chip select pin: %d", pinMap->chipSelect());
                sensorA = new MagneticSensorMT6701SSI(pinMap->chipSelect());;
            } 
            else 
            {
                LogHandler::error(_TAG, "Invalid Motor A ChipSelect pin %d", pinMap->chipSelect());
                snprintf(setupError, sizeof setupError, "Invalid Motor A ChipSelect pin %d", pinMap->chipSelect());
                SettingsHandler::addPersistentError(setupError);
                return false;
            }
        } 
        else if(encoderAType == BLDCEncoderType::PWM) 
        {
            LogHandler::info(_TAG, "Selected Motor A encoder: PWM");
            if(pinMap->encoder() > -1) 
            {
                LogHandler::info(_TAG, "Setup BLDC motor A on PWM encoder pin: %d", pinMap->encoder());
                sensorA = new MagneticSensorPWM(pinMap->encoder(), 5, 928);;
            } 
            else 
            {
                LogHandler::error(_TAG, "Invalid Motor A encoder pin %d", pinMap->encoder());
                snprintf(setupError, sizeof setupError, "Invalid Motor A encoder pin %d", pinMap->encoder());
                SettingsHandler::addPersistentError(setupError);
                return false;
            }
        } 
        else 
        {
            if(pinMap->chipSelect() > -1) 
            {
                LogHandler::info(_TAG, "Selected Motor A encoder: SPI");
                LogHandler::info(_TAG, "Setup BLDC motor A on SPI chip select pin: %d", pinMap->chipSelect());
                // sensorA = new MagneticSensorSPI(pinMap->chipSelect(), 14, 0x3FFF);
                ReadEncoder readEncoder(pinMap->chipSelect(), &SPI);
                sensorA = new BLDCTCodeSensorSPI(readEncoder, initEncoder);
            } 
            else 
            {
                snprintf(setupError, sizeof setupError, "Invalid Motor A ChipSelect pin %d", pinMap->chipSelect());
                SettingsHandler::addPersistentError(setupError);
                return false;
            }
        }

        if(m_deviceType == DeviceType::SSR2)
        {
            BLDCEncoderType encoderBType = (BLDCEncoderType)BLDC_ENCODER_DEFAULT;
            m_settingsFactory->getValue(BLDC_MOTORB_ENCODER, encoderBType);
            LogHandler::debug(_TAG, "Motor B Encoder type: %d", encoderBType);
            if(encoderBType == BLDCEncoderType::MT6701) 
            {
                LogHandler::info(_TAG, "Selected Motor B encoder: MT6701");
                if(pinMap->motorBChipSelect() > -1) 
                {
                    LogHandler::info(_TAG, "Setup BLDC motor B on MT6701 chip select pin: %d", pinMap->motorBChipSelect());
                    sensorB = new MagneticSensorMT6701SSI(pinMap->motorBChipSelect());;
                } 
                else 
                {
                    LogHandler::error(_TAG, "Invalid ChipSelect pin %d", pinMap->motorBChipSelect());
                    snprintf(setupError, sizeof setupError, "Invalid ChipSelect pin %d", pinMap->motorBChipSelect());
                    SettingsHandler::addPersistentError(setupError);
                    return false;
                }
            } 
            else if(encoderBType == BLDCEncoderType::PWM) 
            {
                LogHandler::info(_TAG, "Selected Motor B encoder: PWM");
                if(pinMap->motorBEncoder() > -1) 
                {
                    LogHandler::info(_TAG, "Setup BLDC motor B on PWM encoder pin: %d", pinMap->motorBEncoder());
                    sensorB = new MagneticSensorPWM(pinMap->motorBEncoder(), 5, 928);;
                } 
                else 
                {
                    LogHandler::error(_TAG, "Invalid Motor B encoder pin %d", pinMap->motorBEncoder());
                    snprintf(setupError, sizeof setupError, "Invalid Motor B encoder pin %d", pinMap->motorBEncoder());
                    SettingsHandler::addPersistentError(setupError);
                    return false;
                }
            } 
            else 
            {
                if(pinMap->motorBChipSelect() > -1) 
                {
                    LogHandler::info(_TAG, "Selected Motor B encoder: SPI");
                    LogHandler::info(_TAG, "Setup BLDC motor B on SPI chip select pin: %d", pinMap->motorBChipSelect());
                    //sensorB = new MagneticSensorSPI(pinMap->motorBChipSelect(), 14, 0x3FFF);
                    ReadEncoder readEncoder(pinMap->motorBChipSelect(), &SPI);
                    sensorB = new BLDCTCodeSensorSPI(readEncoder, initEncoder);
                } 
                else 
                {
                    LogHandler::error(_TAG, "Invalid Motor B ChipSelect pin %d", pinMap->motorBChipSelect());
                    snprintf(setupError, sizeof setupError, "Invalid Motor B ChipSelect pin %d", pinMap->motorBChipSelect());
                    SettingsHandler::addPersistentError(setupError);
                    return false;
                }
            }
        }
        // BLDC motor & driver instance
        motorA = new BLDCMotor(11,11.1);
        if(m_deviceType == DeviceType::SSR2)
            motorB = new BLDCMotor(11,11.1);
        // BLDCDriver3PWM driver = BLDCDriver3PWM(pwmA, pwmB, pwmC, Enable(optional));
        LogHandler::info(_TAG, "Setup Motor A BLDC pins PWM1: %d, PWM2: %d, PWM3: %d, enable: %d", pinMap->pwmChannel1(), pinMap->pwmChannel2(), pinMap->pwmChannel3(), pinMap->enable());
        driverA = new BLDCDriver3PWM(pinMap->pwmChannel1(), pinMap->pwmChannel2(), pinMap->pwmChannel3(), pinMap->enable());
        if(motorB)
        {
            LogHandler::info(_TAG, "Setup Motor B BLDC pins PWM1: %d, PWM2: %d, PWM3: %d, enable: %d", pinMap->motorBPwmChannel1(), pinMap->motorBPwmChannel2(), pinMap->motorBPwmChannel3(), pinMap->motorBEnable());
            driverB = new BLDCDriver3PWM(pinMap->motorBPwmChannel1(), pinMap->motorBPwmChannel2(), pinMap->motorBPwmChannel3(), pinMap->motorBEnable());
        }

        // Start serial connection and report status
        m_tcode->setup(FIRMWARE_VERSION_NAME);

        // #ESP32# Enable EEPROM
        //EEPROM.begin(320); Done in TCode class

        // Register device axes
        m_tcode->RegisterAxis(TCODE_CHANNEL_STROKE, "Up");
        if(m_deviceType == DeviceType::SSR2)
            m_tcode->RegisterAxis(TCODE_CHANNEL_TWIST, "Twist");

        m_settingsFactory->getValue(BLDC_USEHALLSENSOR, m_useHallSensor);
        if(m_useHallSensor)
        {
            m_hallSensorPin = pinMap->hallEffect();
            if(m_hallSensorPin > -1) 
            {
                LogHandler::info(_TAG, "Using Hall Sensor pin: %i", m_hallSensorPin);
                // Set pinmode for hall sensor
                pinMode(m_hallSensorPin, INPUT_PULLUP);
            } 
            else
            {
                LogHandler::warning(_TAG, "Use hall sensor true but pin is invalid %i...ignoring", m_hallSensorPin);
                m_useHallSensor = false;
                // m_settingsFactory->setValue(BLDC_USEHALLSENSOR, m_useHallSensor);
            }
        }
        
        // initialise encoder hardware
        if(sensorA)
        {
            if(encoderAType == BLDCEncoderType::MT6701) 
            {
                //SPI.begin(pinMap->i2cScl(), pinMap->i2cSda(), 11, pinMap->chipSelect()); // Do we need MOSI custom?
                static_cast<MagneticSensorMT6701SSI*>(sensorA)->init();
                LogHandler::debug(_TAG, "init Motor A sensorMT6701");
            } 
            else if(encoderAType == BLDCEncoderType::PWM) 
            {
                static_cast<MagneticSensorPWM*>(sensorA)->init();
                LogHandler::debug(_TAG, "init Motor A sensorPWM");
            } 
            else 
            { 
                //SPI.begin(pinMap->i2cScl(), pinMap->i2cSda(), 11, pinMap->chipSelect()); // Do we need this custom?
                // static_cast<MagneticSensorSPI*>(sensorA)->init();
                static_cast<BLDCTCodeSensorSPI*>(sensorA)->init();
                LogHandler::debug(_TAG, "init Motor A sensorSPI");
            }
        }
        if(sensorB)
        {
            if(encoderAType == BLDCEncoderType::MT6701) 
            {
                //SPI.begin(pinMap->i2cScl(), pinMap->i2cSda(), 11, pinMap->chipSelect()); // Do we need MOSI custom?
                static_cast<MagneticSensorMT6701SSI*>(sensorB)->init();
                LogHandler::debug(_TAG, "init Motor B sensorMT6701");
            } 
            else if(encoderAType == BLDCEncoderType::PWM) 
            {
                static_cast<MagneticSensorPWM*>(sensorB)->init();
                LogHandler::debug(_TAG, "init Motor B sensorPWM");
            } 
            else 
            { 
                //SPI.begin(pinMap->i2cScl(), pinMap->i2cSda(), 11, pinMap->chipSelect()); // Do we need this custom?
                // static_cast<MagneticSensorSPI*>(sensorB)->init();
                static_cast<BLDCTCodeSensorSPI*>(sensorB)->init();
                LogHandler::debug(_TAG, "init Motor B sensorSPI");
            }
        }

        // driver config
        // Max DC voltage allowed - default voltage_limit
        double motorAVoltage = BLDC_MOTORA_VOLTAGE_DEFAULT;
        m_settingsFactory->getValue(BLDC_MOTORA_VOLTAGE, motorAVoltage);
        LogHandler::debug(_TAG, "Motor A Voltage: %f", motorAVoltage);
        driverA->voltage_limit = motorAVoltage;
        // motorA->voltage_limit = motorAVoltage;
        // power supply voltage [V]
        double supplyAVoltage = BLDC_MOTORA_SUPPLY_DEFAULT;
        m_settingsFactory->getValue(BLDC_MOTORA_SUPPLY, supplyAVoltage);
        driverA->voltage_power_supply = supplyAVoltage;

        if(driverB)
        {
            double motorBVoltage = BLDC_MOTORB_VOLTAGE_DEFAULT;
            m_settingsFactory->getValue(BLDC_MOTORB_VOLTAGE, motorBVoltage);
            LogHandler::debug(_TAG, "Motor B Voltage: %f", motorBVoltage);
            driverB->voltage_limit = motorBVoltage;
            // motorB->voltage_limit = motorAVoltage;
            // power supply voltage [V]
            double supplyBVoltage = BLDC_MOTORB_SUPPLY_DEFAULT;
            m_settingsFactory->getValue(BLDC_MOTORB_SUPPLY, supplyBVoltage);
            driverB->voltage_power_supply = supplyBVoltage;
        }
        // driver init
        driverA->init();
        if(driverB)
            driverB->init();

        // limiting motor movements
        double motorACurrent = BLDC_MOTORA_CURRENT_DEFAULT;
        m_settingsFactory->getValue(BLDC_MOTORA_CURRENT, motorACurrent);
        LogHandler::debug(_TAG, "Motor A Current: %f", motorACurrent);
        motorA->current_limit = motorACurrent;   // [Amps] 

        // set control loop type to be used
        motorA->torque_controller = TorqueControlType::voltage;
        motorA->controller = MotionControlType::torque;

        if(motorB)
        {
            // limiting motor movements
            double motorBCurrent = BLDC_MOTORB_CURRENT_DEFAULT;
            m_settingsFactory->getValue(BLDC_MOTORB_CURRENT, motorBCurrent);
            LogHandler::debug(_TAG, "Motor B Current: %f", motorBCurrent);
            motorB->current_limit = motorBCurrent;   // [Amps] 

            // set control loop type to be used
            motorB->torque_controller = TorqueControlType::voltage;
            motorB->controller = MotionControlType::torque;
        }

        // link the motor to the sensor
        motorA->linkSensor(sensorA);
        if(motorB && sensorB)
            motorB->linkSensor(sensorB);
        // link the motor and the driver
        motorA->linkDriver(driverA);
        if(motorB && driverB)
            motorB->linkDriver(driverB);

        // initialize motor
        motorA->init();
        if(motorB)
            motorB->init();
        m_settingsFactory->resetMotorStatus();
        PrintMotorStatus* motorAStatus = new PrintMotorStatus("motorAStatus");
        motorA->useMonitoring(*motorAStatus);
        if(motorB) 
        {
            PrintMotorStatus* motorBStatus = new PrintMotorStatus("motorBStatus");
            motorB->useMonitoring(*motorBStatus);
        }
        PrintMotorStatus* motorStatus = new PrintMotorStatus("motorStatus");
        SimpleFOCDebug::enable(motorStatus);

        // init current sense
        double zeroElecAngleA = BLDC_MOTORA_ZEROELECANGLE_DEFAULT;
        m_settingsFactory->getValue(BLDC_MOTORA_ZEROELECANGLE, zeroElecAngleA);
        if(zeroElecAngleA > NOT_SET) 
        {
            // Set sensor angle and pre-set zero angle to current angle
            LogHandler::info(_TAG, "Setting Motor A parameters: %f", zeroElecAngleA);
            motorA->sensor_direction = MotorA_SensorDirection;
            motorA->zero_electric_angle  = zeroElecAngleA; // rad
        }

        if(motorB)
        {
            double zeroElecAngleB = BLDC_MOTORB_ZEROELECANGLE_DEFAULT;
            m_settingsFactory->getValue(BLDC_MOTORB_ZEROELECANGLE, zeroElecAngleB);
            if(zeroElecAngleB > NOT_SET) 
            {
                // Set sensor angle and pre-set zero angle to current angle
                LogHandler::info(_TAG, "Setting Motor B parameters: %f", zeroElecAngleB);
                motorB->sensor_direction = MotorB_SensorDirection;
                motorB->zero_electric_angle  = zeroElecAngleB; // rad
            }
        }

        if (motorA->initFOC())  
        {
            LogHandler::info(_TAG, "Motor A FOC init success!");
        } 
        else 
        {
            LogHandler::error(_TAG, "Motor A FOC init failed!");
            snprintf(setupError, sizeof setupError, "Motor A FOC init failed!");
            SettingsHandler::addPersistentError(setupError);
            return false;
        }
        LogHandler::info(_TAG, "BLDC_MotorA_ZeroElecAngle %f", motorA->zero_electric_angle);
        if(motorB)
        {
            if (motorB->initFOC())  
            {
                LogHandler::info(_TAG, "Motor B FOC init success!");
            } 
            else 
            {
                LogHandler::error(_TAG, "Motor B FOC init failed!");
                snprintf(setupError, sizeof setupError, "Motor b FOC init failed!");
                SettingsHandler::addPersistentError(setupError);
                return false;
            }
            LogHandler::info(_TAG, "BLDC_MotorB_ZeroElecAngle %f", motorB->zero_electric_angle);
        }

        
        // link the motor to the sensor
        sensorA->update();
        zeroAngleA = sensorA->getAngle();

        if(sensorB)
        {
            sensorB->update();
            zeroAngleB = sensorB->getAngle();
        }


        setupCommon();

        // Signal ready to start
        m_initialized = true;
        LogHandler::info(_TAG, "Ready!");
        return true;
    }

    void read(byte inByte) override 
    {
        m_tcode->read(inByte);
    }
    
    void read(const char* input, size_t len) override
    {
        for (int i = 0; i < len; i++) 
        {
            read(input[i]);
        }
    }

    void setMessageCallback(TCodeCommandCallback function) override 
    {
        m_tcode->setMessageCallback(function);
    }

    void execute() override 
    {
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
       
        log();
    }

private:
    class PrintMotorStatus: public Print {
        public:

        PrintMotorStatus(const char* name) : m_name(name) {}
        
        size_t write(uint8_t buffer)
        {
            if (buffer == 0) {
                return 0;
            }
            const char* string = (const char*)&buffer;
            if(strcmp(string, "\r\n") == 0)
                return 0;
            SettingsFactory::getInstance()->addMotorStatus(m_name, string);
            return LogHandler::info("MotorState", string);
        }

        size_t write(const uint8_t *buffer, size_t size)
        {
            if (buffer == 0) {
                return 0;
            }
            const char* string = (const char*)buffer;
            if(strcmp(string, "\r\n") == 0)
                return 0;
            SettingsFactory::getInstance()->addMotorStatus(m_name, string);
            return LogHandler::info("MotorState", string);
        }
        private:
        const char* m_name;
    };

    const char* _TAG = TagHandler::BLDCHandler;
    const char* initDeviceTypeError = "No device type selected. Visit the web config or use the command to set a device before starting the firmware.";
    const char* initEncoderTypeError = "No encoder type selected. Visit the web config or use the command to set an encoder before starting the firmware.";
    char setupError[50] = {0};
    bool m_initialized = false;
    SettingsFactory* m_settingsFactory;
    DeviceType m_deviceType;
    bool m_useHallSensor = false;
    int8_t m_hallSensorPin = -1;
    BLDCBootMode m_bootmode = BLDCBootMode::CALIBRATE;
    // Drive Parameters

    // The control code needs to know the angle of the motor relative to the encoder - "Zero elec. angle".
    // If a value is not entered it will perform a quick operation on startup to estimate this.
    // This will be displayed in the serial monitor each time the device starts up.
    // If the device is noticably faster in one direction the angle is out of alignment, try increasing or decreasing it by small increments (eg +/- 0.1).
    Direction MotorA_SensorDirection = Direction::CW; // Do not change. If the motor is showing CCW rotate the motor connector 180 degrees to reverse the motor.
    // BLDC motorA & driver instance
    BLDCMotor* motorA = 0;
    BLDCDriver3PWM* driverA = 0;
    BLDCEncoderType encoderAType = (BLDCEncoderType)BLDC_ENCODER_DEFAULT;
    Sensor* sensorA = 0;
    float zeroAngleA = 0.00;
    float sensorAngleA = 0.00;
    float m_motorAnglePositionA = 0.00;
    float motorVoltageA = 0.00;

    ///SSR2 only
    Direction MotorB_SensorDirection = Direction::CW;// Do not change. If the motor is showing CCW rotate the motor connector 180 degrees to reverse the motor.
    // BLDC motorB & driver instance
    BLDCMotor* motorB = 0;
    BLDCDriver3PWM* driverB = 0;
    BLDCEncoderType encoderBType = (BLDCEncoderType)BLDC_ENCODER_DEFAULT;
    Sensor* sensorB = 0;
    float zeroAngleB = 0.00;
    float sensorAngleB = 0.00;
    float m_motorAnglePositionB = 0.00;
    float motorVoltageB = 0.00;

    float strokePosition;
    float twistPosition;
    int startPoint;
    ///

    // Position variables
    //float zeroAngle = 0.00;
    //float xPosition = 0.00;
    unsigned long startTime = 0;
    //float motorVoltage = 0.00;

    // IGNORE!
    unsigned long previousMillis = 0; // variable to store the time of the last report
    const long interval = 500; // interval at which to send reports (in ms)
    int counter = 0;

    // Derived constants
    float angToPos; // Number to convert a motor angle to a 0-10000 axis position
    float topStartOffset; // Angle turned by pulley for a full stroke
    float endStopOffset;  // Offset angle from bottom endstop on startup (rad)

    void executeSSR1(int& strokeTCode) 
    {
        if(!startTime) 
        {
            // Record start time
            startTime = millis();
            LogHandler::verbose(_TAG, "startTime: %ld", startTime);
        }
        // Run motor FOC loop
        if(motorA && motorA->enabled)
            motorA->loopFOC();

        // Update sensor position
        if(sensorA)
        {
            sensorA->update();
            sensorAngleA = sensorA->getAngle();
        }
        // Determine the linear position of the receiver in (0-10000)
        strokePosition = (sensorAngleA - zeroAngleA)*angToPos; 
        //LogHandler::verbose(_TAG, "zeroAngle: %f", zeroAngle);

        // Control constants
        float pidProportionalConst = m_settingsFactory->getBLDCPIDProportionalConst(); // Motor PID proportional constant
        float lowPassFilter = m_settingsFactory->getBLDCLowPassFilter(); // Low pass filter factor for static noise reduction ( number < 1, 0 = none)
        // Control by motor voltage
        float motorVoltageNew;
        // Mode 0 is startup mode. 
        // Distance of travel is 12,000 (>10,000) just to make sure that the receiver reaches the top/bottom.
        if (m_bootmode == BLDCBootMode::CALIBRATE) 
        {
            // If using a hall sensor, roll upwards until the magnet triggers the hall effect sensor
            if (m_useHallSensor) 
            {
                //LogHandler::verbose(_TAG, "Hall senso millis()-startTime: %ld", millis()-startTime);
                strokeTCode = map(millis()-startTime,0,2000,0,12000);
                if (!digitalRead(m_hallSensorPin)) 
                {
                    LogHandler::debug(_TAG, "Set bootmode false read hall");
                    m_bootmode = BLDCBootMode::NORMAL;
                    zeroAngleA = sensorAngleA - topStartOffset;
                } 
                else if (millis() > (startTime + 2000)) 
                {
                    // Timeout after two seconds if sensor not triggered
                    m_bootmode = BLDCBootMode::NORMAL;
                    LogHandler::debug(_TAG, "Set bootmode false hall timeout");
                    zeroAngleA = sensorAngleA - topStartOffset - endStopOffset;
                }
                motorVoltageNew = pidProportionalConst*(strokeTCode - strokePosition);
            } 
            else 
            {
                // Otherwise roll downwards for two seconds and press against bottom stop.
                // LogHandler::verbose(_TAG, "millis()-startTime: %ld", millis()-startTime);
                strokeTCode  = map(millis()-startTime,0,2000,0,-12000);
                if (millis() > (startTime + 2000)) 
                {
                    m_bootmode = BLDCBootMode::NORMAL;
                    LogHandler::debug(_TAG, "Set bootmode false NO HALL timeout");
                    zeroAngleA = sensorAngleA + endStopOffset;
                }
                motorVoltageNew = pidProportionalConst*(strokeTCode - strokePosition);
                if (motorVoltageNew < -0.5) { motorVoltageNew = -0.5; }
            } 
        } 
        else 
        {
            // Otherwise set motor voltage based on position 
            motorVoltageNew = pidProportionalConst*(strokeTCode - strokePosition);
        }
        // Low pass filter to reduce motor noise
        motorVoltageA = lowPassFilter*motorVoltageA + (1-lowPassFilter)*motorVoltageNew;  
        // Motion control function
        if(motorA && motorA->enabled)
            motorA->move(motorVoltageA);
    }

    void executeSSR2(int strokeTCode, int twistTCode)
    { 
        if(!startTime) 
        {
            // Record start time
            startTime = millis();
            LogHandler::verbose(_TAG, "startTime: %ld", startTime);
        }
        // Run motor FOC loop
        if(motorA && motorA->enabled)
            motorA->loopFOC();
        if(motorB && motorB->enabled)
            motorB->loopFOC();

        float twistMultiplier = m_settingsFactory->getBLDCTwistMultiplier();
        float twistLimit = m_settingsFactory->getBLDCTwistLimit();

        // Update sensor position
        if(sensorA)
        {
            sensorA->update();
            sensorAngleA = sensorA->getAngle();
        }
        if(sensorB)
        {
            sensorB->update();
            sensorAngleB = sensorB->getAngle();
        }
        // Determine the linear position of the receiver in (0-10000)
        m_motorAnglePositionA = -(sensorAngleA - zeroAngleA)*angToPos;
        m_motorAnglePositionB = (sensorAngleB - zeroAngleB)*angToPos;
        strokePosition = 0.5*(m_motorAnglePositionA + m_motorAnglePositionB);
        twistPosition = m_motorAnglePositionB - m_motorAnglePositionA;

        // Position control
        float strokeTargetPosition, twistTargetPosition;
        if (m_bootmode == BLDCBootMode::CALIBRATE) 
        {
            // LogHandler::debug(_TAG, "Boot mode CALIBRATE: startTime %i, millis: %ld !digitalRead(m_hallSensorPin)", startTime, millis());
            if(m_useHallSensor)
            {
                strokeTargetPosition = map(millis()-startTime,0,2000,0,12000);
                twistTargetPosition = 0;
                if (!digitalRead(m_hallSensorPin)) 
                {
                    LogHandler::debug(_TAG, "Boot mode HOMING: startTime %ld, millis: %ld !digitalRead(m_hallSensorPin)", startTime, millis());
                    m_bootmode = BLDCBootMode::HOMING;
                    zeroAngleA = sensorAngleA + 10000/angToPos;
                    zeroAngleB = sensorAngleB - 10000/angToPos;
                    LogHandler::debug(_TAG, "m_useHallSensor zeroAngleA %i", zeroAngleA);
                    LogHandler::debug(_TAG, "m_useHallSensor zeroAngleB %i", zeroAngleB);
                    startPoint = 9999;
                    startTime = millis();
                    Serial.println("Ready!");
                } 
                else if (millis() > (startTime + 2000)) 
                {
                    LogHandler::debug(_TAG, "Boot mode HOMING: startTime %ld, millis: %ld m_useHallSensor millis() > (startTime + 2000)", startTime, millis());
                    m_bootmode = BLDCBootMode::HOMING;
                    zeroAngleA = sensorAngleA + 10000/angToPos + topStartOffset;
                    zeroAngleB = sensorAngleB - 10000/angToPos - topStartOffset;
                    LogHandler::debug(_TAG, "m_useHallSensor millis() > (startTime + 2000) zeroAngleA %i", zeroAngleA);
                    LogHandler::debug(_TAG, "m_useHallSensor millis() > (startTime + 2000) zeroAngleB %i", zeroAngleB);
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
                    LogHandler::debug(_TAG, "Boot mode HOMING: startTime %ld, millis: %ld", startTime, millis());
                    m_bootmode = BLDCBootMode::HOMING;
                    zeroAngleA = sensorAngleA - topStartOffset;
                    zeroAngleB = sensorAngleB + topStartOffset;
                    LogHandler::debug(_TAG, "zeroAngleA %i", zeroAngleA);
                    LogHandler::debug(_TAG, "zeroAngleB %i", zeroAngleB);
                    startPoint = 0;
                    startTime = millis();
                    Serial.println("Ready!");
                }
            }
        } 
        else if (m_bootmode == BLDCBootMode::HOMING) 
        {
            strokeTargetPosition = map(millis()-startTime,0,2000,startPoint,strokeTCode);
            twistTargetPosition = 0;
            if (millis() > (startTime + 2000)) 
            {
                LogHandler::debug(_TAG, "Boot mode NORMAL: startTime %i, millis: %ld", startTime, millis());
                m_bootmode = BLDCBootMode::NORMAL;
            }
        } 
        else 
        {
            // Otherwise set motor voltage based on position   
            strokeTargetPosition = strokeTCode;
            twistTargetPosition = twistMultiplier*(5000 - twistTCode);
        }

        // Control constants
        float pidProportionalConst = m_settingsFactory->getBLDCPIDProportionalConst(); // Motor PID proportional constant
        float lowPassFilter = m_settingsFactory->getBLDCLowPassFilter();  // Low pass filter factor for static noise reduction ( number < 1, 0 = none)
        //LogHandler::debug(_TAG, "pidProportionalConst %f, lowPassFilter: %f", pidProportionalConst, lowPassFilter);
        float strokeVoltage, twistVoltage;
        strokeVoltage = pidProportionalConst*(strokeTargetPosition - strokePosition);
        twistVoltage = pidProportionalConst*(twistTargetPosition - twistPosition);
        if (twistVoltage > twistLimit) {twistVoltage = twistLimit;}
        if (twistVoltage < -twistLimit) {twistVoltage = -twistLimit;}

        // Control by motor voltage
        float motorVoltageNewA;
        motorVoltageNewA = -strokeVoltage + twistVoltage;
        if (m_bootmode == BLDCBootMode::CALIBRATE) 
        {
            if (motorVoltageNewA < -0.5) { motorVoltageNewA = -0.5; }
            if (motorVoltageNewA > 0.5) { motorVoltageNewA = 0.5; } 
        }

        // Low pass filter to reduce motor noise
        motorVoltageA = lowPassFilter*motorVoltageA + (1-lowPassFilter)*motorVoltageNewA;  
        // Motion control function
        if(motorA && motorA->enabled)
            motorA->move(motorVoltageA);

            // Control by motor voltage
        float motorVoltageNewB;
        motorVoltageNewB = strokeVoltage + twistVoltage;
        if (m_bootmode == BLDCBootMode::CALIBRATE) 
        {
            if (motorVoltageNewB < -0.5) { motorVoltageNewB = -0.5; }
            if (motorVoltageNewB > 0.5) { motorVoltageNewB = 0.5; }
        }

        // Low pass filter to reduce motor noise
        motorVoltageB = lowPassFilter*motorVoltageB + (1-lowPassFilter)*motorVoltageNewB;  
        // Motion control function
        if(motorB && motorB->enabled)
            motorB->move(motorVoltageB);
    }

    void log() 
    {
        if(LogHandler::getLogLevel() == LogLevel::VERBOSE) 
        {
            unsigned long currentMillis = millis();
            if (currentMillis - previousMillis >= interval) 
            {
                previousMillis = currentMillis;
                LogHandler::verbose(_TAG, "%s position: %f \t motorVoltage: %f \t bootmode: %ld \t tcode: %ld \t zeroAngleA: %f \t sensorAngleA: %f", 
                    "Motor A", m_motorAnglePositionA, motorVoltageA, m_bootmode, zeroAngleA, sensorAngleA);
                if(motorB) 
                {
                    //SIMPLEFOC_DEBUG("Motor B position: %f", m_motorAnglePositionB);
                    LogHandler::verbose(_TAG, "%s position: %f \t motorVoltage: %f \t bootmode: %ld \t tcode: %ld \t zeroAngleB: %f \t sensorAngleB: %f", 
                        "Motor B", m_motorAnglePositionB, motorVoltageB, m_bootmode, zeroAngleB, sensorAngleB);
                }                      
                counter = 0;
            }
            counter++;
        }
    }
};
