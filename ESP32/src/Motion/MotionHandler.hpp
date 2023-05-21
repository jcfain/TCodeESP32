#pragma once

#include <Arduino.h>
#include "SettingsHandler.h"
#include "LogHandler.h"
#include "TagHandler.h"
#include "MotionGenerator.hpp"

class MotionHandler {
public:
    void setup(TCodeVersion version)
    {
        motionChannels.clear();
        for (auto x : SettingsHandler::motionChannels) {
            MotionGenerator motionChannel;
            motionChannel.setup(version, x);
            motionChannels.push_back(motionChannel);
        }
    }

    void getMovement(char buf[255]) {
        buf[0] = {0};
        LogHandler::verbose(TagHandler::MotionHandler, "getMovement Enter %s" , buf);
        if(!enabled || !motionChannels.size()) {
            return;
        }
        for (int i = 0; i < motionChannels.size(); i++) {
            char temp[25];
            motionChannels[i].getMovement(temp);
            if(strlen(temp) == 0)
                continue;
            strcat(buf, temp);
            strcat(buf, " ");
        }
        buf[strlen(buf) - 1] = '\n';
        buf[strlen(buf) - 1] = '\0';
        LogHandler::verbose(TagHandler::MotionHandler, "Exit %s" , buf);
    }

    // int getPeriod() {
    //     if(periodRandomMode) {
    //         if(millis() > lastRandomPeriodExecutionChange + randomPeriodExecutionPeriod) {
    //             lastRandomPeriodExecutionChange = millis();
    //             updatePeriodRandom();
    //         }
    //         return periodRandom;
    //     }
    //     return period;
    // };

    // // Offset from center 0
    // int getOffset() {
    //     if(offsetRandomMode) {
    //         if(millis() > lastRandomOffsetExecutionChange + randomOffsetExecutionPeriod) {
    //             lastRandomOffsetExecutionChange = millis();
    //             updateOffsetRandom();
    //         }
    //         return offsetRandom;
    //     }
    //     return offset;
    // };

    // // The amplitude of the motion
    // int getAmplitude() {
    //     if(amplitudeRandomMode) {
    //         if(millis() > lastRandomAmplitudeExecutionChange + randomAmplitudeExecutionPeriod) {
    //             lastRandomAmplitudeExecutionChange = millis();
    //             updateAmplitudeRandom();
    //         }
    //         return amplitudeRandom;
    //     }
    //     return amplitude;
    // };

    // // Initial phase in degrees. The phase should ideally be between (offset-amplitude/2) and (offset+amplitude/2)
    // int getPhase() {
    //     return phase;
    // };

    // // reverse cycle direction 
    // bool getReverse() {
    //     return reversed;
    // };

    // float getStopAtCycle() {
    //     return stopAt;
    // };

    void setEnabled(bool enable) {
        enabled = enable;
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].setEnabled(enable);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setEnabled: %ld", enable);
    }

    void setUpdate(int value) {
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].setUpdate(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setUpdate: %ld", value);
    };

    // In miliseconds this is the duty cycle (lower is faster default 2000)
    void setPeriod(int value) {
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].setPeriod(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setPeriod: %ld", value);
    };

    void setPeriodRandom(bool value) {
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].setPeriodRandom(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setPeriodRandom: enabled %ld", value);
    }
    void setPeriodRandomMin(int min) {
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].setPeriodRandomMin(min);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setPeriodRandomMin: %ld", min);
    }
    void setPeriodRandomMax(int max) {
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].setPeriodRandomMax(max);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setPeriodRandomMax: %ld", max);
    }

    // Offset from center 0
    void setOffset(int value) {
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].setOffset(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setOffset: %ld", value);
    };

    void setOffsetRandom(bool value) {
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].setOffsetRandom(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setOffsetRandom: enabled %ld", value);
    }
    void setOffsetRandomMin(int min) {
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].setOffsetRandomMin(min);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setOffsetRandomMin: %ld", min);
    }
    void setOffsetRandomMax(int max) {
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].setOffsetRandomMax(max);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setOffsetRandomMax: %ld", max);
    }

    // The amplitude of the motion
    void setAmplitude(int value) {
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].setAmplitude(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setAmplitude: %ld", value);
    };

    void setAmplitudeRandom(bool value) {
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].setAmplitudeRandom(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setAmplitudeRandom: enabled %ld", value);
    }
    void setAmplitudeRandomMin(int min) {
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].setAmplitudeRandomMin(min);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setAmplitudeRandomMin: %ld", min);
    }
    void setAmplitudeRandomMax(int max) {
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].setAmplitudeRandomMax(max);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setAmplitudeRandomMax: %ld", max);
    }

    void setMotionRandomChangeMin(int min) {
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].setMotionRandomChangeMin(min);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setMotionRandomChangeMin: %ld", min);
    }
    void setMotionRandomChangeMax(int max) {
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].setMotionRandomChangeMax(max);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setMotionRandomChangeMax: %ld", max);
    }

    // Initial phase in degrees. The phase should ideally be between (offset-amplitude/2) and (offset+amplitude/2)
    void setPhase(int value) {
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].setPhase(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setPhase: %ld", value);
    };

    // reverse cycle direction 
    void setReverse(bool value) {
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].setReverse(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setReverse: %ld", value);
    };

    void stopAtCycle(float value) {
        for (int i = 0; i < motionChannels.size(); i++) {
            motionChannels[i].stopAtCycle(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "stopAtCycle: %f", value);
    };

private:
    std::vector<MotionGenerator> motionChannels;
    bool enabled = false;
};

