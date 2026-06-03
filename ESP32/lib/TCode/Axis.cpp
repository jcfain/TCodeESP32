// TCODE AXIS LIBRARY
// by TempestMAx 1-6-26
// This class handles the movement of a single TCode axis
// v0.1 Experimental build
//
//
// MIT License
//
// Copyright (c) 2021 Richard Unger
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#include "Axis.h"

// ============================================
//           AXIS HANDLER CLASS
// ============================================
Axis::Axis(uint16_t startPosIn)
    : startTime(millis()),                    // stamp the moment of creation
      duration(0),
      startPos(startPosIn),
      endPos(startPosIn),
      gradMode(false),
      lastShortTime(0),
      lastShortPos(startPosIn),
      meanLiveInterval(SHORT_MOVE_INTERVAL),
      liveCovergeSteps(1),
      liveMode(false),
      coeffA(0),
      coeffB(0),
      coeffC(0),
      coeffD((int64_t)startPosIn << 16),   // ready for instant hold
      bufferSet(false)
{

}

// Returns the current axis position
uint16_t Axis::getPosition()
{
    // Check whether time is outside of movement bounds
    // zero duration means the axis is stopped
    // if out, auto-set new movement parameters via setTimeout() function
    if (millis() - startTime > duration && duration != 0 ) { setTimeout(); }

    // Get and return the current position
    uint16_t position = getPositionFromCurve(millis() - startTime);
    return position;
}

// Returns the current axis velocity
int32_t Axis::getVelocity(int perInterval)
{
    // Check whether time is outside of movement bounds
    // zero duration means the axis is stopped
    // if out, auto-set new movement parameters via setTimeout() function
    if (millis() - startTime > duration && duration != 0 ) { setTimeout(); }

    // Get and return the current position
    int32_t velocity = getVelocityFromCurve(millis() - startTime, perInterval);
    return velocity;
}

  // Returns the time of the last received command
unsigned long Axis::getLast() {
    return startTime;
}

// Loads the axis buffer
void Axis::prepAxis(uint16_t targetPos,
                  InputType inputType,
                  uint32_t extendedParam,
                  MoveStyle style,
                  int32_t gradient)
{

    // Set buffer parameters
    bufferTargetPos = targetPos;
    bufferInputType = inputType;
    bufferExtendedParam = extendedParam;
    bufferMoveStyle = style;
    bufferGradient = gradient;
    bufferSet = true;

}

// Process the buffer parameters into current movement parameters and execute
bool Axis::setAxis()
{
    // Skip this function and return a negative if the buffer isn't set
    if (!bufferSet) { return false; }

    // If very short time intervals are being set these are redundant - ignore!
    if (bufferInputType == InputType::INTERVAL && bufferExtendedParam <= SHORT_MOVE_INTERVAL) {
        bufferInputType = InputType::SHORT;
    }

    // Starting position for any new movement is always the current position
    uint16_t startPosIn = getPosition();
    uint32_t durationIn;    // This will be used to calculate the duration of the movement

    // Any new movement will be a SHORT command, or an extended command: SPEED or INTERVAL
    bool usePoint;
    switch (bufferInputType) {
        case InputType::SHORT:
            // Run the short command handler function
            usePoint = shortCmd(durationIn);
            if (!usePoint) return true;  // Live mode doesn't directly execute some commands
            break;
        case InputType::SPEED:
            liveMode = false;  // Extended command cancels live mode
            // Work out absolute distance to travel then work out time interval from speed
            durationIn = (startPosIn >= bufferTargetPos) ? (startPosIn - bufferTargetPos) : (bufferTargetPos - startPosIn);
            durationIn *= 100;  // Conversion from strange TCode "S" unit
            durationIn /= bufferExtendedParam;
            break;
        case InputType::INTERVAL:
            liveMode = false;  // Extended command cancels live mode
            // Time interval is time interval - simple
            durationIn = bufferExtendedParam;
            break;
    }

    gradMode = false;  // most movements do not use end gradient mode
    switch (bufferMoveStyle) {
        // Load movement into current parameters using setMotion() function
        case MoveStyle::RAMPED:     setMotion(durationIn, startPosIn, bufferTargetPos, false, false); break;
        case MoveStyle::EASE_IN:    setMotion(durationIn, startPosIn, bufferTargetPos, true, false); break;
        case MoveStyle::EASE_OUT:   setMotion(durationIn, startPosIn, bufferTargetPos, false, true); break;
        case MoveStyle::EASE_BOTH:  setMotion(durationIn, startPosIn, bufferTargetPos, true, true); break;
        // For gradient mode load movement into current parameters using setCubic() function
        case MoveStyle::GRADIENT:
            gradMode = true;
            setCubic(durationIn,startPosIn,bufferTargetPos,getVelocity(),bufferGradient);   // or however you want to map it
            break;
    }

    // The buffer has now been transfered to current parameters, so set the buffer as empty
    bufferSet = false;

    // The start point for the new axis movement is the current time
    startTime = millis();

    return true; // Flag to return that the axis movement has been updated
}

void Axis::setPos(uint16_t pos) {
    if (pos > 10000) pos = 10000;
    setMotion(0,pos,pos);
}

void Axis::stop() {
    uint16_t pos = getPosition();
    setMotion(0,pos,pos);
}

// Handler function for short commands
bool Axis::shortCmd(uint32_t& durationIn) {
    // Flag whether to use this live point or hold pending extrapolation
    bool usePoint = true;

    // If short commands are being received faster than the trigger threshold, use live mode.
    unsigned long lastInterval = millis() - lastShortTime;
    if (lastInterval < LIVE_TRIGGER) {
        liveMode = true;   // This triggers live mode

        // The rolling mean average time interval between live commands is calculated here
        meanLiveInterval = meanLiveInterval*(MEAN_INTERVAL_STEPS-1) + lastInterval;
        meanLiveInterval /= MEAN_INTERVAL_STEPS;

        // Live commands use a linear ramped movement
        bufferMoveStyle = MoveStyle::RAMPED;

        // Extrapolated interval for this movement will be 1.25x the average time interval.
        // This means that the setTimeout() is called much less than 50% of the time
        durationIn = meanLiveInterval + meanLiveInterval/4;  
        
        // Run a gradual convergence if recovering from a signal gap
        // Also needs an acceleration limit imposed?
        if (liveCovergeSteps > 1) {
            // Project a point several time steps ahead of the new curve based on new data points
            int32_t projTargetPos = map(liveCovergeSteps*durationIn,0,durationIn,lastShortPos,bufferTargetPos);
            // Move to a point on this line that's 1 time step ahead
            projTargetPos = map(durationIn,0,liveCovergeSteps*durationIn,lastShortPos,projTargetPos);
            // Clamp and make this the movement target
            if (projTargetPos > 10000) { projTargetPos = 10000; }
            if (projTargetPos < 0) { projTargetPos = 0; }
            bufferTargetPos = (uint16_t)projTargetPos;
            // Move the coutner down toward 1 each received command
            liveCovergeSteps--;
        }

    // If we are still in live mode but a single command has come in after a delay
        } else if (liveMode) {
        liveCovergeSteps = LIVE_CONVERGE_STEPS;  // Reset the convergence counter
        usePoint = false;  // Do not use this data point to change motion
    } else {
        // Isolated command, so this is a simple move
        durationIn = SHORT_MOVE_INTERVAL;       
        bufferMoveStyle = MoveStyle::RAMPED;
    }

    // Record the last short command as a data point for future extrapolation
    lastShortTime = millis();
    lastShortPos = bufferTargetPos; 

    return usePoint;   
}


// Pre-compute the cubic Hermite spline coefficients.
// This is the "setup" phase — done once per movement.
// We use Q16.16 fixed-point throughout so the hot evaluation path stays fast
// and accurate enough for the 0–10000 position range.
void Axis::setCubic(uint32_t durationIn,
                    uint16_t startPosIn,
                    uint16_t endPosIn,
                    int32_t startGradient,
                    int32_t endGradient)
{
    duration = durationIn;
    startPos = startPosIn;
    endPos   = endPosIn;
    gradMode = true;

    // Edge case: zero-duration move is just an instant jump
    if (duration == 0) {
        endPos = startPos;
        coeffA = coeffB = coeffC = 0;
        coeffD = (int64_t)startPos << 16;   // Q16.16 version of startPos
        return;
    }

    // Convert positions to Q16.16
    int64_t p0 = (int64_t)startPos << 16;
    int64_t p1 = (int64_t)endPos   << 16;

    // === GRADIENT SCALING ===
    // Gradient unit: position units per 100 µs
    // We need tangent in position units (not normalised) then convert to Q16.16
    //
    // Formula: m = gradient * (duration / 100) * 65536
    // The division by 100 converts the weird "per-100µs" unit into total delta over the whole move.
    int64_t m0 = ((int64_t)startGradient * (int64_t)duration * 65536LL) / 100LL;
    int64_t m1 = ((int64_t)endGradient   * (int64_t)duration * 65536LL) / 100LL;

    // === CUBIC HERMITE COEFFICIENTS ===
    // Standard cubic Hermite basis (p0, p1, m0, m1) gives the smoothest curve
    // that matches both positions and the desired start/end velocities.
    //
    //   pos(τ) = Aτ³ + Bτ² + Cτ + D
    //
    // where τ goes from 0.0 to 1.0
    coeffA = 2LL * p0 - 2LL * p1 + m0 + m1;
    coeffB = -3LL * p0 + 3LL * p1 - 2LL * m0 - m1;
    coeffC = m0;
    coeffD = p0;
}


// Pre-compute the cubic coefficients for a linear or eased in/out movement
// This is an alternate for the "setup" phase — done once per movement.
// As with setCubic we use Q16.16 fixed-point throughout so the hot evaluation path
// stays fast and accurate enough for the 0–10000 position range.
void Axis::setMotion(uint32_t durationIn,
                     uint16_t startPosIn,
                     uint16_t endPosIn,
                     bool easeIn,
                     bool easeOut)
{
    duration = durationIn;
    startPos = startPosIn;
    endPos   = endPosIn;

    // Edge case: zero-duration move is just an instant jump
    if (duration == 0) {
        endPos = startPos;
        coeffA = coeffB = coeffC = 0;
        coeffD = (int64_t)startPos << 16;
        return;
    }

    // Convert positions to Q16.16
    int64_t p0 = (int64_t)startPos << 16;
    int64_t p1 = (int64_t)endPos   << 16;
    int32_t deltaPos = (int32_t)endPos - (int32_t)startPos;

    if (!easeIn && !easeOut) {
        // === LINEAR ===
        // This is easy - set the coefficients directly
        // pos(τ) = p0 + (p1 - p0) * τ
        // → A=0, B=0, C=(p1-p0), D=p0   (all in Q16.16)
        coeffA = 0;
        coeffB = 0;
        coeffC = p1 - p0;                    // already scaled correctly
        coeffD = p0;

    } else { 

        // 
        int32_t startGrad = 0;
        int32_t endGrad   = 0;

        if (easeIn && !easeOut) {
            // === QUADRATIC EASE-IN (flat at start) ===
            // Velocity starts at 0, peaks at end
            startGrad = 0;
            // Average velocity is still delta/time, peak is twice that for pure quad
            int32_t deltaPos = (int32_t)endPos - (int32_t)startPos;
            if (duration > 0) {
                endGrad = (deltaPos * 200LL) / (int32_t)duration;   // 2× average
            }
        } else if (!easeIn && easeOut) {
            // === QUADRATIC EASE-OUT (flat at end) ===
            // Velocity peaks at start, ends at 0
            int32_t deltaPos = (int32_t)endPos - (int32_t)startPos;
            if (duration > 0) {
                startGrad = (deltaPos * 200LL) / (int32_t)duration; // 2× average
            }
            endGrad = 0;
        } else {
            // === CUBIC BOTH ENDS FLAT ===
            startGrad = 0;
            endGrad   = 0;
        }

        // Hand off to the full cubic calculator (reuses all fixed-point logic)
        setCubic(durationIn, startPosIn, endPosIn, startGrad, endGrad);
    }

    // Unset grad mode if motion has been set this way
    gradMode = false;
}


// Parabolic braking routine — called once the previous segment has run past its duration.
// Pure constant-deceleration quadratic curve (velocity drops linearly to zero).
void Axis::setDecelStop(uint16_t currentPos, int32_t  currentVel, int32_t decelRate)
{
    // Safety: zero velocity or nonsense decel → instant hold
    if (currentVel == 0 || decelRate <= 0) {
        setMotion(0, currentPos, currentPos, false, false);
        return;
    }

    int64_t v0 = currentVel;                     // signed velocity
    int64_t d  = decelRate;                      // positive deceleration magnitude
    int64_t dir = (v0 > 0) ? 1LL : ((v0 < 0) ? -1LL : 0LL);

    // Simple abs for int64_t (Arduino doesn't always expose llabs cleanly)
    if (v0 < 0) v0 = -v0;

    // === KINEMATIC CALCULATION ===
    // t (in 100 µs units) = v0 / d
    int64_t t100us = v0 / d;
    if (t100us <= 0) t100us = 1;

    // Stopping distance = v0² / (2 d) = (v0 * t100us) / 2
    int64_t deltaPos = (v0 * t100us) / 2;

    int64_t projected = (int64_t)currentPos + dir * deltaPos;

    uint16_t targetPos = (uint16_t)projected;
    uint32_t brakeDurationUs = (uint32_t)(t100us * 100ULL);

    // === CLAMP HANDLING — increase deceleration if position would overshoot ===
    if (projected > 10000 || projected < 0) {
        uint16_t bound = (projected > 10000) ? 10000 : 0;
        int64_t maxDelta = (int64_t)bound - (int64_t)currentPos;

        // If we're already moving away from the bound, just hold position
        if (dir * maxDelta <= 0) {
            setMotion(0, currentPos, currentPos, false, false);
            return;
        }

        // Recalculate stronger deceleration to stop exactly at the bound
        d = (v0 * v0) / (2LL * (maxDelta > 0 ? maxDelta : -maxDelta));
        if (d < 1) d = 1;

        t100us = v0 / d;
        if (t100us <= 0) t100us = 1;

        brakeDurationUs = (uint32_t)(t100us * 100ULL);
        targetPos = bound;
    }

    // Final safety
    if (brakeDurationUs == 0) brakeDurationUs = 1;

    // Hand off to quadratic ease-out (flat at end)
    setMotion(brakeDurationUs, currentPos, targetPos, false, true);
}


// Called if the movement goes out of bounds 
void Axis::setTimeout() {
    
    // Set new movement start point at end of old one
    startTime += duration;

    // If currently running under a gradient mode, set a deceleration
    if (gradMode || liveMode) {
        setDecelStop(endPos, getVelocityFromCurve(duration), DECEL_CONST);
    } else {
        // If we're not doing gradients - simple movement termination
        liveMode = false;                 // Live mode times out here
        setMotion(0,endPos,endPos);
    }

}

// Returns the interpolated position at the given elapsed time.
// Uses pure integer fixed-point arithmetic for speed on AVR.
uint16_t Axis::getPositionFromCurve(unsigned long elapsedUs)
{
    // If the time is out of bounds, set end case 
    if (elapsedUs >= duration) { return endPos; }
    if (elapsedUs <= 0) { return startPos; }

    // τ in Q16.16
    uint64_t tau  = ((uint64_t)elapsedUs << 16) / duration;

    uint64_t tau2 = (tau * tau) >> 16;           // Q16.16
    uint64_t tau3 = (tau2 * tau) >> 16;          // Q16.16

    // Full polynomial in Q32.32 → shift back to Q16.16
    int64_t pos_fp = (coeffA * (int64_t)tau3 >> 16)
                   + (coeffB * (int64_t)tau2 >> 16)
                   + (coeffC * (int64_t)tau  >> 16)
                   + coeffD;

    int32_t pos = (int32_t)((pos_fp + (1LL << 15)) >> 16);

    if (pos < 0)   return 0;
    if (pos > 10000) return 10000;
    return (uint16_t)pos;
}


// Returns instantaneous velocity at the given elapsed time.
// Output unit matches your gradient input: position units per 100 microseconds.
int32_t Axis::getVelocityFromCurve(unsigned long elapsedUs, int perInterval)
{
    // clamp time to duration range
    if (elapsedUs > duration) { elapsedUs = duration; }
    if (elapsedUs <= 0) { elapsedUs = 0; }

    // Zero-duration means the axis is stopped
    if (duration == 0) { return 0; }

    // Prevent div-by-zero or silly values
    if (perInterval < 1) perInterval = 1;
    if (perInterval > 100000) perInterval = 100000;

    // τ in Q16.16
    uint64_t tau  = ((uint64_t)elapsedUs << 16) / duration;
    uint64_t tau2 = (tau * tau) >> 16;

    // Derivative with respect to τ, still in Q16.16
    int64_t dpdtau_fp = (3LL * coeffA * (int64_t)tau2 >> 16)   // 3Aτ²
                      + (2LL * coeffB * (int64_t)tau  >> 16)   // 2Bτ
                      + coeffC;                                // C

    // === FINAL VELOCITY SCALING ===
    // dpdtau_fp * (perInterval * 1000) / (65536 * duration)
    int64_t numerator   = dpdtau_fp * (int64_t)perInterval;
    int64_t denominator = (int64_t)duration << 16;

    int64_t velocity = numerator / denominator;

    return (int32_t)velocity;
}