
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
#include "SettingsHandler.h"
#include "ToyComs.h"
#include "Global.h"
#include "MotorHandler.h"
#include "TagHandler.h"

/* volatile int twistFeedBackPin = m_settingsFactory->TwistFeedBack_PIN;
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

class ServoHandler0_2 : public MotorHandler
{
private:
    const char* _TAG = TagHandler::ServoHandler2;
	SettingsFactory* m_settingsFactory;
	DeviceType m_deviceType;
    int MainServo_Freq;
    int PitchServo_Freq;
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
    int RightServo_PIN;
    int LeftServo_PIN;
    int RightUpperServo_PIN;
    int LeftUpperServo_PIN;
    int PitchLeftServo_PIN;
    int PitchRightServo_PIN;
    int ValveServo_PIN;
    int TwistServo_PIN;
    int Vibe0_PIN;
    int Vibe1_PIN;
    int LubeManual_PIN;

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

	int lubeAmount = m_settingsFactory->getLubeAmount();

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
    void setup() override 
    {
		m_settingsFactory = SettingsFactory::getInstance();
        m_settingsFactory->getValue(MS_PER_RAD, ms_per_rad);
        m_settingsFactory->getValue(SERVO_FREQUENCY, MainServo_Freq);
        m_settingsFactory->getValue(PITCH_FREQUENCY, PitchServo_Freq);
        toy.setup();
        toy.identifyTCode();

        RightServo.setPeriodHertz(MainServo_Freq);
        RightUpperServo.setPeriodHertz(MainServo_Freq);
        LeftServo.setPeriodHertz(MainServo_Freq);
        LeftUpperServo.setPeriodHertz(MainServo_Freq);
        PitchLeftServo.setPeriodHertz(MainServo_Freq);
        PitchRightServo.setPeriodHertz(PitchServo_Freq);
		int valveFreq = VALVE_FREQUENCY_DEFAULT;
        m_settingsFactory->getValue(VALVE_FREQUENCY, valveFreq);
		ValveServo.setPeriodHertz(valveFreq);
		int twistFreq = TWIST_FREQUENCY_DEFAULT;
        m_settingsFactory->getValue(TWIST_FREQUENCY, PitchServo_Freq);
		TwistServo.setPeriodHertz(twistFreq);
        m_settingsFactory->getValue(DEVICE_TYPE, m_deviceType);
        PinMap* pinMap;
        if (m_deviceType == DeviceType::SR6) 
        {
            pinMap = PinMapSR6::getInstance();
        } 
        else 
        {
            pinMap = PinMapOSR::getInstance();
			RightServo_PIN = ((PinMapOSR*)pinMap)->rightServo();
			LeftServo_PIN = ((PinMapOSR*)pinMap)->leftServo();
			PitchLeftServo_PIN =((PinMapOSR*)pinMap)->pitchLeft();
        }

        if (m_deviceType == DeviceType::SR6) 
		{
			RightUpperServo_PIN = ((PinMapSR6*)pinMap)->pitchLeft();
			LeftUpperServo_PIN = m_settingsFactory->getLeftUpperServo_PIN();
			PitchRightServo_PIN = m_settingsFactory->getPitchRightServo_PIN();
		}
		ValveServo_PIN = m_settingsFactory->getValveServo_PIN();
		TwistServo_PIN = m_settingsFactory->getTwistServo_PIN();
		Vibe0_PIN = m_settingsFactory->getVibe0_PIN();
		Vibe1_PIN = m_settingsFactory->getVibe1_PIN();

        if(!DEBUG_BUILD) {// The default pins for these are used on the debugger board.
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
		if(m_settingsFactory->sr6Mode)
		{
			leftUpperServoConnected = LeftUpperServo.attach(LeftUpperServo_PIN);
			if (leftUpperServoConnected == 0) 
			{
				// Serial.print("Failure to connect to left upper pin: ");
				// Serial.println(LeftUpperServo_PIN);
				LogHandler::error(_TAG, "Failure to connect to left upper pin: %i", LeftUpperServo_PIN);
			}
			if(!DEBUG_BUILD) {// The default pins for these are used on the debugger board.
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
        	RightServo.writeMicroseconds(m_settingsFactory->getRightServo_ZERO());
		}
        if (leftServoConnected != 0) 
		{
        	LeftServo.writeMicroseconds(m_settingsFactory->getLeftServo_ZERO());
		}
		if(m_settingsFactory->getDeviceType() == DeviceType::SR6)
		{
			if (rightUpperServoConnected != 0) 
			{
				RightUpperServo.writeMicroseconds(m_settingsFactory->getRightUpperServo_ZERO());
			}
			if (pitchRightServoConnected != 0) 
			{
				PitchRightServo.writeMicroseconds(m_settingsFactory->getPitchRightServo_ZERO());
			}
			if (leftUpperServoConnected != 0) 
			{
				LeftUpperServo.writeMicroseconds(m_settingsFactory->getLeftUpperServo_ZERO());
			}
		}
        if (pitchServoConnected != 0) 
		{
        	PitchLeftServo.writeMicroseconds(m_settingsFactory->getPitchLeftServo_ZERO());
		}
        if (valveServoConnected != 0) 
		{
        	ValveServo.writeMicroseconds(m_settingsFactory->getValveServo_ZERO());
		}
        if (twistServoConnected != 0) 
		{
        	TwistServo.writeMicroseconds(m_settingsFactory->getTwistServo_ZERO());
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
        tick = 1000/m_settingsFactory->getServoFrequency(); //ms
        // Set time for first pulse
        nextPulse = millis() + tick;

        // Velocity tracker, for T-Valve
        xLast = 500;
        xValve = 0;

		// Initiate position tracking for twist
		pinMode(m_settingsFactory->getTwistFeedBack_PIN(), INPUT);
		attachInterrupt(m_settingsFactory->getTwistFeedBack_PIN(), twistChange, CHANGE);

        // Signal done
        toy.sendMessage("Ready!");

    }

    void setMessageCallback(TCODE_FUNCTION_PTR_T function) override {
        toy.setMessageCallback(function);
    }

    void read(const String &input) override
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
			if (m_settingsFactory->getAutoValve())
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
            if (m_settingsFactory->getFeedbackTwist() && !m_settingsFactory->getContinuousTwist()) 
			{
				// Calculate twist position
                //noInterrupts();
                float dutyCycle = twistPulseLength;
                dutyCycle = dutyCycle/lastTwistPulseCycle;
                //interrupts();
				float angPos = (dutyCycle - 0.029)/0.942;
				angPos = constrain(angPos,0,1) - 0.5;
				if (angPos - twistServoAngPos < - 0.8) { twistTurns += 1; }
				if (angPos - twistServoAngPos > 0.8) { twistTurns -= 1; }
				twistServoAngPos = angPos;
				twistPos = 1000*(angPos + twistTurns);
			}

			if(m_settingsFactory->getDeviceType() == DeviceType::SR6)
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
				if(m_settingsFactory->getInverseStroke()) 
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
					LeftServo.writeMicroseconds(m_settingsFactory->getLeftServo_ZERO() - lowerLeftValue);
				if (leftUpperServoConnected != 0) 
					LeftUpperServo.writeMicroseconds(m_settingsFactory->getLeftUpperServo_ZERO() + upperLeftValue);
				if (rightServoConnected != 0) 
					RightServo.writeMicroseconds(m_settingsFactory->getRightServo_ZERO() + lowerRightValue);
				if (rightUpperServoConnected != 0) 
					RightUpperServo.writeMicroseconds(m_settingsFactory->getRightUpperServo_ZERO() - upperRightValue);
				if (pitchServoConnected != 0) 
					PitchLeftServo.writeMicroseconds(constrain(m_settingsFactory->getPitchLeftServo_ZERO() - pitchLeftValue, m_settingsFactory->PitchLeftServo_ZERO-600, m_settingsFactory->PitchLeftServo_ZERO+1000));
				if (pitchRightServoConnected != 0) 
					PitchRightServo.writeMicroseconds(constrain(m_settingsFactory->getPitchRightServo_ZERO() + pitchRightValue, m_settingsFactory->PitchRightServo_ZERO-1000, m_settingsFactory->PitchRightServo_ZERO+600));
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
				// Serial.print("m_settingsFactory->continousTwist: ");
				// Serial.println(m_settingsFactory->continousTwist);
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

				if(m_settingsFactory->getInverseStroke()) 
				{
					if (rightServoConnected != 0) 
						RightServo.writeMicroseconds(m_settingsFactory->getRightServo_ZERO() + stroke + roll);
					if (leftServoConnected != 0) 
						LeftServo.writeMicroseconds(m_settingsFactory->getLeftServo_ZERO() - stroke + roll);
				} 
				else 
				{
					if (rightServoConnected != 0) 
						RightServo.writeMicroseconds(m_settingsFactory->getRightServo_ZERO() - stroke + roll);
					if (leftServoConnected != 0) 
						LeftServo.writeMicroseconds(m_settingsFactory->getLeftServo_ZERO() + stroke + roll);
				}
				if(m_settingsFactory->getInversePitch()) 
				{
					if (pitchServoConnected != 0) 
						PitchLeftServo.writeMicroseconds(m_settingsFactory->getPitchLeftServo_ZERO() + pitch);
				}
				else 
				{
					if (pitchServoConnected != 0) 
						PitchLeftServo.writeMicroseconds(m_settingsFactory->getPitchLeftServo_ZERO() - pitch);
				}
			}

			int valve,twist;
        	valve  = xValve - 500;
			valve = constrain(valve, -500, 500);
        	if (m_settingsFactory->getInverseValve()) { valve = -valve; }
			if(m_settingsFactory->getValveServo90Degrees())
			{
				if (m_settingsFactory->getInverseValve()) { 
					valve = map(valve,0,500,-500,500);
				} 
				else
				{
					valve = map(valve,-500,0,-500,500);
				}
			}

        	if (m_settingsFactory->getFeedbackTwist() && !m_settingsFactory->getContinuousTwist()) 
			{
				twist = 2*(xRot - map(twistPos,-1500,1500,1000,1));
				twist = constrain(twist, -750, 750);
			} 
			else 
			{
    			twist  = map(xRot,0,1000,1000,-1000);
			}

			if (valveServoConnected != 0) 
				ValveServo.writeMicroseconds(m_settingsFactory->getValveServo_ZERO() + valve);
			if (twistServoConnected != 0) 
				TwistServo.writeMicroseconds(m_settingsFactory->getTwistServo_ZERO() + twist);

            // Done with servo channels

            // Output vibration channels
            // These should drive PWM pins connected to vibration motors via MOSFETs or H-bridges.
            if ((vibe0 > 1) && (vibe0 <= 1000)) 
                analogWrite(Vibe0_PIN, (uint16_t)map(vibe0,2,1000,31,255));
			else 
                analogWrite(Vibe0_PIN,0);

            if ((vibe1 > 1) && (vibe1 <= 1000)) 
                analogWrite(Vibe1_PIN, (uint16_t)map(vibe1,2,1000,31,255));
			else if (digitalRead(LubeManual_PIN) == HIGH) 
				// For iLube - if no software action, check the manual button too
				analogWrite(Vibe1_PIN, lubeAmount);
			else 
      			analogWrite(Vibe1_PIN,0);

            // Done with vibration channels
            
        }
    }
};
