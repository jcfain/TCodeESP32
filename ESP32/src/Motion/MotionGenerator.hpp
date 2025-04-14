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

#include <Arduino.h>
#include "SettingsHandler.h"
// #include "LogHandler.h"
#include "TagHandler.h"

class MotionGenerator {
public:
    void setup(TCodeVersion version, const MotionChannel& channel)
    {
        strcpy(m_channel, channel.name);
        tcodeVersion = version;
        setEnabled(SettingsHandler::getMotionEnabled());
        updateProfile(channel);
    }

    void updateProfile(const MotionChannel& channel) {
        //setRange(SettingsHandler::getChannelMin(channel.name), SettingsHandler::getChannelMax(channel.name));
        updateRange();
        setAmplitude(channel.motionAmplitudeGlobal);
        setOffset(channel.motionOffsetGlobal);
        setPeriod(channel.motionPeriodGlobal);
        setUpdate(channel.motionUpdateGlobal);
        setPhase(channel.motionPhaseGlobal);
        setReverse(channel.motionReversedGlobal);
        setAmplitudeRandom(channel.motionAmplitudeGlobalRandom);
        setAmplitudeRandomMin(channel.motionAmplitudeGlobalRandomMin);
        setAmplitudeRandomMax(channel.motionAmplitudeGlobalRandomMax);
        setPeriodRandom(channel.motionPeriodGlobalRandom);
        setPeriodRandomMin(channel.motionPeriodGlobalRandomMin);
        setPeriodRandomMax(channel.motionPeriodGlobalRandomMax);
        setOffsetRandom(channel.motionOffsetGlobalRandom);
        setOffsetRandomMin(channel.motionOffsetGlobalRandomMin);
        setOffsetRandomMax(channel.motionOffsetGlobalRandomMax);
        setPhaseRandom(channel.motionPhaseRandom);
        setPhaseRandomMin(channel.motionPhaseRandomMin);
        setPhaseRandomMax(channel.motionPhaseRandomMax);
    }

    void updateRange() {
        setRange(SettingsHandler::getChannelUserMin(m_channel), SettingsHandler::getChannelUserMax(m_channel));
    }
    void getMovement(char* buf, size_t len) {
        calculateNext(buf, len);
    }

    void getName(char buf[3]) {
        strcpy(buf, m_channel);
    }

    int getPeriod() {
        if(periodRandomMode) {
            if(millis() > lastRandomPeriodExecutionChange + randomPeriodExecutionPeriod) {
                lastRandomPeriodExecutionChange = millis();
                updatePeriodRandom();
            }
            return periodRandom;
        }
        return period;
    };

    // Offset from center 0
    int getOffset() {
        if(offsetRandomMode) {
            if(millis() > lastRandomOffsetExecutionChange + randomOffsetExecutionPeriod) {
                lastRandomOffsetExecutionChange = millis();
                updateOffsetRandom();
            }
            return offsetRandom;
        }
        return offset;
    };

    // The amplitude of the motion
    int getAmplitude() {
        if(amplitudeRandomMode) {
            if(millis() > lastRandomAmplitudeExecutionChange + randomAmplitudeExecutionPeriod) {
                lastRandomAmplitudeExecutionChange = millis();
                updateAmplitudeRandom();
            }
            return amplitudeRandom;
        }
        return amplitude;
    };

    // Initial phase in degrees. The phase should ideally be between (offset-amplitude/2) and (offset+amplitude/2)
    int getPhase() {
        if(phaseRandomMode) {
            if(millis() > lastRandomPhaseExecutionChange + randomPhaseExecutionPeriod) {
                lastRandomPhaseExecutionChange = millis();
                updatePhaseRandom();
            }
            return phaseRandom;
        }
        return phase;
    };

    // reverse cycle direction 
    bool getReverse() {
        return reversed;
    };

    float getStopAtCycle() {
        return stopAt;
    };

    void setEnabled(bool enable) {
        enabled = enable;
        if(enabled)
            updatePhaseIncrement();
        LogHandler::verbose(TagHandler::MotionHandler, "%s setEnabled: %d", m_channel, enable);
    }
    void setRange(uint16_t min, uint16_t max) {
        m_min = min;
        m_max = max;
    }

    void setUpdate(int value) {
        updateRate = value;
        updatePhaseIncrement();
        LogHandler::verbose(TagHandler::MotionHandler, "%s setUpdate: %d", m_channel, updateRate);
    };

    // In miliseconds this is the duty cycle (lower is faster default 2000)
    void setPeriod(int value) {
        period = value; 
        updatePhaseIncrement();
        LogHandler::verbose(TagHandler::MotionHandler, "%s setPeriod: %d", m_channel, value);
    };

    void setPeriodRandom(bool value) {
        periodRandomMode = value;
        LogHandler::verbose(TagHandler::MotionHandler, "%s setPeriodRandom: enabled %d", m_channel, value);
        if(value) {
            updatePeriodRandom();
        } else {
            updatePhaseIncrement();
        }
    }
    void setPeriodRandomMin(int min) {
        periodRandomMin = min;
        LogHandler::verbose(TagHandler::MotionHandler, "%s setPeriodRandomMin: %d", m_channel, min);
        if(periodRandomMode) {
            updatePeriodRandom();
        }
    }
    void setPeriodRandomMax(int max) {
        periodRandomMax = max;
        LogHandler::verbose(TagHandler::MotionHandler, "%s setPeriodRandomMax: %d", m_channel, max);
        if(periodRandomMode) {
            updatePeriodRandom();
        }
    }

    // Offset from center 0
    void setOffset(int value) {
        mapTCodeToDegrees(value, offset);
        LogHandler::verbose(TagHandler::MotionHandler, "%s setOffset: %d calculated degree: %d", m_channel, value, offset);
    };

    void setOffsetRandom(bool value) {
        offsetRandomMode = value;
        LogHandler::verbose(TagHandler::MotionHandler, "%s setOffsetRandom: enabled %d", m_channel, value);
        if(value) {
            updateOffsetRandom();
        }
    }
    void setOffsetRandomMin(int min) {
        mapTCodeToDegrees(min, offsetRandomMin);
        LogHandler::verbose(TagHandler::MotionHandler, "%s setOffsetRandomMin: %d calculated degree: %d", m_channel, min, offsetRandomMin);
        if(offsetRandomMode) {
            updateOffsetRandom();
        }
    }
    void setOffsetRandomMax(int max) {
        mapTCodeToDegrees(max, offsetRandomMax);
        LogHandler::verbose(TagHandler::MotionHandler, "%s setOffsetRandomMax: %d calculated degree: %d", m_channel, max, offsetRandomMax);
        if(offsetRandomMode) {
            updateOffsetRandom();
        }
    }

    // The amplitude of the motion
    void setAmplitude(int value) {
        amplitude = map(value, 0, 100, 0, 90);
        LogHandler::verbose(TagHandler::MotionHandler, "%s setAmplitude: %d calculated: %d", m_channel, value, amplitude);
    };

    void setAmplitudeRandom(bool value) {
        amplitudeRandomMode = value;
        LogHandler::verbose(TagHandler::MotionHandler, "%s setAmplitudeRandom: enabled %d", m_channel, value);
        if(value) {
            updateAmplitudeRandom();
        }
    }
    void setAmplitudeRandomMin(int min) {
        amplitudeRandomMin = map(min, 0, 100, 0, 90);
        LogHandler::verbose(TagHandler::MotionHandler, "%s setAmplitudeRandomMin: %d calculated: %d", m_channel, min, amplitudeRandomMin);
        if(amplitudeRandomMode) {
            updateAmplitudeRandom();
        }
    }
    void setAmplitudeRandomMax(int max) {
        amplitudeRandomMax = map(max, 0, 100, 0, 90);
        LogHandler::verbose(TagHandler::MotionHandler, "%s setAmplitudeRandomMax: %d calculated: %d", m_channel, max, amplitudeRandomMax);
        if(amplitudeRandomMode) {
            updateAmplitudeRandom();
        }
    }

    void setMotionRandomChangeMin(int min) {
        motionRandomChangeMin = min;
        LogHandler::verbose(TagHandler::MotionHandler, "%s setMotionRandomChangeMin: %d", m_channel, min);
        if(amplitudeRandomMode || periodRandomMode || offsetRandomMode) {
            lastExecutionPeriodChange = 0;
            checkUpdateRandomExecutionPeriod();
        }
    }
    void setMotionRandomChangeMax(int max) {
        motionRandomChangeMax = max;
        LogHandler::debug(TagHandler::MotionHandler, "%s setMotionRandomChangeMax: %d", m_channel, max);
        if(amplitudeRandomMode) {
            lastExecutionPeriodChange = 0;
            checkUpdateRandomExecutionPeriod();
        }
    }

    // Initial phase in degrees. The phase should ideally be between (offset-amplitude/2) and (offset+amplitude/2)
    void setPhase(int value) {
        phase = degreesToRadian(value);
        LogHandler::verbose(TagHandler::MotionHandler, "%s setPhase: %d", m_channel, value);
    };
    void setPhaseRandom(bool value) {
        phaseRandomMode = value;
        LogHandler::verbose(TagHandler::MotionHandler, "%s setPhaseRandom: enabled %d", m_channel, value);
        if(value) {
            updatePhaseRandom();
        }
    }
    void setPhaseRandomMin(int min) {
        phaseRandomMin = min;
        LogHandler::verbose(TagHandler::MotionHandler, "%s setPhaseRandomMin: %d", m_channel, min);
        if(offsetRandomMode) {
            updatePhaseRandom();
        }
    }
    void setPhaseRandomMax(int max) {
        phaseRandomMax = max;
        LogHandler::verbose(TagHandler::MotionHandler, "%s setPhaseRandomMax: %d", m_channel, max);
        if(offsetRandomMode) {
            updatePhaseRandom();
        }
    }

    // reverse cycle direction 
    void setReverse(bool value) {
        reversed = value;
        LogHandler::verbose(TagHandler::MotionHandler, "%s setReverse: %d", m_channel, value);
    };

    void stopAtCycle(float value) {
        stopAt = currentPhase + 2*M_PI*value;
        LogHandler::verbose(TagHandler::MotionHandler, "%s stopAtCycle: %f, stopAt %f", m_channel, value, stopAt);
    };

private:
    TCodeVersion tcodeVersion;
    bool enabled = false;
    uint16_t m_min;
    uint16_t m_max;
    char m_channel[3];
    int updateRate = 100;
    int period = 2000;    
    int periodRandom = 2000;    
    bool periodRandomMode = false;
    int periodRandomMin = 500;
    int periodRandomMax = 6000;
    int amplitude = 60;  
    int amplitudeRandom = 60; 
    bool amplitudeRandomMode = false;
    int amplitudeRandomMin = 20;
    int amplitudeRandomMax = 60;
    int offset = 0;      
    int offsetRandom = 0; 
    bool offsetRandomMode = false;
    int offsetRandomMin = 20;
    int offsetRandomMax = 80;
    float phase = 0;  
    float phaseRandom = 0;
    bool phaseRandomMode = false;
    float phaseRandomMin = 0;
    float phaseRandomMax = 180;
    bool reversed = false;
    
    int motionRandomChangeMin = 3000;
    int motionRandomChangeMax = 10000;

//Internally updated
    long lastExecutionPeriodChangePeriod = random(motionRandomChangeMin, motionRandomChangeMax);
    int randomPeriodExecutionPeriod = random(motionRandomChangeMin, motionRandomChangeMax);
    int randomAmplitudeExecutionPeriod = random(motionRandomChangeMin, motionRandomChangeMax);
    int randomOffsetExecutionPeriod = random(motionRandomChangeMin, motionRandomChangeMax);
    int randomPhaseExecutionPeriod = random(motionRandomChangeMin, motionRandomChangeMax);
    long lastExecutionPeriodChange = millis();
    long lastRandomPeriodExecutionChange = millis();
    long lastRandomAmplitudeExecutionChange = millis();
    long lastRandomOffsetExecutionChange = millis();
    long lastRandomPhaseExecutionChange = millis();
    // The current phase angle (radians)
    float currentPhase = 0.0;    
    // By how much to increment phase on every position update
    float phaseIncrement = 1;    
    // Will be true if the oscillation is stopped 
    bool stopped = false;       
    // Set a cycle to stop the movement
    float stopAt = 0;  
    // Last time the servo position was updated   
    int lastUpdate = 0; 
    // The time in between the last update and the current update
    int interval = 0; 
    
    void updateOffsetRandom() {
        offsetRandom = random(offsetRandomMin, offsetRandomMax);
        LogHandler::debug(TagHandler::MotionHandler, "%s New random offset %d", m_channel, offsetRandom);
    }
    void updateAmplitudeRandom() {
        amplitudeRandom = random(amplitudeRandomMin, amplitudeRandomMax);
        LogHandler::debug(TagHandler::MotionHandler, "%s New random amplitude %d", m_channel, amplitudeRandom);
    }
    void updatePeriodRandom() {
        periodRandom = random(periodRandomMin, periodRandomMax);
        LogHandler::debug(TagHandler::MotionHandler, "%s New random period %d", m_channel, periodRandom);
        updatePhaseIncrement();
    }
    void updatePhaseRandom() {
        phaseRandom = random(phaseRandomMin, phaseRandomMax);
        LogHandler::debug(TagHandler::MotionHandler, "%s New random phase %d", m_channel, periodRandom);
    }
    /** Gate the random change period between all attributes in between two values. 
     * If this random value time outs and any attribute hasnt timed out yet,
     * it will wait till the next timeout. */
    void checkUpdateRandomExecutionPeriod() {
        if((offsetRandomMode || amplitudeRandomMode || periodRandomMode)  
            && millis() > lastExecutionPeriodChange + lastExecutionPeriodChangePeriod) {

            if(offsetRandomMode && millis() > lastRandomOffsetExecutionChange + randomOffsetExecutionPeriod)
                randomPeriodExecutionPeriod = random(motionRandomChangeMin, motionRandomChangeMax);
            if(amplitudeRandomMode && millis() > lastRandomAmplitudeExecutionChange + randomAmplitudeExecutionPeriod)
                randomAmplitudeExecutionPeriod = random(motionRandomChangeMin, motionRandomChangeMax);
            if(periodRandomMode && millis() > lastRandomPeriodExecutionChange + randomPeriodExecutionPeriod) 
                randomOffsetExecutionPeriod = random(motionRandomChangeMin, motionRandomChangeMax);

            lastExecutionPeriodChangePeriod = random(motionRandomChangeMin, motionRandomChangeMax);
        }
    }

    void calculateNext(char* buf, size_t len) {
        if(!canUpdate()) {
            buf[0] = {0};
            return;
        }

        checkUpdateRandomExecutionPeriod();
        if(periodRandomMode)
            getPeriod(); // Just call this to check for random updates.

        if (!stopped) {
            int pos = (int)round(getAmplitude() * sin(currentPhase + getPhase()) + getOffset());
            // pos = constrain(value, -90, 90);
            if (getReverse())
                pos = -pos;
                
            // if(tcodeVersion == TCodeVersion::v0_2) {
            //     sprintf(buf, "%s%03dI%u", m_channel, constrain((uint16_t)map(pos, -90, 90, 0, 999), m_min, m_max), interval);
            // } else {
                snprintf(buf, len, "%s%04dI%u", m_channel, (uint16_t)map(pos, -90, 90, m_min, m_max), interval);
            // }

            LogHandler::verbose(TagHandler::MotionHandler, "%s pos: %d" , m_channel, pos);
            LogHandler::verbose(TagHandler::MotionHandler, "%s buf: %s" , m_channel, buf);
        }
        currentPhase = currentPhase + phaseIncrement;

        if (stopAt && currentPhase > stopAt) {
            stopped = true;
            stopAt = 0;
        }
    }

    bool canUpdate() {
        if(enabled) {
            uint32_t now = millis();

            //LogHandler::verbose(TagHandler::MotionHandler, "%s enabled interval %d lastUpdate  %d now  %d" , m_channel, interval, lastUpdate, now);
            interval = now - lastUpdate;
            if(interval > updateRate) {
                lastUpdate = now;
                return true;
            }
        }
        //LogHandler::verbose(TagHandler::MotionHandler, "%s cant update %d" , m_channel, enabled);
        return false;
    }

    /** If the user has changed the global settings update accordingly */
    // void checkLiveUpdate() {
    //     if(SettingsHandler::motionPeriodGlobalRandom) {
    //         if(millis() > lastRandomPeriodExecutionChange + randomPeriodExecutionPeriod) {
    //             lastRandomPeriodExecutionChange = millis();
    //             updatePeriodRandom();
    //             updatePhaseIncrement();
    //         }
    //     } else {
    //         if(period != SettingsHandler::motionPeriodGlobal || updateRate != SettingsHandler::motionUpdateGlobal) {
    //             if(updateRate != SettingsHandler::motionUpdateGlobal) {
    //                 updateRate = SettingsHandler::motionUpdateGlobal;
    //             }
    //             if(period != SettingsHandler::motionPeriodGlobal) {
    //                 period = SettingsHandler::motionPeriodGlobal;
    //             }
    //             updatePhaseIncrement();
    //         }
    //     }
    //     if(phase != SettingsHandler::motionPhaseGlobal)
    //         setPhase(SettingsHandler::motionPhaseGlobal);
    // }

    void updatePhaseIncrement() {
        LogHandler::debug(TagHandler::MotionHandler, "%s Update phase increment: period '%d' updateRate '%d'" , m_channel, getPeriod(), updateRate);
        phaseIncrement = 2.0*M_PI / ((float)getPeriod() / updateRate);
        LogHandler::debug(TagHandler::MotionHandler, "%s New phase increment: %f" , m_channel, phaseIncrement);
    }

    int degreesToRadian(int degrees) {
        return ((degrees)*M_PI)/180;
    }

void mapTCodeToDegrees(int tcode, int &degreeVariable) {
    // if(tcodeVersion == TCodeVersion::v0_2) {
    //     if(tcode > 999) {//Lazy Hack
    //         tcode = map(tcode, 0, 9999, 0, 999);
    //     }
    //     degreeVariable = map(tcode, 0, 999, -90, 90);
    // } else {
        degreeVariable = map(tcode, 0, 9999, -90, 90);
    // }
}
    // float w_sweep(float t, float f0, float t0, float f1, float t1) {
    //     float const freq = Lerp(f0, f1, (t - t0)/(t1 - t0));
    //     return std::sin(PI * (f0 + freq) * (t-t0));
    // }

    // /// <summary>
    // /// Linearly interpolates a value between two floats
    // /// </summary>
    // /// <param name="start_value">Start value</param>
    // /// <param name="end_value">End value</param>
    // /// <param name="pct">Our progress or percentage. [0,1]</param>
    // /// <returns>Interpolated value between two floats</returns>
    // float Lerp(float start_value, float end_value, float pct) {
    //     return (start_value + (end_value - start_value) * pct);
    // }

    // float EaseIn(float t) {
    //     return t * t;
    // }
    // float EaseOut(float t) {
    //     return Flip(Square(Flip(t)));
    // }
    // // float EaseInOut(float t)
    // // {
    // //     return Lerp(EaseIn(t), EaseOut(t), t);
    // // }
    // float Flip(float x) {
    //     return 1 - x;
    // }
    // float Square(float x) {
    //     return pow(x, 2);
    // }
};

