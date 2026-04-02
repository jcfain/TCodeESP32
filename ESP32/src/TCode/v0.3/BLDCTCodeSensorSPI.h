#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <SimpleFOC.h>

// This code is a custom method for accessing the AS5048a encoders on the
// SSR2 motors. This code produces a significant improvement of the read 
// rate of the encoders, which makes the motors run a lot quieter!
class ReadEncoder
{
public:
    ReadEncoder(const int& pin, SPIClass* spi) : pin(pin), spi(spi) {}
    float operator()() const {
        digitalWrite(pin, LOW);
        uint16_t raw = spi->transfer16(0xFFFF);  // read angle command (NOP)
        digitalWrite(pin, HIGH);
        auto mask = raw & 0x3FFF;  // mask to 14-bit angle
        // Convert to radians [0, 2π)
        return (float)mask * (2 * M_PI) / 16384.0f;
    }
private:
    int pin;
    SPIClass* spi;
};

class InitEncoders
{
public:
    InitEncoders(const int& pinA, const int& pinB = -1, SPIClass* spi = &SPI) : pinA(pinA), pinB(pinB), spi(spi) {}
    void operator()() const {
        // Called for both sensors → do shared init only
        pinMode(pinA, OUTPUT);
        digitalWrite(pinA, HIGH);
        if(pinB > -1)
        {
            pinMode(pinB, OUTPUT);
            digitalWrite(pinB, HIGH);
        }

        spi->begin();                     // default VSPI pins
        spi->setDataMode(SPI_MODE1);      // required for AS5048A
        spi->setFrequency(10000000);   // 10 MHz - AS5048A max spec
    }
private:
    int pinA = -1;
    int pinB = -1;
    SPIClass* spi = &SPI;
};

class BLDCTCodeSensorSPI: public Sensor
{
 public:
    BLDCTCodeSensorSPI(ReadEncoder readEncoder, InitEncoders initEncoder) 
        : readEncoder(readEncoder), initEncoder(initEncoder)
    {}

    // initialize the sensor hardware
    void init()
    {
        //sensor.hardwareInit();
        this->initEncoder();
        this->Sensor::init(); // call base class init
    }

    // Get current shaft angle from the sensor hardware, and 
    // return it as a float in radians, in the range 0 to 2PI.
    //  - This method is pure virtual and must be implemented in subclasses.
    //    Calling this method directly does not update the base-class internal fields.
    //    Use update() when calling from outside code.
    float getSensorAngle()
    {
        // Get current shaft angle from the sensor hardware, and 
        // return it as a float in radians, in the range 0 to 2PI.
        return this->readEncoder();
    }
private:
    ReadEncoder readEncoder;
    InitEncoders initEncoder;
};


// class InitEncoder
// {
// public:
//     InitEncoder(const int& pin) : pin(pin) {}
//     void operator()() const 
//     {
//         // Called for both sensors → do shared init only
//         pinMode(pin, OUTPUT);
//         digitalWrite(pin, HIGH);
//     }
// private:
//     int pin;
// };

// OG
// // Shared SPI instance (ESP32 VSPI defaults: SCK=18, MISO=19, MOSI=23)
// SPIClass* spi = &SPI;  // or create your own: SPIClass spi(VSPI);

// // Helper function to read raw 14-bit angle from one encoder
// uint16_t readAS5048A(int cs_pin) {
//   digitalWrite(cs_pin, LOW);
//   uint16_t raw = spi->transfer16(0xFFFF);  // read angle command (NOP)
//   digitalWrite(cs_pin, HIGH);
//   return raw & 0x3FFF;  // mask to 14-bit angle
// }

// // Callback for encoder A
// float readEncoderA() {
//   uint16_t raw = readAS5048A(ChipSelectA_PIN);
//   // Convert to radians [0, 2π)
//   return (float)raw * (2 * M_PI) / 16384.0f;
// }

// // Callback for encoder B
// float readEncoderB() {
//   uint16_t raw = readAS5048A(ChipSelectB_PIN);
//   return (float)raw * (2 * M_PI) / 16384.0f;
// }

// // One-time initialization (called once per sensor)
// void initEncoder() {
//   // Called for both sensors → do shared init only
//   pinMode(ChipSelectA_PIN, OUTPUT);
//   pinMode(ChipSelectB_PIN, OUTPUT);
//   digitalWrite(ChipSelectA_PIN, HIGH);
//   digitalWrite(ChipSelectB_PIN, HIGH);

//   spi->begin();                     // default VSPI pins
//   spi->setDataMode(SPI_MODE1);      // required for AS5048A
//   spi->setFrequency(10000000);   // 10 MHz - AS5048A max spec
// }