#pragma once
volatile int twistFeedBackPin;
// Twist position monitor variables
volatile int twistPulseLength = 0;
volatile int twistPulseCycle = 1099;
volatile int twistPulseStart = 0;
// Libraries used
//#include <EEPROM.h> // Permanent memory


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