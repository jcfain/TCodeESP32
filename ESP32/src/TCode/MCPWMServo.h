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

#include "esp_idf_version.h"
#include <esp_err.h>

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#define MCPWM_V5
#include "driver/mcpwm_prelude.h"
#else
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#endif

#define MCPWM_MAX_SERVO_OUTPUTS 12

/**
 * MCPWM-based servo PWM controller.
 *
 * Replaces LEDC for servo-frequency outputs. Duty values use the same
 * 0 .. (2^resolution - 1) range that LEDC used, so callers need no
 * changes to their duty math.
 *
 * Resource allocation:
 *   ESP-IDF 5.x  – 2 groups × 3 operators × 2 generators = 12 outputs
 *   ESP-IDF 4.x  – 2 units  × 3 timers    × 2 generators = 12 outputs
 */
class MCPWMServo
{
public:
    static MCPWMServo &getInstance()
    {
        static MCPWMServo instance;
        return instance;
    }

    MCPWMServo(const MCPWMServo &) = delete;
    void operator=(const MCPWMServo &) = delete;

    /**
     * Attach a GPIO pin as an MCPWM servo output.
     * @param gpio_num  GPIO pin number
     * @param freq_hz   PWM frequency (e.g. 50 for standard servos)
     * @param resolution_bits  PWM resolution in bits (16 on ESP32, 14 on S3)
     * @return true on success
     */
    bool attachPin(int gpio_num, uint32_t freq_hz, uint8_t resolution_bits)
    {
        if (m_count >= MCPWM_MAX_SERVO_OUTPUTS || gpio_num < 0)
        {
            return false;
        }
#ifdef MCPWM_V5
        return attachV5(gpio_num, freq_hz, resolution_bits);
#else
        return attachLegacy(gpio_num, freq_hz, resolution_bits);
#endif
    }

    /**
     * Set duty on an MCPWM servo output.
     * @param gpio_num  The GPIO pin (must have been previously attached)
     * @param duty      Duty value, 0 .. (2^resolution_bits - 1)
     * @return true on success
     */
    bool write(int gpio_num, uint32_t duty)
    {
        for (int i = 0; i < m_count; i++)
        {
            if (m_outputs[i].gpio_num == gpio_num)
            {
#ifdef MCPWM_V5
                return mcpwm_comparator_set_compare_value(
                           m_outputs[i].comparator, duty) == ESP_OK;
#else
                uint32_t period_us = 1000000 / m_outputs[i].freq_hz;
                uint32_t duty_us = (uint32_t)((uint64_t)duty * period_us / m_outputs[i].max_duty);
                return mcpwm_set_duty_in_us(
                           m_outputs[i].unit, m_outputs[i].timer_num,
                           m_outputs[i].gen, duty_us) == ESP_OK;
#endif
            }
        }
        return false;
    }

    /**
     * Detach a previously-attached pin: disable + delete its generator and
     * comparator, decrement the operator's gen_count, and free the operator/
     * timer once empty. Pin is forced LOW before teardown.
     *
     * @return true if pin was attached and successfully detached
     */
    bool detachPin(int gpio_num)
    {
#ifdef MCPWM_V5
        return detachV5(gpio_num);
#else
        // Legacy IDF: per-pin teardown not implemented. The mcpwm_init/timer
        // model doesn't make per-pin detach safe without full driver state
        // tracking we don't have. Treat as best-effort.
        (void)gpio_num;
        return false;
#endif
    }

    /** Detach every tracked pin. */
    void detachAll()
    {
        while (m_count > 0)
        {
            int gpio = m_outputs[m_count - 1].gpio_num;
            if (!detachPin(gpio))
            {
                // detach failed; drop the slot to avoid an infinite loop.
                m_count--;
            }
        }
    }

private:
    MCPWMServo() = default;

#ifdef MCPWM_V5
    /* ---- ESP-IDF 5.x new driver ---- */

    struct ServoOutput
    {
        int gpio_num = -1;
        mcpwm_cmpr_handle_t comparator = nullptr;
        mcpwm_gen_handle_t generator = nullptr;
        int8_t group = -1;
        int8_t op = -1;
    };

    struct TimerSlot
    {
        mcpwm_timer_handle_t handle = nullptr;
        uint32_t freq_hz = 0;
    };

    struct OperatorSlot
    {
        mcpwm_oper_handle_t handle = nullptr;
        uint8_t gen_count = 0;
    };

    TimerSlot m_timers[2] = {};      // 1 timer per MCPWM group
    OperatorSlot m_opers[2][3] = {}; // 3 operators per group
    ServoOutput m_outputs[MCPWM_MAX_SERVO_OUTPUTS] = {};
    int m_count = 0;

    bool attachV5(int gpio_num, uint32_t freq_hz, uint8_t resolution_bits)
    {
        uint32_t period_ticks = 1u << resolution_bits;
        uint32_t resolution_hz = freq_hz * period_ticks;

        // MCPWM_TIMER_CLK_SRC_DEFAULT is the PLL/APB-derived 160 MHz clock on
        // classic ESP32 and 80 MHz on S3/C3. If `resolution_hz` exceeds the
        // source clock, IDF's `mcpwm_new_timer` computes prescale = src/res = 0
        // and the timer init divides by prescale → IntegerDivideByZero panic.
        // Servo-style configs (50 Hz @ 14-bit ≈ 819 kHz) sit far below this
        // limit; this guard blocks misuse like vibe @ 8 kHz / 15-bit
        // (262 MHz) from triggering an unhandled exception.
        constexpr uint32_t MCPWM_MAX_RESOLUTION_HZ = 80'000'000u;
        if (resolution_hz == 0 || resolution_hz > MCPWM_MAX_RESOLUTION_HZ)
        {
            return false;
        }

        // Find a group whose timer matches this frequency (or is free) AND still has
        // an operator slot available.  A group that matches frequency but is fully
        // occupied must be skipped so we can try the other group.
        int grp = -1;
        for (int g = 0; g < 2; g++)
        {
            if (!m_timers[g].handle || m_timers[g].freq_hz == freq_hz)
            {
                // Check there is at least one operator with room
                bool hasRoom = false;
                for (int o = 0; o < 3; o++)
                {
                    if (m_opers[g][o].gen_count < 2)
                    {
                        hasRoom = true;
                        break;
                    }
                }
                if (hasRoom)
                {
                    grp = g;
                    break;
                }
            }
        }
        if (grp < 0)
            return false;

        // Create timer on first use
        if (!m_timers[grp].handle)
        {
            mcpwm_timer_config_t cfg = {};
            cfg.group_id = grp;
            cfg.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT;
            cfg.resolution_hz = resolution_hz;
            cfg.count_mode = MCPWM_TIMER_COUNT_MODE_UP;
            cfg.period_ticks = period_ticks;
            if (mcpwm_new_timer(&cfg, &m_timers[grp].handle) != ESP_OK)
                return false;
            m_timers[grp].freq_hz = freq_hz;
            mcpwm_timer_enable(m_timers[grp].handle);
            mcpwm_timer_start_stop(m_timers[grp].handle,
                                   MCPWM_TIMER_START_NO_STOP);
        }

        // Find an operator with room (each holds up to 2 generators)
        int op = -1;
        for (int o = 0; o < 3; o++)
        {
            if (m_opers[grp][o].gen_count < 2)
            {
                op = o;
                break;
            }
        }
        if (op < 0)
            return false;

        // Create operator on first use
        if (!m_opers[grp][op].handle)
        {
            mcpwm_operator_config_t cfg = {};
            cfg.group_id = grp;
            if (mcpwm_new_operator(&cfg, &m_opers[grp][op].handle) != ESP_OK)
                return false;
            mcpwm_operator_connect_timer(m_opers[grp][op].handle,
                                         m_timers[grp].handle);
        }

        // Comparator (one per output)
        mcpwm_comparator_config_t cmp_cfg = {};
        cmp_cfg.flags.update_cmp_on_tez = true;
        mcpwm_cmpr_handle_t cmp = nullptr;
        if (mcpwm_new_comparator(m_opers[grp][op].handle, &cmp_cfg, &cmp) != ESP_OK)
            return false;
        mcpwm_comparator_set_compare_value(cmp, 0);

        // Generator (bound to the GPIO)
        mcpwm_generator_config_t gen_cfg = {};
        gen_cfg.gen_gpio_num = gpio_num;
        mcpwm_gen_handle_t gen = nullptr;
        if (mcpwm_new_generator(m_opers[grp][op].handle, &gen_cfg, &gen) != ESP_OK)
            return false;

        // PWM actions: HIGH on timer-zero, LOW on compare-match
        mcpwm_generator_set_action_on_timer_event(gen,
                                                  MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,
                                                                               MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH));
        mcpwm_generator_set_action_on_compare_event(gen,
                                                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP,
                                                                                   cmp, MCPWM_GEN_ACTION_LOW));

        m_opers[grp][op].gen_count++;

        m_outputs[m_count].gpio_num   = gpio_num;
        m_outputs[m_count].comparator = cmp;
        m_outputs[m_count].generator  = gen;
        m_outputs[m_count].group      = (int8_t)grp;
        m_outputs[m_count].op         = (int8_t)op;
        m_count++;
        return true;
    }

    bool detachV5(int gpio_num)
    {
        int idx = -1;
        for (int i = 0; i < m_count; i++)
        {
            if (m_outputs[i].gpio_num == gpio_num) { idx = i; break; }
        }
        if (idx < 0) return false;

        ServoOutput &o = m_outputs[idx];

        // Force pin LOW so a connected servo doesn't see a stale duty.
        if (o.generator)
        {
            mcpwm_generator_set_force_level(o.generator, 0, true);
        }

        // Tear down generator and comparator.
        if (o.generator)
            mcpwm_del_generator(o.generator);
        if (o.comparator)
            mcpwm_del_comparator(o.comparator);

        // Decrement operator's generator count, free the operator if empty.
        int8_t grp = o.group;
        int8_t op  = o.op;
        if (grp >= 0 && op >= 0)
        {
            if (m_opers[grp][op].gen_count > 0)
                m_opers[grp][op].gen_count--;
            if (m_opers[grp][op].gen_count == 0 && m_opers[grp][op].handle)
            {
                mcpwm_del_operator(m_opers[grp][op].handle);
                m_opers[grp][op].handle = nullptr;
            }

            // If all operators in the group are empty, free the timer too.
            bool grpEmpty = true;
            for (int i = 0; i < 3; i++)
            {
                if (m_opers[grp][i].handle) { grpEmpty = false; break; }
            }
            if (grpEmpty && m_timers[grp].handle)
            {
                mcpwm_timer_start_stop(m_timers[grp].handle, MCPWM_TIMER_STOP_EMPTY);
                mcpwm_timer_disable(m_timers[grp].handle);
                mcpwm_del_timer(m_timers[grp].handle);
                m_timers[grp].handle  = nullptr;
                m_timers[grp].freq_hz = 0;
            }
        }

        // Compact m_outputs.
        for (int i = idx; i + 1 < m_count; i++)
            m_outputs[i] = m_outputs[i + 1];
        m_outputs[m_count - 1] = ServoOutput{};
        m_count--;
        return true;
    }

#else
    /* ---- ESP-IDF 4.x legacy driver ---- */

    struct ServoOutput
    {
        int gpio_num = -1;
        uint32_t freq_hz = 0;
        uint32_t max_duty = 0;
        mcpwm_unit_t unit;
        mcpwm_timer_t timer_num;
        mcpwm_generator_t gen;
    };

    ServoOutput m_outputs[MCPWM_MAX_SERVO_OUTPUTS] = {};
    int m_count = 0;
    int m_nextSlot = 0;
    bool m_timerInited[2][3] = {};

    bool attachLegacy(int gpio_num, uint32_t freq_hz,
                      uint8_t resolution_bits)
    {
        if (m_nextSlot >= 12)
            return false;

        int slot = m_nextSlot++;
        mcpwm_unit_t unit = (mcpwm_unit_t)(slot / 6);
        int within = slot % 6;
        mcpwm_timer_t timer = (mcpwm_timer_t)(within / 2);
        mcpwm_generator_t gen = (within & 1) ? MCPWM_GEN_B : MCPWM_GEN_A;
        mcpwm_io_signals_t sig = (mcpwm_io_signals_t)(within);

        mcpwm_gpio_init(unit, sig, gpio_num);

        if (!m_timerInited[(int)unit][(int)timer])
        {
            mcpwm_config_t pwm_cfg = {};
            pwm_cfg.frequency = freq_hz;
            pwm_cfg.cmpr_a = 0;
            pwm_cfg.cmpr_b = 0;
            pwm_cfg.duty_mode = MCPWM_DUTY_MODE_0;
            pwm_cfg.counter_mode = MCPWM_UP_COUNTER;
            mcpwm_init(unit, timer, &pwm_cfg);
            m_timerInited[(int)unit][(int)timer] = true;
        }

        mcpwm_set_duty_type(unit, timer, gen, MCPWM_DUTY_MODE_0);

        m_outputs[m_count].gpio_num = gpio_num;
        m_outputs[m_count].freq_hz = freq_hz;
        m_outputs[m_count].max_duty = (1u << resolution_bits) - 1;
        m_outputs[m_count].unit = unit;
        m_outputs[m_count].timer_num = timer;
        m_outputs[m_count].gen = gen;
        m_count++;
        return true;
    }

#endif
};
