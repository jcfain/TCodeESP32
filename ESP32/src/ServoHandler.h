
// OSR-Release v2.4,
// by TempestMAx 1-7-20
// Please copy, share, learn, innovate, give attribution.
// Decodes T-code commands and uses them to control servos and vibration motors
// Can handle three linear channels (L0, L1, L2), three rotation channels (R0, R1, R2) 
// and two vibration channels (V0, V1)
// This code is designed to drive the OSR series of robot, but is also intended to be
// used as a template to be adapted to run other t-code controlled arduino projects
// Have fun, play safe!
// History:
// v2.0 - TCode v0.2 compatible, 28-1-2020
// v2.1 - OSR2 release, 1-2-2020
// v2.2 - OSR2+ release, 1-3-2020
// v2.3 - T-Valve support added, 1-5-2020
// v2.4 - T-wist support added; LR servos now +/- 350 for the sake of Raser1's sanity, 7-10-2020

#include <ESP32Servo.h>
#include "ToyComs.h"

class ServoHandler 
{

private:
    ToyComms toy; 
    // Declare servos
    Servo ForeAftServo;  // Fore-Aft Servo
    Servo RightServo;  // Right Servo
    Servo LeftServo;  // Left Servo
    Servo PitchServo;  // Pitch Servo
    Servo ValveServo;  // Valve Servo
    Servo TwistServo;  // Twist Servo

    // Specify which pins are attached to what here
    #define ForeAftServo_PIN 19  // Fore-Aft Servo
    #define RightServo_PIN 12  // Right Servo
    #define LeftServo_PIN 13  // Left Servo
    #define PitchServo_PIN 14  // Pitch Servo
    #define ValveServo_PIN 15  // Valve Servo
    #define TwistServo_PIN 2  // Twist Servo
    #define Vibe0_PIN 22   // Vibration motor 1
    #define Vibe1_PIN 23   // Vibration motor 2
    #define Pot1_PIN 33   // Twist potentiometer

    // Declare timing variables
    unsigned long nextPulse;
    int tick;

    // Position variables
    float xLin,yLin,zLin;
    // Rotation variables
    float xRot,yRot,zRot;
    // Vibration variables
    float vibe0,vibe1;
    // Velocity tracker variables, for T-Valve
    float xLast;
    float xValve;

public:
    // Setup function
    // This is run once, when the arduino starts
    void setup() 
    {
        toy.identifyTCode();
        
        // Declare servos and set zero
        int forAftChannel = ForeAftServo.attach(ForeAftServo_PIN);
        if (forAftChannel == 0) 
        {
            Serial.print("Failure to connect to foraft pin: ");
            Serial.println(ForeAftServo_PIN);
        }
        int rightChannel = RightServo.attach(RightServo_PIN);
        if (rightChannel == 0) 
        {
            Serial.print("Failure to connect to right pin: ");
            Serial.println(RightServo_PIN);
        }
        int leftChannel = LeftServo.attach(LeftServo_PIN);
        if (leftChannel == 0) 
        {
            Serial.print("Failure to connect to left pin: ");
            Serial.println(LeftServo_PIN);
        }
        int pitchChannel = PitchServo.attach(PitchServo_PIN);
        if (pitchChannel == 0) 
        {
            Serial.print("Failure to connect to pitch pin: ");
            Serial.println(PitchServo_PIN);
        }
        int valveChannel = ValveServo.attach(ValveServo_PIN);
        if (valveChannel == 0) 
        {
            Serial.print("Failure to connect to valve pin: ");
            Serial.println(ValveServo_PIN);
        }
        int twistChannel = TwistServo.attach(TwistServo_PIN); 
        if (twistChannel == 0) 
        {
            Serial.print("Failure to connect to twist pin: ");
            Serial.println(TwistServo_PIN);
        }

        pinMode(Pot1_PIN,INPUT);
        adcAttachPin(Pot1_PIN);
        analogReadResolution(11);
        analogSetAttenuation(ADC_6db);

        delay(500);
        
        ForeAftServo.writeMicroseconds(1500);
        RightServo.writeMicroseconds(1500);
        LeftServo.writeMicroseconds(1500);
        PitchServo.writeMicroseconds(1500);
        ValveServo.writeMicroseconds(1500);
        TwistServo.writeMicroseconds(1500);

        // Set vibration PWM pins
        pinMode(Vibe0_PIN,OUTPUT);
        pinMode(Vibe1_PIN,OUTPUT);
        // Test vibration channels
        analogWrite(Vibe0_PIN,127);
        delay(300);
        analogWrite(Vibe0_PIN,0);
        analogWrite(Vibe1_PIN,127);
        delay(300);
        analogWrite(Vibe1_PIN,0);

        // Set servo pulse interval
        tick = 20; //ms
        // Set time for first pulse
        nextPulse = millis() + tick;

        // Velocity tracker
        xLast = 500;
        xValve = 0;

        // Signal done
        Serial.println("Ready!");

    }

    // ----------------------------
    //   MAIN
    // ----------------------------
    // This loop runs continuously

    void read(byte inByte) 
    {
        //Serial.print(inByte);
        // This will run continuously
        // Send the serial bytes to the t-code object
        // This is the only required input for the object
        toy.serialRead(inByte);
    }

    void execute() 
    {

        // Pulse Servos based on time interval
        // This function will run every 20ms, sending a pulse to the servos
        if (millis() > nextPulse) { 
            unsigned long t = nextPulse;
            nextPulse = nextPulse + tick;
            // Collect inputs
            // These functions query the t-code object for the position/level at a specified time
            // Number recieved will be an integer, 1-1000
                // Serial.print("SH xLinear ");
                // Serial.printf("n: 0, t: %lu\n", t);
            xLin = toy.xLinear(0,t);
            //Serial.printf("xLin: %u, t: %lu\n", xLin, t);
            yLin = toy.xLinear(1,t);
            //zLin = toy.xLinear(2,t); (not used)
            xRot = toy.xRotate(0,t);
            yRot = toy.xRotate(1,t);
            zRot = toy.xRotate(2,t);
            // Serial.print("zRot ");
            // Serial.println(zRot);
            //Serial.printf("zRot: %u, t: %lu\n", zRot, t);
            vibe0 = toy.xVibe(0,t);
            vibe1 = toy.xVibe(1,t);

            // If you want to mix your servos differently, enter your code below:
            
            // Forward-Backward compensation
            // This calculates platform movement to account for the arc of the servos
            float lin1,lin2;
            int b2;
            lin1 = xLin-500;
            lin1 = lin1*0.00157079632;
            lin2 = 0.853-cos(lin1);
            lin2 = 1133*lin2;
            b2 = lin2;

            // Calculate valve position
            float Vel,ValveCmd,suck;
            Vel = xLin - xLast;
            xLast = xLin;
            suck = 20;
            if (Vel > suck) {
                ValveCmd = Vel-suck;
            } else if (Vel < 0){
                ValveCmd = -Vel;
            } else {
                ValveCmd = 0;
            }
            xValve = (4*xValve + ValveCmd)/5;
            int e;
            e = 20*xValve;
            if (e > 1000) {e = 1000;}


            // Mix and send servo channels
            // Linear scale inputs to servo appropriate numbers
            int a,b,c,d,twist;
            a = map(xLin,1,1000,-350,350);
            b = map(yLin,1,1000,-180,180);
            c = map(yRot,1,1000,-180,180);
            d = map(zRot,1,1000,-350,350);
            int potentiometerIn = analogRead(Pot1_PIN);
            if (potentiometerIn > 0) {
                twist = 5 * (xRot - map(potentiometerIn,923,100,1,1000));
                twist = constrain(twist, -750, 750);
            } else {
                twist = map(xRot,480,520,-180,180);
            }

            //Serial.printf("a %d, b %d, c %d, d %d, twist %d\n", a,b,c,d,twist);
            //Serial.printf("zRot %d, yLin %d, yRot %d, zRot %d, xRot %d\n", xLin,yLin,yRot,zRot,xRot);
            
            // Send signals to the servos
            // Note: 1000 = -45deg, 2000 = +45deg
            ForeAftServo.writeMicroseconds(1500 - b + b2);
            RightServo.writeMicroseconds(1500 + a + c);
            LeftServo.writeMicroseconds(1500 - a + c);
            PitchServo.writeMicroseconds(1500 - d);
            ValveServo.writeMicroseconds(2000 - e);
            TwistServo.writeMicroseconds(1500 + twist);

            // Done with servo channels

            // Output vibration channels
            // These should drive PWM pins connected to vibration motors via MOSFETs or H-bridges.
            if ((vibe0 > 1) && (vibe0 <= 1000)) {
                analogWrite(Vibe0_PIN,map(vibe0,2,1000,31,255));
            } else {
                analogWrite(Vibe0_PIN,0);
            }
            if ((vibe1 > 1) && (vibe1 <= 1000)) {
                analogWrite(Vibe1_PIN,map(vibe1,2,1000,31,255));
            } else {
                analogWrite(Vibe1_PIN,0);
            }

            // Done with vibration channels
            
        }
    }
};
