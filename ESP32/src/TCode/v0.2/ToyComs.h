
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
// v2.5 - Experimental build. Servo Library replaced with alternative capable of high frequencies. 23-7-2020


// ----------------------------
//   Serial Comms Interface
// ----------------------------
// This is a t-code object that manages the serial comms from the computer
// Leave this section of the code alone unless you know what you're doing
#pragma once

#include "../TCodeBase.h"
#include "../../TagHandler.h"

class ToyComms : public TCodeBase {
  public:

	/** V0.3+ only. DO NOT USE. FOR BACKWARDS COMPATIBILITY ONLY! */
  void setup(const char* firmware, const char* tcode) override {};
	/** V0.3+ only. DO NOT USE. FOR BACKWARDS COMPATIBILITY ONLY! */
	void RegisterAxis(String ID, String axisName) override {};
	/** V0.3+ only. DO NOT USE. FOR BACKWARDS COMPATIBILITY ONLY! */
	void ByteInput(byte inByte) override {};
	/** V0.3+ only. DO NOT USE. FOR BACKWARDS COMPATIBILITY ONLY! */
	void StringInput(String inString) override {};
	/** V0.3+ only. DO NOT USE. FOR BACKWARDS COMPATIBILITY ONLY! */
	void AxisInput(String ID, int magnitude, char extension, long extMagnitude) override {};
	/** V0.3+ only. DO NOT USE. FOR BACKWARDS COMPATIBILITY ONLY! */
	int AxisRead(String ID) override { return -1;  };
	/** V0.3+ only. DO NOT USE. FOR BACKWARDS COMPATIBILITY ONLY! */
	unsigned long AxisLast(String ID) override { return -1; };
	/** V0.3+ only. DO NOT USE. FOR BACKWARDS COMPATIBILITY ONLY! */
	void getDeviceSettings(char* settings) override {};

    // Setup function
    void setup() {
      // Centralise everything
      xL1[0] = 500;
      xL1[1] = 500;
      xL1[2] = 500;
      xL1[3] = 500;
      xR1[0] = 500;
      xR1[1] = 500;
      xR1[2] = SettingsHandler::sr6Mode ? 750 : 500;
    }

    // Function to process serial input
    void serialRead(byte inByte) {
		//command += (const char)inByte;
      switch (inByte) {

        // Start - Linear Motion input
        case 'l':
        case 'L':
          linear = true;
          vibration = false;
          rotation = false;
          device = false;
          inNum1 = 1;
          interval = false;
          velocity = false;
          inNum2 = 0;
        break;

        // Start - Vibration input
        case 'v':
        case 'V':
          vibration = true;
          rotation = false;
          linear = false;
          device = false;
          inNum1 = 1;
          interval = false;
          velocity = false;
          inNum2 = 0;
        break;

        // Start - Rotation input
        case 'r':
        case 'R':
          rotation = true;
          linear = false;
          vibration = false;
          device = false;
          inNum1 = 1;
          interval = false;
          velocity = false;
          inNum2 = 0;
        break;

        // Start - Device input
        case 'd':
        case 'D':
          device = true;
          rotation = false;
          linear = false;
          vibration = false;
          inNum1 = 0;
          interval = false;
          velocity = false;
          inNum2 = 0;
        break;

        // Append Interval
        case 'i':
        case 'I':
          if (linear || vibration || rotation) {
            interval = true;
            velocity = false;
            inNum2 = 0;
          }
        break;

        // Append Velocity
        case 's':
        case 'S':
          if (linear || vibration || rotation) {
            velocity = true;
            interval = false;
            inNum2 = 0;
          }
        break;

        // Record Number
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
              //Serial.printf("inByte: %u\n", inByte);
          if (interval || velocity) {
            // Update the number
            inNum2 = inNum2*10;
            if (inNum2 > 10000) {inNum2 = inNum2 % 10000;}
            inNum2 = inNum2 + (inByte - 48);
              //Serial.printf("inNum2: %u\n", inNum2);
          // If L,V or R  
          } else if (linear || vibration || rotation || device) {
            // If less than 4 digits so far, update the number
            if (inNum1 < 10000) {
              inNum1 = inNum1*10;
              inNum1 = inNum1 + (inByte - 48);
              //Serial.printf("inNum1: %u\n", inNum1);
            }
            
          }
        break;


        // If any other character
          default:
          // Has a command been input?
          if (linear || vibration || rotation) {

            // Check a channel and a number have been entered
            if (inNum1 < 100) {
              linear = false;
              vibration = false;
              rotation = false;
            }

            // Increase to 4 digits, if not entered
            while (inNum1 < 10000) { 
              inNum1 = inNum1*10;
            }
            // Eliminate "1" marker
            inNum1 = inNum1 - 10000;
            
            // Extract commanded position "x" from last three digits
            int x,i;
            x = inNum1 % 1000;
            // Extract commanded axis "i" from first digit
            i = (inNum1 - x)/1000;
            i = i % 10;

            // If the commanded axis exists, process command
            if (0<=i && i<=3) {

              //If it's a linear command
              if (linear) {

                // Save axis command as 1-1000
                xLbuff1[i] = x+1;
                
                if (interval) {
                  // Time interval
                  xLbuff2[i] = inNum2;
                  xLbuffSpd[i] = false;
                } else if (velocity) {
                  // Speed
                  xLbuff2[i] = inNum2;
                  xLbuffSpd[i] = true;
                } else {
                  // Instant
                  xLbuff2[i] = 0;
                }
                
              }


              //If it's a linear command
              if (rotation) {

                // Save axis command as 1-1000
                xRbuff1[i] = x+1;
                
                if (interval) {
                  // Time interval
                  xRbuff2[i] = inNum2;
                  xRbuffSpd[i] = false;
                } else if (velocity) {
                  // Speed
                  xRbuff2[i] = inNum2;
                  xRbuffSpd[i] = true;
                } else {
                  // Instant
                  xRbuff2[i] = 0;
                }
                
              }


              //If it's a linear command
              if (vibration) {

                // Save axis command as 1-1000
                xVbuff1[i] = x+1;
                
                if (interval) {
                  // Time interval
                  xVbuff2[i] = inNum2;
                  xVbuffSpd[i] = false;
                } else if (velocity) {
                  // Speed
                  xVbuff2[i] = inNum2;
                  xVbuffSpd[i] = true;
                } else {
                  // Instant
                  xVbuff2[i] = 0;
                }
                
              }
              
            }
          } else if (device) {
            Dbuff = inNum1;
            device = false;
          }



          // Clear input buffer
          linear = false;
          vibration = false;
          rotation = false;
          device = false;
          inNum1 = 0;
          interval = false;
          velocity = false;
          inNum2 = 0;

          // If it's a new line character
          if (inByte == 10) {
			//Serial.println("execute ");
			//Serial.println(command);
			//command.clear();
            // Mark input time
            unsigned long t;
            t = millis();

            byte n;
            // Execute Linear Channels
            for (n = 0; n <= 3; n++) {
              if (xLbuff1[n]>0) {

                // Execute control command
                // Serial.print("ToyCom xLinear ");
                // Serial.printf("n: %u, t: %lu\n", n, t);
                xL0[n] = xLinear(n,t);
                xL1[n] = xLbuff1[n];
                // Write the initial time
                tL0[n] = t;
                
                // Is the second number a speed
                if (xLbuffSpd[n]) {
                  // Speed
                  tL1[n] = xL1[n]-xL0[n];
                  if (tL1[n] < 0) {tL1[n] = -tL1[n];}
                  tL1[n] = tL0[n] + (1000*tL1[n])/xLbuff2[n];  
                } else {
                  // Time interval
                  tL1[n] = tL0[n] + xLbuff2[n];
                }
                  // if(n==2) {
                  //   Serial.print("tL0[n] ");
                  //   Serial.println(tL0[n]);
                  //   Serial.print("tL1[n] ");
                  //   Serial.println(tL1[n]);
                  //   Serial.print("xLbuff2[n] ");
                  //   Serial.println(xLbuff2[n]);
                  //   Serial.print("(1000*tL1[n])/xLbuff2[n] ");
                  //   Serial.println((1000*tL1[n])/xLbuff2[n]);
                  // }
                // Smoothing limit
                if (tL1[n]-tL0[n] < 20) { tL1[n] = tL0[n] + 20; }
                // Clear channel buffer
                xLbuff1[n] = 0;
                xLbuff2[n] = 0;
                xLbuffSpd[n] = false;
                  
              }
            }

            // Execute Rotation Channels
            for (n = 0; n <= 2; n++) {
              if (xRbuff1[n]>0) {

                // Execute control command
                xR0[n] = xRotate(n,t);
                xR1[n] = xRbuff1[n];
                // Write the initial time
                tR0[n] = t;
                
                // Is the second number a speed
                if (xRbuffSpd[n]) {
                  // Speed
                  tR1[n] = xR1[n]-xR0[n];
                  if (tR1[n] < 0) {tR1[n] = -tR1[n];}
                  tR1[n] = tR0[n] + (1000*tR1[n])/xRbuff2[n];  
                } else {
                  // Time interval
                  tR1[n] = tR0[n] + xRbuff2[n];
                }
                // Smoothing limit
                if (tR1[n]-tR0[n] < 20) { tR1[n] = tR0[n] + 20; }
                // Clear channel buffer
                xRbuff1[n] = 0;
                xRbuff2[n] = 0;
                xRbuffSpd[n] = false;
                  
              }
            }

            // Execute Vibration Channels
            for (n = 0; n <= 1; n++) {
              if (xVbuff1[n]>0) {

                // Execute control command
                xV0[n] = xVibe(n,t);
                xV1[n] = xVbuff1[n];
                // Write the initial time
                tV0[n] = t;
                
                // Is the second number a speed
                if (xVbuffSpd[n]) {
                  // Speed
                  tV1[n] = xV1[n]-xV0[n];
                  if (tV1[n] < 0) {tV1[n] = -tV1[n];}
                  tV1[n] = tV0[n] + (1000*tV1[n])/xVbuff2[n];  
                } else {
                  // Time interval
                  tV1[n] = tV0[n] + xVbuff2[n];
                }
                // Clear channel buffer
                xVbuff1[n] = 0;
                xVbuff2[n] = 0;
                xVbuffSpd[n] = false;
                  
              }
            }

            // Execute device commands
            if (Dbuff > 0) {
              if (Dbuff == 1) {
                identifyTCode();
              }


              Dbuff = 0;
            }
            
          }
        
        
        break;
                    
      }
    }

    // Establish linear position from time (1-1000)
    int xLinear(int i,unsigned long t) {
      // i is axis
      // t is time point
      // x will be the return value
      int x;
      
                //  Serial.print("xLinear ");
                //  Serial.printf("i: %u, t: %lu, tL1[i]: %lu, tL0[i]: %lu\n", i, t, tL1[i], tL0[i]);
      // Ramp value
      if (t >= tL1[i]) {
                //Serial.println("t > tL1[i]");
        x = xL1[i];
      } else if (t < tL0[i]) {
                //Serial.println("t < tL0[i]");
        x = xL0[i];
      } else {
                //Serial.println("else map");
        x = map(t,tL0[i],tL1[i],xL0[i],xL1[i]);
      }
      
      return x;
    }
    
    // Establish rotation position from time (1-1000)
    int xRotate(int i,unsigned long t) {
      // i is axis
      // t is time point
      // x will be the return value
      int x;
      
                //  Serial.print("xRotate ");
                //  Serial.printf("i: %u, t: %lu, tR1[i]: %lu, tR0[i]: %lu\n", i, t, tR1[i], tR0[i]);
      // Ramp value
      if (t >= tR1[i]) {
        // if(i==0) {
        //         Serial.print("t >= tR1[i] ");
        //         Serial.println(t);
        //         Serial.println(tR0[i]);
        //         Serial.println(tR1[i]);
        //         Serial.println(xR0[i]);
        //         Serial.println(xR1[i]);
        // }
        x = xR1[i];
      } else if (t < tR0[i]) {
        // if(i==0) {
        //         Serial.print("t < tR0[i] ");
        //         Serial.println(t);
        //         Serial.println(tR0[i]);
        //         Serial.println(tR1[i]);
        //         Serial.println(xR0[i]);
        //         Serial.println(xR1[i]);
        // }
        x = xR0[i];
      } else {
        // if(i==0) {
        //         Serial.println("map");
        //         Serial.println(t);
        //         Serial.println(tR0[i]);
        //         Serial.println(tR1[i]);
        //         Serial.println(xR0[i]);
        //         Serial.println(xR1[i]);
        // }
        x = map(t,tR0[i],tR1[i],xR0[i],xR1[i]);
      }
      
      return x;
    }

    // Establish vibration level from time (1-1000)
    int xVibe(int i,unsigned long t) {
      // i is level
      // t is time point
      // x will be the return value
      int x;
      
      // Ramp value
      if (t >= tV1[i]) {
        x = xV1[i];
      } else if (t < tV0[i]) {
        x = xV0[i];
      } else {
        x = map(t,tV0[i],tV1[i],xV0[i],xV1[i]);
      }
      
      return x;
    }

    // Function to identify the current TCode type over serial
    void identifyTCode() {
      sendMessage("TCode v0.2");
      //Serial.println("TCode v0.2");
    }


  private:
    const char* _TAG = TagHandler::ToyHandler;

    // Input parameters
    boolean linear;
    boolean vibration;
    boolean rotation;
    boolean device;
    int inNum1;
    boolean interval;
    boolean velocity;
    int inNum2;

    // Linear motion
    int xLbuff1[4];
    int xLbuff2[4];
    boolean xLbuffSpd[4];
    int xL0[4];
    int xL1[4];
    long tL0[4];
    long tL1[4];

    // Rotation
    int xRbuff1[3];
    int xRbuff2[3];
    boolean xRbuffSpd[3];
    int xR0[3];
    int xR1[3];
    long tR0[3];
    long tR1[3];

    // Vibration
    int xVbuff1[2];
    int xVbuff2[2];
    boolean xVbuffSpd[2];
    int xV0[2];
    int xV1[2];
    long tV0[2];
    long tV1[2];

    // Device commands
    int Dbuff;
    float mapRange(float x, float in_min, float in_max, float out_min, float out_max) 
    {
      x = constrain(x, in_min, in_max);
      return (x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
    }
};
