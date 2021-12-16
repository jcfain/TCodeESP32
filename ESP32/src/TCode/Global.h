#pragma once

// Servo microseconds per radian
// (Standard: 637 μs/rad)
// (LW-20: 700 μs/rad)
#define ms_per_rad 637  // (μs/rad)

// Servo operating frequencies
#define VibePWM_Freq 8000   // Vibe motor control PWM frequency

// Other functions
#define VALVE_DEFAULT 5000        // Auto-valve default suction level (low-high, 0-9999) 
#define VIBE_TIMEOUT 2000         // Timeout for vibration channels (milliseconds).

// T-Code Channels
#define CHANNELS 11                // Number of channels of each type (LRVA)

// ----------------------------
//  Auto Settings
// ----------------------------
// Do not change

// Servo PWM channels
#define LowerLeftServo_PWM 0     // Lower Left Servo
#define UpperLeftServo_PWM 1     // Upper Left Servo
#define LowerRightServo_PWM 2    // Lower Right Servo
#define UpperRightServo_PWM 3    // Upper Right Servo
#define LeftPitchServo_PWM 4     // Left Pitch Servo
#define RightPitchServo_PWM 5    // Right Pitch Servo
#define TwistServo_PWM 6         // Twist Servo
#define ValveServo_PWM 7         // Valve Servo
#define TwistFeedback_PWM 8      // Twist Servo
#define Vibe0_PWM 9              // Vibration motor 1
#define Vibe1_PWM 10             // Vibration motor 2
#define Heater_PWM 11             // Heatting pad


// Twist position monitor variables
volatile int twistPulseLength = 0;
volatile int twistPulseCycle = 1099;
volatile int twistPulseStart = 0;
// Libraries used
//#include <EEPROM.h> // Permanent memory

volatile int lastTwistPulseCycle; 
// Twist position detection functions
void IRAM_ATTR twistChange() 
{
	//Thanks to AberrantJ for the following changes. https://discord.com/channels/664171761415356426/673141343320670210/919994227423858728
  	noInterrupts();
	if(digitalRead(SettingsHandler::TwistFeedBack_PIN) == HIGH)
	{
		twistPulseCycle = micros()-twistPulseStart;
  		twistPulseStart = micros();
	}
	else if (lastTwistPulseCycle != twistPulseCycle) 
	{
      int currentPulseLength = micros()-twistPulseStart;
      if (currentPulseLength <= 1000000/50) {
        twistPulseLength = currentPulseLength;
        lastTwistPulseCycle = twistPulseCycle;
      }
	}
  	interrupts();
}

