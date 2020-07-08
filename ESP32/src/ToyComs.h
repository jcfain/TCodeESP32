// ----------------------------
//   Serial Comms Interface
// ----------------------------
// This is a t-code object that manages the serial comms from the computer
// Leave this section of the code alone unless you know what you're doing

#include <BluetoothSerial.h>

class ToyComms {
  public:

    // Setup function
    ToyComms() {
      // Centralise everything
      xL1[0] = 500;
      xL1[1] = 500;
      xL1[2] = 500;
      xR1[0] = 500;
      xR1[1] = 500;
      xR1[2] = 500;
    }

    // Function to process serial input
    void serialRead(byte inByte) {
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
          if (interval || velocity) {
            // Update the number
            inNum2 = inNum2*10;
            if (inNum2 > 10000) {inNum2 = inNum2 % 10000;}
            inNum2 = inNum2 + (inByte - 48);
          // If L,V or R  
          } else if (linear || vibration || rotation || device) {
            // If less than 4 digits so far, update the number
            if (inNum1 < 10000) {
              inNum1 = inNum1*10;
              inNum1 = inNum1 + (inByte - 48);
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
            if (0<=i && i<=2) {

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

            // Mark input time
            unsigned long t;
            t = millis();

            byte n;
            // Execute Linear Channels
            for (n = 0; n <= 2; n++) {
              if (xLbuff1[n]>0) {

                // Execute control command
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
      
      // Ramp value
      if (t > tL1[i]) {
        x = xL1[i];
      } else if (t < tL0[i]) {
        x = xL0[i];
      } else {
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
      
      // Ramp value
      if (t > tR1[i]) {
        x = xR1[i];
      } else if (t < tR0[i]) {
        x = xR0[i];
      } else {
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
      if (t > tV1[i]) {
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
      Serial.println("TCode v0.2");
      SerialBT.println("TCode v0.2");
    }

  private:

    BluetoothSerial SerialBT;
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
    int xLbuff1[3];
    int xLbuff2[3];
    boolean xLbuffSpd[3];
    int xL0[3];
    int xL1[3];
    long tL0[3];
    long tL1[3];

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
    
};
