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

#include <ArduinoJson.h>
#include "enum.h"
#include "struct/channel.h"
#include "constants.h"
#define BLDC_V3_CHANNEL_COUNT 10
#define SERVO_V3_CHANNEL_COUNT 14
class ChannelMap {
public:
    const Channel Stroke = {"L0", "Stroke", false, false, TCODE_MIN, TCODE_MID, TCODE_MAX, TCODE_MIN, TCODE_MID, TCODE_MAX};
    const Channel Surge = {"L1", "Surge", false, true, TCODE_MIN, TCODE_MID, TCODE_MAX, TCODE_MIN, TCODE_MID, TCODE_MAX};
    const Channel Sway = {"L2", "Sway", false, true, TCODE_MIN, TCODE_MID, TCODE_MAX, TCODE_MIN, TCODE_MID, TCODE_MAX};
    const Channel Twist = {"R0", "Twist", false, false, TCODE_MIN, TCODE_MID, TCODE_MAX, TCODE_MIN, TCODE_MID, TCODE_MAX};
    const Channel Roll = {"R1", "Roll", false, false, TCODE_MIN, TCODE_MID, TCODE_MAX, TCODE_MIN, TCODE_MID, TCODE_MAX};
    const Channel Pitch = {"R2", "Pitch", false, false, TCODE_MIN, TCODE_MID, TCODE_MAX, TCODE_MIN, TCODE_MID, TCODE_MAX};
    const Channel Vibe1 = {"V0", "Vibe 1", true, false, TCODE_MIN, TCODE_MIN, TCODE_MAX, TCODE_MIN, TCODE_MIN, TCODE_MAX};
    const Channel Vibe2 = {"V1", "Vibe 2", true, false, TCODE_MIN, TCODE_MIN, TCODE_MAX, TCODE_MIN, TCODE_MIN, TCODE_MAX};
    const Channel Vibe3 = {"V2", "Vibe 3", true, false, TCODE_MIN, TCODE_MIN, TCODE_MAX, TCODE_MIN, TCODE_MIN, TCODE_MAX};
    const Channel Vibe4 = {"V3", "Vibe 4", true, false, TCODE_MIN, TCODE_MIN, TCODE_MAX, TCODE_MIN, TCODE_MIN, TCODE_MAX};
    const Channel SuckManual = {"A0", "Suck manual", false, false, TCODE_MIN, TCODE_MID, TCODE_MAX, TCODE_MIN, TCODE_MID, TCODE_MAX};
    const Channel SuckLevel = {"A1", "Suck level", false, false, TCODE_MIN, TCODE_MID, TCODE_MAX, TCODE_MIN, TCODE_MID, TCODE_MAX};
    const Channel Lube = {"A2", "Lube", true, false, TCODE_MIN, TCODE_MIN, TCODE_MAX, TCODE_MIN, TCODE_MIN, TCODE_MAX};
    const Channel Squeeze = {"A3", "Aux3", false, false, TCODE_MIN, TCODE_MID, TCODE_MAX, TCODE_MIN, TCODE_MID, TCODE_MAX};

    // Channel ChannelListV2[9] = {
    //     Stroke,
    //     Surge,
    //     Sway,
    //     {"L3", "Suck", false, false, TCODE_MIN, TCODE_MID, TCODE_MAX},
    //     Twist,
    //     Roll,
    //     Pitch,
    //     {"V0", "Vibe 0", true, false, TCODE_MIN, TCODE_MIN, TCODE_MAX},
    //     {"V1", "Vibe 1/Lube", true, false, TCODE_MIN, TCODE_MIN, TCODE_MAX}
    // };

    Channel ChannelListV3[SERVO_V3_CHANNEL_COUNT] = {
        Stroke,
        Surge,
        Sway,
        Twist,
        Roll,
        Pitch,
        Vibe1,
        Vibe2,
        Vibe3,
        Vibe4,
        SuckManual,
        SuckLevel,
        Lube,
        Squeeze
    };

    Channel ChannelListBLDCV3[BLDC_V3_CHANNEL_COUNT] = {
        Stroke,
        Twist,
        Vibe1,
        Vibe2,
        Vibe3,
        Vibe4,
        SuckManual,
        SuckLevel,
        Lube,
        Squeeze
    };

    void init(const TCodeVersion version, const MotorType motorType, const DeviceType deviceType) {
        m_version = version;
        m_motorType = motorType;
        m_deviceType = deviceType;
        setChannelLimits();
    }

    void serialize(JsonArray& arr) {
        arr.clear();
        if(m_motorType == MotorType::BLDC) {
            serializeBLDC(arr);
            return;
        }
        serializeServo(arr);
    }

    uint8_t count() {
        switch(m_motorType)
        {
            case MotorType::BLDC:
            return BLDC_V3_CHANNEL_COUNT;
            default:
            return SERVO_V3_CHANNEL_COUNT;
        }
    }

    Channel* get() {
        switch(m_motorType)
        {
            case MotorType::BLDC:
            return ChannelListBLDCV3;
            default:
            return ChannelListV3;
        }
    }
    
    Channel* get(uint8_t index) {
        if(index > count())
        {
            return nullptr;
        }
        Channel* currentChannels = get();
        return &currentChannels[index];
    }

    Channel* get(const char* name) {
        Channel* currentChannels = get();
        for (size_t i = 0; i < count(); i++) {
            if(strcmp(currentChannels[i].Name, name) == 0)
                return &currentChannels[i];
        }
        return nullptr;
    }

    void tCodeHome(char buf[MAX_COMMAND], uint16_t speed = 1000) {
        buf[0] = {0};
        if(m_motorType == MotorType::BLDC) {
            for(auto channel : ChannelListBLDCV3) {
                char bufTemp[MAX_COMMAND];
                formatTCodeChannel(channel, bufTemp, channel.isSwitch ? channel.min : channel.mid, speed);
                strcat(buf, bufTemp);
                strcat(buf, " ");
            }
            return;
        }
        switch (m_version) {
            // case TCodeVersion::v0_2:
            //     for(auto channel : ChannelListV2) {
            //         char bufTemp[MAX_COMMAND];
            //         formatTCodeChannel(channel, bufTemp, channel.isSwitch ? channel.min : channel.mid, speed);
            //         strcat(buf, bufTemp);
            //         strcat(buf, " ");
            //     }
            //     break;
            case TCodeVersion::v0_3:
                for(auto channel : ChannelListV3) {
                    char bufTemp[MAX_COMMAND];
                    formatTCodeChannel(channel, bufTemp, channel.isSwitch ? channel.min : channel.mid, speed);
                    strcat(buf, bufTemp);
                    strcat(buf, " ");
                }
                break;
            case TCodeVersion::v0_4:// Not supported yet
                //v5ToJson(arr);
                break;
        }
        strcat(buf, "\n");
    }
private:
    TCodeVersion m_version;
    MotorType m_motorType;
    DeviceType m_deviceType;

    // void v2ToJson(JsonArray& arr) {
    //     for (Channel channel : ChannelListV2) {
    //         auto obj = arr.createNestedObject();
    //         channel.toJson(obj);
    //     }
    // }
    // void v3ToJson(JsonArray& arr) {
    //     for (Channel channel : ChannelListV3) {
    //         auto obj = arr.createNestedObject();
    //         channel.toJson(obj);
    //     }
    // }
    // void v3BLDCToJson(JsonArray& arr) {
    //     for (Channel channel : ChannelListBLDCV3) {
    //         auto obj = arr.createNestedObject();
    //         channel.toJson(obj);
    //     }
    // }
    
    void formatTCodeChannel(const Channel& channel, char* buf, int value, int speed = -1) {
        if(value < 0) {
            value = channel.isSwitch ? channel.min : channel.mid;
        }
        char valueString[11];
        sprintf(valueString, "%04d", value);
        if(speed < 1) {
            sprintf(buf, "%s%s", channel.Name, valueString);
            return;
        }
        sprintf(buf, "%s%sS%d", channel.Name, valueString, speed);
    }

    void setChannelLimits() {
        // for (Channel &channel : ChannelListV2) {
        //     channel.min = 0;
        //     channel.mid = 500;
        //     channel.max = 999;
        // }
        for (Channel &channel : ChannelListV3) {
            channel.min = 0;
            channel.mid = 5000;
            channel.max = 9999;
        }
        for (Channel &channel : ChannelListBLDCV3) {
            channel.min = 0;
            channel.mid = 5000;
            channel.max = 9999;
        }
    }

    void serializeBLDC(JsonArray& arr) {
        switch (m_version) {
            // case TCodeVersion::v0_2:// Not supported in BLDC
            //     break;
            case TCodeVersion::v0_3:
                //v3ToJson(arr);
                toJson(ChannelListBLDCV3, arr, sizeof(ChannelListBLDCV3)/sizeof(Channel));
                break;
            case TCodeVersion::v0_4:// Not supported yet
                //v5ToJson(arr);
                break;
        }
    }
    
    void serializeServo(JsonArray& arr) {
        switch (m_version) {
            // case TCodeVersion::v0_2:
            //     toJson(ChannelListV2, arr, sizeof(ChannelListV2)/sizeof(Channel));
            //     break;
            case TCodeVersion::v0_3:
                //v3ToJson(arr);
                toJson(ChannelListV3, arr, sizeof(ChannelListV3)/sizeof(Channel));
                break;
            case TCodeVersion::v0_4:// Not supported yet
                //v5ToJson(arr);
                break;
        }
    }

    void toJson(const Channel channels[], JsonArray& arr, uint16_t len) {
        for (int i=0; i < len; i++) {
            auto channel = channels[i];
            if(m_deviceType != DeviceType::SR6 && channel.sr6Only)
                continue;
            auto obj = arr.add<JsonObject>();
            channel.toJson(obj);
        }
    }
};