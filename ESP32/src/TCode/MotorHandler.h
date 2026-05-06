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
#include "TCodeBase.h"
#include "MCPWMServo.h"
#include "PwmManager.h"
#include "settings/SettingsHandler.h"
#include "logging/TagHandler.h"
#include "callback.h"

class MotorHandler
{
public:
    virtual void setup() = 0;
    virtual void read(byte inByte) = 0;
    virtual void read(const String& input) = 0;
    virtual void read(const char* input, size_t len) = 0;
    virtual void execute() = 0;
    virtual void setMessageCallback(TCodeCommandCallback function) = 0;

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
     * Handlers that allocate (e.g. v0.4 axes via `new TCodeAxis`) should
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
     * Register the active motor handler so static helpers (e.g. command
     * handlers) can route reapply requests at it.
     */
    static void setActive(MotorHandler* handler) { s_active = handler; }
    static MotorHandler* getActive() { return s_active; }

protected:
    static volatile bool s_reapplyRequested;
    static volatile bool s_identifyActive;
    static MotorHandler* s_active;
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
     * Write a duty value to a previously-attached pin (servo or LEDC).
     * PwmManager routes to the correct backend.
     */
    void writeServo(uint8_t pin, uint32_t duty)
    {
        PwmManager::instance().write((int8_t)pin, duty);
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
     * Write an 8-bit (0..255) duty value to a vibe / lube pin. Scales the
     * value up to the channel's actual resolution and routes via
     * PwmManager so MCPWM-fallback pins also receive the write. Replaces
     * direct ledcWrite(pin, byteDuty) call sites that broke after we
     * defaulted the LEDC resolution to SERVO_PWM_RES (14-bit) and after
     * the LEDC->MCPWM fallback was added.
     */
    void writeVibe8(uint8_t pin, uint8_t duty8)
    {
        PwmManager& pm = PwmManager::instance();
        uint8_t res = pm.getResolution((int8_t)pin);
        if (res == 0) // not attached; PwmManager will swallow the write.
        {
            LogHandler::verbose(Tags::Motor,
                "writeVibe8: pin %u not attached (duty8=%u)",
                (unsigned)pin, (unsigned)duty8);
            pm.write((int8_t)pin, duty8);
            return;
        }
        uint32_t maxDuty = (res >= 32) ? 0xFFFFFFFFu : ((1u << res) - 1u);
        uint32_t scaled = (uint32_t)duty8 * maxDuty / 255u;
        pm.write((int8_t)pin, scaled);
    }

    /**
     * This method gets the period of the frequency 1/f
     * and converts the units to microseconds * 1000000
     */
    int frequencyToMicroseconds(int freq)
    {
        return 1000000 / freq;
    }
};

// Static member definitions (header-only class -> use inline storage).
inline volatile bool MotorHandler::s_reapplyRequested = false;
inline volatile bool MotorHandler::s_identifyActive = false;
inline MotorHandler* MotorHandler::s_active = nullptr;