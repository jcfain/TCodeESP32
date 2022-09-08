// TCode-Axis-H v1.0,
// protocal by TempestMAx (https://www.patreon.com/tempestvr)
// implemented by Eve 05/02/2022
// usage of this class can be found at (https://github.com/Dreamer2345/Arduino_TCode_Parser)
// Please copy, share, learn, innovate, give attribution.
// Container for TCode Axis's
// History:
//
#pragma once
#ifndef TCODE_AXIS_H
#define TCODE_AXIS_H
#include "Arduino.h"
#include "TCodeConstants.h"
#include "TCodeEnums.h"

/**
 * @brief Value used to define the default value for a TCode Axis
 */
#define TCODE_DEFAULT_AXIS_RETURN_VALUE 5000;

/**
 * @brief Value used to define the auto-smooth interval minimum range used in live commands (ms)
 */
#define TCODE_MIN_AXIS_SMOOTH_INTERVAL 3 // Minimum auto-smooth ramp interval for live commands (ms)

/**
 * @brief Value used to define the auto-smooth interval maximum range used in live commands (ms)
 */
#define TCODE_MAX_AXIS_SMOOTH_INTERVAL 100 // Maximum auto-smooth ramp interval for live commands (ms)

/**
 * @brief Class used to represent a TCode Axis
 */
class TCodeAxis
{
public:
    TCodeAxis();
    void set(int x, TCode_Axis_Extention_Type ext, long y); // Function to set the axis dynamic parameters
    int getPosition();                                      // Function to return the current position of this axis
    void stop();                                            // Function to stop axis movement at current position

    bool changed(); // Function to check if an axis has changed since last getPosition

    void setRampType(TCode_Axis_Ramp_Type e); // Function to set the ramp type for get position

    void setName(String value); // Function to set the name for this axis
    String getName();           // Function to get the name for this axis
    bool isUsed();              // Function to return if this axis has a Name used for figuring out if it is used

    unsigned long lastT;       // The last time this Axis was updated represented in millis
    TCode_Axis_Ramp_Type ramp; // The ramp type used by this axis

private:
    String axisName;
    int lastPosition;
    int rampStart;
    unsigned long rampStartTime;
    int rampStop;
    unsigned long rampStopTime;
    int minInterval;
};

#if defined(TCODE_HAS_FPU)
float lerp(float start, float stop, float t)
{
    t = constrain(t, 0.0f, 1.0f);
    return (((1 - t) * start) + (t * stop));
}

float easeIn(float t)
{
    t = constrain(t, 0.0f, 1.0f);
    return t * t;
}

float easeOut(float t)
{
    t = constrain(t, 0.0f, 1.0f);
    return 1.0 - ((1 - t) * (1 - t));
}

int mapEaseIn(int in, int inStart, int inEnd, int outStart, int outEnd)
{
    float t = in - inStart;
    t /= (inEnd - inStart);
    t = easeIn(t);
    t = constrain(t, 0.0f, 1.0f);
    t *= (outEnd - outStart);
    t += outStart;
    t += 0.5f;
    return (int)t;
}

int mapEaseOut(int in, int inStart, int inEnd, int outStart, int outEnd)
{
    float t = in - inStart;
    t /= (inEnd - inStart);
    t = easeOut(t);
    t = constrain(t, 0.0f, 1.0f);
    t *= (outEnd - outStart);
    t += outStart;
    t += 0.5f;
    return (int)t;
}

int mapEaseInOut(int in, int inStart, int inEnd, int outStart, int outEnd)
{
    float t = in - inStart;
    t /= (inEnd - inStart);
    t = lerp(easeIn(t), easeOut(t), t);
    t = constrain(t, 0.0f, 1.0f);
    t *= (outEnd - outStart);
    t += outStart;
    t += 0.5f;
    return (int)t;
}
#else
// This is for processors which lack an FPU
#include "TCodeFixed.h"
#endif

// Constructor for Axis Class
TCodeAxis::TCodeAxis()
{
    ramp = TCode_Axis_Ramp_Type::Linear;
    rampStartTime = 0;
    rampStart = TCODE_DEFAULT_AXIS_RETURN_VALUE;
    rampStopTime = rampStart;
    rampStop = rampStart;
    axisName = "";
    lastT = 0;
    minInterval = TCODE_MAX_AXIS_SMOOTH_INTERVAL;
}

/**
 * @brief Sets the ramp type for the axis
 * @param e the TCode Axis Ramp Type to be set
 */
void TCodeAxis::setRampType(TCode_Axis_Ramp_Type e)
{
    ramp = e;
}

/**
 * @brief sets the axis' dynamic parameters
 * @param x the target value
 * @param ext the extention type for the axis e.g. Time,Speed
 * @param y the extention value
 */
void TCodeAxis::set(int x, TCode_Axis_Extention_Type ext, long y)
{
    unsigned long t = millis(); // This is the time now
    x = constrain(x, 0, TCODE_MAX_AXIS);
    y = constrain(y, 0, TCODE_MAX_AXIS_MAGNITUDE);
    // Set ramp parameters, based on inputs
    switch (ext)
    {
    case TCode_Axis_Extention_Type::Speed:
    {
        rampStart = getPosition(); // Start from current position
        int d = x - rampStart;     // Distance to move
        if (d < 0)
        {
            d = -d;
        }
        long dt = d; // Time interval (time = dist/speed)
        dt *= 100;
        dt /= y;
        rampStopTime = t + dt; // Time to arrive at new position
    }
    break;
    case TCode_Axis_Extention_Type::Time:
    {
        rampStart = getPosition(); // Start from current position
        rampStopTime = t + y;      // Time to arrive at new position
    }
    break;
    default:
        if (y == 0)
        {
            int lastInterval = t - rampStartTime;
            if ((lastInterval > minInterval) && (minInterval < TCODE_MIN_AXIS_SMOOTH_INTERVAL))
            {
                minInterval += 1;
            }
            else if ((lastInterval < minInterval) && (minInterval > TCODE_MAX_AXIS_SMOOTH_INTERVAL))
            {
                minInterval -= 1;
            }
            // Set ramp parameters
            rampStart = getPosition();
            rampStopTime = t + minInterval;
        }
    }
    rampStartTime = t;
    rampStop = x;
    lastT = t;
}

/**
 * @brief gets the current position of the axis
 * @returns current position of this axis as an int
 */
int TCodeAxis::getPosition()
{
    int x; // This is the current axis position, 0-9999
    unsigned long t = millis();
    if (t > rampStopTime)
    {
        x = rampStop;
    }
    else if (t > rampStartTime)
    {
        switch (ramp)
        {
        case TCode_Axis_Ramp_Type::Linear:
            x = map(t, rampStartTime, rampStopTime, rampStart, rampStop);
            break;
        case TCode_Axis_Ramp_Type::EaseIn:
            x = mapEaseIn(t, rampStartTime, rampStopTime, rampStart, rampStop);
            break;
        case TCode_Axis_Ramp_Type::EaseOut:
            x = mapEaseOut(t, rampStartTime, rampStopTime, rampStart, rampStop);
            break;
        case TCode_Axis_Ramp_Type::EaseInOut:
            x = mapEaseInOut(t, rampStartTime, rampStopTime, rampStart, rampStop);
            break;
        default:
            x = map(t, rampStartTime, rampStopTime, rampStart, rampStop);
        }
    }
    else
    {
        x = rampStart;
    }
    x = constrain(x, 0, 9999);
    return x;
}

/**
 * @brief stops axis movement at its current position
 */
void TCodeAxis::stop()
{
    unsigned long t = millis(); // This is the time now
    rampStart = getPosition();
    rampStartTime = t;
    rampStop = rampStart;
    rampStopTime = t;
}

/**
 * @brief stops axis movement at its current position
 * @returns returns true if the axis has changed position since last check
 */
bool TCodeAxis::changed()
{
    if (lastPosition != getPosition())
    {
        lastPosition = getPosition();
        return true;
    }
    return false;
}

/**
 * @brief sets the name of the Axis
 */
void TCodeAxis::setName(String value)
{
    axisName = value;
}

/**
 * @brief gets the name of the axis
 * @returns returns the string representation of the name stored
 */
String TCodeAxis::getName()
{
    return axisName;
}

/**
 * @brief checks if an axis is registered if so then returns true
 * @returns returns true if axisName is not empty
 */
bool TCodeAxis::isUsed()
{
    return (axisName != "");
}

#endif
