
#pragma once

// -----------------------------
// Class to handle each axis
// -----------------------------
class Axis {

private:
    int MIN_SMOOTH_INTERVAL = 3;
    int MAX_SMOOTH_INTERVAL = 100;
  public:
  // Setup function
  Axis() {

    // Set default dynamic parameters
    rampStartTime = 0;
    rampStart = 5000;
    rampStopTime = rampStart;
    rampStop = rampStart;

    // Set Empty Name
    Name = "";
    lastT = 0;

    // Live command auto-smooth
    minInterval = MAX_SMOOTH_INTERVAL;
      
  }

  // Function to set the axis dynamic parameters
  void Set(int x, char ext, long y) {
    unsigned long t = millis(); // This is the time now
    x = constrain(x,0,9999);
    y = constrain(y,0,9999999);
    // Set ramp parameters, based on inputs
    // Live command
    if ( y == 0 || ( ext != 'S' && ext != 'I' ) ) {
      // update auto-smooth regulator
      int lastInterval = t - rampStartTime;
      if ( lastInterval > minInterval && minInterval < MAX_SMOOTH_INTERVAL ) { minInterval += 1; }
      else if ( lastInterval < minInterval && minInterval > MIN_SMOOTH_INTERVAL ) { minInterval -= 1; } 
      // Set ramp parameters
      rampStart = GetPosition();
      rampStopTime = t + minInterval;  
    } 
    // Speed command
    else if ( ext == 'S' ) {
      rampStart = GetPosition();  // Start from current position
      int d = x - rampStart;  // Distance to move
      if (d<0) { d = -d; }
      long dt = d;  // Time interval (time = dist/speed)
      dt *= 100;
      dt /= y; 
      rampStopTime = t + dt;  // Time to arrive at new position
      //if (rampStopTime < t + minInterval) { rampStopTime = t + minInterval; }
    }
    // Interval command
    else if ( ext == 'I' ) {
      rampStart = GetPosition();  // Start from current position
      rampStopTime = t + y;  // Time to arrive at new position
      //if (rampStopTime < t + minInterval) { rampStopTime = t + minInterval; }
    }
    rampStartTime = t;
    rampStop = x;
    lastT = t;
  }

  // Function to return the current position of this axis
  int GetPosition() {
    int x; // This is the current axis position, 0-9999
    unsigned long t = millis(); 
    if (t > rampStopTime) {
      x = rampStop;
    } else if (t > rampStartTime) { 
      x = map(t,rampStartTime,rampStopTime,rampStart,rampStop);
    } else {
      x = rampStart;
    }
    x = constrain(x,0,9999);
    return x;
  }

  // Function to stop axis movement at current position
  void Stop() {
    unsigned long t = millis(); // This is the time now
    rampStart = GetPosition();
    rampStartTime = t;
    rampStop = rampStart;
    rampStopTime = t;
  }

  // Public variables
  String Name;  // Function name of this axis
  unsigned long lastT;  //

  private:
  
  // Movement positions
  int rampStart;
  unsigned long rampStartTime;
  int rampStop;
  unsigned long rampStopTime;

  // Live command auto-smooth regulator
  int minInterval;

};

