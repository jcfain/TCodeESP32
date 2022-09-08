// TCode-Fixed-Point-H v1.0,
// protocal by TempestMAx (https://www.patreon.com/tempestvr)
// implemented by Eve 05/02/2022
// usage of this class can be found at (https://github.com/Dreamer2345/Arduino_TCode_Parser)
// Please copy, share, learn, innovate, give attribution.
// Q16 Implementation for fractional calculations on hardware without an FPU
// History:
//
#pragma once
#ifndef TCODE_FIXED_POINT_H
#define TCODE_FIXED_POINT_H
#include "Arduino.h"

#define Q 16
#define Q16Fixed int32_t
#define Q16FixedL int64_t

/*
int32_t sat_Q16(int64_t x)
{
    if (x > 0x7FFFFFFF) return 0x7FFFFFFF;
    else if (x < -0x80000000) return -0x80000000;
    else return (int32_t)x;
}
*/

/**
 * @brief Adds two Q16 Values Together a + b
 * @param a
 * @param b
 * @return The addition sum of those to values
 */
Q16Fixed addQ16(Q16Fixed a, Q16Fixed b)
{
    Q16FixedL tmp = static_cast<Q16FixedL>(a) + static_cast<Q16FixedL>(b);
    return tmp;
}

/**
 * @brief Subtracts one Q16 Value from another a - b
 * @param a
 * @param b
 * @return The subtraction of b from a
 */
Q16Fixed subQ16(Q16Fixed a, Q16Fixed b)
{
    return a - b;
}

/**
 * @brief Multiplies two Q16 Values together
 * @param a
 * @param b
 * @return The multiplicative sum of a and b
 */
Q16Fixed multQ16(Q16Fixed a, Q16Fixed b)
{
    Q16FixedL tmp = (static_cast<Q16FixedL>(a) * static_cast<Q16FixedL>(b)) >> Q;
    return tmp;
}

/**
 * @brief Divides two Q16 Values a / b
 * @param a
 * @param b
 * @return The division output of a / b
 */
Q16Fixed divQ16(Q16Fixed a, Q16Fixed b)
{
    return (((static_cast<Q16FixedL>(a)) << Q) / b);
}

/**
 * @brief constrains a Q16 to between two Q16 values min, max
 * @param v value to be contrained
 * @param min minimum threshold
 * @param max maximum threshold
 * @return the constrained value (if min is larger than max the values are swapped)
 */
Q16Fixed constrainQ16(Q16Fixed v, Q16Fixed min, Q16Fixed max)
{
    if (min > max)
    {
        Q16Fixed temp = min;
        min = max;
        max = temp;
    }

    if (v < min)
        return min;
    if (v > max)
        return max;
    return v;
}

/**
 * @brief Converts an integer to a Q16
 * @param i Value to be converted
 * @return The Q16 representation of the integer passed
 */
Q16Fixed Q16fromInt(int i)
{
    return (static_cast<Q16Fixed>(i)) << Q;
}

/**
 * @brief Converts an float to a Q16
 * @param i Value to be converted
 * @return The Q16 representation of the float passed
 */
Q16Fixed Q16fromFloat(float i)
{
    float newValuef = i * (1 << Q);
    return static_cast<Q16Fixed>(newValuef);
}

/**
 * @brief Converts an double to a Q16
 * @param i Value to be converted
 * @return The Q16 representation of the double passed
 */
Q16Fixed Q16fromDouble(double i)
{
    double newValued = i * (1 << Q);
    return static_cast<Q16Fixed>(newValued);
}

/**
 * @brief Converts an Q16 to a Integer
 * @param i Value to be converted
 * @return The Integer Value of the Q16 rounded
 */
int IntfromQ16(Q16Fixed i)
{
    i = addQ16(i, Q16fromFloat(0.5f));
    return static_cast<int>(i >> Q);
}

/**
 * @brief Converts an Q16 to a Float
 * @param i Value to be converted
 * @return The Float Value of the Q16 rounded
 */
float FloatfromQ16(Q16Fixed i)
{
    return ((static_cast<float>(i)) / (1 << Q));
}

/**
 * @brief Converts an Q16 to an Double
 * @param i Value to be converted
 * @return The Double Value of the Q16 rounded
 */
double DoublefromQ16(Q16Fixed i)
{
    return ((static_cast<double>(i)) / (1 << Q));
}

/**
 * @brief Linearly interpolates between two Q16 values based on a third Q16 value which is from 0.0 to 1.0
 * @param start the starting value
 * @param stop the targer value
 * @param t the position between the start and stop values
 * @return The mixed values based on the t variable represented as a Q16
 */
Q16Fixed lerpQ16(Q16Fixed start, Q16Fixed stop, Q16Fixed t)
{
    // lerp is defined as mixed = ((1.0 - t) * a) + (t * b)
    t = constrainQ16(t, 0, Q16fromInt(1));
    Q16Fixed tn = subQ16(Q16fromInt(1), t);
    Q16Fixed a = multQ16(tn, start);
    Q16Fixed b = multQ16(t, stop);
    return addQ16(a, b);
}

/**
 * @brief Gives an eased value based on an input t value which is from 0.0 to 1.0
 * @param t the position value to be converted to an exponential curve
 * @return the interpolated value of t from 0.0 to 1.0
 */
Q16Fixed easeIn(Q16Fixed t)
{
    t = constrainQ16(t, 0, Q16fromInt(1));
    t = multQ16(t, t);
    return t;
}

/**
 * @brief Gives an inverse eased value based on an input t value which is from 0.0 to 1.0
 * @param t the position value to be converted to an exponential curve
 * @return the inverse interpolated value of t from 0.0 to 1.0
 */
Q16Fixed easeOut(Q16Fixed t)
{
    t = constrainQ16(t, 0, Q16fromInt(1));
    t = subQ16(Q16fromInt(1), t);
    t = multQ16(t, t);
    return subQ16(Q16fromInt(1), t);
}

/**
 * @brief Maps a given range of values to an output range based on the ease in curve
 * @param in value to be mapped
 * @param inStart the starting min range
 * @param inEnd the starting max range
 * @param outStart the ending min range
 * @param outEnd the ending max range
 * @return the mapped value from the in range to out range based on the Ease in Curve
 */
int mapEaseIn(int in, int inStart, int inEnd, int outStart, int outEnd)
{
    Q16Fixed t = Q16fromInt(in - inStart);                         // Find the distance of the in value from inStart
    t = divQ16(t, Q16fromInt(inEnd - inStart));                    // Find where the inValue lies within the range between inStart and inEnd should be (0.0 - 1.0) for values where in is in the range specified
    t = easeIn(t);                                                 // get the ease in curve value
    t = constrainQ16(t, 0, Q16fromInt(1));                         // constrain the value from 0.0 to 1.0
    t = multQ16(t, Q16fromInt(outEnd - outStart));                 // multiply the value of how far along the range between outStart and outEnd
    t = addQ16(t, Q16fromInt(outStart));                           // Get the value to lie between outStart and outEnd
    t = constrainQ16(t, Q16fromInt(outStart), Q16fromInt(outEnd)); // Constrain the value so it does not go out of bounds
    return IntfromQ16(t);                                          // Convert the value back to an int
}

/**
 * @brief Maps a given range of values to an output range based on the ease out curve
 * @param in value to be mapped
 * @param inStart the starting min range
 * @param inEnd the starting max range
 * @param outStart the ending min range
 * @param outEnd the ending max range
 * @return the mapped value from the in range to out range based on the Ease out Curve
 */
int mapEaseOut(int in, int inStart, int inEnd, int outStart, int outEnd)
{
    Q16Fixed t = Q16fromInt(in - inStart);                         // Find the distance of the in value from inStart
    t = divQ16(t, Q16fromInt(inEnd - inStart));                    // Find where the inValue lies within the range between inStart and inEnd should be (0.0 - 1.0) for values where in is in the range specified
    t = easeOut(t);                                                // get the ease out curve value
    t = constrainQ16(t, 0, Q16fromInt(1));                         // constrain the value from 0.0 to 1.0
    t = multQ16(Q16fromInt(outEnd - outStart), t);                 // multiply the value of how far along the range between outStart and outEnd
    t = addQ16(t, Q16fromInt(outStart));                           // Get the value to lie between outStart and outEnd
    t = constrainQ16(t, Q16fromInt(outStart), Q16fromInt(outEnd)); // Constrain the value so it does not go out of bounds
    return IntfromQ16(t);                                          // Convert the value back to an int
}

/**
 * @brief Maps a given range of values to an output range based on the ease in out curve
 * @param in value to be mapped
 * @param inStart the starting min range
 * @param inEnd the starting max range
 * @param outStart the ending min range
 * @param outEnd the ending max range
 * @return the mapped value from the in range to out range based on the Ease in out Curve
 */
int mapEaseInOut(int in, int inStart, int inEnd, int outStart, int outEnd)
{
    Q16Fixed t = Q16fromInt(in - inStart);                         // Find the distance of the in value from inStart
    t = divQ16(t, Q16fromInt(inEnd - inStart));                    // Find where the inValue lies within the range between inStart and inEnd should be (0.0 - 1.0) for values where in is in the range specified
    t = lerpQ16(easeIn(t), easeOut(t), t);                         // Find the lerped value of the curve based on where the in value lies
    t = constrainQ16(t, 0, Q16fromInt(1));                         // Constrain the curve value between 0.0 and 1.0
    t = multQ16(Q16fromInt(outEnd - outStart), t);                 // multiply the value of how far along the range between outStart and outEnd
    t = addQ16(t, Q16fromInt(outStart));                           // Get the value to lie between outStart and outEnd
    t = constrainQ16(t, Q16fromInt(outStart), Q16fromInt(outEnd)); // Constrain the value so it does not go out of bounds
    return IntfromQ16(t);                                          // Convert the value back to an int
}

#endif