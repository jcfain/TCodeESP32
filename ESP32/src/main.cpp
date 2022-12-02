/* MIT License

Copyright (c) 2022 Jason C. Fain

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

// #define ST(A) #A
// #define STR(A) ST(A)
// #ifdef DEBUG
// #pragma message STR(DEBUG)
// #endif
// #ifdef ISAAC_NEWTONGUE_BUILD
// #pragma message STR(ISAAC_NEWTONGUE_BUILD)
// #endif
// #ifdef FULL_BUILD
// #pragma message STR(FULL_BUILD)
// #endif
// #ifdef FW_VERSION
// #pragma message STR(FW_VERSION)
// #endif

#include <Arduino.h>
#include <SPIFFS.h>
#include "LogHandler.h"
#include "SettingsHandler.h"
#if WIFI_TCODE == 1
	#include "WifiHandler.h"
#endif

#if TEMP_ENABLED == 1
	#include "TemperatureHandler.h"
#endif
#if DISPLAY_ENABLED == 1
	#include "DisplayHandler.h"
#endif
#if BLUETOOTH_TCODE == 1
	#include "BluetoothHandler.h"
#endif
#include "TCode/ServoHandler.h"

#if DEBUG_BUILD == 0 && TCODE_V2 == 1//Too much memory needed with debug
	#include "TCode/v0.2/ServoHandler0_2.h"
#endif
#include "TCode/v0.3/ServoHandler0_3.h"
//#include "TCode/v1.0/ServoHandler1_0.h"

#if WIFI_TCODE == 1
#include "UdpHandler.h"
//#include "TcpHandler.h"
#include "WebHandler.h"
#endif
//#include "OTAHandler.h"
#include "BLEHandler.h"

#if WIFI_TCODE == 1
#include "WebSocketHandler.h"
#endif

#if WIFI_TCODE == 1
Udphandler* udpHandler = 0;
#endif

#if BLUETOOTH_TCODE == 1
	BluetoothHandler* btHandler = 0;
#endif

//TcpHandler tcpHandler;
ServoHandler* servoHandler;

#if WIFI_TCODE == 1
WifiHandler wifi;
WebHandler webHandler;
WebSocketHandler* webSocketHandler = 0;
#endif

#if TEMP_ENABLED == 1
	TemperatureHandler* temperatureHandler = 0;
#endif

BLEHandler* bleHandler = 0;

#if DISPLAY_ENABLED == 1
	DisplayHandler* displayHandler;
	TaskHandle_t displayTask;
	#if ISAAC_NEWTONGUE_BUILD == 1
		TaskHandle_t animationTask;
	#endif
#endif
#if TEMP_ENABLED == 1
	TaskHandle_t temperatureTask;
#endif
// This has issues running with the webserver.
//OTAHandler otaHandler;
bool apMode = false;
bool setupSucceeded = false;
bool restarting = false;
bool deviceinUse = false;

char udpData[255];
char webSocketData[255];

void displayPrint(String text) {
	#if DISPLAY_ENABLED == 1
		displayHandler->println(text);
	#endif
}

void CommandCallback(const char* in) {
	log_d("main", "Enter TCode Command callback");
#if BLUETOOTH_TCODE == 1
	if (SettingsHandler::bluetoothEnabled && btHandler->isConnected())
		btHandler->CommandCallback(in);
#endif
#if WIFI_TCODE == 1
	if(webSocketHandler)
		webSocketHandler->CommandCallback(in);
	if(udpHandler)
		udpHandler->CommandCallback(in);
#endif
	if(Serial)
		Serial.println(in);
}

void logCallBack(const char* in, LogLevel level) {
#if WIFI_TCODE == 1
	// if(webSocketHandler) {
	// 	webSocketHandler->sendDebug(in, level);
	// }
#endif
}
#if TEMP_ENABLED
void tempChangeCallBack(TemperatureType type, const char* message, float temp) {
#if WIFI_TCODE == 1
	if(webSocketHandler) {
		if (strpbrk(message, "{") == nullptr) {
			webSocketHandler->sendCommand(message);
		} else {
			if(type == TemperatureType::SLEEVE) {
				webSocketHandler->sendCommand("sleeveTempStatus", message);
			} else {
				webSocketHandler->sendCommand("internalTempStatus", message);
			}
		}
	}
#endif
#if DISPLAY_ENABLED
	if(displayHandler) {
		if(type == TemperatureType::SLEEVE) {
			displayHandler->setSleeveTemp(temp);
		} else {
			displayHandler->setInternalTemp(temp);
		}
	}
#endif
}
void tempStateChangeCallBack(TemperatureType type, const char* state) {
#if DISPLAY_ENABLED
	if(displayHandler) {
		if(type == TemperatureType::SLEEVE) {
			LogHandler::verbose("main-temp", "tempStateChangeCallBack heat: %s", state);
			displayHandler->setHeateState(state);
			if(temperatureHandler)
				displayHandler->setHeateStateShort(temperatureHandler->getShortSleeveControlStatus(state));
		} else {
			LogHandler::verbose("main-temp", "tempStateChangeCallBack fan: %s", state);
			displayHandler->setFanState(state);
		}
	}
#endif
}
#endif
void startWeb(bool apMode) {
#if WIFI_TCODE == 1
	if(!webHandler.initialized) {
		displayPrint("Starting web server");
		webSocketHandler = new WebSocketHandler();
		webHandler.setup(SettingsHandler::webServerPort, SettingsHandler::hostname, SettingsHandler::friendlyName, webSocketHandler, apMode);
	}
#endif
}

void startBLE() {
	if(!bleHandler) {
		displayPrint("Starting BLE");
		bleHandler = new BLEHandler();
		bleHandler->setup();
	}
}

void startBlueTooth() {
#if BLUETOOTH_TCODE == 1
	if(!btHandler) {
		displayPrint("Starting Bluetooth serial");
		btHandler = new BluetoothHandler();
		btHandler->setup();
	}
	startBLE();
#endif
}

void startUDP() {
#if WIFI_TCODE == 1
	if(!udpHandler) {
		displayPrint("Starting UDP");
		udpHandler = new Udphandler();
		udpHandler->setup(SettingsHandler::udpServerPort);
	}
#endif
}

void startConfigMode(bool withBle= true) {
#if WIFI_TCODE == 1
	apMode = true;
	LogHandler::info("main-setup", "Starting in APMode");
	displayPrint("Starting in APMode");
	if (wifi.startAp()) 
	{
		LogHandler::info("main-setup", "APMode started");
		displayPrint("APMode started");
		startWeb(true);
	}
	else 
	{
		LogHandler::error("main-setup", "APMode start failed");
		displayPrint("APMode start failed");
	}
#endif

// After attempting to connect wifi, ble cause crash
	if(withBle) { 
		startBLE();
	}
}


#if WIFI_TCODE == 1
void wifiStatusCallBack(WiFiStatus status, WiFiReason reason) {
	if(status == WiFiStatus::CONNECTED) {
		if(reason == WiFiReason::AP_MODE) {
			#if ESP32_DA == 0
            if(bleHandler)
              bleHandler->stop(); // If a client connects to the ap stop the BLE to save memory.
			#endif
		}
	} else {
		// wifi.dispose();
		// startApMode();
		if(reason == WiFiReason::NO_AP || reason == WiFiReason::UNKNOWN) {
			startConfigMode(false);
		} else if(reason == WiFiReason::AUTH) {
            LogHandler::warning("main-setup", "Connection auth failed: Resetting wifi password and restarting");
            strcpy(SettingsHandler::wifiPass, SettingsHandler::defaultWifiPass);
            SettingsHandler::save();
            ESP.restart();
		}  else if(reason == WiFiReason::AP_MODE) {
			// #if ESP32_DA == 0
			// if(bleHandler)
			// 	bleHandler->setup();
			// #endif
		}
	}
}
#endif

void setup() 
{
	// see if we can use the onboard led for status
	//https://github.com/kriswiner/ESP32/blob/master/PWM/ledcWrite_demo_ESP32.ino
  	//digitalWrite(5, LOW);// Turn off on-board blue led


	Serial.begin(115200);
	
    LogHandler::setLogLevel(LogLevel::INFO);
	LogHandler::setMessageCallback(logCallBack);
	#if WIFI_TCODE == 1
		wifi.setWiFiStatusCallback(wifiStatusCallBack);
	#endif

	uint32_t chipId = 0;
	for(int i=0; i<17; i=i+8) {
	  chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
	Serial.println();
	LogHandler::info("main-setup", "ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
	LogHandler::info("main-setup", "This chip has %d cores\n", ESP.getChipCores());
 	LogHandler::info("main-setup", "Chip ID: %u\n", chipId);

    // esp_log_level_set("*", ESP_LOG_VERBOSE);
	// LogHandler::debug("main", "this is verbose");
	// LogHandler::debug("main", "this is debug");
	// LogHandler::info("main", "this is info");
	// LogHandler::warning("main", "this is warning");
	// LogHandler::error("main", "this is error");

	if(!SPIFFS.begin(true))
	{
		LogHandler::error("main-setup", "An Error has occurred while mounting SPIFFS");
		setupSucceeded = false;
		return;
	}

	SettingsHandler::load();
	LogHandler::info("main-setup", "Version: %s", SettingsHandler::ESP32Version);

	if(SettingsHandler::TCodeVersionEnum == TCodeVersion::v0_3) {
		servoHandler = new ServoHandler0_3();
	}
#if DEBUG_BUILD == 0 && TCODE_V2 == 1
	else if(SettingsHandler::TCodeVersionEnum == TCodeVersion::v0_2)
		servoHandler = new ServoHandler0_2();
#endif
	else {
		return;
		//servoHandler = new ServoHandler1_0();
	}

	servoHandler->setMessageCallback(CommandCallback);

#if TEMP_ENABLED == 1
	if(SettingsHandler::tempSleeveEnabled || SettingsHandler::tempInternalEnabled) {
		temperatureHandler = new TemperatureHandler();
		temperatureHandler->setup();
		temperatureHandler->setMessageCallback(tempChangeCallBack);
		temperatureHandler->setStateChangeCallback(tempStateChangeCallBack);
		xTaskCreatePinnedToCore(
			TemperatureHandler::startLoop,/* Function to implement the task */
			"TempTask", /* Name of the task */
			10000,  /* Stack size in words */
			temperatureHandler,  /* Task input parameter */
			1,  /* Priority of the task */
			&temperatureTask,  /* Task handle. */
			APP_CPU_NUM); /* Core where the task should run */
	}
#endif
#if DISPLAY_ENABLED == 1
	displayHandler = new DisplayHandler();
	if(SettingsHandler::displayEnabled)
	{
		displayHandler->setup();
		// #if ISAAC_NEWTONGUE_BUILD == 1
		// 	xTaskCreatePinnedToCore(
		// 		DisplayHandler::startAnimationDontPanic,/* Function to implement the task */
		// 		"DisplayTask", /* Name of the task */
		// 		10000,  /* Stack size in words */
		// 		displayHandler,  /* Task input parameter */
		// 		25,  /* Priority of the task */
		// 		&animationTask,  /* Task handle. */
		// 		1); /* Core where the task should run */
		// #endif
	}
#endif

	#if BLUETOOTH_TCODE == 1
		if(SettingsHandler::bluetoothEnabled) {
			startBlueTooth();
		} else 
	#endif
	
	#if WIFI_TCODE == 1
		if (strcmp(SettingsHandler::wifiPass, SettingsHandler::defaultWifiPass) != 0 && SettingsHandler::ssid != nullptr) {
			displayPrint("Setting up wifi...");
			displayPrint("Connecting to: ");
			displayPrint(SettingsHandler::ssid);
			if (wifi.connect(SettingsHandler::ssid, SettingsHandler::wifiPass)) { 
				displayPrint("Connected IP: " + wifi.ip().toString());
		#if DISPLAY_ENABLED == 1
			displayHandler->setLocalIPAddress(wifi.ip());
		#endif
				startUDP();
				startWeb(false);
			} 
		} else {
			startConfigMode();
		}
	#endif
    //otaHandler.setup();
	displayPrint("Setting up servos");
    servoHandler->setup(SettingsHandler::servoFrequency, SettingsHandler::pitchFrequency, SettingsHandler::valveFrequency, SettingsHandler::twistFrequency, SettingsHandler::msPerRad);
	setupSucceeded = true;
#if DISPLAY_ENABLED == 1
	if(SettingsHandler::displayEnabled)
	{
		xTaskCreatePinnedToCore(
			DisplayHandler::startLoop,/* Function to implement the task */
			"DisplayTask", /* Name of the task */
			10000,  /* Stack size in words */
			displayHandler,  /* Task input parameter */
			1,  /* Priority of the task */
			&displayTask,  /* Task handle. */
			APP_CPU_NUM); /* Core where the task should run */
	}
#endif
}

void loop() 
{
	
	if (SettingsHandler::restartRequired || restarting) {  // check the flag here to determine if a restart is required
		if(!restarting) {
			Serial.printf("Restarting ESP\n\r");
			ESP.restart();
        	restarting = true;
		}
		return;
	}
	if(setupSucceeded && !SettingsHandler::saving)
	{
		//otaHandler.handle();
		String serialData;
		#if WIFI_TCODE == 1
			if(webSocketHandler)
				webSocketHandler->getTCode(webSocketData);
			if(udpHandler)
				udpHandler->read(udpData);
		if (strlen(webSocketData) > 0) 
		{
			deviceinUse = true;
			//LogHandler::debug("main-loop", "webSocket writing: %s", webSocketData);
			servoHandler->read(webSocketData);
		}
		else if (!apMode && strlen(udpData) > 0) 
		{
			deviceinUse = true;
			//LogHandler::debug("main-loop", "udp writing: %s", udpData);
			servoHandler->read(udpData);
		} 
		else 
		#endif
		if (Serial.available() > 0) 
		{
			deviceinUse = true;
			serialData = Serial.readStringUntil('\n');
			servoHandler->read(serialData);
		} 
		#if BLUETOOTH_TCODE == 1
			else if (SettingsHandler::bluetoothEnabled && btHandler && btHandler->isConnected() && btHandler->available() > 0) 
			{
				deviceinUse = true;
				serialData = btHandler->readStringUntil('\n');
				servoHandler->read(serialData);
			}
		#endif

		if (strlen(udpData) == 0 && strlen(webSocketData) == 0 && serialData.length() == 0) // No wifi or websocket data
		{
			servoHandler->execute();
		}
#if TEMP_ENABLED == 1
		if(temperatureHandler && temperatureHandler->isRunning()) {
			if(SettingsHandler::tempSleeveEnabled) {
				temperatureHandler->setHeaterState();
			}
			if(SettingsHandler::fanControlEnabled) {
				temperatureHandler->setFanState();
			}
		}
#endif
	}
}