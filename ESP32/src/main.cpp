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

#if DEBUG_BUILD
    #define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
    #include "esp_log.h"
#endif
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
	#include "HTTP/HTTPBase.h"
	#include "HTTP/WebSocketBase.h"
	#if !SECURE_WEB
		#include "WebHandler.h"
	#else
		#include "HTTP\HTTPSHandler.hpp"
	#endif
	#include "MDNSHandler.hpp"
#endif
//#include "OTAHandler.h"
#if BLE_TCODE
	#include "BLEHandler.hpp"
#endif

#if WIFI_TCODE
	#if !SECURE_WEB
		#include "WebSocketHandler.h"
	#else
		#include "HTTP/SecureWebSocketHandler.hpp"
	#endif
#endif

#if BLUETOOTH_TCODE
	BluetoothHandler* btHandler = 0;
#endif

#include "BatteryHandler.h"
#include "Motion/MotionHandler.hpp"
#include "VoiceHandler.hpp"
#include "ButtonHandler.hpp"


//TcpHandler tcpHandler;
MotorHandler* motorHandler;
BatteryHandler* batteryHandler;
TaskHandle_t batteryTask;
TaskHandle_t httpsTask;

MotionHandler motionHandler;
VoiceHandler* voiceHandler;
ButtonHandler* buttonHandler = 0;
TaskHandle_t voiceTask;

#if WIFI_TCODE
	Udphandler* udpHandler = 0;
	WifiHandler wifi;
	MDNSHandler mdnsHandler;
	HTTPBase* webHandler = 0;
	WebSocketBase* webSocketHandler = 0;
	
#endif

#if TEMP_ENABLED
	TemperatureHandler* temperatureHandler = 0;
#endif

#if BLE_TCODE
	BLEHandler* bleHandler = 0;
#endif

#if DISPLAY_ENABLED
	DisplayHandler* displayHandler;
	TaskHandle_t displayTask;
	// #if ISAAC_NEWTONGUE_BUILD
	// 	TaskHandle_t animationTask;
	// #endif
#endif
#if TEMP_ENABLED
	TaskHandle_t temperatureTask;
#endif
// This has issues running with the webserver.
//OTAHandler otaHandler;
bool setupSucceeded = false;
bool restarting = false;

char udpData[600];
char webSocketData[600];
#if BLE_TCODE
	char bleData[600];
#endif
char movement[600];
char buttonCommand[MAX_COMMAND];

unsigned long bench[10];
unsigned long benchLast[10];
bool benchEnable = false;
bool benchEnableZero = false;
unsigned long benchThreshHold = 1300;

void benchStart(int benchNumber) {
	if(benchEnable || (benchEnableZero && benchNumber == 0))
		bench[benchNumber] = micros();
}
void benchFinish(const char* systemUnderBench, int benchNumber) {
	if(benchEnable || (benchEnableZero && benchNumber == 0)) {
		unsigned long timeTaken = micros() - bench[benchNumber];
		if(timeTaken > benchThreshHold) {
			Serial.printf("%s:							%lu\n", systemUnderBench, timeTaken);
			bench[benchNumber] = 0;
			benchLast[benchNumber] = timeTaken;
		}
	}
}

void displayPrint(String text) {
	#if DISPLAY_ENABLED
		displayHandler->println(text);
	#endif
}


void TCodeCommandCallback(const char* in) {

	if(strpbrk("$", in) != nullptr || strpbrk("#", in) != nullptr) {
		SystemCommandHandler::process(in);
	} else {
		#if BLUETOOTH_TCODE
			if (SettingsHandler::bluetoothEnabled && btHandler->isConnected())
				btHandler->CommandCallback(in);
		#endif
		#if BLE_TCODE

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
void profileChangeCallback(uint8_t profile) {
	
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
			LogHandler::verbose(TagHandler::Main, "tempStateChangeCallBack heat: %s", state);
			displayHandler->setHeateState(state);
			if(temperatureHandler)
				displayHandler->setHeateStateShort(temperatureHandler->getShortSleeveControlStatus(state));
		} else {
			LogHandler::verbose(TagHandler::Main, "tempStateChangeCallBack fan: %s", state);
			displayHandler->setFanState(state);
		}
	}
#endif
}
#endif
void startWeb(bool apMode) {
#if WIFI_TCODE
	if(!webHandler) {
		displayPrint("Starting web server");
		#if !SECURE_WEB
			webHandler = new WebHandler();
			webSocketHandler = new WebSocketHandler();
		#else
			webHandler = new HTTPSHandler();
			webSocketHandler = new SecureWebSocketHandler();
		#endif
		webHandler->setup(SettingsHandler::webServerPort, webSocketHandler, apMode);
		mdnsHandler.setup(SettingsHandler::hostname, SettingsHandler::friendlyName);
		
		#if SECURE_WEB
			LogHandler::debug(TagHandler::Main, "Start https task");
			auto httpsStatus = xTaskCreateUniversal(
				HTTPSHandler::startLoop,/* Function to implement the task */
				"HTTPSTask", /* Name of the task */
				8192 * 3,  /* Stack size in words */
				webHandler,  /* Task input parameter */
				3,  /* Priority of the task */
				&httpsTask,  /* Task handle. */
				-1); /* Core where the task should run */
			if(httpsStatus != pdPASS) {
				LogHandler::error(TagHandler::Main, "Could not start https task.");
			}
		#endif
	}
#endif
}


#if BLE_TCODE
void startBLE() {
	if(!bleHandler) {
		displayPrint("Starting BLE");
		bleHandler = new BLEHandler();
		bleHandler->setup();
	}
}
#endif

#if BLUETOOTH_TCODE
void startBlueTooth() {
	if(!btHandler) {
		displayPrint("Starting Bluetooth serial");
		btHandler = new BluetoothHandler();
		btHandler->setup();
	}
	startBLE();
}
#endif

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
	SettingsHandler::apMode = true;
	LogHandler::info(TagHandler::Main, "Starting in APMode");
	displayPrint("Starting in APMode");
	if (wifi.startAp()) 
	{
		LogHandler::info(TagHandler::Main, "APMode started");
		displayPrint("APMode started");
		startWeb(true);
	}
	else 
	{
		LogHandler::error(TagHandler::Main, "APMode start failed");
		displayPrint("APMode start failed");
	}
#endif

#if BLE_TCODE
// After attempting to connect wifi, ble cause crash
	if(withBle) { 
		startBLE();
	}
#endif
}


#if WIFI_TCODE
void wifiStatusCallBack(WiFiStatus status, WiFiReason reason) {
	if(status == WiFiStatus::CONNECTED) {
        LogHandler::debug(TagHandler::Main, "wifiStatusCallBack WiFiStatus::CONNECTED");
		if(reason == WiFiReason::AP_MODE) {
        	LogHandler::debug(TagHandler::Main, "wifiStatusCallBack WiFiReason::AP_MODE");
#if BLUETOOTH_TCODE
			#if !ESP32_DA
            if(bleHandler)
              bleHandler->stop(); // If a client connects to the ap stop the BLE to save memory.
			#endif
#endif
		}
	} else {
		// wifi.dispose();
		// startApMode();
        LogHandler::debug(TagHandler::Main, "wifiStatusCallBack Not connected");
		if(reason == WiFiReason::NO_AP || reason == WiFiReason::UNKNOWN) {
        	LogHandler::debug(TagHandler::Main, "wifiStatusCallBack WiFiReason::NO_AP || WiFiReason::UNKNOWN");
			startConfigMode(false);
		} else if(reason == WiFiReason::AUTH) {
        	LogHandler::debug(TagHandler::Main, "wifiStatusCallBack WiFiReason::AUTH");
            LogHandler::warning(TagHandler::Main, "Connection auth failed: Resetting wifi password and restarting");
            strcpy(SettingsHandler::wifiPass, SettingsHandler::defaultWifiPass);
            SettingsHandler::saveSettings();
            ESP.restart();
		}  else if(reason == WiFiReason::AP_MODE) {
        	LogHandler::debug(TagHandler::Main, "wifiStatusCallBack WiFiReason::AP_MODE");
			// #if !ESP32_DA
			// if(bleHandler)
			// 	bleHandler->setup();
			// #endif
		}
	}
}
#endif

void batteryVoltageCallback(float capacityRemainingPercentage, float capacityRemaining, float voltage, float temperature) {
	#if DISPLAY_ENABLED
		if(displayHandler) {
			displayHandler->setBatteryInformation(capacityRemainingPercentage, voltage, temperature);
		}
	#endif
	#if WIFI_TCODE
		if(webSocketHandler) {
			String statusJson("{\"batteryCapacityRemaining\":\"" + String(capacityRemaining) + "\", \"batteryCapacityRemainingPercentage\":\"" + String(capacityRemainingPercentage) + "\", \"batteryVoltage\":\""+String(voltage)+"\", \"batteryTemperature\":\""+String(temperature)+"\"}");
			webSocketHandler->sendCommand("batteryStatus", statusJson.c_str());
		}
	#endif
}

void settingChangeCallback(const char* group, const char* settingThatChanged) {
    LogHandler::debug(TagHandler::Main, "settingChangeCallback: %s", settingThatChanged);
	if(strcmp(group, "motionGenerator") == 0) {
		if(strcmp(settingThatChanged, "motionSelectedProfileIndex") == 0 || strcmp(settingThatChanged, "motionProfile") == 0) 
			motionHandler.setMotionChannels(SettingsHandler::getMotionChannels());
		// else if(strcmp(settingThatChanged, "motionChannels") == 0) 
		// 	motionHandler.setMotionChannels(SettingsHandler::getMotionChannels());
		else if(strcmp(settingThatChanged, "motionEnabled") == 0) 
			motionHandler.setEnabled(SettingsHandler::getMotionEnabled());
		// else if(strcmp(settingThatChanged, "motionAmplitudeGlobal") == 0) 
		// 	motionHandler.setAmplitude(SettingsHandler::getMotionAmplitudeGlobal());
		// else if(strcmp(settingThatChanged, "motionOffsetGlobal") == 0) 
		// 	motionHandler.setOffset(SettingsHandler::getMotionOffsetGlobal());
		// else if(strcmp(settingThatChanged, "motionPeriodGlobal") == 0) 
		// 	motionHandler.setPeriod(SettingsHandler::getMotionPeriodGlobal());
		// else if(strcmp(settingThatChanged, "motionUpdateGlobal") == 0) 
		// 	motionHandler.setUpdate(SettingsHandler::getMotionUpdateGlobal());
		// else if(strcmp(settingThatChanged, "motionPhaseGlobal") == 0) 
		// 	motionHandler.setPhase(SettingsHandler::getMotionPhaseGlobal());
		// else if(strcmp(settingThatChanged, "motionReversedGlobal") == 0) 
		// 	motionHandler.setReverse(SettingsHandler::getMotionReversedGlobal());
		// else if(strcmp(settingThatChanged, "motionAmplitudeGlobalRandom") == 0) 
		// 	motionHandler.setAmplitudeRandom(SettingsHandler::getMotionAmplitudeGlobalRandom());
		// else if(strcmp(settingThatChanged, "motionAmplitudeGlobalRandomMin") == 0) 
		// 	motionHandler.setAmplitudeRandomMin(SettingsHandler::getMotionAmplitudeGlobalRandomMin());
		// else if(strcmp(settingThatChanged, "motionAmplitudeGlobalRandomMax") == 0) 
		// 	motionHandler.setAmplitudeRandomMax(SettingsHandler::getMotionAmplitudeGlobalRandomMax());
		// else if(strcmp(settingThatChanged, "motionPeriodGlobalRandom") == 0) 
		// 	motionHandler.setPeriodRandom(SettingsHandler::getMotionPeriodGlobalRandom());
		// else if(strcmp(settingThatChanged, "motionPeriodGlobalRandomMin") == 0) 
		// 	motionHandler.setPeriodRandomMin(SettingsHandler::getMotionPeriodGlobalRandomMin());
		// else if(strcmp(settingThatChanged, "motionPeriodGlobalRandomMax") == 0) 
		// 	motionHandler.setPeriodRandomMax(SettingsHandler::getMotionPeriodGlobalRandomMax());
		// else if(strcmp(settingThatChanged, "motionOffsetGlobalRandom") == 0) 
		// 	motionHandler.setOffsetRandom(SettingsHandler::getMotionOffsetGlobalRandom());
		// else if(strcmp(settingThatChanged, "motionOffsetGlobalRandomMin") == 0) 
		// 	motionHandler.setOffsetRandomMin(SettingsHandler::getMotionOffsetGlobalRandomMin());
		// else if(strcmp(settingThatChanged, "motionOffsetGlobalRandomMax") == 0) 
		// 	motionHandler.setOffsetRandomMax(SettingsHandler::getMotionOffsetGlobalRandomMax());
		// else if(strcmp(settingThatChanged, "motionRandomChangeMin") == 0) 
		// 	motionHandler.setMotionRandomChangeMin(SettingsHandler::getMotionRandomChangeMin());
		// else if(strcmp(settingThatChanged, "motionRandomChangeMax") == 0) 
		// 	motionHandler.setMotionRandomChangeMax(SettingsHandler::getMotionRandomChangeMax());
	} else if(voiceHandler && strcmp(group, "voiceHandler") == 0) {
		if(strcmp(settingThatChanged, "voiceMuted") == 0) 
			voiceHandler->setMuteMode(SettingsHandler::getVoiceMuted());
		else if(strcmp(settingThatChanged, "voiceVolume") == 0) 
			voiceHandler->setVolume(SettingsHandler::getVoiceVolume());
		else if(strcmp(settingThatChanged, "voiceWakeTime") == 0) 
			voiceHandler->setWakeTime(SettingsHandler::getVoiceWakeTime());
	} else if(buttonHandler && strcmp(group, "buttonCommand") == 0) {
		if(strcmp(settingThatChanged, "bootButtonCommand") == 0) 
			buttonHandler->updateBootButtonCommand(SettingsHandler::bootButtonCommand);
		else if(strcmp(settingThatChanged, "analogButtonCommands") == 0) {
			buttonHandler->updateAnalogButtonCommands(SettingsHandler::buttonSets);
		} else if(strcmp(settingThatChanged, "buttonAnalogDebounce") == 0) {
			buttonHandler->updateAnalogDebounce(SettingsHandler::buttonAnalogDebounce);
		}
	}
	
}
void loadI2CModules() {
#if DISPLAY_ENABLED
	if(SettingsHandler::displayEnabled)
	{
    	LogHandler::debug(TagHandler::Main, "Start Display task");
		auto displayStatus = xTaskCreatePinnedToCore(
			DisplayHandler::startLoop,/* Function to implement the task */
			"DisplayTask", /* Name of the task */
			5000,  /* Stack size in words */
			displayHandler,  /* Task input parameter */
			3,  /* Priority of the task */
			&displayTask,  /* Task handle. */
			APP_CPU_NUM); /* Core where the task should run */
			if(displayStatus != pdPASS) {
    			LogHandler::error(TagHandler::Main, "Could not start display task.");
			}
	}
#endif
	if(SettingsHandler::batteryLevelEnabled) {
		batteryHandler = new BatteryHandler();
		if(batteryHandler->setup()) {
			LogHandler::debug(TagHandler::Main, "Start Battery task");
			auto batteryStatus = xTaskCreatePinnedToCore(
				BatteryHandler::startLoop,/* Function to implement the task */
				"BatteryTask", /* Name of the task */
				4028,  /* Stack size in words */
				batteryHandler,  /* Task input parameter */
				2,  /* Priority of the task */
				&batteryTask,  /* Task handle. */
				APP_CPU_NUM); /* Core where the task should run */
				if(batteryStatus != pdPASS) {
					LogHandler::error(TagHandler::Main, "Could not start battery task.");
				}
				batteryHandler->setMessageCallback(batteryVoltageCallback);
		}
	}
	if(SettingsHandler::getVoiceEnabled()) {
		voiceHandler = new VoiceHandler();
		voiceHandler->setMessageCallback(TCodeCommandCallback);
		if(voiceHandler->setup()) {
			LogHandler::debug(TagHandler::Main, "Start Voice task");
			auto voiceStatus = xTaskCreatePinnedToCore(
				VoiceHandler::startLoop,/* Function to implement the task */
				"VoiceTask", /* Name of the task */
				4028,  /* Stack size in words */
				voiceHandler,  /* Task input parameter */
				2,  /* Priority of the task */
				&voiceTask,  /* Task handle. */
				APP_CPU_NUM); /* Core where the task should run */
				if(voiceStatus != pdPASS) {
					LogHandler::error(TagHandler::Main, "Could not start voice task.");
				}
		}
	}
}
void setup() 
{
	// see if we can use the onboard led for status
	//https://github.com/kriswiner/ESP32/blob/master/PWM/ledcWrite_demo_ESP32.ino
  	//digitalWrite(5, LOW);// Turn off on-board blue led


	Serial.begin(115200);
	
    LogHandler::setLogLevel(LogLevel::INFO);
	LogHandler::setMessageCallback(logCallBack);
	#if WIFI_TCODE
		wifi.setWiFiStatusCallback(wifiStatusCallBack);
	#endif

	uint32_t chipId = 0;
	for(int i=0; i<17; i=i+8) {
	  chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
	Serial.println();
	LogHandler::info(TagHandler::Main, "ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
	LogHandler::info(TagHandler::Main, "This chip has %d cores\n", ESP.getChipCores());
 	LogHandler::info(TagHandler::Main, "Chip ID: %u\n", chipId);

    // esp_log_level_set("*", ESP_LOG_VERBOSE);
	// LogHandler::debug("main", "this is verbose");
	// LogHandler::debug("main", "this is debug");
	// LogHandler::info("main", "this is info");
	// LogHandler::warning("main", "this is warning");
	// LogHandler::error("main", "this is error");

	if(!SPIFFS.begin(true))
	{
		LogHandler::error(TagHandler::Main, "An Error has occurred while mounting SPIFFS");
		setupSucceeded = false;
		return;
	}

	SettingsHandler::init();
	LogHandler::info(TagHandler::Main, "Version: %s", SettingsHandler::getFirmwareVersion());

#if MOTOR_TYPE == 0
	if(SettingsHandler::TCodeVersionEnum == TCodeVersion::v0_3) {
		motorHandler = new ServoHandler0_3();
	}
	#if !DEBUG_BUILD && TCODE_V2
		else if(SettingsHandler::TCodeVersionEnum == TCodeVersion::v0_2)
			motorHandler = new ServoHandler0_2();
	#endif
		else {
			LogHandler::error(TagHandler::Main, "Invalid TCode version: %ld", SettingsHandler::TCodeVersionEnum);
			return;// TODO: this stops apmode and not what we want
			//motorHandler = new ServoHandler1_0();
		}
#elif MOTOR_TYPE == 1
	motorHandler = new BLDCHandler0_3();
#else
	LogHandler::error(TagHandler::Main, "Invalid motor type defined!");
	return;
#endif

	motorHandler->setMessageCallback(TCodeCommandCallback);
	//SystemCommandHandler::registerOtherCommandCallback(TCodeCommandCallback);

#if TEMP_ENABLED
	if(SettingsHandler::tempSleeveEnabled || SettingsHandler::tempInternalEnabled) {
		temperatureHandler = new TemperatureHandler();
		temperatureHandler->setup();
		temperatureHandler->setMessageCallback(tempChangeCallBack);
		temperatureHandler->setStateChangeCallback(tempStateChangeCallBack);
    	LogHandler::debug(TagHandler::Main, "Start temperature task");
		auto tempStartStatus = xTaskCreatePinnedToCore(
			TemperatureHandler::startLoop,/* Function to implement the task */
			"TempTask", /* Name of the task */
			5000,  /* Stack size in words */
			temperatureHandler,  /* Task input parameter */
			3,  /* Priority of the task */
			&temperatureTask,  /* Task handle. */
			APP_CPU_NUM); /* Core where the task should run */
		if(tempStartStatus != pdPASS) {
			LogHandler::error(TagHandler::Main, "Could not start temperature task.");
		}
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
	displayPrint("Setting up motor");
    motorHandler->setup();
	motionHandler.setup(SettingsHandler::TCodeVersionEnum);
	loadI2CModules();
	buttonHandler = new ButtonHandler();

	buttonHandler->init(SettingsHandler::buttonAnalogDebounce, SettingsHandler::bootButtonCommand, SettingsHandler::buttonSets);

	SettingsHandler::setMessageCallback(settingChangeCallback);
	setupSucceeded = true;
    LogHandler::debug(TagHandler::Main, "Setup finished");
    SettingsHandler::printFree();
}

void loop() {
	// if(setupSucceeded && SettingsHandler::saving) {
	// 	motorHandler->execute();
	// 	vTaskDelay(250/portTICK_PERIOD_MS);
	// 	return;
	// }
	//LogHandler::verbose(TagHandler::MainLoop, "Enter loop ############################################");
	benchStart(0);
	if (SystemCommandHandler::restartRequired || restarting) {  // check the flag here to determine if a restart is required
		if(!restarting) {
			LogHandler::info(TagHandler::Main, "Restarting ESP");
			ESP.restart();
        	restarting = true;
		}
        vTaskDelay(1000/portTICK_PERIOD_MS);
	} 
#if TEMP_ENABLED
	else if (SettingsHandler::tempInternalEnabled && temperatureHandler && temperatureHandler->isMaxTempTriggered()) {
		motorHandler->read("STOP\n");
		motorHandler->execute();
		LogHandler::error(TagHandler::Main, "Internal temp has reached maximum user set. Main loop disabled! Restart system to enable the loop.");
			if(SettingsHandler::fanControlEnabled) {
				temperatureHandler->setFanState();
			}
        vTaskDelay(5000/portTICK_PERIOD_MS);
	} 
#endif
	else {
		if(setupSucceeded)
		{
			//otaHandler.handle();

			String serialData;
#if WIFI_TCODE
			if(webSocketHandler) {
				
				benchStart(1);
				webSocketHandler->getTCode(webSocketData);
				benchFinish("Websocket get", 1);
			}
			if(udpHandler) {
				benchStart(2);
				udpHandler->read(udpData);
				benchFinish("Udp get", 2);
			}
			
#if BLE_TCODE
			if(bleHandler) {
				bleHandler->read(bleData);
			}
#endif
			if(buttonHandler) {
				buttonHandler->read(buttonCommand);
				if(strlen(buttonCommand) > 0) {
					motorHandler->read(buttonCommand);
				}
			}
			
			benchStart(3);
			if (!SettingsHandler::getMotionEnabled() && strlen(webSocketData) > 0) {
				LogHandler::verbose(TagHandler::MainLoop, "webSocket writing: %s", webSocketData);
				motorHandler->read(webSocketData);
			} else if (!SettingsHandler::apMode && !SettingsHandler::getMotionEnabled() && strlen(udpData) > 0) {
				benchStart(6);
				LogHandler::verbose(TagHandler::MainLoop, "udp writing: %s", udpData);
				motorHandler->read(udpData);
				benchFinish("Udp write", 6);
			} 
#if BLE_TCODE
			else if (!SettingsHandler::getMotionEnabled() && strlen(bleData) > 0) {
				motorHandler->read(bleData);
			}
#endif
			else 
#endif
			if (!SettingsHandler::getMotionEnabled() && Serial.available() > 0) {
				serialData = Serial.readStringUntil('\n');
				LogHandler::verbose(TagHandler::MainLoop, "serial writing: %s", serialData.c_str());
				motorHandler->read(serialData);
			} 
#if BLUETOOTH_TCODE
			else if (!SettingsHandler::getMotionEnabled() && SettingsHandler::bluetoothEnabled && btHandler && btHandler->isConnected() && btHandler->available() > 0) {
				deviceinUse = true;
				serialData = btHandler->readStringUntil('\n');
				servoHandler->read(serialData);
			}
#endif

			else if (SettingsHandler::getMotionEnabled()) {
				// Read and process tcode $ and # commands
				if(Serial.available() > 0) {
					serialData = Serial.readStringUntil('\n');
					if(serialData.startsWith("$") || serialData.startsWith("#")) {
						motorHandler->read(serialData);
					}
				}
#if WIFI_TCODE
				else if(strlen(udpData) > 0 && (strpbrk("$", udpData) != nullptr || strpbrk("#", udpData) != nullptr)) {
					motorHandler->read(udpData);
				}
				else if(strlen(webSocketData) > 0 && (strpbrk("$", webSocketData) != nullptr || strpbrk("#", webSocketData) != nullptr)) {
					motorHandler->read(webSocketData);
				}
#endif
#if BLUETOOTH_TCODE
				else if (SettingsHandler::bluetoothEnabled && btHandler && btHandler->isConnected() && btHandler->available() > 0) {
					deviceinUse = true;
					serialData = btHandler->readStringUntil('\n');
					servoHandler->read(serialData);
				}
#endif
				motionHandler.getMovement(movement);
				if(strlen(movement) > 0) {
					LogHandler::verbose(TagHandler::MainLoop, "motion handler writing: %s", movement);
					motorHandler->read(movement);
				}
			} 
			
			benchFinish("Input check", 3);

			if (strlen(movement) == 0 && strlen(udpData) == 0 && strlen(webSocketData) == 0 && serialData.length() == 0) {// No data from above
				//LogHandler::verbose(TagHandler::MainLoop, "No data executing");
				
				benchStart(4);
				motorHandler->execute();
				benchFinish("Execute", 4);
			}
		}
	}
#if TEMP_ENABLED
		benchStart(5);
		if(setupSucceeded && temperatureHandler && temperatureHandler->isRunning()) {
			if(SettingsHandler::tempSleeveEnabled) {
				temperatureHandler->setHeaterState();
			}
			if(SettingsHandler::fanControlEnabled) {
				temperatureHandler->setFanState();
			}
		}
		benchFinish("Temp check", 5);
#endif
	if(!setupSucceeded) {
		LogHandler::error(TagHandler::Main, "There was an issue in setup");
        vTaskDelay(5000/portTICK_PERIOD_MS);
	}
	
	benchFinish("Main loop", 0);
}