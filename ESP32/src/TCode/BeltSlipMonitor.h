/* MIT License

Copyright (c) 2026 Jason C. Fain

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

#include <Arduino.h>
#include "logging/LogHandler.h"
#include "logging/TagHandler.h"

/*
 * Belt / position slip detection from the sensors the SSR1 already has.
 *
 * The MT6701 rides on the motor shaft, so on its own it can never see slip:
 * if the belt skips a tooth the motor and its encoder agree perfectly, and
 * only the carriage moves somewhere unexpected.  Detecting slip needs
 * something referenced to the *carriage*.
 *
 * No hall sensor is required.  Only detector 1 uses one, and it stays dormant
 * unless a hall is both fitted and enabled; every other detector works on the
 * bare encoder-plus-endstop SKU.
 *
 * 1. Hall index re-referencing (optional, only when a hall is fitted).
 *    The magnet is on the carriage and the sensor is on the rail, so the
 *    motor angle recorded as the carriage crosses the hall trigger point is
 *    a fixed property of the machine.  Every later crossing that reads a
 *    different angle has measured belt slip directly, in radians, which
 *    converts straight to millimetres through the pulley circumference.
 *    Up-crossings and down-crossings are referenced separately so the hall's
 *    switching hysteresis cancels out instead of showing up as slip.
 *    This is the only detector that measures slip *during* normal use.
 *
 * 2. Home re-referencing (always available, this is the no-hall path).
 *    Homing ends with the carriage against a physical reference - the bottom
 *    end stop on a hall-less machine, the hall trigger otherwise - and the
 *    shaft angle there is just as fixed a property as the hall angle is.
 *    Comparing it against the previous home measures every millimetre the
 *    belt slipped in between.  It only reads out when homing runs, so on a
 *    hall-less SKU slip is measured on demand, by pressing Recalibrate,
 *    rather than continuously.  Reported, never faulted: the home that
 *    measured the drift has already corrected it.
 *
 * 3. Travel-limit violation (always available).
 *    The carriage cannot go meaningfully past either end of the rail.  If
 *    the encoder-derived position does, registration has been lost - slip,
 *    or a bad home.  Zero false positives, but it only fires once the error
 *    is already large.
 *
 * 4. Unexplained velocity step (always available, heuristic).
 *    A tooth skip is a mechanical discontinuity: the load releases, then
 *    slams back into engagement.  Either edge is a velocity step far larger
 *    than the commanded current could produce.  This catches the event as it
 *    happens rather than at the next reference crossing, but the threshold is
 *    machine-specific, so it only counts and logs - it never faults.
 *
 * A carriage-referenced encoder (magnetic tape + AS5311, or a second sensor
 * on an idler that the belt cannot slip against) is the only way to measure
 * slip continuously on a hall-less machine.  2 + 3 cover cumulative drift and
 * gross loss of registration, which is what actually matters here.
 */

// Hall crossings closer together than this are contact bounce.
#define SLIP_REARM_MS 150
// Ignore hall edges below this shaft speed (rad/s) - direction is unreliable.
#define SLIP_MIN_CROSS_VEL 0.5f
// Single-crossing drift that warrants a warning / a fault.
#define SLIP_WARN_MM 1.5f
#define SLIP_FAULT_MM 6.0f
// Accumulated absolute drift since boot that warrants a fault.
#define SLIP_FAULT_TOTAL_MM 25.0f
// Axis counts (0-9999) beyond either end of travel that count as lost registration.
#define SLIP_TRAVEL_MARGIN 600
#define SLIP_TRAVEL_CONFIRM_MS 250
// Velocity-step detector: window, step (mm/s) and the drive current (Amps)
// below which such a step cannot have been produced by the motor.
#define SLIP_STEP_WINDOW_MS 8
#define SLIP_STEP_MM_S 120.0f
#define SLIP_STEP_MAX_CURRENT 0.25f

class BeltSlipMonitor
{
public:
    /**
     * @param pulleyCircumferenceMm  drive pulley circumference
     * @param hallFitted             whether the hall reference is usable
     */
    void begin(float pulleyCircumferenceMm, bool hallFitted)
    {
        m_mmPerRad = pulleyCircumferenceMm / (2.0f * PI);
        m_hallFitted = hallFitted;
        LogHandler::info(Tags::Motor, "Slip monitor: %.3f mm/rad, %s",
                         m_mmPerRad,
                         hallFitted ? "hall + home referencing" : "home referencing only (no hall fitted)");
    }

    /**
     * Called once homing completes.
     *
     * @param angle           shaft angle the machine homed at
     * @param referenceValid  true when homing actually reached a physical
     *                        reference (end stop pressed, or hall triggered).
     *                        False for a homing run that merely timed out,
     *                        where the carriage could be anywhere and the
     *                        angle means nothing.
     */
    void home(float angle, bool referenceValid, uint32_t nowMs)
    {
        m_refValid[0] = m_refValid[1] = false;
        m_correctedRad = 0.0f;
        m_netSlipMm = 0.0f;
        m_lastCrossMm = 0.0f;
        m_lastAngle = angle;
        m_stepVelocity = 0.0f;
        m_stepMs = nowMs;
        m_lastMs = nowMs;
        m_velocity = 0.0f;
        m_outOfRangeSinceMs = 0;
        m_fault = false;
        m_warn = false;

        if (!referenceValid)
        {
            // Nothing to compare against, and the old reference is no longer
            // trustworthy either now that zeroAngle has been rewritten blind.
            m_homeRefValid = false;
            LogHandler::debug(Tags::Motor, "Slip: home did not reach a reference, drift not measured");
            return;
        }

        if (m_homeRefValid)
        {
            // The reference is a physical feature of the machine, so any
            // change in the shaft angle at which we reach it is belt slip
            // accumulated since the last home.
            m_lastHomeDriftMm = (angle - m_homeRef) * m_mmPerRad;
            m_totalSlipMm += fabsf(m_lastHomeDriftMm);
            if (fabsf(m_lastHomeDriftMm) >= SLIP_FAULT_MM)
            {
                m_warn = true;
                LogHandler::error(Tags::Motor, "Slip: %.2f mm since last home, total %.2f mm - check belt tension",
                                  m_lastHomeDriftMm, m_totalSlipMm);
            }
            else if (fabsf(m_lastHomeDriftMm) >= SLIP_WARN_MM)
            {
                m_warn = true;
                LogHandler::warning(Tags::Motor, "Slip: %.2f mm since last home, total %.2f mm",
                                    m_lastHomeDriftMm, m_totalSlipMm);
            }
            else
            {
                LogHandler::info(Tags::Motor, "Slip: %.2f mm since last home, total %.2f mm",
                                 m_lastHomeDriftMm, m_totalSlipMm);
            }
        }
        m_homeRef = angle;
        m_homeRefValid = true;
    }

    /**
     * @param angle          raw shaft angle (rad)
     * @param axisPosition   encoder-derived axis position, 0-9999
     * @param commandCurrent voltage being commanded this iteration
     * @param hallActive     true while the carriage is over the hall magnet
     */
    void update(float angle, float axisPosition, float commandCurrent, bool hallActive, uint32_t nowMs)
    {
        const uint32_t dtMs = nowMs - m_lastMs;
        if (dtMs > 0)
        {
            const float dt = dtMs * 0.001f;
            const float v = (angle - m_lastAngle) / dt;
            m_velocity = VELOCITY_FILTER * m_velocity + (1.0f - VELOCITY_FILTER) * v;
            m_lastAngle = angle;
            m_lastMs = nowMs;
        }

        if (m_hallFitted)
            checkHallCrossing(angle, hallActive, nowMs);
        checkTravelLimits(axisPosition, nowMs);
        checkVelocityStep(commandCurrent, nowMs);
    }

    /**
     * Slip measured but not yet folded into the caller's zero angle, in
     * radians.  Reading it clears it; add it to zeroAngle to re-home against
     * the hall reference so measured slip corrects itself.
     */
    float takeZeroAngleCorrection()
    {
        const float correction = m_pendingCorrectionRad;
        m_pendingCorrectionRad = 0.0f;
        return correction;
    }

    bool warn() const { return m_warn; }
    bool fault() const { return m_fault; }
    void clearWarn() { m_warn = false; }
    /** Signed drift at the most recent hall crossing (mm). */
    float lastCrossingMm() const { return m_lastCrossMm; }
    /** Signed belt slip measured between the last two homing runs (mm). */
    float lastHomeDriftMm() const { return m_lastHomeDriftMm; }
    /** True once a home has established a reference to measure drift against. */
    bool homeReferenced() const { return m_homeRefValid; }
    /** Sum of absolute drift measured since the last home (mm). */
    float totalSlipMm() const { return m_totalSlipMm; }
    /** Net signed drift not yet corrected (mm). */
    float netSlipMm() const { return m_netSlipMm; }
    uint32_t crossings() const { return m_crossings; }
    uint32_t stepEvents() const { return m_stepEvents; }
    const char *faultReason() const { return m_faultReason; }

private:
    static constexpr float VELOCITY_FILTER = 0.90f;

    void checkHallCrossing(float angle, bool hallActive, uint32_t nowMs)
    {
        if (hallActive == m_hallPrev)
            return;
        m_hallPrev = hallActive;
        if (!hallActive)
            return; // only the entering edge is used; leaving it is the hysteresis side
        if (nowMs - m_lastEdgeMs < SLIP_REARM_MS)
            return;
        if (fabsf(m_velocity) < SLIP_MIN_CROSS_VEL)
            return;

        m_lastEdgeMs = nowMs;
        const uint8_t dir = m_velocity > 0.0f ? 1 : 0;
        if (!m_refValid[dir])
        {
            // First crossing in this direction establishes the reference.
            m_ref[dir] = angle;
            m_refValid[dir] = true;
            LogHandler::debug(Tags::Motor, "Slip: hall reference %s set at %.4f rad",
                              dir ? "up" : "down", angle);
            return;
        }

        m_crossings++;
        // The reference is in raw-encoder space, so subtract whatever we have
        // already folded into zeroAngle to avoid counting the same slip twice.
        const float slipRad = (angle - m_ref[dir]) - m_correctedRad;
        const float slipMm = slipRad * m_mmPerRad;
        m_lastCrossMm = slipMm;
        m_netSlipMm += slipMm;
        m_totalSlipMm += fabsf(slipMm);

        // Fold it into the caller's zero angle so the axis stays calibrated.
        // Both directions share one reference frame, hence one correction.
        m_correctedRad += slipRad;
        m_pendingCorrectionRad += slipRad;

        if (fabsf(slipMm) >= SLIP_FAULT_MM)
        {
            raiseFault("belt slip at hall crossing");
            LogHandler::error(Tags::Motor, "Slip: %.2f mm at hall crossing (%s), total %.2f mm - FAULT",
                              slipMm, dir ? "up" : "down", m_totalSlipMm);
        }
        else if (m_totalSlipMm >= SLIP_FAULT_TOTAL_MM)
        {
            raiseFault("cumulative belt slip");
            LogHandler::error(Tags::Motor, "Slip: cumulative %.2f mm since home - FAULT", m_totalSlipMm);
        }
        else if (fabsf(slipMm) >= SLIP_WARN_MM)
        {
            m_warn = true;
            LogHandler::warning(Tags::Motor, "Slip: %.2f mm at hall crossing (%s), total %.2f mm",
                                slipMm, dir ? "up" : "down", m_totalSlipMm);
        }
        else
        {
            LogHandler::verbose(Tags::Motor, "Slip: %.3f mm at hall crossing (%s), total %.2f mm",
                                slipMm, dir ? "up" : "down", m_totalSlipMm);
        }
    }

    void checkTravelLimits(float axisPosition, uint32_t nowMs)
    {
        if (axisPosition > -(float)SLIP_TRAVEL_MARGIN && axisPosition < 9999.0f + SLIP_TRAVEL_MARGIN)
        {
            m_outOfRangeSinceMs = 0;
            return;
        }
        if (!m_outOfRangeSinceMs)
        {
            m_outOfRangeSinceMs = nowMs;
            return;
        }
        // Held outside the rail long enough that it is not an overshoot.
        if (nowMs - m_outOfRangeSinceMs >= SLIP_TRAVEL_CONFIRM_MS && !m_fault)
        {
            raiseFault("position outside rail travel");
            LogHandler::error(Tags::Motor, "Slip: position %.0f is outside travel - registration lost", axisPosition);
        }
    }

    void checkVelocityStep(float commandCurrent, uint32_t nowMs)
    {
        if (nowMs - m_stepMs < SLIP_STEP_WINDOW_MS)
            return;
        const float stepMmS = fabsf(m_velocity - m_stepVelocity) * m_mmPerRad;
        m_stepVelocity = m_velocity;
        m_stepMs = nowMs;

        // At this little drive current the motor cannot have produced the step
        // itself, so something mechanical let go or slammed home.
        if (stepMmS > SLIP_STEP_MM_S && fabsf(commandCurrent) < SLIP_STEP_MAX_CURRENT)
        {
            m_stepEvents++;
            LogHandler::warning(Tags::Motor, "Slip: unexplained %.0f mm/s velocity step at %.2fA (event %lu)",
                                stepMmS, commandCurrent, (unsigned long)m_stepEvents);
        }
    }

    void raiseFault(const char *reason)
    {
        m_fault = true;
        m_faultReason = reason;
    }

    float m_mmPerRad = 1.0f;
    bool m_hallFitted = false;

    float m_ref[2] = {0.0f, 0.0f};
    bool m_refValid[2] = {false, false};
    // Shaft angle at the physical home reference, and the drift against it.
    float m_homeRef = 0.0f;
    bool m_homeRefValid = false;
    float m_lastHomeDriftMm = 0.0f;
    bool m_hallPrev = false;
    uint32_t m_lastEdgeMs = 0;

    float m_correctedRad = 0.0f;
    float m_pendingCorrectionRad = 0.0f;
    float m_lastCrossMm = 0.0f;
    float m_netSlipMm = 0.0f;
    float m_totalSlipMm = 0.0f;
    uint32_t m_crossings = 0;

    float m_lastAngle = 0.0f;
    float m_velocity = 0.0f;
    uint32_t m_lastMs = 0;

    float m_stepVelocity = 0.0f;
    uint32_t m_stepMs = 0;
    uint32_t m_stepEvents = 0;

    uint32_t m_outOfRangeSinceMs = 0;
    bool m_warn = false;
    bool m_fault = false;
    const char *m_faultReason = "";
};
