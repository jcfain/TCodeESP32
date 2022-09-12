
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
// v2.7 - T-valve suction level control added to L3. 25-11-2020
// v2.7b - T-valve suction level algorithm modified. 30-11-2020

#pragma once

#include <ESP32Servo.h>
#include "ToyComs.h"
#include "../Global.h"
#include "../ServoHandler.h"

/* volatile int twistFeedBackPin = SettingsHandler::TwistFeedBack_PIN;
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
} */

class ServoHandler0_2 : public ServoHandler
{

private:
	const char* _TAG = "ServoHandler0_2";
    ToyComms toy; 
    // Declare servos
    Servo RightServo;
    Servo LeftServo;
    Servo RightUpperServo;
    Servo LeftUpperServo;
    Servo PitchLeftServo;
    Servo PitchRightServo;
    Servo ValveServo;
    Servo TwistServo;

    // Specify which pins are attached to what here
    int RightServo_PIN = SettingsHandler::RightServo_PIN;
    int LeftServo_PIN = SettingsHandler::LeftServo_PIN;
    int RightUpperServo_PIN = SettingsHandler::RightUpperServo_PIN;
    int LeftUpperServo_PIN = SettingsHandler::LeftUpperServo_PIN;
    int PitchLeftServo_PIN = SettingsHandler::PitchLeftServo_PIN;
    int PitchRightServo_PIN = SettingsHandler::PitchRightServo_PIN;
    int ValveServo_PIN = SettingsHandler::ValveServo_PIN;
    int TwistServo_PIN = SettingsHandler::TwistServo_PIN;
    int Vibe0_PIN = SettingsHandler::Vibe0_PIN;
    int Vibe1_PIN = SettingsHandler::Vibe1_PIN;
    int LubeManual_PIN = SettingsHandler::LubeManual_PIN;

	int rightServoConnected = 0;
	int rightUpperServoConnected = 0;
	int leftServoConnected = 0;
	int leftUpperServoConnected = 0;
	int pitchServoConnected = 0;
	int pitchRightServoConnected = 0;
	int valveServoConnected = 0;
	int twistServoConnected = 0;

    // Declare timing variables
    unsigned long nextPulse;
    int tick;

    // Position variables
    float xLin,yLin,zLin;
    // Rotation variables
    float xRot,yRot,zRot;
    // Vibration variables
    float vibe0,vibe1;
	
	int suck;
    // Velocity tracker variables, for T-Valve
    float xLast;
    float xValve;

	// Twist position monitor variables
	float twistServoAngPos = 0.5;
	int twistTurns = 0;
	float twistPos;

	int lubeAmount = SettingsHandler::lubeAmount;

	// Function to calculate the angle for the main arm servos
	// Inputs are target x,y coords of receiver pivot in 1/100 of a mm
	int SetMainServo(float x, float y) 
	{
		x /= 100; y /= 100;          // Convert to mm
		float gamma = atan2(x,y);    // Angle of line from servo pivot to receiver pivot
		float csq = sq(x) + sq(y);   // Square of distance between servo pivot and receiver pivot
		float c = sqrt(csq);         // Distance between servo pivot and receiver pivot
		float beta = acos((csq - 28125)/(100*c));  // Angle between c-line and servo arm
		int out = ms_per_rad*(gamma + beta - 3.14159); // Servo signal output, from neutral
		return out;
	}


	// Function to calculate the angle for the pitcher arm servos
	// Inputs are target x,y,z coords of receiver upper pivot in 1/100 of a mm
	// Also pitch in 1/100 of a degree
	int SetPitchServo(float x, float y, float z, float pitch) 
	{
		pitch *= 0.0001745; // Convert to radians
		x += 5500*sin(0.2618 + pitch);
		y -= 5500*cos(0.2618 + pitch);
		x /= 100; y /= 100; z /= 100;   // Convert to mm
		float bsq = 36250 - sq(75 + z); // Equivalent arm length
		float gamma = atan2(x,y);       // Angle of line from servo pivot to receiver pivot
		float csq = sq(x) + sq(y);      // Square of distance between servo pivot and receiver pivot
		float c = sqrt(csq);            // Distance between servo pivot and receiver pivot
		float beta = acos((csq + 5625 - bsq)/(150*c)); // Angle between c-line and servo arm
		int out = ms_per_rad*(gamma + beta - 3.14159); // Servo signal output, from neutral
		return out;
	}


public:
    // Setup function
    // This is run once, when the arduino starts
    void setup(int servoFrequency, int pitchFrequency, int valveFrequency, int twistFrequency) override 
    {
		toy.setup();
        toy.identifyTCode();

        RightServo.setPeriodHertz(servoFrequency);
        RightUpperServo.setPeriodHertz(servoFrequency);
        LeftServo.setPeriodHertz(servoFrequency);
        LeftUpperServo.setPeriodHertz(servoFrequency);
        PitchLeftServo.setPeriodHertz(pitchFrequency);
        PitchRightServo.setPeriodHertz(pitchFrequency);
		ValveServo.setPeriodHertz(valveFrequency);
		TwistServo.setPeriodHertz(twistFrequency);

		RightServo_PIN = SettingsHandler::RightServo_PIN;
		LeftServo_PIN = SettingsHandler::LeftServo_PIN;
		RightUpperServo_PIN = SettingsHandler::RightUpperServo_PIN;
		LeftUpperServo_PIN = SettingsHandler::LeftUpperServo_PIN;
		PitchLeftServo_PIN = SettingsHandler::PitchLeftServo_PIN;
		PitchRightServo_PIN = SettingsHandler::PitchRightServo_PIN;
		ValveServo_PIN = SettingsHandler::ValveServo_PIN;
		TwistServo_PIN = SettingsHandler::TwistServo_PIN;
		Vibe0_PIN = SettingsHandler::Vibe0_PIN;
		Vibe1_PIN = SettingsHandler::Vibe1_PIN;

        if(DEBUG_BUILD == 0) {
			// Declare servos and set zero
			rightServoConnected = RightServo.attach(RightServo_PIN);
			if (rightServoConnected == 0) 
			{
				// Serial.print("Failure to connect to right pin: ");
				// Serial.println(RightServo_PIN);
				LogHandler::error(_TAG, "Failure to connect to right pin: %i", RightServo_PIN);
			}
			leftServoConnected = LeftServo.attach(LeftServo_PIN);
			if (leftServoConnected == 0) 
			{
				// Serial.print("Failure to connect to left pin: ");
				// Serial.println(LeftServo_PIN);
				LogHandler::error(_TAG, "Failure to connect to left pin: %i", LeftServo_PIN);
			}
		}
		if(SettingsHandler::sr6Mode)
		{
			leftUpperServoConnected = LeftUpperServo.attach(LeftUpperServo_PIN);
			if (leftUpperServoConnected == 0) 
			{
				// Serial.print("Failure to connect to left upper pin: ");
				// Serial.println(LeftUpperServo_PIN);
				LogHandler::error(_TAG, "Failure to connect to left upper pin: %i", LeftUpperServo_PIN);
			}
			if(DEBUG_BUILD == 0) {
				rightUpperServoConnected = RightUpperServo.attach(RightUpperServo_PIN);
				if (rightUpperServoConnected == 0) 
				{
					// Serial.print("Failure to connect to right upper pin: ");
					// Serial.println(RightUpperServo_PIN);
					LogHandler::error(_TAG, "Failure to connect to right upper pin: %i", RightUpperServo_PIN);
				}
				pitchRightServoConnected = PitchRightServo.attach(PitchRightServo_PIN);
				if (pitchRightServoConnected == 0) 
				{
					// Serial.print("Failure to connect to pitch right pin: ");
					// Serial.println(PitchRightServo_PIN);
					LogHandler::error(_TAG, "Failure to connect to pitch right pin: %i", PitchRightServo_PIN);
				}
			}
		}
        pitchServoConnected = PitchLeftServo.attach(PitchLeftServo_PIN);
        if (pitchServoConnected == 0) 
        {
            // Serial.print("Failure to connect to pitch left pin: ");
            // Serial.println(PitchLeftServo_PIN);
			LogHandler::error(_TAG, "Failure to connect to pitch left pin: %i", PitchLeftServo_PIN);
        }
        valveServoConnected = ValveServo.attach(ValveServo_PIN);
        if (valveServoConnected == 0) 
        {
            // Serial.print("Failure to connect to valve pin: ");
            // Serial.println(ValveServo_PIN);
			LogHandler::error(_TAG, "Failure to connect to valve pin: %i", ValveServo_PIN);
        }
        twistServoConnected = TwistServo.attach(TwistServo_PIN); 
        if (twistServoConnected == 0) 
        {
            // Serial.print("Failure to connect to twist pin: ");
            // Serial.println(TwistServo_PIN);
			LogHandler::error(_TAG, "Failure to connect to twist pin: %i", TwistServo_PIN);
        }

        delay(500);
        
        if (rightServoConnected != 0) 
		{
        	RightServo.writeMicroseconds(SettingsHandler::RightServo_ZERO);
		}
        if (leftServoConnected != 0) 
		{
        	LeftServo.writeMicroseconds(SettingsHandler::LeftServo_ZERO);
		}
		if(SettingsHandler::sr6Mode)
		{
			if (rightUpperServoConnected != 0) 
			{
				RightUpperServo.writeMicroseconds(SettingsHandler::RightUpperServo_ZERO);
			}
			if (pitchRightServoConnected != 0) 
			{
				PitchRightServo.writeMicroseconds(SettingsHandler::PitchRightServo_ZERO);
			}
			if (leftUpperServoConnected != 0) 
			{
				LeftUpperServo.writeMicroseconds(SettingsHandler::LeftUpperServo_ZERO);
			}
		}
        if (pitchServoConnected != 0) 
		{
        	PitchLeftServo.writeMicroseconds(SettingsHandler::PitchLeftServo_ZERO);
		}
        if (valveServoConnected != 0) 
		{
        	ValveServo.writeMicroseconds(SettingsHandler::ValveServo_ZERO);
		}
        if (twistServoConnected != 0) 
		{
        	TwistServo.writeMicroseconds(SettingsHandler::TwistServo_ZERO);
		}

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
  		pinMode(LubeManual_PIN,INPUT);

        // Set servo pulse interval
        tick = 1000/servoFrequency; //ms
        // Set time for first pulse
        nextPulse = millis() + tick;

        // Velocity tracker, for T-Valve
        xLast = 500;
        xValve = 0;

		// Initiate position tracking for twist
		pinMode(SettingsHandler::TwistFeedBack_PIN, INPUT);
		attachInterrupt(SettingsHandler::TwistFeedBack_PIN, twistChange, CHANGE);

        // Signal done
        toy.sendMessage("Ready!");

    }

    void setMessageCallback(TCODE_FUNCTION_PTR_T function) override {
        toy.setMessageCallback(function);
    }

    void read(String input) override
    {
		for (int x = 0; x < sizeof(input.length() + 1); x++) { 
			read(input[x]); 
			execute();
		} 
    }

    void read(byte inByte) override 
    {
        // Send the serial bytes to the t-code object
        // This is the only required input for the object
        toy.serialRead(inByte);
    }

    void execute() override 
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

			// SR6 /////////////////
			yLin = toy.xLinear(1,t);
			zLin = toy.xLinear(2,t);
			////////////////////////

    		suck = toy.xLinear(3,t);
            xRot = toy.xRotate(0,t);
            yRot = toy.xRotate(1,t);
            zRot = toy.xRotate(2,t);
            // Serial.print("zRot ");
            // Serial.println(zRot);
            // Serial.printf("zRot: %u, t: %lu\n", zRot, t);
            vibe0 = toy.xVibe(0,t);
            vibe1 = toy.xVibe(1,t);

            // If you want to mix your servos differently, enter your code below:
            

            //Calculate valve position
			//Track receiver velocity
			if (SettingsHandler::autoValve)
			{
				float Vel,ValveCmd,localSuck;
				Vel = xLin - xLast;
				Vel = 50*Vel/tick;
				xLast = xLin;
				localSuck = 20;
				if (Vel > localSuck) {
					ValveCmd = Vel-localSuck;
				} else if (Vel < 0){
					ValveCmd = -Vel;
				} else {
					ValveCmd = 0;
				}
				xValve = (4*xValve + ValveCmd)/5;
			} 
			else 
			{
				float Vel,ValveCmd;
				Vel = xLin - xLast;
				xLast = xLin;
				if (Vel < 0) {
					ValveCmd = 1000;
				} else {
					ValveCmd = 1000-suck;
				}
				xValve = (2*xValve + ValveCmd)/3;     
			}

            //Serial.print("xValve: ");
            //Serial.println(xValve);
            if (SettingsHandler::feedbackTwist && !SettingsHandler::continuousTwist) 
			{
				// Calculate twist position
                noInterrupts();
                float dutyCycle = twistPulseLength;
                dutyCycle = dutyCycle/lastTwistPulseCycle;
                interrupts();
				float angPos = (dutyCycle - 0.029)/0.942;
				angPos = constrain(angPos,0,1) - 0.5;
				if (angPos - twistServoAngPos < - 0.8) { twistTurns += 1; }
				if (angPos - twistServoAngPos > 0.8) { twistTurns -= 1; }
				twistServoAngPos = angPos;
				twistPos = 1000*(angPos + twistTurns);
			}

			if (SettingsHandler::sr6Mode) 
			{
				//Serial.print("SR6 mode");
				int roll,pitch,fwd,thrust,side;
				roll = map(yRot,0,1000,-3000,3000);
				pitch = map(zRot,0,1000,-2500,2500);
				fwd = map(yLin,0,1000,-3000,3000);
				thrust = map(xLin,0,1000,-6000,6000);
    			side = map(zLin,0,1000,-3000,3000);

				// Main
            	int lowerLeftValue,upperLeftValue,pitchLeftValue,pitchRightValue,upperRightValue,lowerRightValue;
				if(SettingsHandler::inverseStroke) 
				{
					lowerLeftValue = SetMainServo(16248 - fwd, 1500 - thrust - roll);
					upperLeftValue = SetMainServo(16248 - fwd, 1500 + thrust + roll);
					upperRightValue = SetMainServo(16248 - fwd, 1500 + thrust - roll);
					lowerRightValue = SetMainServo(16248 - fwd, 1500 - thrust + roll);
					pitchLeftValue = SetPitchServo(16248 - fwd, 4500 + thrust, -side + 1.5*roll, -pitch);
					pitchRightValue = SetPitchServo(16248 - fwd, 4500 + thrust, side - 1.5*roll, -pitch);
				} 
				else 
				{
					lowerLeftValue = SetMainServo(16248 - fwd, 1500 + thrust + roll);
					upperLeftValue = SetMainServo(16248 - fwd, 1500 - thrust - roll);
					upperRightValue = SetMainServo(16248 - fwd, 1500 - thrust + roll);
					lowerRightValue = SetMainServo(16248 - fwd, 1500 + thrust - roll);
					pitchLeftValue = SetPitchServo(16248 - fwd, 4500 - thrust, side - 1.5*roll, -pitch);
					pitchRightValue = SetPitchServo(16248 - fwd, 4500 - thrust, -side + 1.5*roll, -pitch);
				}
				if (leftServoConnected != 0) 
					LeftServo.writeMicroseconds(SettingsHandler::LeftServo_ZERO - lowerLeftValue);
				if (leftUpperServoConnected != 0) 
					LeftUpperServo.writeMicroseconds(SettingsHandler::LeftUpperServo_ZERO + upperLeftValue);
				if (rightServoConnected != 0) 
					RightServo.writeMicroseconds(SettingsHandler::RightServo_ZERO + lowerRightValue);
				if (rightUpperServoConnected != 0) 
					RightUpperServo.writeMicroseconds(SettingsHandler::RightUpperServo_ZERO - upperRightValue);
				if (pitchServoConnected != 0) 
					PitchLeftServo.writeMicroseconds(constrain(SettingsHandler::PitchLeftServo_ZERO - pitchLeftValue, SettingsHandler::PitchLeftServo_ZERO-600, SettingsHandler::PitchLeftServo_ZERO+1000));
				if (pitchRightServoConnected != 0) 
					PitchRightServo.writeMicroseconds(constrain(SettingsHandler::PitchRightServo_ZERO + pitchRightValue, SettingsHandler::PitchRightServo_ZERO-1000, SettingsHandler::PitchRightServo_ZERO+600));
			}
			else 
			{
				//Serial.print("OSR mode");
				// Mix and send servo channels
				// Linear scale inputs to servo appropriate numbers
				int stroke,roll,pitch;
				stroke = map(xLin,1,1000,-350,350);
				roll   = map(yRot,1,1000,-180,180);
				pitch  = map(zRot,1,1000,-350,350);  
				//valve  = map(valve,1000,1, 1,1000);  
				//valve  = constrain(xValve, 0, 1000);
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

				if(SettingsHandler::inverseStroke) 
				{
					if (rightServoConnected != 0) 
						RightServo.writeMicroseconds(SettingsHandler::RightServo_ZERO + stroke + roll);
					if (leftServoConnected != 0) 
						LeftServo.writeMicroseconds(SettingsHandler::LeftServo_ZERO - stroke + roll);
				} 
				else 
				{
					if (rightServoConnected != 0) 
						RightServo.writeMicroseconds(SettingsHandler::RightServo_ZERO - stroke + roll);
					if (leftServoConnected != 0) 
						LeftServo.writeMicroseconds(SettingsHandler::LeftServo_ZERO + stroke + roll);
				}
				if(SettingsHandler::inversePitch) 
				{
					if (pitchServoConnected != 0) 
						PitchLeftServo.writeMicroseconds(SettingsHandler::PitchLeftServo_ZERO + pitch);
				}
				else 
				{
					if (pitchServoConnected != 0) 
						PitchLeftServo.writeMicroseconds(SettingsHandler::PitchLeftServo_ZERO - pitch);
				}
			}

			int valve,twist;
        	valve  = xValve - 500;
			valve = constrain(valve, -500, 500);
        	if (SettingsHandler::inverseValve) { valve = -valve; }
			if(SettingsHandler::valveServo90Degrees)
			{
				if (SettingsHandler::inverseValve) { 
					valve = map(valve,0,500,-500,500);
				} 
				else
				{
					valve = map(valve,-500,0,-500,500);
				}
			}

        	if (SettingsHandler::feedbackTwist && !SettingsHandler::continuousTwist) 
			{
				twist = 2*(xRot - map(twistPos,-1500,1500,1000,1));
				twist = constrain(twist, -750, 750);
			} 
			else 
			{
    			twist  = map(xRot,0,1000,1000,-1000);
			}

			if (valveServoConnected != 0) 
				ValveServo.writeMicroseconds(SettingsHandler::ValveServo_ZERO + valve);
			if (twistServoConnected != 0) 
				TwistServo.writeMicroseconds(SettingsHandler::TwistServo_ZERO + twist);

            // Done with servo channels

            // Output vibration channels
            // These should drive PWM pins connected to vibration motors via MOSFETs or H-bridges.
            if ((vibe0 > 1) && (vibe0 <= 1000)) 
                analogWrite(Vibe0_PIN,map(vibe0,2,1000,31,255));
			else 
                analogWrite(Vibe0_PIN,0);

            if ((vibe1 > 1) && (vibe1 <= 1000)) 
                analogWrite(Vibe1_PIN,map(vibe1,2,1000,31,255));
			else if (digitalRead(LubeManual_PIN) == HIGH) 
				// For iLube - if no software action, check the manual button too
				analogWrite(Vibe1_PIN, lubeAmount);
			else 
      			analogWrite(Vibe1_PIN,0);

            // Done with vibration channels
            
        }
    }
};
