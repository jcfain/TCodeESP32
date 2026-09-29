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
 * Minimum-current idle hold for the BLDC position loop.
 *
 * The stroker's position loop is a P controller whose output is a q-axis
 * current setpoint in Amps (SimpleFOC converts it with the motor's phase
 * resistance), so a parked carriage sits at whatever steady-state droop
 * balances gravity and friction.  That droop current is DC into a fixed
 * winding pair (the rotor is not turning, so there is no back-EMF and no
 * commutation to spread the loss around) and it is the dominant source of
 * idle heat: the motor cooks one phase for as long as the device sits still.
 * At the rated 1 A into an 11.1 ohm phase that is over 11 W in one place.
 *
 * This class does not replace the P loop.  It wraps it in a decaying
 * *authority envelope*:
 *
 *   Active   - full authority, the P loop runs exactly as before.
 *   Holding  - the target has been static and the carriage has stopped.
 *              The envelope decays from the settled effort towards a learned
 *              floor, so the commanded voltage (and therefore the current)
 *              falls to the smallest value that still holds position.
 *   Released - the envelope reached ~zero without the carriage moving, so
 *              nothing is needed to hold at all.  The caller cuts the drive
 *              and idle current goes to zero.
 *
 * If the carriage drifts while the envelope is shrinking we have shed too
 * much: the level that failed becomes the learned floor (plus a margin) and
 * subsequent holds stop there instead of releasing.  That is the "minimum"
 * part - the hold effort is searched for at runtime rather than guessed at
 * compile time, so it adapts to orientation, sleeve weight and belt tension.
 *
 * Drift is measured against the position at hold entry, never against the
 * target, because a P controller with droop is *expected* to sit well away
 * from its target under load.
 */

// Target changes smaller than this (0-9999 counts) are treated as noise.
#define HOLD_TARGET_EPSILON 8
// Carriage speed below this (counts/s) counts as "stopped".
#define HOLD_MOTION_LIMIT 60.0f
// Static target + stopped carriage for this long before the envelope decays.
#define HOLD_ENTER_MS 250
// Ceiling on commanded Amps once holding, and on the recovery after a drift.
// A third of the motor's 1 A rating is far more than a sleeve needs to hold.
#define HOLD_MAX_CURRENT 0.7f
// Time constant for shedding the envelope. Larger = gentler sag.
#define HOLD_DECAY_TAU_S 0.60f
// Drift (counts) from the hold-entry position that re-arms the P loop.
#define HOLD_DRIFT_LIMIT 120
// Position error (counts) inside which the axis counts as "arrived". Outside
// it the loop is still working and must keep full authority, however still the
// carriage looks: a stopped axis a long way from its target is a stall, not a
// hold, and shedding effort there would latch the drive off for good.
#define HOLD_ARRIVE_BAND 600
// Envelope at or below this is treated as "nothing is needed to hold".
// 0.03 A into 11.1 ohm is ~10 mW, which is thermally nothing.
#define HOLD_RELEASE_CURRENT 0.03f
// ...and it has to stay there this long before the drive is actually cut.
#define HOLD_RELEASE_MS 1000
// A drift at I Amps sets the learned floor to I * this.
#define HOLD_FLOOR_MARGIN 1.30f
// Each hold that survives without drifting relaxes the floor by this factor,
// so a floor learned under a heavy load is unlearned when the load goes away.
#define HOLD_FLOOR_RELAX 0.97f

class BLDCHoldController
{
public:
    enum class State : uint8_t
    {
        Active,   // P loop has full authority
        Holding,  // authority envelope is decaying towards the floor
        Released, // envelope hit zero, drive can be cut entirely
    };

    void reset(int target, float measured, uint32_t nowMs)
    {
        m_state = State::Active;
        m_target = target;
        m_holdPos = measured;
        m_lastPos = measured;
        m_lastMs = nowMs;
        m_movingSinceMs = nowMs;
        m_releaseSinceMs = 0;
        m_envelope = 0.0f;
        m_velocity = 0.0f;
        m_recovering = false;
    }

    /**
     * @param target   commanded axis position, 0-9999
     * @param measured encoder-derived axis position, same scale
     * @param pCurrent what the position loop wants to command this iteration, Amps
     * @param nowMs    millis()
     * @return the current setpoint to actually command, Amps
     */
    float update(int target, float measured, float pCurrent, uint32_t nowMs)
    {
        const uint32_t dtMs = nowMs - m_lastMs;
        const float dt = dtMs * 0.001f;
        m_lastMs = nowMs;

        // Velocity is only used as a "has it stopped" test, so a heavy filter
        // over a coarse (1ms) clock is fine and keeps encoder jitter out.
        if (dtMs > 0)
        {
            const float v = (measured - m_lastPos) / dt;
            m_velocity = VELOCITY_FILTER * m_velocity + (1.0f - VELOCITY_FILTER) * v;
            m_lastPos = measured;
        }

        const float posError = fabsf((float)target - measured);

        // A new command, or an axis that is nowhere near where it was asked to
        // be, always wins: full authority, immediately.
        if (abs(target - m_target) > HOLD_TARGET_EPSILON || posError > HOLD_ARRIVE_BAND)
        {
            m_target = target;
            if (m_state != State::Active)
                LogHandler::debug(Tags::Motor, "Hold: full authority restored (target %d, error %.0f)",
                                  target, posError);
            goActive(measured, nowMs, false);
            return pCurrent;
        }

        switch (m_state)
        {
        case State::Active:
            // Wait for the carriage to actually settle before shedding effort.
            // posError is already known to be within the arrival band here.
            if (fabsf(m_velocity) > HOLD_MOTION_LIMIT)
                m_movingSinceMs = nowMs;
            else if (nowMs - m_movingSinceMs >= HOLD_ENTER_MS)
            {
                m_state = State::Holding;
                m_holdPos = measured;
                m_releaseSinceMs = 0;
                // Seed the envelope at the effort that is currently holding it,
                // never below the floor we have already learned.
                m_envelope = fmaxf(fabsf(pCurrent), m_floor);
                m_envelope = fminf(m_envelope, HOLD_MAX_CURRENT);
                m_recovering = false;
                LogHandler::debug(Tags::Motor, "Hold: engaged at %.3fA (floor %.3fA, pos %.0f)",
                                  m_envelope, m_floor, measured);
            }
            return m_recovering ? clampTo(pCurrent, HOLD_MAX_CURRENT) : pCurrent;

        case State::Holding:
            if (fabsf(measured - m_holdPos) > HOLD_DRIFT_LIMIT)
            {
                // Shed too far - the level we were at could not hold the load.
                learnFloor(m_envelope);
                LogHandler::debug(Tags::Motor, "Hold: drifted %.0f counts at %.3fA, floor now %.3fA",
                                  measured - m_holdPos, m_envelope, m_floor);
                goActive(measured, nowMs, true);
                return clampTo(pCurrent, HOLD_MAX_CURRENT);
            }

            // Exponential decay towards the learned floor.
            if (dt > 0.0f)
                m_envelope = m_floor + (m_envelope - m_floor) * expf(-dt / HOLD_DECAY_TAU_S);

            if (m_envelope <= HOLD_RELEASE_CURRENT)
            {
                if (!m_releaseSinceMs)
                    m_releaseSinceMs = nowMs;
                else if (nowMs - m_releaseSinceMs >= HOLD_RELEASE_MS)
                {
                    m_state = State::Released;
                    // Nothing was needed to hold, so any floor we carried is stale.
                    m_floor = 0.0f;
                    LogHandler::info(Tags::Motor, "Hold: drive released, idle current zero");
                    return 0.0f;
                }
            }
            else
            {
                m_releaseSinceMs = 0;
                // Credit a hold that is surviving below the floor it was given.
                m_floor *= HOLD_FLOOR_RELAX;
                if (m_floor < HOLD_RELEASE_CURRENT)
                    m_floor = 0.0f;
            }
            return clampTo(pCurrent, m_envelope);

        case State::Released:
        default:
            if (fabsf(measured - m_holdPos) > HOLD_DRIFT_LIMIT)
            {
                // It does need holding after all - come back gently, and next
                // time stop shedding before we get here.
                learnFloor(HOLD_RELEASE_CURRENT);
                LogHandler::debug(Tags::Motor, "Hold: drifted %.0f counts while released, floor now %.3fA",
                                  measured - m_holdPos, m_floor);
                goActive(measured, nowMs, true);
                return clampTo(pCurrent, HOLD_MAX_CURRENT);
            }
            return 0.0f;
        }
    }

    State state() const { return m_state; }
    bool driveShouldBeOff() const { return m_state == State::Released; }
    bool isHolding() const { return m_state != State::Active; }
    float envelope() const { return m_envelope; }
    float floorCurrent() const { return m_floor; }

    const char *stateName() const
    {
        switch (m_state)
        {
        case State::Active:
            return "active";
        case State::Holding:
            return "holding";
        default:
            return "released";
        }
    }

private:
    static constexpr float VELOCITY_FILTER = 0.95f;

    void goActive(float measured, uint32_t nowMs, bool recovering)
    {
        m_state = State::Active;
        m_holdPos = measured;
        m_movingSinceMs = nowMs;
        m_releaseSinceMs = 0;
        m_envelope = 0.0f;
        m_recovering = recovering;
    }

    void learnFloor(float failedAt)
    {
        m_floor = fminf(fmaxf(failedAt, HOLD_RELEASE_CURRENT) * HOLD_FLOOR_MARGIN, HOLD_MAX_CURRENT);
    }

    static float clampTo(float v, float limit)
    {
        if (v > limit)
            return limit;
        if (v < -limit)
            return -limit;
        return v;
    }

    State m_state = State::Active;
    int m_target = 0;
    float m_holdPos = 0.0f;
    float m_lastPos = 0.0f;
    float m_velocity = 0.0f;
    float m_envelope = 0.0f;
    // Smallest current known to hold the load, in Amps. Learned, not configured.
    float m_floor = 0.0f;
    bool m_recovering = false;
    uint32_t m_lastMs = 0;
    uint32_t m_movingSinceMs = 0;
    uint32_t m_releaseSinceMs = 0;
};
