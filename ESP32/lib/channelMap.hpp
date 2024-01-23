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

class ChannelMap {
public:
    const Channel Stroke = {Name: "L0", FriendlyName: "Stroke", isSwitch: false, sr6Only: false};
    const Channel Surge = {Name: "L1", FriendlyName: "Surge", isSwitch: false, sr6Only: true};
    const Channel Sway = {Name: "L2", FriendlyName: "Sway", isSwitch: false, sr6Only: true};
    const Channel Twist = {Name: "R0", FriendlyName: "Twist", isSwitch: false, sr6Only: false};
    const Channel Roll = {Name: "R1", FriendlyName: "Roll", isSwitch: false, sr6Only: false};
    const Channel Pitch = {Name: "R2", FriendlyName: "Pitch", isSwitch: false, sr6Only: false};
    const Channel Vibe1 = {Name: "V0", FriendlyName: "Vibe 1", isSwitch: true, sr6Only: false};
    const Channel Vibe2 = {Name: "V1", FriendlyName: "Vibe 2", isSwitch: true, sr6Only: false};
    const Channel Vibe3 = {Name: "V2", FriendlyName: "Vibe 3", isSwitch: true, sr6Only: false};
    const Channel Vibe4 = {Name: "V3", FriendlyName: "Vibe 4", isSwitch: true, sr6Only: false};
    const Channel SuckManual = {Name: "A0", FriendlyName: "Suck manual", isSwitch: false, sr6Only: false};
    const Channel SuckLevel = {Name: "A1", FriendlyName: "Suck level", isSwitch: false, sr6Only: false};
    const Channel Lube = {Name: "A2", FriendlyName: "Lube", isSwitch: true, sr6Only: false};
    const Channel Squeeze = {Name: "A3", FriendlyName: "Squeeze", isSwitch: false, sr6Only: false};

    const Channel ChannelListV2[9] = {
        Stroke,
        Surge,
        Sway,
        {Name: "L3", FriendlyName: "Suck", isSwitch: false, sr6Only: false},
        Twist,
        Roll,
        Pitch,
        {Name: "V0", FriendlyName: "Vibe 0", isSwitch: true, sr6Only: false},
        {Name: "V1", FriendlyName: "Vibe 1/Lube", isSwitch: true, sr6Only: false}
    };

    const Channel ChannelListV3[14] = {
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

    const Channel ChannelListBLDCV3[10] = {
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

    void init(const TCodeVersion version, const MotorType motorType) {
        m_version = version;
        m_motorType = motorType;
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
private:
    TCodeVersion m_version;
    MotorType m_motorType;

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

    void setChannelLimits() {
        for (Channel channel : ChannelListV2) {
            channel.min = 0;
            channel.mid = 500;
            channel.max = 999;
        }
        for (Channel channel : ChannelListV3) {
            channel.min = 0;
            channel.mid = 5000;
            channel.max = 9999;
        }
        for (Channel channel : ChannelListBLDCV3) {
            channel.min = 0;
            channel.mid = 5000;
            channel.max = 9999;
        }
    }

    void serializeBLDC(JsonArray& arr) {
        switch (m_version) {
            case TCodeVersion::v0_2:// Not supported in BLDC
                break;
            case TCodeVersion::v0_3:
                //v3ToJson(arr);
                toJson(ChannelListBLDCV3, arr, sizeof(ChannelListBLDCV3)/sizeof(Channel));
                break;
            case TCodeVersion::v0_5:// Not supported yet
                //v5ToJson(arr);
                break;
        }
    }
    
    void serializeServo(JsonArray& arr) {
        switch (m_version) {
            case TCodeVersion::v0_2:
                toJson(ChannelListV2, arr, sizeof(ChannelListV2)/sizeof(Channel));
                break;
            case TCodeVersion::v0_3:
                //v3ToJson(arr);
                toJson(ChannelListV3, arr, sizeof(ChannelListV3)/sizeof(Channel));
                break;
            case TCodeVersion::v0_5:// Not supported yet
                //v5ToJson(arr);
                break;
        }
    }

    void toJson(const Channel channels[], JsonArray& arr, uint16_t len) {
        for (int i=0; i < len; i++) {
            auto channel = channels[i];
            auto obj = arr.createNestedObject();
            channel.toJson(obj);
        }
    }
};