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
// v2.4 - T-wist support added; LR servos now +/- 350 for the sake of Raser1's sanity, 1-7-2020
// v2.4.1 - Adding support for ESP32 bluetooth


// Libraries to include
#include <ESP32Servo.h>
#include <BluetoothSerial.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

// ----------------------------
//   Serial Comms Interface
// ----------------------------
// This is a t-code object that manages the serial comms from the computer
// Leave this section of the code alone unless you know what you're doing

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
      if (t >= tL1[i]) {
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
      if (t >= tR1[i]) {
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
      Serial.println("TCode v0.2");
      SerialBT.println("TCode v0.2");
    }

  private:

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








// ----------------------------
//   SETUP
// ----------------------------
// This code runs once, on startup

// Declare class
// This uses the t-code object above
ToyComms toy; 

// Declare servos
Servo Servo0;  // Fore-Aft Servo
Servo Servo1;  // Right Servo
Servo Servo2;  // Left Servo
Servo Servo3;  // Pitch Servo
Servo Servo4;  // Valve Servo
Servo Servo5;  // Twist Servo

// Specify which pins are attached to what here
#define Servo0_PIN 19  // Fore-Aft Servo
#define Servo1_PIN 12  // Right Servo
#define Servo2_PIN 13  // Left Servo
#define Servo3_PIN 14  // Pitch Servo
#define Servo4_PIN 15  // Valve Servo
#define Servo5_PIN 2  // Twist Servo
#define Vibe0_PIN 22   // Vibration motor 1
#define Vibe1_PIN 23   // Vibration motor 2
#define Pot1_PIN 33   // Twist potentiometer

// Declare timing variables
unsigned long nextPulse;
int tick;

// Position variables
int xLin,yLin,zLin;
// Rotation variables
int xRot,yRot,zRot;
// Vibration variables
int vibe0,vibe1;
// Velocity tracker variables, for T-Valve
int xLast;
float xValve;

// Setup function
// This is run once, when the arduino starts
void setup() {

  // Start serial
  Serial.begin(115200);
  if(!SerialBT.begin("TCodeESP32")){
    Serial.println("An error occurred initializing Bluetooth");
  }
  Serial.println("Bluetooth started");
  toy.identifyTCode();

  // Declare servos and set zero
  Servo0.attach(Servo0_PIN);
  Servo1.attach(Servo1_PIN);
  Servo2.attach(Servo2_PIN);
  Servo3.attach(Servo3_PIN);
  Servo4.attach(Servo4_PIN);
  Servo5.attach(Servo5_PIN); // AAK

  pinMode(Pot1_PIN,INPUT);
  adcAttachPin(Pot1_PIN);
  analogReadResolution(11);
  analogSetAttenuation(ADC_6db);
  
  delay(500);
  Servo0.writeMicroseconds(1500);
  Servo1.writeMicroseconds(1500);
  Servo2.writeMicroseconds(1500);
  Servo3.writeMicroseconds(1500);
  Servo4.writeMicroseconds(1500);
  Servo5.writeMicroseconds(1500); //AAK

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
  SerialBT.println("Ready!");

}




// ----------------------------
//   MAIN
// ----------------------------
// This loop runs continuously

void loop() {

  // Read serial
  // This will run continuously
  if (Serial.available() > 0) {
    // Send the serial bytes to the t-code object
    // This is the only required input for the object
    toy.serialRead(Serial.read());
  } else if (SerialBT.available() > 0) {
    toy.serialRead(SerialBT.read());
  }

  // Pulse Servos based on time interval
  // This function will run every 20ms, sending a pulse to the servos
  if (millis() > nextPulse) {
    unsigned long t = nextPulse;
    nextPulse = nextPulse + tick;

    // Collect inputs
    // These functions query the t-code object for the position/level at a specified time
    // Number recieved will be an integer, 1-1000
    xLin = toy.xLinear(0,t);
    yLin = toy.xLinear(1,t);
    //zLin = toy.xLinear(2,t); (not used)
    xRot = toy.xRotate(0,t);
    yRot = toy.xRotate(1,t);
    zRot = toy.xRotate(2,t);
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
    
    // Send signals to the servos
    // Note: 1000 = -45deg, 2000 = +45deg
    Servo0.writeMicroseconds(1500 - b + b2);
    Servo1.writeMicroseconds(1500 + a + c);
    Servo2.writeMicroseconds(1500 - a + c);
    Servo3.writeMicroseconds(1500 - d);
    Servo4.writeMicroseconds(2000 - e);
    Servo5.writeMicroseconds(1500 + twist);

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
