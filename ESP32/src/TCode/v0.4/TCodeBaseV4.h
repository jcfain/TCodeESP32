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
#include "TCodeBase.h"

#include <TCode.h>
using namespace TCode;
using namespace TCode::Axis;
using namespace TCode::Datatypes;

class TCodeBaseV4 : public TCodeBase {
public:
	virtual void RegisterAxis(TCodeAxis* axis) = 0;
    virtual void setAxisData(TCodeAxis* channel, const float value, const AxisExtentionType extentionType, const unsigned long commandExtention, AxisRampData rampIn, AxisRampData rampOut) = 0;
    virtual void setAxisData(TCodeAxis* channel, const float value, const AxisExtentionType extentionType, const unsigned long commandExtention) = 0;
    virtual void setAxisData(TCodeAxis* channel, const AxisData &data) = 0;
	virtual uint16_t getChannelPosition(TCodeAxis* channel) = 0;
	virtual unsigned long getAxisLastCommandTime(TCodeAxis* channel) = 0;
	virtual void updateInterfaces() = 0;
	virtual void getDeviceSettings(char* settings) = 0;
protected:
	AxisRampData m_DefaultRampData = {
		0.0,
		0.0,
		false,
		false,
		false,
	};
};