#pragma once

#include <Arduino.h>
#include "LogHandler.h"
#include "TagHandler.h"

class MotionHandler {
public:
    void setup(TCodeVersion version)
    {
        tcodeVersion = version;
        setEnabled(SettingsHandler::getMotionEnabled());
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

    void getMovement(char buf[255]) {
        calculateNext(buf);
    }

    int getPeriod() {
        if(periodRandomMode) {
            if(millis() > lastRandomPeriodExecutionChange + randomPeriodExecutionPeriod) {
                lastRandomPeriodExecutionChange = millis();
                updatePeriodRandom();
            }
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
        LogHandler::debug(TagHandler::MotionHandler, "setEnabled: %ld", enable);
    }

    void setUpdate(int value) {
        updateRate = value;
        updatePhaseIncrement();
        LogHandler::debug(TagHandler::MotionHandler, "setUpdate: %ld", updateRate);
    };

    // In miliseconds this is the duty cycle (lower is faster default 2000)
    void setPeriod(int value) {
        period = value; 
        updatePhaseIncrement();
        periodRandomMode = false;
        LogHandler::debug(TagHandler::MotionHandler, "setPeriod: %ld", value);
    };

    void setPeriodRandom(bool value) {
        periodRandomMode = value;
        LogHandler::debug(TagHandler::MotionHandler, "setPeriodRandom: enabled %ld", value);
        if(value) {
            updatePeriodRandom();
        }
    }
    void setPeriodRandomMin(int min) {
        periodRandomMin = min;
        LogHandler::debug(TagHandler::MotionHandler, "setPeriodRandomMin: %ld", min);
        if(periodRandomMode) {
            updatePeriodRandom();
        }
    }
    void setPeriodRandomMax(int max) {
        periodRandomMax = max;
        LogHandler::debug(TagHandler::MotionHandler, "setPeriodRandomMax: %ld", max);
        if(periodRandomMode) {
            updatePeriodRandom();
        }
    }

    // Offset from center 0
    void setOffset(int value) {
        offset = value;
        offsetRandomMode = false;
        LogHandler::debug(TagHandler::MotionHandler, "setOffset: %ld", value);
    };

    void setOffsetRandom(bool value) {
        offsetRandomMode = value;
        LogHandler::debug(TagHandler::MotionHandler, "setOffsetRandom: enabled %ld", value);
        if(value) {
            updateOffsetRandom();
        }
    }
    void setOffsetRandomMin(int min) {
        offsetRandomMin = min;
        LogHandler::debug(TagHandler::MotionHandler, "setOffsetRandomMin: %ld", min);
        if(offsetRandomMode) {
            updateOffsetRandom();
        }
    }
    void setOffsetRandomMax(int max) {
        offsetRandomMax = max;
        LogHandler::debug(TagHandler::MotionHandler, "setOffsetRandomMax: %ld", max);
        if(offsetRandomMode) {
            updateOffsetRandom();
        }
    }

    // The amplitude of the motion
    void setAmplitude(int value) {
        amplitude = value;
        amplitudeRandomMode = false;
        LogHandler::debug(TagHandler::MotionHandler, "setAmplitude: %ld", value);
    };

    void setAmplitudeRandom(bool value) {
        amplitudeRandomMode = value;
        LogHandler::debug(TagHandler::MotionHandler, "setAmplitudeRandom: enabled %ld", value);
        if(value) {
            updateAmplitudeRandom();
        }
    }
    void setAmplitudeRandomMin(int min) {
        amplitudeRandomMin = min;
        LogHandler::debug(TagHandler::MotionHandler, "setAmplitudeRandomMin: %ld", min);
        if(amplitudeRandomMode) {
            updateAmplitudeRandom();
        }
    }
    void setAmplitudeRandomMax(int max) {
        amplitudeRandomMax = max;
        LogHandler::debug(TagHandler::MotionHandler, "setAmplitudeRandomMax: %ld", max);
        if(amplitudeRandomMode) {
            updateAmplitudeRandom();
        }
    }

    // Initial phase in degrees. The phase should ideally be between (offset-amplitude/2) and (offset+amplitude/2)
    void setPhase(int value) {
        phase = degreesToRadian(value);
        LogHandler::debug(TagHandler::MotionHandler, "setPhase: %ld", value);
    };

    // reverse cycle direction 
    void setReverse(bool value) {
        reversed = value;
        LogHandler::debug(TagHandler::MotionHandler, "setReverse: %ld", value);
    };

    void stopAtCycle(float value) {
        stopAt = currentPhase + 2*M_PI*value;
        LogHandler::debug(TagHandler::MotionHandler, "stopAtCycle: %f, stopAt %f", value, stopAt);
    };

private:
    TCodeVersion tcodeVersion;
    bool enabled = false;
    
    int updateRate = 100;
    int period = 2000;    
    bool periodRandomMode = false;
    int periodRandomMin = 500;
    int periodRandomMax = 6000;
    int amplitude = 60;  
    bool amplitudeRandomMode = false;
    int amplitudeRandomMin = 20;
    int amplitudeRandomMax = 60;
    int offset = 0;      
    bool offsetRandomMode = false;
    int offsetRandomMin = 20;
    int offsetRandomMax = 80;
    float phase = 0;  
    bool reversed = false;

//Internally updated
    int randomPeriodExecutionPeriod = random(3000, 10000);
    int randomAmplitudeExecutionPeriod = random(3000, 10000);
    int randomOffsetExecutionPeriod = random(3000, 10000);
    long lastRandomPeriodExecutionChange = millis();
    long lastRandomAmplitudeExecutionChange = millis();
    long lastRandomOffsetExecutionChange = millis();
    long lastExecutionPeriodChangePeriod = random(3000, 30000);
    long lastExecutionPeriodChange = millis();
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
        offset = random(offsetRandomMin, offsetRandomMax);
    }
    void updateAmplitudeRandom() {
        amplitude = random(amplitudeRandomMin, amplitudeRandomMax);
    }
    void updatePeriodRandom() {
        period = random(periodRandomMin, periodRandomMax);
        updatePhaseIncrement();
    }

    void checkUpdateRandomExecutionPeriod() {
        if((offsetRandomMode || amplitudeRandomMode || periodRandomMode)  
            && millis() > lastExecutionPeriodChange + lastExecutionPeriodChangePeriod) {

            lastExecutionPeriodChange - millis();
            if(offsetRandomMode)
                randomPeriodExecutionPeriod = random(3000, 10000);
            if(amplitudeRandomMode)
                randomAmplitudeExecutionPeriod = random(3000, 10000);
            if(periodRandomMode) 
                randomOffsetExecutionPeriod = random(3000, 10000);
            lastExecutionPeriodChangePeriod = random(3000, 30000);
        }
    }

    void calculateNext(char buf[255]) {
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
                sprintf(buf, "L0%03dI%ld\n", map(pos, -90, 90, 0, 999), interval);
            } else {
                sprintf(buf, "L0%04dI%ld\n", map(pos, -90, 90, 0, 9999), interval);
            }
            LogHandler::verbose(TagHandler::MotionHandler, "pos: %ld" , pos);
            LogHandler::verbose(TagHandler::MotionHandler, "buf: %s" , buf);
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

            interval = now - lastUpdate;
            if(interval > updateRate) {
                lastUpdate = now;
                return true;
            }
        }
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
        LogHandler::debug(TagHandler::MotionHandler, "Update phase increment: period '%ld' updateRate '%ld'" , period, updateRate);
        phaseIncrement = 2.0*M_PI / ((float)period / updateRate);
        LogHandler::debug(TagHandler::MotionHandler, "New phase increment: %f" , phaseIncrement);
    }

    int degreesToRadian(int degrees) {
        return ((degrees)*M_PI)/180;
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
    // float EaseInOut(float t)
    // {
    //     return Lerp(EaseIn(t), EaseOut(t), t);
    // }
    // float Flip(float x) {
    //     return 1 - x;
    // }
    // float Square(float x) {
    //     return pow(x, 2);
    // }
};

