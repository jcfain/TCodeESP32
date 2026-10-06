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

#include "settingsFactory.h"
#include "logging/LogHandler.h"
#include "logging/TagHandler.h"
#include "tasks/TaskHandler.h"
#include "enum.h"

using TEMPERATURE_STATE_FUNCTION_PTR_T = void (*)(TemperatureType type, const char* status, float tempC);

class TemperatureHandler : public TaskHandler::Task
{
public:
	TemperatureHandler() : Task(TaskHandler::Rates::SLOW) {}

	// TaskHandler entry point. Configuration is done via setup(...) overload below.
	void setup() override
	{
	}

	// Backward-compatible configuration API used by legacy call sites.
	void setup(bool internalTempEnabled,
		bool sleeveTempEnabled,
		int8_t sleeveTempPin,
		int8_t internalTempPin,
		int8_t heaterPin,
		int8_t heaterChannel,
		int8_t caseFanPin,
		int8_t caseFanChannel,
		int heaterFrequency,
		int heaterResolution,
		bool fanControlEnabled,
		int fanFrequency,
		int fanResolution,
		int maxFanPWM)
	{
		(void)internalTempEnabled;
		(void)sleeveTempEnabled;
		(void)sleeveTempPin;
		(void)internalTempPin;
		(void)heaterPin;
		(void)heaterChannel;
		(void)caseFanPin;
		(void)caseFanChannel;
		(void)heaterFrequency;
		(void)heaterResolution;
		(void)fanControlEnabled;
		(void)fanFrequency;
		(void)fanResolution;
		(void)maxFanPWM;
		m_running = true;
	}

	void loop() override
	{
		if (!m_running)
		{
			return;
		}
		// Placeholder until full temperature control logic is restored.
		this->sleep(5000);
	}

	void setMessageCallback(TEMPERATURE_STATE_FUNCTION_PTR_T callback)
	{
		m_messageCallback = callback;
	}

	bool isMaxTempTriggered() const
	{
		return false;
	}

	float getInternalTemp() const
	{
		return m_internalTemp;
	}

	float getSleeveTemp() const
	{
		return m_sleeveTemp;
	}

	const char* getInternalState() const
	{
		return "Unknown";
	}

	const char* getSleeveState() const
	{
		return "Unknown";
	}

	void stopRunning()
	{
		m_running = false;
	}

private:
	TEMPERATURE_STATE_FUNCTION_PTR_T m_messageCallback = nullptr;
	bool m_running = false;
	float m_internalTemp = 0.0f;
	float m_sleeveTemp = 0.0f;
};