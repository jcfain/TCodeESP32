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



// Twist position monitor variables
volatile int twistPulseLength = 0;
volatile int twistPulseCycle = 1099;
volatile int twistPulseStart = 0;
// Libraries used
//#include <EEPROM.h> // Permanent memory

volatile int lastTwistPulseCycle; 
volatile int twistFeedBackPin = 0; 
// Twist position detection functions
void IRAM_ATTR twistChange() 
{
	long currentMicros = esp_timer_get_time();
	if(!twistFeedBackPin) {
    	SettingsFactory* m_settingsFactory = SettingsFactory::getInstance();
		twistFeedBackPin = m_settingsFactory->getPins()->twistFeedBack();
	}
	if(digitalRead(twistFeedBackPin) == HIGH)
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