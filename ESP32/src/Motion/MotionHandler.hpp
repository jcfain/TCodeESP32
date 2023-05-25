#pragma once

#include <Arduino.h>
#include <mutex>
#include "SettingsHandler.h"
#include "LogHandler.h"
#include "TagHandler.h"
#include "MotionGenerator.hpp"

class MotionHandler {
public:
    void setup(TCodeVersion version)
    {
        m_tcodeVersion = version;
        setMotionChannels(SettingsHandler::getMotionChannels());
    }

    void getMovement(char buf[255]) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        buf[0] = {0};
        LogHandler::verbose(TagHandler::MotionHandler, "getMovement Enter %s" , buf);
        if(!enabled || !m_motionChannels.size()) {
            return;
        }
        for (int i = 0; i < m_motionChannels.size(); i++) {
            char temp[25];
            m_motionChannels[i].getMovement(temp);
            if(strlen(temp) == 0)
                continue;
            strcat(buf, temp);
            strcat(buf, " ");
        }
        buf[strlen(buf) - 1] = '\n';
        buf[strlen(buf) - 1] = '\0';
        LogHandler::verbose(TagHandler::MotionHandler, "Exit %s" , buf);
        xSemaphoreGive(xMutex);
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

    void setMotionChannels(const std::vector<MotionChannel> &motionChannels) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        bool enabledBak = enabled;
        enabled = false;
        m_motionChannels.clear();
        for (size_t i = 0; i < motionChannels.size(); i++)
        {
            LogHandler::debug(TagHandler::MotionHandler, "Setup motion channel: %s", motionChannels[i].name);
            MotionGenerator motionChannel;
            motionChannel.setup(m_tcodeVersion, motionChannels[i].name);
            motionChannel.setPhase(motionChannels[i].phase);
            motionChannel.setRange(SettingsHandler::getChannelMin(motionChannels[i].name), SettingsHandler::getChannelMax(motionChannels[i].name));
            motionChannel.setReverse(motionChannels[i].reverse);
            m_motionChannels.push_back(motionChannel);
        }
        enabled = enabledBak;
        xSemaphoreGive(xMutex);
    }

    void setEnabled(bool enable) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        enabled = enable;
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].setEnabled(enable);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setEnabled: %ld", enable);
        xSemaphoreGive(xMutex);
    }

    void setUpdate(int value) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].setUpdate(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setUpdate: %ld", value);
        xSemaphoreGive(xMutex);
    };

    // In miliseconds this is the duty cycle (lower is faster default 2000)
    void setPeriod(int value) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].setPeriod(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setPeriod: %ld", value);
        xSemaphoreGive(xMutex);
    };

    void setPeriodRandom(bool value) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].setPeriodRandom(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setPeriodRandom: enabled %ld", value);
        xSemaphoreGive(xMutex);
    }
    void setPeriodRandomMin(int min) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].setPeriodRandomMin(min);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setPeriodRandomMin: %ld", min);
        xSemaphoreGive(xMutex);
    }
    void setPeriodRandomMax(int max) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].setPeriodRandomMax(max);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setPeriodRandomMax: %ld", max);
        xSemaphoreGive(xMutex);
    }

    // Offset from center 0
    void setOffset(int value) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].setOffset(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setOffset: %ld", value);
        xSemaphoreGive(xMutex);
    };

    void setOffsetRandom(bool value) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].setOffsetRandom(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setOffsetRandom: enabled %ld", value);
        xSemaphoreGive(xMutex);
    }
    void setOffsetRandomMin(int min) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].setOffsetRandomMin(min);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setOffsetRandomMin: %ld", min);
        xSemaphoreGive(xMutex);
    }
    void setOffsetRandomMax(int max) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].setOffsetRandomMax(max);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setOffsetRandomMax: %ld", max);
        xSemaphoreGive(xMutex);
    }

    // The amplitude of the motion
    void setAmplitude(int value) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].setAmplitude(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setAmplitude: %ld", value);
        xSemaphoreGive(xMutex);
    };

    void setAmplitudeRandom(bool value) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].setAmplitudeRandom(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setAmplitudeRandom: enabled %ld", value);
        xSemaphoreGive(xMutex);
    }
    void setAmplitudeRandomMin(int min) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].setAmplitudeRandomMin(min);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setAmplitudeRandomMin: %ld", min);
        xSemaphoreGive(xMutex);
    }
    void setAmplitudeRandomMax(int max) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].setAmplitudeRandomMax(max);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setAmplitudeRandomMax: %ld", max);
        xSemaphoreGive(xMutex);
    }

    void setMotionRandomChangeMin(int min) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].setMotionRandomChangeMin(min);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setMotionRandomChangeMin: %ld", min);
        xSemaphoreGive(xMutex);
    }
    void setMotionRandomChangeMax(int max) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].setMotionRandomChangeMax(max);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setMotionRandomChangeMax: %ld", max);
        xSemaphoreGive(xMutex);
    }

    // Initial phase in degrees. The phase should ideally be between (offset-amplitude/2) and (offset+amplitude/2)
    void setPhase(int value) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].setPhase(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setPhase: %ld", value);
        xSemaphoreGive(xMutex);
    };

    // reverse cycle direction 
    void setReverse(bool value) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].setReverse(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "setReverse: %ld", value);
        xSemaphoreGive(xMutex);
    };

    void stopAtCycle(float value) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        for (int i = 0; i < m_motionChannels.size(); i++) {
            m_motionChannels[i].stopAtCycle(value);
        }
        LogHandler::debug(TagHandler::MotionHandler, "stopAtCycle: %f", value);
        xSemaphoreGive(xMutex);
    };

private:
    std::vector<MotionGenerator> m_motionChannels;
    TCodeVersion m_tcodeVersion;
    bool enabled = false;
	SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();
};

