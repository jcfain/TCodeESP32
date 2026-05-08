/* MIT License

Copyright (c) 2024 Jason C. Fain

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

/**
 * PwmManager — unified PWM front-end for both LEDC and MCPWM backends.
 *
 * Goals:
 *   1. One place that owns the decision of which hardware drives a pin.
 *   2. Pair servos at the same frequency onto the same MCPWM operator/timer
 *      automatically (handled by the MCPWMServo backend), and let LEDC's
 *      Arduino-3 manager reuse a timer when freq+resolution match.
 *   3. Detect and report resource exhaustion. Auto-fallback from MCPWM to
 *      LEDC when MCPWM is full, and warn (don't fail) callers.
 *   4. Auto-allocate LEDC channels — never pass a stored "channel" value
 *      directly to hardware. The stored channel is just a hint for which
 *      timer config the user picked in the GUI, and on ESP32-S3 the HIGH*
 *      enum values (8..15) aren't valid LEDC channels at all.
 *   5. Track pin->backend so write(pin, duty) and detach(pin) route to the
 *      right driver.
 *
 * This class is header-only and used as a singleton.
 */

#include <Arduino.h>
#include "soc/soc_caps.h"
#include "MCPWMServo.h"
#include "logging/TagHandler.h"
#include "enum.h"

#ifndef SOC_LEDC_CHANNEL_NUM
#define SOC_LEDC_CHANNEL_NUM 16
#endif

#ifndef PWM_MANAGER_MAX_PINS
#define PWM_MANAGER_MAX_PINS 24
#endif

#ifndef PWM_MANAGER_MAX_FAILURES
#define PWM_MANAGER_MAX_FAILURES 16
#endif

class PwmManager
{
public:
    enum class Backend : uint8_t
    {
        NONE = 0,
        MCPWM = 1,
        LEDC = 2,
    };

    /**
     * One slot in the failure table. Surfaced via failures()/failureCount()
     * so the GUI (systemInfo) can flag pins that couldn't bind to any
     * hardware backend after the auto-fallback attempts.
     */
    struct Failure
    {
        char     name[20] = "";
        int8_t   pin = -1;
        uint32_t freq = 0;
        uint8_t  resolution = 0;
        Backend  triedFirst = Backend::NONE;
    };

    static PwmManager& instance()
    {
        static PwmManager s;
        return s;
    }

    PwmManager(const PwmManager&) = delete;
    PwmManager& operator=(const PwmManager&) = delete;

    /**
     * Attach a servo-frequency PWM output.
     *
     * @param name        diagnostic label
     * @param pin         GPIO number (-1 = no-op, returns NONE)
     * @param freq        target frequency in Hz (typically 50 or 333 for servos)
     * @param resolution  duty resolution in bits (matches MCPWMServo / LEDC)
     * @param preferred   PwmDriver hint from the timer config. MCPWM tries
     *                    MCPWM first then LEDC; LEDC skips MCPWM entirely.
     * @return Backend actually attached (NONE on failure).
     */
    Backend attachServo(const char* name, int8_t pin, uint32_t freq,
        uint8_t resolution, PwmDriver preferred)
    {
        if (pin < 0)
            return Backend::NONE;

        // Already attached? Detach first so re-setup works.
        if (Entry* existing = find(pin))
        {
            LogHandler::warning(Tags::Motor,
                "PwmManager: pin %d already attached (backend=%d), detaching first",
                pin, (int)existing->backend);
            detach(pin);
        }

        if (preferred == PwmDriver::MCPWM)
        {
            // Force push-pull OUTPUT before MCPWM grabs the GPIO matrix.
            // The peripheral routes its signal via the GPIO matrix but does
            // not change the IO_MUX drive mode, so a pin left as INPUT or
            // open-drain from boot / a prior assignment will mask the PWM.
            pinMode((uint8_t)pin, OUTPUT);
            digitalWrite((uint8_t)pin, LOW);
            if (MCPWMServo::getInstance().attachPin(pin, freq, resolution))
            {
                track(pin, Backend::MCPWM, freq, resolution);
                clearFailure(pin);
                LogHandler::info(Tags::Motor,
                    "PwmManager: %s -> pin %d, %u Hz, MCPWM",
                    name, pin, (unsigned)freq);
                return Backend::MCPWM;
            }
            LogHandler::warning(Tags::Motor,
                "PwmManager: %s pin %d MCPWM full, falling back to LEDC",
                name, pin);
        }

        // LEDC path (either explicitly requested or MCPWM fallback).
        Backend r = _attachLedc(name, pin, freq, resolution, /*allowMcpwmFallback=*/preferred != PwmDriver::MCPWM);
        if (r != Backend::NONE)
            return r;

        recordFailure(name, pin, freq, resolution, preferred == PwmDriver::MCPWM ? Backend::MCPWM : Backend::LEDC);
        return Backend::NONE;
    }

    /**
     * Test/diagnostic attach: bind `pin` to exactly the requested backend
     * with NO fallback. Used by the GUI's manual-PWM test panel so the
     * operator can verify a specific peripheral path drives a specific pad.
     * Detaches whatever was previously bound to the pin first. After this
     * call the only way back to "normal" allocation is a reapplyPwm or
     * device reboot.
     */
    Backend attachExclusive(const char* name, int8_t pin, uint32_t freq,
        uint8_t resolution, Backend backend)
    {
        if (pin < 0 || backend == Backend::NONE)
            return Backend::NONE;

        // Drop any prior binding (LEDC or MCPWM) so the requested backend
        // owns the pad cleanly.
        if (find(pin))
            detach(pin);

        // Force push-pull OUTPUT before either peripheral grabs the GPIO
        // matrix — same reasoning as attachServo / _attachLedc.
        pinMode((uint8_t)pin, OUTPUT);
        digitalWrite((uint8_t)pin, LOW);

        if (backend == Backend::MCPWM)
        {
            if (MCPWMServo::getInstance().attachPin((int)pin, freq, resolution))
            {
                track(pin, Backend::MCPWM, freq, resolution);
                clearFailure(pin);
                LogHandler::info(Tags::Motor,
                    "PwmManager(test): %s -> pin %d, %u Hz, %u-bit, MCPWM (exclusive)",
                    name, pin, (unsigned)freq, (unsigned)resolution);
                return Backend::MCPWM;
            }
            LogHandler::error(Tags::Motor,
                "PwmManager(test): MCPWM exclusive attach failed for %s pin %d",
                name, pin);
            recordFailure(name, pin, freq, resolution, Backend::MCPWM);
            return Backend::NONE;
        }

#ifdef ESP_ARDUINO3
        if (ledcAttach((uint8_t)pin, freq, resolution))
        {
            track(pin, Backend::LEDC, freq, resolution);
            clearFailure(pin);
            LogHandler::info(Tags::Motor,
                "PwmManager(test): %s -> pin %d, %u Hz, %u-bit, LEDC (exclusive)",
                name, pin, (unsigned)freq, (unsigned)resolution);
            return Backend::LEDC;
        }
#endif
        LogHandler::error(Tags::Motor,
            "PwmManager(test): LEDC exclusive attach failed for %s pin %d",
            name, pin);
        recordFailure(name, pin, freq, resolution, Backend::LEDC);
        return Backend::NONE;
    }

    /**
     * Attach a generic LEDC output (vibration, lube, fan, heater, indicator).
     *
     * Always uses Arduino-3 ledcAttach() which auto-allocates a hardware
     * channel and reuses an existing LEDC timer when freq+resolution match.
     */
    Backend attachLedc(const char* name, int8_t pin, uint32_t freq, uint8_t resolution)
    {
        if (pin < 0)
            return Backend::NONE;

        if (Entry* existing = find(pin))
        {
            LogHandler::warning(Tags::Motor,
                "PwmManager: pin %d already attached (backend=%d), detaching first",
                pin, (int)existing->backend);
            detach(pin);
        }

        Backend r = _attachLedc(name, pin, freq, resolution, /*allowMcpwmFallback=*/true);
        if (r != Backend::NONE)
            return r;

        recordFailure(name, pin, freq, resolution, Backend::LEDC);
        return Backend::NONE;
    }

    /**
     * Write a duty value to an attached pin. Routes to the correct backend.
     * @return true on success
     */
    bool write(int8_t pin, uint32_t duty)
    {
        if (pin < 0)
            return false;
        Entry* e = find(pin);
        if (!e)
        {
            // Fallback: try LEDC by pin (Arduino 3 handles pin->channel internally).
            // This keeps legacy paths working if someone bypasses PwmManager.
#ifdef ESP_ARDUINO3
            ledcWrite((uint8_t)pin, duty);
            return true;
#else
            return false;
#endif
        }
        switch (e->backend)
        {
        case Backend::MCPWM:
            return MCPWMServo::getInstance().write(pin, duty);
        case Backend::LEDC:
#ifdef ESP_ARDUINO3
            ledcWrite((uint8_t)pin, duty);
            return true;
#else
            return false;
#endif
        default:
            return false;
        }
    }

    /**
     * Detach a previously attached pin. Frees the LEDC channel (Arduino-3
     * ledcDetach) and tears down the MCPWM generator/comparator (V5).
     */
    bool detach(int8_t pin)
    {
        if (pin < 0)
            return false;
        Entry* e = find(pin);
        if (!e)
            return false;

        Backend backend = e->backend;
        if (backend == Backend::LEDC)
        {
#ifdef ESP_ARDUINO3
            ledcDetach((uint8_t)pin);
#endif
        }
        else if (backend == Backend::MCPWM)
        {
            MCPWMServo::getInstance().detachPin((int)pin);
        }

        // Compact array
        int idx = (int)(e - m_entries);
        for (int i = idx; i + 1 < m_count; i++)
            m_entries[i] = m_entries[i + 1];
        m_count--;
        return true;
    }

    /**
     * Detach every tracked pin. Used by reapplyPwm() to drop all current
     * hardware bindings before re-attaching from refreshed settings.
     */
    void detachAll()
    {
        while (m_count > 0)
        {
            detach(m_entries[m_count - 1].pin);
        }
        // Wipe the failure table too — reapplyPwm() rebuilds from scratch
        // and any prior errors should be re-evaluated against the new
        // configuration, not surfaced stale.
        m_failureCount = 0;
    }

    /**
     * Read-only access to the failure table. The GUI shows these as
     * configuration errors ("pin X couldn't be attached") so the user
     * can pick a different pin, lower the timer freq/res, or unset.
     */
    const Failure* failures() const { return m_failures; }
    int failureCount() const { return m_failureCount; }
    /**
     * Resolution (in bits) actually negotiated for the pin's hardware
     * channel. Returns 0 if the pin isn't attached. Callers use this to
     * scale legacy 8-bit duty values (e.g. vibe 0..255) up to the real
     * channel range (e.g. 0..16383 at 14-bit) so attaching at a higher
     * resolution doesn't silently divide the output by 64x.
     */
    uint8_t getResolution(int8_t pin) const
    {
        for (int i = 0; i < m_count; i++)
            if (m_entries[i].pin == pin)
                return m_entries[i].resolution;
        return 0;
    }
    int countByBackend(Backend b) const
    {
        int n = 0;
        for (int i = 0; i < m_count; i++)
            if (m_entries[i].backend == b) n++;
        return n;
    }

    int ledcCount() const { return countByBackend(Backend::LEDC); }
    int mcpwmCount() const { return countByBackend(Backend::MCPWM); }

private:
    PwmManager() = default;

    struct Entry
    {
        int8_t   pin = -1;
        Backend  backend = Backend::NONE;
        uint8_t  resolution = 0;
        uint32_t freq = 0;
    };

    Entry m_entries[PWM_MANAGER_MAX_PINS];
    int   m_count = 0;

    Failure m_failures[PWM_MANAGER_MAX_FAILURES];
    int     m_failureCount = 0;

    void recordFailure(const char* name, int8_t pin, uint32_t freq,
        uint8_t resolution, Backend triedFirst)
    {
        // Replace existing entry for the same pin if present
        for (int i = 0; i < m_failureCount; i++)
        {
            if (m_failures[i].pin == pin)
            {
                strlcpy(m_failures[i].name, name ? name : "", sizeof(m_failures[i].name));
                m_failures[i].freq = freq;
                m_failures[i].resolution = resolution;
                m_failures[i].triedFirst = triedFirst;
                return;
            }
        }
        if (m_failureCount >= PWM_MANAGER_MAX_FAILURES)
            return;
        Failure& f = m_failures[m_failureCount++];
        strlcpy(f.name, name ? name : "", sizeof(f.name));
        f.pin = pin;
        f.freq = freq;
        f.resolution = resolution;
        f.triedFirst = triedFirst;
    }

    void clearFailure(int8_t pin)
    {
        for (int i = 0; i < m_failureCount; i++)
        {
            if (m_failures[i].pin == pin)
            {
                for (int j = i; j + 1 < m_failureCount; j++)
                    m_failures[j] = m_failures[j + 1];
                m_failureCount--;
                return;
            }
        }
    }

    Entry* find(int8_t pin)
    {
        for (int i = 0; i < m_count; i++)
            if (m_entries[i].pin == pin) return &m_entries[i];
        return nullptr;
    }

    void track(int8_t pin, Backend backend, uint32_t freq, uint8_t resolution)
    {
        if (m_count >= PWM_MANAGER_MAX_PINS)
        {
            LogHandler::error(Tags::Motor,
                "PwmManager: tracking table full (max %d), pin %d untracked",
                PWM_MANAGER_MAX_PINS, pin);
            return;
        }
        m_entries[m_count++] = { pin, backend, resolution, freq };
    }

    Backend _attachLedc(const char* name, int8_t pin, uint32_t freq, uint8_t resolution,
        bool allowMcpwmFallback)
    {
#ifdef ESP_ARDUINO3
        // Force push-pull OUTPUT before ledcAttach grabs the GPIO matrix.
        // ledcAttach binds the LEDC peripheral via the GPIO matrix but does
        // not change the IO_MUX drive mode — so a pin previously set as
        // INPUT, open-drain, or held by a stale Wire/I2C bus stays in that
        // mode and the PWM signal never reaches the pad. Explicit pinMode
        // guarantees a clean push-pull driver before the LEDC signal
        // takes over.
        pinMode((uint8_t)pin, OUTPUT);
        digitalWrite((uint8_t)pin, LOW);

        // Always use auto-channel allocation. Arduino-3's LEDC manager picks
        // a free channel and reuses an existing timer when freq+resolution
        // match. This avoids the S3 channel-out-of-range issue (HIGH* enum
        // values 8..15 aren't valid LEDC channels) and avoids accidental
        // channel collisions between servos and vibe outputs.
        bool ok = ledcAttach((uint8_t)pin, freq, resolution);
        if (ok)
        {
            track(pin, Backend::LEDC, freq, resolution);
            clearFailure(pin);
            LogHandler::info(Tags::Motor,
                "PwmManager: %s -> pin %d, %u Hz, %u-bit, LEDC",
                name, pin, (unsigned)freq, (unsigned)resolution);
            return Backend::LEDC;
        }

        // LEDC failed (no free timers, freq+res not achievable, etc.).
        // Try MCPWM as a fallback for servo-style frequencies if allowed.
        // MCPWM has its own pool independent of LEDC and uses a 16-bit
        // counter so common 50 Hz / 14-bit servo configs map cleanly.
        if (allowMcpwmFallback)
        {
            LogHandler::warning(Tags::Motor,
                "PwmManager: %s pin %d LEDC full, trying MCPWM fallback",
                name, pin);
            if (MCPWMServo::getInstance().attachPin((int)pin, freq, resolution))
            {
                track(pin, Backend::MCPWM, freq, resolution);
                clearFailure(pin);
                LogHandler::info(Tags::Motor,
                    "PwmManager: %s -> pin %d, %u Hz, %u-bit, MCPWM (LEDC fallback)",
                    name, pin, (unsigned)freq, (unsigned)resolution);
                return Backend::MCPWM;
            }
        }

        LogHandler::error(Tags::Motor,
            "PwmManager: %s LEDC attach failed for pin %d @ %u Hz / %u-bit "
            "(LEDC channels in use: %d/%d)",
            name, pin, (unsigned)freq, (unsigned)resolution,
            ledcCount(), (int)SOC_LEDC_CHANNEL_NUM);
        return Backend::NONE;
#else
        // Legacy IDF path: ledcSetup uses an explicit channel. We don't have
        // a free-channel allocator here, so this path requires the caller's
        // bookkeeping. Return NONE and let MotorHandler's legacy code path
        // handle it (this path isn't exercised on the current build).
        LogHandler::error(Tags::Motor,
            "PwmManager: %s LEDC attach not supported on legacy IDF (pin %d)",
            name, pin);
        (void)name; (void)pin; (void)freq; (void)resolution;
        (void)allowMcpwmFallback;
        return Backend::NONE;
#endif
    }
};
