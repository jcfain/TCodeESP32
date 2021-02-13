/* MIT License

Copyright (c) 2020 Jason C. Fain

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

#include <OneWire.h>
#include <DallasTemperature.h>
#include <mutex>

#pragma once

class TemperatureHandler
{
	private: 
	static OneWire oneWire;
	static DallasTemperature sensors;
	static float _currentTemp;
	static bool _isRunning;
	static std::mutex mtx; 

	public: 
	static void setup()
	{
		oneWire.begin(SettingsHandler::Temp_PIN);
		sensors.setOneWire(&oneWire);
	}

	static void startLoop(void * parameter)
	{
		_isRunning = true;
		//Serial.print("Temp Core: ");
		//Serial.println(xPortGetCoreID());
		while(_isRunning)
		{
			mtx.lock();
			sensors.requestTemperatures();
			_currentTemp = sensors.getTempCByIndex(0);
			mtx.unlock();
        	vTaskDelay(1000/portTICK_PERIOD_MS);
		}
	}

	static float getTemp()
	{  	mtx.lock();
		float temp = _currentTemp;
		mtx.unlock();
		return temp;
	}

	static void stop()
	{
		mtx.lock();
		_isRunning = false;
		mtx.unlock();
	}
};
float TemperatureHandler::_currentTemp = 0.0f;
bool TemperatureHandler::_isRunning = false;
std::mutex TemperatureHandler::mtx; 
OneWire TemperatureHandler::oneWire;
DallasTemperature TemperatureHandler::sensors;