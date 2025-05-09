
#pragma once

#include <vector>
#include "TCodeBaseV4.h"
#include "TagHandler.h"
#include "OutputStream.h"
#include "EventHandler.h"


class TCode0_4 : public TCodeBaseV4
{

public:
	void setup(const char *firmware) override
	{
		firmwareID = firmware;

		// #ESP32# Enable EEPROM
		m_tcode.setOutputStream(&m_outputStream);
		m_tcode.registerEventObserver(&m_eventHandler);

		// m_tcode.registerInterface(&button);
	}

	// Function to name and activate axis
	void RegisterAxis(TCodeAxis* axis) override
	{
		m_tcode.registerAxis(axis);
	}

	// void RegisterAxis(const String &ID, const String& axisName) override
	// {
	// 	auto itr = std::find_if(m_axisPointers.begin(), m_axisPointers.end(), [axisName](const TCodeAxis* channel) {
	// 		return !strcmp(channel->getName(), axisName.c_str());
	// 	});
	// 	if(itr != m_axisPointers.end())
	// 		m_tcode.registerAxis(*itr.base());
	// }

	// Function to read off individual bytes as input
	void read(byte inByte) override
	{
		m_tcode.read(inByte);
	}

	// Function to read off whole strings as input
	void read(const String &inString) override
	{
		m_tcode.read(inString);
	}
	void setAxisData(TCodeAxis* channel, const AxisData &data) override {
		m_tcode.setAxisData(channel->getId(), data);
	}
	void setAxisData(TCodeAxis* channel, 
						const float value, 
						const AxisExtentionType extentionType, 
						const unsigned long commandExtension, 
						AxisRampData rampIn, 
						AxisRampData rampOut) override
	{
		AxisData data = {
			value,
			commandExtension,
			extentionType,
			rampIn,
			rampOut
		};
		setAxisData(channel, data);
	}

	void setMessageCallback(TCODE_FUNCTION_PTR_T f) override
	{
		m_eventHandler.registerOnNotify(f);
		TCodeBaseV4::setMessageCallback(f);
	}
	
    virtual void setAxisData(TCodeAxis* channel, const float value, const AxisExtentionType extentionType, const unsigned long commandExtension) {
		AxisData data = {
			value,
			commandExtension,
			extentionType,
			m_DefaultRampData,
			m_DefaultRampData
		};
		setAxisData(channel, data);
	}

// float lastXLin = 0;
	// Function to read the current position of an axis
	uint16_t  getAxisPosition(TCodeAxis* channel) override
	{
		float value = channel->getPosition();
        // if(channel->getId().channel == 0 && channel->getId().type == AxisType::Linear && lastXLin != value) {
        //     Serial.print("getAxisPosition: ");
        //     Serial.println(value);
        //     lastXLin = value;
        // }
		return value * 10000;
	}

	// Function to query when an axis was last commanded
	unsigned long getAxisLastCommandTime(TCodeAxis* channel) override
	{
		return channel->getLastCommandTime();
	}

	void updateInterfaces() {
		m_tcode.updateInterfaces();
	}

	void getDeviceSettings(char *settings) override
	{
		//m_tcode.
	}
protected:
	EventHandler m_eventHandler;
private:
	const char *_TAG = TagHandler::TCodeHandler;
	OutputStream m_outputStream;
	const char *firmwareID;
	const static int m_axisCount = 11;
	TCodeManager m_tcode;

	// std::vector<TCodeAxis*> m_axisPointers;

	// AxisId toAxisID(const char* id) {
	// 	return {toAxisType(id), toAxisChannel(id)};
	// }

	// AxisType toAxisType(const char* id) {
	// 	if(!strlen(id)) {
	// 		return AxisType::None;
	// 	}
	// 	if(id[0] == 'L') {
	// 		return AxisType::Linear;
	// 	}
	// 	if(id[0] == 'R') {
	// 		return AxisType::Rotation;
	// 	}
	// 	if(id[0] == 'V') {
	// 		return AxisType::Vibration;
	// 	}
	// 	if(id[0] == 'A') {
	// 		return AxisType::Auxiliary;
	// 	}
	// 	return AxisType::None;
	// }

	// uint8_t toAxisChannel(const char* id) {
	// 	if(!strlen(id)) {
	// 		return 10;// >9 is invalid
	// 	}
	// 	int value = atoi(id);
	// 	return static_cast<uint8_t>(value);
	// }

	// AxisExtentionType toExtensionType(const char &extension) {
	// 	if(extension == 'S') {
	// 		return AxisExtentionType::Speed;
	// 	} 
	// 	if(extension == 'I') {
	// 		return AxisExtentionType::Time;
	// 	} 
	// 	return AxisExtentionType::None;
	// }
};
