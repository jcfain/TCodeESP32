#pragma once

#include <Arduino.h>
#include "SettingsHandler.h"
#include "LogHandler.h"
#include "TagHandler.h"

class MotionGenerator {
public:
    void setup(TCodeVersion version, const char channel[3])
    {
        strcpy(m_channel, channel);
        tcodeVersion = version;
        setEnabled(SettingsHandler::getMotionEnabled());
        updateProfile();
    }

    void updateProfile() {
        setAmplitude(SettingsHandler::getMotionAmplitudeGlobal());
        setOffset(SettingsHandler::getMotionOffsetGlobal());
        setPeriod(SettingsHandler::getMotionPeriodGlobal());
        setUpdate(SettingsHandler::getMotionUpdateGlobal());
        setPhase(SettingsHandler::getMotionPhaseGlobal());
        setReverse(SettingsHandler::getMotionReversedGlobal());
        setAmplitudeRandom(SettingsHandler::getMotionAmplitudeGlobalRandom());
        setAmplitudeRandomMin(SettingsHandler::getMotionAmplitudeGlobalRandomMin());
        setAmplitudeRandomMax(SettingsHandler::getMotionAmplitudeGlobalRandomMax());
        setPeriodRandom(SettingsHandler::getMotionPeriodGlobalRandom());
        setPeriodRandomMin(SettingsHandler::getMotionPeriodGlobalRandomMin());
        setPeriodRandomMax(SettingsHandler::getMotionPeriodGlobalRandomMax());
        setOffsetRandom(SettingsHandler::getMotionOffsetGlobalRandom());
        setOffsetRandomMin(SettingsHandler::getMotionOffsetGlobalRandomMin());
        setOffsetRandomMax(SettingsHandler::getMotionOffsetGlobalRandomMax());
    }

    void getMovement(char buf[25]) {
        calculateNext(buf);
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
        LogHandler::verbose(TagHandler::MotionHandler, "%s setEnabled: %ld", m_channel, enable);
    }
    void setRange(uint16_t min, uint16_t max) {
        m_min = min;
        m_max = max;
    }

    void setUpdate(int value) {
        updateRate = value;
        updatePhaseIncrement();
        LogHandler::verbose(TagHandler::MotionHandler, "%s setUpdate: %ld", m_channel, updateRate);
    };

    // In miliseconds this is the duty cycle (lower is faster default 2000)
    void setPeriod(int value) {
        period = value; 
        updatePhaseIncrement();
        LogHandler::verbose(TagHandler::MotionHandler, "%s setPeriod: %ld", m_channel, value);
    };

    void setPeriodRandom(bool value) {
        periodRandomMode = value;
        LogHandler::verbose(TagHandler::MotionHandler, "%s setPeriodRandom: enabled %ld", m_channel, value);
        if(value) {
            updatePeriodRandom();
        } else {
            updatePhaseIncrement();
        }
    }
    void setPeriodRandomMin(int min) {
        periodRandomMin = min;
        LogHandler::verbose(TagHandler::MotionHandler, "%s setPeriodRandomMin: %ld", m_channel, min);
        if(periodRandomMode) {
            updatePeriodRandom();
        }
    }
    void setPeriodRandomMax(int max) {
        periodRandomMax = max;
        LogHandler::verbose(TagHandler::MotionHandler, "%s setPeriodRandomMax: %ld", m_channel, max);
        if(periodRandomMode) {
            updatePeriodRandom();
        }
    }

    // Offset from center 0
    void setOffset(int value) {
        mapTCodeToDegrees(value, offset);
        LogHandler::verbose(TagHandler::MotionHandler, "%s setOffset: %ld", m_channel, value);
    };

    void setOffsetRandom(bool value) {
        offsetRandomMode = value;
        LogHandler::verbose(TagHandler::MotionHandler, "%s setOffsetRandom: enabled %ld", m_channel, value);
        if(value) {
            updateOffsetRandom();
        }
    }
    void setOffsetRandomMin(int min) {
        mapTCodeToDegrees(min, offsetRandomMin);
        LogHandler::verbose(TagHandler::MotionHandler, "%s setOffsetRandomMin: %ld", m_channel, min);
        if(offsetRandomMode) {
            updateOffsetRandom();
        }
    }
    void setOffsetRandomMax(int max) {
        mapTCodeToDegrees(max, offsetRandomMax);
        LogHandler::verbose(TagHandler::MotionHandler, "%s setOffsetRandomMax: %ld", m_channel, max);
        if(offsetRandomMode) {
            updateOffsetRandom();
        }
    }

    // The amplitude of the motion
    void setAmplitude(int value) {
        amplitude = map(value, 0, 100, 0, 90);
        LogHandler::verbose(TagHandler::MotionHandler, "%s setAmplitude: %ld", m_channel, value);
    };

    void setAmplitudeRandom(bool value) {
        amplitudeRandomMode = value;
        LogHandler::verbose(TagHandler::MotionHandler, "%s setAmplitudeRandom: enabled %ld", m_channel, value);
        if(value) {
            updateAmplitudeRandom();
        }
    }
    void setAmplitudeRandomMin(int min) {
        amplitudeRandomMin = map(min, 0, 100, 0, 90);
        LogHandler::verbose(TagHandler::MotionHandler, "%s setAmplitudeRandomMin: %ld", m_channel, min);
        if(amplitudeRandomMode) {
            updateAmplitudeRandom();
        }
    }
    void setAmplitudeRandomMax(int max) {
        amplitudeRandomMax = map(max, 0, 100, 0, 90);
        LogHandler::verbose(TagHandler::MotionHandler, "%s setAmplitudeRandomMax: %ld", m_channel, max);
        if(amplitudeRandomMode) {
            updateAmplitudeRandom();
        }
    }

    void setMotionRandomChangeMin(int min) {
        motionRandomChangeMin = min;
        LogHandler::verbose(TagHandler::MotionHandler, "%s setMotionRandomChangeMin: %ld", m_channel, min);
        if(amplitudeRandomMode || periodRandomMode || offsetRandomMode) {
            lastExecutionPeriodChange = 0;
            checkUpdateRandomExecutionPeriod();
        }
    }
    void setMotionRandomChangeMax(int max) {
        motionRandomChangeMax = max;
        LogHandler::debug(TagHandler::MotionHandler, "%s setMotionRandomChangeMax: %ld", m_channel, max);
        if(amplitudeRandomMode) {
            lastExecutionPeriodChange = 0;
            checkUpdateRandomExecutionPeriod();
        }
    }

    // Initial phase in degrees. The phase should ideally be between (offset-amplitude/2) and (offset+amplitude/2)
    void setPhase(int value) {
        phase = degreesToRadian(value);
        LogHandler::verbose(TagHandler::MotionHandler, "%s setPhase: %ld", m_channel, value);
    };

    // reverse cycle direction 
    void setReverse(bool value) {
        reversed = value;
        LogHandler::verbose(TagHandler::MotionHandler, "%s setReverse: %ld", m_channel, value);
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
    bool reversed = false;
    
    int motionRandomChangeMin = 3000;
    int motionRandomChangeMax = 10000;

//Internally updated
    long lastExecutionPeriodChangePeriod = random(motionRandomChangeMin, motionRandomChangeMax);
    int randomPeriodExecutionPeriod = random(motionRandomChangeMin, motionRandomChangeMax);
    int randomAmplitudeExecutionPeriod = random(motionRandomChangeMin, motionRandomChangeMax);
    int randomOffsetExecutionPeriod = random(motionRandomChangeMin, motionRandomChangeMax);
    long lastExecutionPeriodChange = millis();
    long lastRandomPeriodExecutionChange = millis();
    long lastRandomAmplitudeExecutionChange = millis();
    long lastRandomOffsetExecutionChange = millis();
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
        LogHandler::debug(TagHandler::MotionHandler, "%s New random offset %ld", m_channel, offsetRandom);
    }
    void updateAmplitudeRandom() {
        amplitudeRandom = random(amplitudeRandomMin, amplitudeRandomMax);
        LogHandler::debug(TagHandler::MotionHandler, "%s New random amplitude %ld", m_channel, amplitudeRandom);
    }
    void updatePeriodRandom() {
        periodRandom = random(periodRandomMin, periodRandomMax);
        LogHandler::debug(TagHandler::MotionHandler, "%s New random period %ld", m_channel, periodRandom);
        updatePhaseIncrement();
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

    void calculateNext(char buf[25]) {
        if(!canUpdate()) {
            buf[0] = {0};
            return;
        }

        checkUpdateRandomExecutionPeriod();
        if(periodRandomMode)
            getPeriod(); // Just call this to check for random updates.

        if (!stopped) {
            int pos = constrain((int)round(getAmplitude() * sin(currentPhase + getPhase()) + getOffset()), -90, 90);
            if (getReverse())
                pos = -pos;
                
            if(tcodeVersion == TCodeVersion::v0_2) {
                sprintf(buf, "%s%03dI%u", m_channel, constrain((uint16_t)map(pos, -90, 90, 0, 999), m_min, m_max), interval);
            } else {
                sprintf(buf, "%s%04dI%u", m_channel, constrain((uint16_t)map(pos, -90, 90, 0, 9999), m_min, m_max), interval);
            }

            LogHandler::verbose(TagHandler::MotionHandler, "%s pos: %ld" , m_channel, pos);
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

            //LogHandler::verbose(TagHandler::MotionHandler, "%s enabled interval %ld lastUpdate  %ld now  %ld" , m_channel, interval, lastUpdate, now);
            interval = now - lastUpdate;
            if(interval > updateRate) {
                lastUpdate = now;
                return true;
            }
        }
        //LogHandler::verbose(TagHandler::MotionHandler, "%s cant update %ld" , m_channel, enabled);
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
        LogHandler::debug(TagHandler::MotionHandler, "%s Update phase increment: period '%ld' updateRate '%ld'" , m_channel, getPeriod(), updateRate);
        phaseIncrement = 2.0*M_PI / ((float)getPeriod() / updateRate);
        LogHandler::debug(TagHandler::MotionHandler, "%s New phase increment: %f" , m_channel, phaseIncrement);
    }

    int degreesToRadian(int degrees) {
        return ((degrees)*M_PI)/180;
    }

void mapTCodeToDegrees(int tcode, int &degreeVariable) {
    if(tcodeVersion == TCodeVersion::v0_2) {
        if(tcode > 999) {//Lazy Hack
            tcode = map(tcode, 0, 9999, 0, 999);
        }
        degreeVariable = map(tcode, 0, 999, -90, 90);
    } else {
        degreeVariable = map(tcode, 0, 9999, -90, 90);
    }
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

