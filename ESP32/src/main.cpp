/* MIT License

Copyright (c) 2023 Jason C. Fain

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
#if BUILD_TYPE == RELEASE
#include <Arduino.h>
#endif

#include "utils.h"
#include <SPIFFS.h>
#include "LogHandler.h"
#include "SettingsHandler.h"
#include "SystemCommandHandler.h"
#if WIFI_TCODE
	#include "WifiHandler.h"
#endif

#if TEMP_ENABLED
	#include "TemperatureHandler.h"
#endif
#if DISPLAY_ENABLED
	#include "DisplayHandler.h"
#endif
#if BLUETOOTH_TCODE
	#include "BluetoothHandler.h"
#endif
#include "TCode/MotorHandler.h"

#if MOTOR_TYPE == 0
	#if TCODE_V2//Too much memory needed with debug
		#include "TCode/v0.2/ServoHandler0_2.h"
	#endif

	#include "TCode/v0.3/ServoHandler0_3.h"
	//#include "TCode/v1.0/ServoHandler1_0.h"
#elif MOTOR_TYPE == 1
	#include "TCode/v0.3/BLDCHandler0_3.h"
#endif

#if WIFI_TCODE
	#include "UdpHandler.h"
	//#include "TcpHandler.h"
	#include "WebHandler.h"
#endif
//#include "OTAHandler.h"
#include "BLEHandler.h"

#if WIFI_TCODE
	#include "WebSocketHandler.h"
#endif

#if BLUETOOTH_TCODE
	BluetoothHandler* btHandler = 0;
#endif

//TcpHandler tcpHandler;
MotorHandler* motorHandler;

#if WIFI_TCODE
	Udphandler* udpHandler = 0;
	WifiHandler wifi;
	WebHandler webHandler;
	WebSocketHandler* webSocketHandler = 0;
#endif

#if TEMP_ENABLED
	TemperatureHandler* temperatureHandler = 0;
#endif

BLEHandler* bleHandler = 0;

#if DISPLAY_ENABLED
	DisplayHandler* displayHandler;
	TaskHandle_t displayTask;
	#if ISAAC_NEWTONGUE_BUILD
		TaskHandle_t animationTask;
	#endif
#endif
#if TEMP_ENABLED
	TaskHandle_t temperatureTask;
#endif
// This has issues running with the webserver.
//OTAHandler otaHandler;
bool apMode = false;
bool setupSucceeded = false;
bool restarting = false;

char udpData[600];
char webSocketData[600];

void displayPrint(String text) {
	#if DISPLAY_ENABLED
		displayHandler->println(text);
	#endif
}


void TCodeCommandCallback(const char* in) {

	if(strpbrk("$", in) != nullptr) {
		SystemCommandHandler::process(in);
	} else {
		#if BLUETOOTH_TCODE
			if (SettingsHandler::bluetoothEnabled && btHandler->isConnected())
				btHandler->CommandCallback(in);
		#endif
		#if WIFI_TCODE
			if(webSocketHandler)
				webSocketHandler->CommandCallback(in);
			if(udpHandler)
				udpHandler->CommandCallback(in);
		#endif
		if(Serial)
			Serial.println(in);
	}
}

void logCallBack(const char* in, LogLevel level) {
#if WIFI_TCODE
	// if(webSocketHandler) {
	// 	webSocketHandler->sendDebug(in, level);
	// }
#endif
}
#if TEMP_ENABLED
void tempChangeCallBack(TemperatureType type, const char* message, float temp) {
#if WIFI_TCODE
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
			LogHandler::debug("main-temp", "tempStateChangeCallBack heat: %s", state);
			displayHandler->setHeateState(state);
			if(temperatureHandler)
				displayHandler->setHeateStateShort(temperatureHandler->getShortSleeveControlStatus(state));
		} else {
			LogHandler::debug("main-temp", "tempStateChangeCallBack fan: %s", state);
			displayHandler->setFanState(state);
		}
	}
#endif
}
#endif
void startWeb(bool apMode) {
#if WIFI_TCODE
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
#if BLUETOOTH_TCODE
	if(!btHandler) {
		displayPrint("Starting Bluetooth serial");
		btHandler = new BluetoothHandler();
		btHandler->setup();
	}
	startBLE();
#endif
}

void startUDP() {
#if WIFI_TCODE
	if(!udpHandler) {
		displayPrint("Starting UDP");
		udpHandler = new Udphandler();
		udpHandler->setup(SettingsHandler::udpServerPort);
	}
#endif
}

void startConfigMode(bool withBle= true) {
#if WIFI_TCODE
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


#if WIFI_TCODE
void wifiStatusCallBack(WiFiStatus status, WiFiReason reason) {
	if(status == WiFiStatus::CONNECTED) {
		if(reason == WiFiReason::AP_MODE) {
			#if !ESP32_DA
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
			// #if !ESP32_DA
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
	
    LogHandler::setLogLevel(LogLevel::DEBUG);
	LogHandler::setMessageCallback(logCallBack);
	#if WIFI_TCODE
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

#if MOTOR_TYPE == 0
	if(SettingsHandler::TCodeVersionEnum == TCodeVersion::v0_3) {
		motorHandler = new ServoHandler0_3();
	}
	#if !DEBUG_BUILD && TCODE_V2
		else if(SettingsHandler::TCodeVersionEnum == TCodeVersion::v0_2)
			motorHandler = new ServoHandler0_2();
	#endif
		else {
			return;
			//motorHandler = new ServoHandler1_0();
		}
#elif MOTOR_TYPE == 1
	motorHandler = new BLDCHandler0_3();
#else
	return;
#endif

	motorHandler->setMessageCallback(TCodeCommandCallback);

#if TEMP_ENABLED
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
#if DISPLAY_ENABLED
	displayHandler = new DisplayHandler();
	if(SettingsHandler::displayEnabled)
	{
		displayHandler->setup();
		// #if ISAAC_NEWTONGUE_BUILD
		// 	xTaskCreatePinnedToCore(
		// 		DisplayHandler::startAnimationDontPanic,/* Function to implement the task */
		// 		"DisplayTask", /* Name of the task */
		// 		10000,  /* Stack size in words */
		// 		displayHandler,  /* Task input parameter */
		// 		25,  /* Priority of the task */
		// 		&animationTask,  /* Task handle. */
		// 		APP_CPU_NUM); /* Core where the task should run */
		// #endif
	}
#endif

	#if BLUETOOTH_TCODE
		if(SettingsHandler::bluetoothEnabled) {
			startBlueTooth();
		} else 
	#endif
	
	#if WIFI_TCODE
		if (strcmp(SettingsHandler::wifiPass, SettingsHandler::defaultWifiPass) != 0 && SettingsHandler::ssid != nullptr) {
			displayPrint("Setting up wifi...");
			displayPrint("Connecting to: ");
			displayPrint(SettingsHandler::ssid);
			if (wifi.connect(SettingsHandler::ssid, SettingsHandler::wifiPass)) { 
				displayPrint("Connected IP: " + wifi.ip().toString());
		#if DISPLAY_ENABLED
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
    motorHandler->setup();
#if DISPLAY_ENABLED
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
	setupSucceeded = true;
}

void loop() {
	if (SystemCommandHandler::restartRequired || restarting) {  // check the flag here to determine if a restart is required
		if(!restarting) {
			LogHandler::info("main", "Restarting ESP");
			ESP.restart();
        	restarting = true;
		}
        vTaskDelay(1000/portTICK_PERIOD_MS);
	} 
#if TEMP_ENABLED
	else if (SettingsHandler::tempInternalEnabled && temperatureHandler->isMaxTempTriggered()) {
		LogHandler::error("main", "Internal temp has reached a maximum of %f. Servos disabled!", SettingsHandler::internalMaxTemp);
        vTaskDelay(5000/portTICK_PERIOD_MS);
	} 
#endif
	else {
		if(setupSucceeded && !SettingsHandler::saving)
		{
			//otaHandler.handle();
			String serialData;
#if WIFI_TCODE
			if(webSocketHandler)
				webSocketHandler->getTCode(webSocketData);
			if(udpHandler)
				udpHandler->read(udpData);
			if (strlen(webSocketData) > 0) {
				//LogHandler::debug("main-loop", "webSocket writing: %s", webSocketData);
				motorHandler->read(webSocketData);
			} else if (!apMode && strlen(udpData) > 0) {
				LogHandler::debug("main-loop", "udp writing: %s", udpData);
				motorHandler->read(udpData);
			} 
			else 
#endif
			if (Serial.available() > 0) {
				serialData = Serial.readStringUntil('\n');
				motorHandler->read(serialData);
			} 
#if BLUETOOTH_TCODE
			else if (SettingsHandler::bluetoothEnabled && btHandler && btHandler->isConnected() && btHandler->available() > 0) {
				deviceinUse = true;
				serialData = btHandler->readStringUntil('\n');
				servoHandler->read(serialData);
			}
#endif

			if (strlen(udpData) == 0 && strlen(webSocketData) == 0 && serialData.length() == 0) {// No wifi or websocket data 
				motorHandler->execute();
			}
		}
	}
	#if TEMP_ENABLED
		if(setupSucceeded && temperatureHandler && temperatureHandler->isRunning()) {
			if(SettingsHandler::tempSleeveEnabled) {
				temperatureHandler->setHeaterState();
			}
			if(SettingsHandler::fanControlEnabled) {
				temperatureHandler->setFanState();
			}
		}
	#endif
	if(!setupSucceeded) {
		LogHandler::error("main", "There was an issue in setup");
        vTaskDelay(5000/portTICK_PERIOD_MS);
	}
	if(SettingsHandler::saving) {
		LogHandler::info("main", "Saving settings, main loop disabled...");
        vTaskDelay(100/portTICK_PERIOD_MS);
	}
}