
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
// v2.6 - Experimental build. For use with Parallax Feedback 360° Servo (900-00360) in T-wist. 23-9-2020

#include <ESP32Servo.h>
#include "ToyComs.h"
#include "SettingsHandler.h"

volatile int twistFeedBackPin = SettingsHandler::TwistFeedBack_PIN;
// Twist position monitor variables
volatile int twistPulseLength = 0;
volatile int twistPulseCycle = 1099;
volatile int twistPulseStart = 0;
// Twist position detection functions
void IRAM_ATTR twistChange() 
{
	if(digitalRead(twistFeedBackPin) == HIGH)
	{
		twistPulseCycle = micros()-twistPulseStart;
  		twistPulseStart = micros();
	}
	else
	{
  		twistPulseLength = micros()-twistPulseStart;
	}
}

class ServoHandler 
{

private:
    ToyComms toy; 
    // Declare servos
    Servo RightServo;
    Servo LeftServo;
    Servo PitchServo;
    Servo ValveServo;
    Servo TwistServo;

    // Specify which pins are attached to what here
    const int RightServo_PIN = SettingsHandler::RightServo_PIN;
    const int LeftServo_PIN = SettingsHandler::LeftServo_PIN;
    const int PitchServo_PIN = SettingsHandler::PitchServo_PIN;
    const int ValveServo_PIN = SettingsHandler::ValveServo_PIN;
    const int TwistServo_PIN = SettingsHandler::TwistServo_PIN;
    const int Vibe0_PIN = SettingsHandler::Vibe0_PIN;
    const int Vibe1_PIN = SettingsHandler::Vibe1_PIN;

    // Arm servo zeros
    // Change these to adjust arm positions
    // (1500 = centre)
    const int RightServo_ZERO = 1500;
    const int LeftServo_ZERO = 1500;
    const int PitchServo_ZERO = 1500;

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

	// Twist position monitor variables
	float twistServoAngPos = 0.5;
	int twistTurns = 0;
	float twistPos;

public:
    // Setup function
    // This is run once, when the arduino starts
    void setup(int servoFrequency) 
    {
        toy.identifyTCode();
        
        RightServo.setPeriodHertz(servoFrequency);
        LeftServo.setPeriodHertz(servoFrequency);
        PitchServo.setPeriodHertz(servoFrequency);
        // ValveServo.setPeriodHertz(servoFrequency);
        // TwistServo.setPeriodHertz(servoFrequency);

        // Declare servos and set zero
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


        delay(500);
        
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
        tick = 1000/servoFrequency; //ms
        // Set time for first pulse
        nextPulse = millis() + tick;

        // Velocity tracker, for T-Valve
        xLast = 500;
        xValve = 0;

		// Initiate position tracking for twist
		pinMode(twistFeedBackPin, INPUT);
		attachInterrupt(twistFeedBackPin, twistChange, CHANGE);

        // Signal done
        Serial.println("Ready!");

    }

    // ----------------------------
    //   MAIN
    // ----------------------------
    // This loop runs continuously

    void read(byte inByte) 
    {
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
            // Serial.printf("zRot: %u, t: %lu\n", zRot, t);
            vibe0 = toy.xVibe(0,t);
            vibe1 = toy.xVibe(1,t);

            // If you want to mix your servos differently, enter your code below:
            

            // Calculate valve position
            float Vel,ValveCmd,suck;
            Vel = xLin - xLast;
            Vel = 50*Vel/tick;
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

            if (!SettingsHandler::continousTwist) {
				// Calculate twist position
				float dutyCycle = twistPulseLength;
				dutyCycle = dutyCycle/twistPulseCycle;
				float angPos = (dutyCycle - 0.029)/0.942;
				angPos = constrain(angPos,0,1) - 0.5;
				if (angPos - twistServoAngPos < - 0.8) { twistTurns += 1; }
				if (angPos - twistServoAngPos > 0.8) { twistTurns -= 1; }
				twistServoAngPos = angPos;
				twistPos = 1000*(angPos + twistTurns);
			}

            // Mix and send servo channels
            // Linear scale inputs to servo appropriate numbers
            int stroke,roll,pitch,valve,twist;
            stroke = map(xLin,1,1000,-350,350);
            roll   = map(yRot,1,1000,-180,180);
            pitch  = map(zRot,1,1000,-350,350);
            valve  = 20*xValve;
            valve  = constrain(valve, 0, 1000);   
            if (!SettingsHandler::continousTwist) {
    			twist  = 2*(xRot - map(twistPos,-1500,1500,1000,1));
                twist = constrain(twist, -750, 750);
            } else {
                twist = map(xRot,1,1000,-180,180);
            }
            // Serial.print("SettingsHandler::continousTwist: ");
            // Serial.println(SettingsHandler::continousTwist);
            // Serial.print("twistPos: ");
            // Serial.println(twistPos);
            // Serial.print("twist: ");
            // Serial.println(twist);
            // Serial.print("xRot: ");
            // Serial.println(xRot);


            //Serial.printf("a %d, b %d, c %d, d %d, twist %d\n", a,b,c,d,twist);
            //Serial.printf("zRot %d, yLin %d, yRot %d, zRot %d, xRot %d\n", xLin,yLin,yRot,zRot,xRot);
            
            // Send signals to the servos
            // Note: 1000 = -45deg, 2000 = +45deg
            RightServo.writeMicroseconds(RightServo_ZERO + stroke + roll);
            LeftServo.writeMicroseconds(LeftServo_ZERO - stroke + roll);
            PitchServo.writeMicroseconds(PitchServo_ZERO - pitch);
            ValveServo.writeMicroseconds(2000 - valve);
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
