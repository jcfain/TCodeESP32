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
#include "soc/soc_caps.h"
#include "Global.h"
#include "MCPWMServo.h"
#include "PwmManager.h"
#include "settings/SettingsHandler.h"
#include "logging/TagHandler.h"
#include "callback.h"
#include "TCodeBase.h"

class MotorHandler
{
public:
    virtual void setup() = 0;
    virtual void read(byte inByte) = 0;
    // virtual void read(const String &input) = 0;
    virtual void read(const char* input, size_t len) = 0;
    virtual void execute() = 0;
    virtual void setMessageCallback(TCodeCommandCallback function) = 0; // Sets the callback function used by TCode

    /**
     * Wiggle a single physical servo by its slot name ("RightServo", "LeftServo",
     * "RightUpperServo", "LeftUpperServo", "PitchServo", "PitchRightServo",
     * "ValveServo", "TwistServo", "SqueezeServo") to allow visual identification.
     * The default implementation is a no-op so BLDC handlers don't need to override.
     */
    virtual void identifyServo(const char* servoName) {}

    /**
     * Identify-wiggle gate. Set by the wiggle task while it owns a servo
     * pin so the motor control loop can skip its normal PWM writes for
     * the duration — otherwise execute() rewrites the centre duty every
     * ~1 ms and the wiggle is invisible.
     */
    static bool isIdentifying() { return s_identifyActive; }
    static void setIdentifying(bool v) { s_identifyActive = v; }

    /**
     * Re-apply PWM hardware bindings without rebooting. Default detaches every
     * PwmManager-tracked pin then re-runs setup(). This is safe for handlers
     * whose setup() is idempotent (no leaked allocations on second call).
     * Handlers that allocate should either make setup() reuse what it
     * allocated (v0.4 axes do, via MotorHandler0_4::registerAxis) or
     * override this to avoid leaks.
     *
     * Called from the motor task via serviceReapply() so the actual hardware
     * teardown/setup happens on the same core that owns the motor loop.
     */
    virtual void reapplyPwm()
    {
        LogHandler::info(Tags::Motor, "reapplyPwm: detaching all PWM outputs");
        PwmManager::instance().detachAll();
        LogHandler::info(Tags::Motor, "reapplyPwm: re-running setup()");
        setup();
        LogHandler::info(Tags::Motor, "reapplyPwm: complete (LEDC=%d, MCPWM=%d)",
            PwmManager::instance().ledcCount(),
            PwmManager::instance().mcpwmCount());
    }

    /**
     * Re-run the position zeroing / homing routine. Only motors that have a
     * home to find implement it; the default is a no-op so servo handlers do
     * not need to care. Called from the motor task via serviceRecalibrate()
     * because it touches the driver and the control loop state.
     */
    virtual void recalibrate() {}

    /**
     * Request a hot-reattach of all PWM outputs. Safe to call from any task.
     * The actual reapply runs on the motor task at the top of its next loop
     * via serviceReapply().
     */
    static void requestReapply()
    {
        s_reapplyRequested = true;
    }

    /**
     * Called by the motor task each loop iteration. If a reapply has been
     * requested, performs it on the current task's core.
     */
    void serviceReapply()
    {
        if (!s_reapplyRequested) return;
        s_reapplyRequested = false;
        reapplyPwm();
    }

    /**
     * Request a re-home. Safe to call from any task; the routine itself runs
     * on the motor task at the top of its next loop.
     */
    static void requestRecalibrate()
    {
        s_recalibrateRequested = true;
    }

    /** Called by the motor task each loop iteration. */
    void serviceRecalibrate()
    {
        if (!s_recalibrateRequested) return;
        s_recalibrateRequested = false;
        recalibrate();
    }

    /**
     * Register the active motor handler so static helpers (e.g. command
     * handlers) can route reapply requests at it.
     */
    static void setActive(MotorHandler* handler) { s_active = handler; }
    static MotorHandler* getActive() { return s_active; }

protected:
    static volatile bool s_reapplyRequested;
    static volatile bool s_recalibrateRequested;
    static volatile bool s_identifyActive;
    static MotorHandler* s_active;

    /**
     * The max duty value callers compute servo positions against, i.e.
     * (2^servoResolution - 1). Set by the subclass during setupCommon().
     * writeServo() normalizes from this domain to 16-bit before routing
     * through PwmManager::writeNormalized(), so the actual backend
     * resolution (which may differ from servoResolution if LEDC auto-
     * bumped) is transparent to callers.
     */
    uint32_t m_servoPWMMaxDuty = 0;
    /**
     * Attach a servo-frequency PWM output via the unified PwmManager.
     *
     * @param channel  Stored timer-channel hint from settings. Kept for API
     *                 compatibility with v0.3/v0.4 handlers but no longer
     *                 drives hardware allocation; PwmManager owns that.
     * @param driver   PwmDriver hint from the timer config. MCPWM tries MCPWM
     *                 first then auto-falls-back to LEDC; LEDC skips MCPWM.
     */
    void attachServoPin(const char* name, uint8_t pin, uint32_t freq,
        int8_t channel = -1, PwmDriver driver = PwmDriver::MCPWM)
    {
        (void)channel;
        PwmManager::instance().attachServo(name, (int8_t)pin, freq, SERVO_PWM_RES, driver);
    }

    /**
     * Write a duty value to a previously-attached servo pin.
     *
     * The duty is in the caller's domain: [0, m_servoPWMMaxDuty].
     * This method normalizes it to [0, 65535] and routes through
     * PwmManager::writeNormalized(), which scales to the backend's
     * actual resolution (which may differ from m_servoPWMMaxDuty if
     * LEDC auto-bumped or MCPWM negotiated differently). Callers never
     * need to know the hardware resolution.
     */
    void writeServo(uint8_t pin, uint32_t duty)
    {
        uint16_t norm = 0;
        if (m_servoPWMMaxDuty > 0)
            norm = (uint16_t)(((uint64_t)duty * 65535U) / m_servoPWMMaxDuty);
        PwmManager::instance().writeNormalized((int8_t)pin, norm);
    }

    /**
     * Attach a LEDC PWM output (vibration motors, lube, heater, fan, etc.).
     * @param channel  Stored hint, ignored at hardware level. PwmManager
     *                 always auto-allocates a free LEDC channel.
     * @param res      Duty resolution. Defaults to SERVO_PWM_RES so vibe /
     *                 lube outputs share a timer with servos at the same
     *                 frequency (LEDC reuses timers only when freq AND
     *                 resolution match). Power users can override via the
     *                 advanced settings interface.
     */
    void attachLedcPin(const char* name, uint8_t pin, uint32_t freq, int8_t channel = -1, uint8_t res = SERVO_PWM_RES)
    {
        (void)channel;
        PwmManager::instance().attachLedc(name, (int8_t)pin, freq, res);
    }

    /**
     * Write an 8-bit (0..255) duty value to a vibe / lube pin.
     *
     * Normalizes to 16-bit (0..65535) and routes through
     * PwmManager::writeNormalized(), which scales to the backend's
     * actual resolution. This replaces the old getResolution() +
     * manual scaling path and works correctly regardless of whether
     * the pin ended up on LEDC (possibly auto-bumped) or MCPWM.
     */
    void writeVibe8(uint8_t pin, uint8_t duty8)
    {
        // Map [0,255] -> [0,65535]. 255 * 257 = 65535 exactly.
        uint16_t norm = (uint16_t)duty8 * 257U;
        PwmManager::instance().writeNormalized((int8_t)pin, norm);
    }

    /**
     * This method gets the period of the frequency 1/f
     * and converts the units to microseconds * 1000000
     */
    int frequencyToMicroseconds(int freq)
    {
        if (freq <= 0)
            return 0;
        return 1000000 / freq;
    }
};

// Static member definitions (header-only class -> use inline storage).
inline volatile bool MotorHandler::s_reapplyRequested = false;
inline volatile bool MotorHandler::s_recalibrateRequested = false;
inline volatile bool MotorHandler::s_identifyActive = false;
inline MotorHandler* MotorHandler::s_active = nullptr;