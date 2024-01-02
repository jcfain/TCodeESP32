/* MIT License

Copyright (c) 2024 Jason C. Fain

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */


#pragma once

// Servo operating frequencies
#define VibePWM_Freq 8000   // Vibe motor control PWM frequency

// Other functions
#define VALVE_DEFAULT 5000        // Auto-valve default suction level (low-high, 0-9999) 
#define VIBE_TIMEOUT 2000         // Timeout for vibration channels (milliseconds).

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

#define Vibe2_PWM 12
#define Vibe3_PWM 13
#define SqueezeServo_PWM 14
#define CaseFan_PWM 15


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
	long currentMicros = esp_timer_get_time();
	//Thanks to AberrantJ for the following changes. https://discord.com/channels/664171761415356426/673141343320670210/919994227423858728
  	//noInterrupts();
	if(digitalRead(SettingsHandler::TwistFeedBack_PIN) == HIGH)
	{
		twistPulseCycle = currentMicros-twistPulseStart;
  		twistPulseStart = currentMicros;
	}
	else if (lastTwistPulseCycle != twistPulseCycle) 
	{
      int currentPulseLength = currentMicros-twistPulseStart;
      if (currentPulseLength <= 1000000/50) {
        twistPulseLength = currentPulseLength;
        lastTwistPulseCycle = twistPulseCycle;
      }
	}
  	//interrupts();
}

using TCODE_FUNCTION_PTR_T = void (*)(const char* input);