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
        if(!enabled || !m_motionGenerators.size()) {
            xSemaphoreGive(xMutex);
            return;
        }
        for (int i = 0; i < m_motionGenerators.size(); i++) {
            char temp[25];
            m_motionGenerators[i].getMovement(temp);
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
    
    void setMotionChannels(const std::vector<MotionChannel> &motionChannels) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        bool enabledBak = enabled;
        enabled = false;
        m_motionGenerators.clear();
        for (size_t i = 0; i < motionChannels.size(); i++)
        {
            LogHandler::debug(TagHandler::MotionHandler, "Setup motion channel: %s", motionChannels[i].name);
            MotionGenerator motionGenerator;
            motionGenerator.setup(m_tcodeVersion, motionChannels[i]);
            m_motionGenerators.push_back(motionGenerator);
        }
        enabled = enabledBak;
        xSemaphoreGive(xMutex);
    }

    void setEnabled(bool enable, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        enabled = enable;
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
            m_motionGenerators[index].setEnabled(enable);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setEnabled(enable);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setEnabled: %ld", enable);
        xSemaphoreGive(xMutex);
    }

    void setUpdate(int value, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setUpdate(value);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setUpdate(value);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setUpdate: %ld", value);
        xSemaphoreGive(xMutex);
    };

    // In miliseconds this is the duty cycle (lower is faster default 2000)
    void setPeriod(int value, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setPeriod(value);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setPeriod(value);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setPeriod: %ld", value);
        xSemaphoreGive(xMutex);
    };

    void setPeriodRandom(bool value, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setPeriodRandom(value);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setPeriodRandom(value);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setPeriodRandom: enabled %ld", value);
        xSemaphoreGive(xMutex);
    }
    void setPeriodRandomMin(int min, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setPeriodRandomMin(min);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setPeriodRandomMin(min);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setPeriodRandomMin: %ld", min);
        xSemaphoreGive(xMutex);
    }
    void setPeriodRandomMax(int max, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setPeriodRandomMax(max);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setPeriodRandomMax(max);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setPeriodRandomMax: %ld", max);
        xSemaphoreGive(xMutex);
    }

    // Offset from center 0
    void setOffset(int value, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setOffset(value);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setOffset(value);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setOffset: %ld", value);
        xSemaphoreGive(xMutex);
    };

    void setOffsetRandom(bool value, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setOffsetRandom(value);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setOffsetRandom(value);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setOffsetRandom: enabled %ld", value);
        xSemaphoreGive(xMutex);
    }
    void setOffsetRandomMin(int min, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setOffsetRandomMin(min);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setOffsetRandomMin(min);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setOffsetRandomMin: %ld", min);
        xSemaphoreGive(xMutex);
    }
    void setOffsetRandomMax(int max, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setOffsetRandomMax(max);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setOffsetRandomMax(max);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setOffsetRandomMax: %ld", max);
        xSemaphoreGive(xMutex);
    }

    // The amplitude of the motion
    void setAmplitude(int value, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setAmplitude(value);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setAmplitude(value);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setAmplitude: %ld", value);
        xSemaphoreGive(xMutex);
    };

    void setAmplitudeRandom(bool value, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setAmplitudeRandom(value);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setAmplitudeRandom(value);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setAmplitudeRandom: enabled %ld", value);
        xSemaphoreGive(xMutex);
    }
    void setAmplitudeRandomMin(int min, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setAmplitudeRandomMin(min);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setAmplitudeRandomMin(min);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setAmplitudeRandomMin: %ld", min);
        xSemaphoreGive(xMutex);
    }
    void setAmplitudeRandomMax(int max, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setAmplitudeRandomMax(max);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setAmplitudeRandomMax(max);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setAmplitudeRandomMax: %ld", max);
        xSemaphoreGive(xMutex);
    }

    void setMotionRandomChangeMin(int min, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setMotionRandomChangeMin(min);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setMotionRandomChangeMin(min);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setMotionRandomChangeMin: %ld", min);
        xSemaphoreGive(xMutex);
    }

    void setMotionRandomChangeMax(int max, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setMotionRandomChangeMax(max);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setMotionRandomChangeMax(max);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setMotionRandomChangeMax: %ld", max);
        xSemaphoreGive(xMutex);
    }

    // Initial phase in degrees. The phase should ideally be between (offset-amplitude/2) and (offset+amplitude/2)
    void setPhase(int value, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setPhase(value);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setPhase(value);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setPhase: %ld", value);
        xSemaphoreGive(xMutex);
    };

    void setPhaseRandom(bool value, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setPhaseRandom(value);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setPhaseRandom(value);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setOffsetRandom: enabled %ld", value);
        xSemaphoreGive(xMutex);
    }
    void setPhaseRandomMin(int min, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setPhaseRandomMin(min);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setPhaseRandomMin(min);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setOffsetRandomMin: %ld", min);
        xSemaphoreGive(xMutex);
    }
    void setPhaseRandomMax(int max, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setPhaseRandomMax(max);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setPhaseRandomMax(max);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setOffsetRandomMax: %ld", max);
        xSemaphoreGive(xMutex);
    }

    // reverse cycle direction 
    void setReverse(bool value, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setReverse(value);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].setReverse(value);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "setReverse: %ld", value);
        xSemaphoreGive(xMutex);
    };

    void stopAtCycle(float value, const char* name = 0) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].stopAtCycle(value);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].stopAtCycle(value);
            }
        }
        LogHandler::debug(TagHandler::MotionHandler, "stopAtCycle: %f", value);
        xSemaphoreGive(xMutex);
    };

    void updateChannelRanges(const char* name = 0, int min = -1, int max = -1) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
        if(min > -1 && max > -1 && name) {
            auto index = getMotionGeneratorIndex(name);
            if(index > -1)
                m_motionGenerators[index].setRange(min, max);
        } else {
            for (int i = 0; i < m_motionGenerators.size(); i++) {
                m_motionGenerators[i].updateRange();
            }
        }
        xSemaphoreGive(xMutex);
    }

private:
    std::vector<MotionGenerator> m_motionGenerators;
    TCodeVersion m_tcodeVersion;
    bool enabled = false;
	SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();

    int getMotionGeneratorIndex(const char* name) {
        for (size_t i = 0; i < m_motionGenerators.size(); i++)
        {
            char buf[3];
            m_motionGenerators[i].getName(buf);
            if(strcmp(buf, name) != 0) {
                return i;
            }
        }
        return -1;
    }
};

