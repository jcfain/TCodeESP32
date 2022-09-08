// TCode-Constants-H v1.0,
// protocal by TempestMAx (https://www.patreon.com/tempestvr)
// implemented by Eve 05/02/2022
// usage of this class can be found at (https://github.com/Dreamer2345/Arduino_TCode_Parser)
// Please copy, share, learn, innovate, give attribution.
// Decodes T-code commands
// It can handle:
//   x linear channels (L0, L1, L2... L9)
//   x rotation channels (R0, R1, R2... L9)
//   x vibration channels (V0, V1, V2... V9)
//   x auxilliary channels (A0, A1, A2... A9)
// History:
//
#pragma once
#ifndef TCODE_CONSTANTS
#define TCODE_CONSTANTS

#ifndef TCODE_MAX_AXIS
/**
 * @brief Value used to define the max range for an axis value
 */
#define TCODE_MAX_AXIS 9999
#endif

#ifndef TCODE_MAX_AXIS_MAGNITUDE
/**
 * @brief Value used to define the max range for an axis magnitude command input
 */
#define TCODE_MAX_AXIS_MAGNITUDE 9999999
#endif

#endif