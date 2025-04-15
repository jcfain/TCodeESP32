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

#include "esp_idf_version.h"
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#define ESP_ARDUINO3
#endif
#if DEBUG_BUILD
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#endif
#include <Arduino.h>
#include <EEPROM.h>

#if PICO_BUILD
// #include <FreeRTOS.h>
#endif

#if ESP8266 == 1
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#endif

#include "utils.h"
#include <LittleFS.h>
#include <TCode.h>
#include "logging/LogHandler.h"
#include "SettingsHandler.h"
#include "SystemCommandHandler.h"
#if WIFI_TCODE
#include "WifiHandler.h"
#endif

#if BUILD_TEMP
#include "TemperatureHandler.h"
#endif
#if BUILD_DISPLAY
#include "DisplayHandler.h"
#endif
#if BLUETOOTH_TCODE
#include "BluetoothHandler.h"
#endif
#include "TCode/MotorHandler.h"
// #include "BLEConfigurationHandler.h"

#if MOTOR_TYPE == 0
#include "ServoHandler0_3.h"
#include "ServoHandler0_4.h"
#elif MOTOR_TYPE == 1
#include "BLDCHandler0_3.h"
#include "BLDCHandler0_4.h"
#endif

#if WIFI_TCODE
#include "UdpHandler2.h"
// #include "TcpHandler.h"
#include "HTTP/HTTPBase.h"
#include "HTTP/WebSocketBase.h"
#if !SECURE_WEB
#include "WebHandler.h"
//#include "WebHandler_psychic.h"
#else
#include "HTTP\HTTPSHandler.hpp"
#endif
#include "MDNSHandler.hpp"
#endif
// #include "OTAHandler.h"
#if BLE_TCODE
#include "BLEHandler.hpp"
#endif

#if WIFI_TCODE
#if !SECURE_WEB
#include "WebSocketHandler.h"
//#include "WebSocketHandler_psychic.h"
#else
#include "HTTP/SecureWebSocketHandler.hpp"
#endif
#endif

#include "BatteryHandler.h"
#include "MotionHandler.hpp"
#include "VoiceHandler.hpp"
#include "ButtonHandler.hpp"

TickType_t pxPreviousWakeTime = millis();
SystemCommandHandler *systemCommandHandler;
SettingsFactory *settingsFactory;
// BLEConfigurationHandler* bleConfigurationHandler;
// TcpHandler tcpHandler;
MotorHandler *motorHandler;
BatteryHandler *batteryHandler;
TaskHandle_t batteryTask;
TaskHandle_t httpsTask;

MotionHandler motionHandler;
VoiceHandler *voiceHandler;
ButtonHandler *buttonHandler = 0;
TaskHandle_t voiceTask;
#if BLUETOOTH_TCODE
BluetoothHandler *btHandler = 0;
#endif

#if WIFI_TCODE
Udphandler *udpHandler = 0;
WifiHandler wifi;
MDNSHandler mdnsHandler;
HTTPBase *webHandler = 0;
WebSocketBase *webSocketHandler = 0;
#endif

#if BUILD_TEMP
TemperatureHandler *temperatureHandler = 0;
#endif

#if BLE_TCODE
BLEHandler *bleHandler = 0;
#endif

#if BUILD_DISPLAY
DisplayHandler *displayHandler;
TaskHandle_t displayTask;
// #if ISAAC_NEWTONGUE_BUILD
// 	TaskHandle_t animationTask;
// #endif
#endif
#if BUILD_TEMP
TaskHandle_t temperatureTask;
#endif
// This has issues running with the webserver.
// OTAHandler otaHandler;
bool setupSucceeded = false;
bool restarting = false;

String serialData;
char commandTCodeData[MAX_COMMAND];
char udpData[MAX_COMMAND];
char webSocketData[MAX_COMMAND];
#if BLE_TCODE
char bleData[MAX_COMMAND];
#endif
#if BLUETOOTH_TCODE
String bluetoothData;
#endif
char movement[MAX_COMMAND];
ButtonModel *buttonCommand = 0;
bool dStopped = false;
bool tcodeV2Recieved = false;
bool bluetoothEnabled = BLUETOOTH_ENABLED_DEFAULT;
bool bleEnabled = BLE_ENABLED_DEFAULT;

unsigned long bench[10];
unsigned long benchLast[10];
bool benchEnable = false;
bool benchEnableZero = false;
unsigned long benchThreshHold = 1300;

void benchStart(int benchNumber)
{
	if (benchEnable || (benchEnableZero && benchNumber == 0))
		bench[benchNumber] = micros();
}
void benchFinish(const char *systemUnderBench, int benchNumber)
{
	if (benchEnable || (benchEnableZero && benchNumber == 0))
	{
		unsigned long timeTaken = micros() - bench[benchNumber];
		if (timeTaken > benchThreshHold)
		{
			Serial.printf("%s:							%lu\n", systemUnderBench, timeTaken);
			bench[benchNumber] = 0;
			benchLast[benchNumber] = timeTaken;
		}
	}
}

void displayPrint(String text)
{
#if BUILD_DISPLAY
	displayHandler->println(text);
#endif
}

void TCodeCommandCallback(const char *in)
{

	if (systemCommandHandler->isCommand(in))
	{
		systemCommandHandler->process(in);
	}
	else
	{
#if BLUETOOTH_TCODE
		if (btHandler && btHandler->isConnected())
			btHandler->CommandCallback(in);
#endif
#if BLE_TCODE
		if (bleHandler && bleHandler->isConnected())
			bleHandler->CommandCallback(in);
#endif
#if WIFI_TCODE
		if (webSocketHandler)
			webSocketHandler->CommandCallback(in);
		if (udpHandler)
			udpHandler->CommandCallback(in);
#endif
		if (Serial)
			Serial.println(in);
	}
}
void TCodePassthroughCommandCallback(const char *in)
{
	if (systemCommandHandler->isCommand(in))
	{
		// This seems wrong but since we are only calling this from one place its fine for now.
		char temp[strlen(in) + 2];
		temp[0] = {0};
		strcpy(temp, in);
		strcat(temp, "\n");
//////////////////////////////////////////////////////////////////////////////////////
#if BLUETOOTH_TCODE
		if (btHandler && btHandler->isConnected())
			btHandler->CommandCallback(temp);
#endif
#if BLE_TCODE

#endif
#if WIFI_TCODE
		if (webSocketHandler)
			webSocketHandler->CommandCallback(temp);
		if (udpHandler)
			udpHandler->CommandCallback(temp);
#endif
		if (Serial)
			Serial.println(temp);
	}
}
void profileChangeCallback(uint8_t profile)
{
}
void logCallBack(const char *input, size_t length, LogLevel level)
{
#if WIFI_TCODE
	// if(webSocketHandler) {
	// 	webSocketHandler->sendDebug(in, level);
	// }
#endif
}
#if BUILD_TEMP
void tempChangeCallBack(TemperatureType type, const char *message, float temp)
{
#if WIFI_TCODE
	if (webSocketHandler)
	{
		if (strpbrk(message, "{") == nullptr)
		{
			webSocketHandler->sendCommand(message);
		}
		else
		{
			if (type == TemperatureType::SLEEVE)
			{
				webSocketHandler->sendCommand("sleeveTempStatus", message);
			}
			else
			{
				webSocketHandler->sendCommand("internalTempStatus", message);
			}
		}
	}
#endif
#if BUILD_DISPLAY
	if (displayHandler)
	{
		if (type == TemperatureType::SLEEVE)
		{
			displayHandler->setSleeveTemp(temp);
		}
		else
		{
			displayHandler->setInternalTemp(temp);
		}
	}
#endif
}
void tempStateChangeCallBack(TemperatureType type, const char *state)
{
#if BUILD_DISPLAY
	if (displayHandler)
	{
		if (type == TemperatureType::SLEEVE)
		{
			LogHandler::verbose(TagHandler::Main, "tempStateChangeCallBack heat: %s", state);
			displayHandler->setHeateState(state);
			if (temperatureHandler)
				displayHandler->setHeateStateShort(temperatureHandler->getShortSleeveControlStatus(state));
		}
		else
		{
			LogHandler::verbose(TagHandler::Main, "tempStateChangeCallBack fan: %s", state);
			displayHandler->setFanState(state);
		}
	}
#endif
}
#endif
#if WIFI_TCODE
void startNetworking(const bool &apMode, const int &port, const int &udpPort, const char *hostname, const char *friendlyName)
{
	if((MODULE_CURRENT != ModuleType::WROOM32 || (!bluetoothEnabled && !bleEnabled)) && !webHandler) 
	{
		displayPrint("Starting web server");
	#if !SECURE_WEB
		webHandler = new WebHandler();
		webSocketHandler = new WebSocketHandler();
	#else
		LogHandler::debug(TagHandler::Main, "Start https task");
		webHandler = new HTTPSHandler();
		webSocketHandler = new SecureWebSocketHandler();
		auto httpsStatus = xTaskCreateUniversal(
			HTTPSHandler::startLoop, /* Function to implement the task */
			"HTTPSTask",			 /* Name of the task */
			8192 * 3,				 /* Stack size in words */
			webHandler,				 /* Task input parameter */
			3,						 /* Priority of the task */
			&httpsTask,				 /* Task handle. */
			-1);					 /* Core where the task should run */
		if (httpsStatus != pdPASS)
		{
			LogHandler::error(TagHandler::Main, "Could not start https task.");
		}
	#endif
		webHandler->setup(port, webSocketHandler, apMode);
    	LogHandler::debug(TagHandler::Main, "Web DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
	} else {
		displayPrint("WebServer disabled");
		LogHandler::info(TagHandler::Main, "WebServer disabled due to bluetooth and chip model");
	}
	if (!apMode) {// mdns breaks apmode?
		mdnsHandler.setup(hostname, friendlyName, port, udpPort);
    	LogHandler::debug(TagHandler::Main, "MDNS DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
	}
}
#endif

void startBLEConfig()
{
	// Disabled. Android Application needs maintenance
	// if(!bleConfigurationHandler) {
	// 	displayPrint("Starting BLE config");
	// 	bleConfigurationHandler = new BLEConfigurationHandler();
	// 	bleConfigurationHandler->setup();
	// }
}

#if BLE_TCODE
void startBLETCode()
{
	if (!bleHandler)
	{
		displayPrint("Starting BLE");
		bleHandler = new BLEHandler();
		bleHandler->setup();
	}
}
#endif

#if BLUETOOTH_TCODE
void startBlueTooth()
{
	if (bluetoothEnabled && !btHandler)
	{
		displayPrint("Starting Bluetooth serial");
		btHandler = new BluetoothHandler();
		btHandler->setup();
	}
}
#endif

#if WIFI_TCODE
bool startUDPTCode(int port)
{
	if (!udpHandler)
	{
		displayPrint("Starting UDP");
		udpHandler = new Udphandler();
		if (!udpHandler->setup(port))
			return false;
	}
	return true;
}
#endif

void startConfigMode(const int &webPort, const int &udpPort, const char *hostname, const char *friendlyName)
{
#if WIFI_TCODE
	SettingsHandler::apMode = true;
	LogHandler::info(TagHandler::Main, "Starting in APMode");
	displayPrint("Starting in APMode");
	if (wifi.startAp())
	{
		LogHandler::info(TagHandler::Main, "APMode started");
		displayPrint("APMode started");
		startNetworking(SettingsHandler::apMode, webPort, udpPort, hostname, friendlyName);
	}
	else
	{
		LogHandler::error(TagHandler::Main, "APMode start failed");
		displayPrint("APMode start failed");
	}
#endif

#if BLE_TCODE || BLUETOOTH_TCODE
	if(bleEnabled || bluetoothEnabled) {
		startBLEConfig();
	}
#endif
}

#if WIFI_TCODE
void wifiStatusCallBack(WiFiStatus status, WiFiReason reason)
{
	if (status == WiFiStatus::CONNECTED)
	{
		LogHandler::debug(TagHandler::Main, "wifiStatusCallBack WiFiStatus::CONNECTED");
		if (reason == WiFiReason::AP_MODE)
		{
			LogHandler::debug(TagHandler::Main, "wifiStatusCallBack WiFiReason::AP_MODE");
			// if(bleConfigurationHandler)
			//   bleConfigurationHandler->stop(); // If a client connects to the ap stop the BLE to save memory.
		}
	}
	else if(status == WiFiStatus::DISCONNECTED)
	{
		// wifi.dispose();
		// startApMode();
		LogHandler::debug(TagHandler::Main, "wifiStatusCallBack Not connected");
		if (reason == WiFiReason::NO_AP || reason == WiFiReason::UNKNOWN)
		{
			LogHandler::debug(TagHandler::Main, "wifiStatusCallBack WiFiReason::NO_AP || WiFiReason::UNKNOWN");
			startConfigMode(
				settingsFactory->getWebServerPort(),
				settingsFactory->getUdpServerPort(),
				settingsFactory->getHostname(),
				settingsFactory->getFriendlyName());
		}
		else if (reason == WiFiReason::AUTH)
		{
			LogHandler::debug(TagHandler::Main, "wifiStatusCallBack WiFiReason::AUTH");
			LogHandler::warning(TagHandler::Main, "Connection auth failed: Resetting wifi password and restarting");
			settingsFactory->defaultValue(WIFI_PASS_SETTING);
			ESP.restart();
		}
		else if (reason == WiFiReason::AP_MODE)
		{
			LogHandler::debug(TagHandler::Main, "wifiStatusCallBack WiFiReason::AP_MODE");
			// #ifdef !ESP32_DA
			// if(bleConfigurationHandler)
			// 	bleConfigurationHandler->setup();
			// #endif
		}
	} 
	else if(status == WiFiStatus::IP) 
	{
#if BUILD_DISPLAY
		String ipaddress = wifi.ip().toString();
		displayPrint("Connected IP: " + ipaddress);
		displayHandler->setLocalIPAddress(wifi.ip());
#endif
	}
}
#endif

void batteryVoltageCallback(float capacityRemainingPercentage, float capacityRemaining, float voltage, float temperature)
{
#if BUILD_DISPLAY
	if (displayHandler)
	{
		displayHandler->setBatteryInformation(capacityRemainingPercentage, voltage, temperature);
	}
#endif
#if WIFI_TCODE
	if (webSocketHandler)
	{
		String statusJson("{\"batteryCapacityRemaining\":\"" + String(capacityRemaining) + "\", \"batteryCapacityRemainingPercentage\":\"" + String(capacityRemainingPercentage) + "\", \"batteryVoltage\":\"" + String(voltage) + "\", \"batteryTemperature\":\"" + String(temperature) + "\"}");
		webSocketHandler->sendCommand("batteryStatus", statusJson.c_str());
	}
#endif
}

void settingChangeCallback(const SettingProfile &profile, const char *settingThatChanged)
{
	LogHandler::verbose(TagHandler::Main, "settingChangeCallback: %s", settingThatChanged);
	if (profile == SettingProfile::System)
	{
		if (!strcmp(settingThatChanged, LOG_LEVEL_SETTING))
		{
			LogHandler::setLogLevel(settingsFactory->getLogLevel());
		}
		else if (!strcmp(settingThatChanged, LOG_INCLUDETAGS))
		{
			LogHandler::setIncludes(settingsFactory->getLogIncludes());
		}
		else if (!strcmp(settingThatChanged, LOG_EXCLUDETAGS))
		{
			LogHandler::setExcludes(settingsFactory->getLogExcludes());
		}
	}
	else if (profile == SettingProfile::MotionProfile)
	{
		if (strcmp(settingThatChanged, MOTION_PROFILE_SELECTED_INDEX) == 0 || strcmp(settingThatChanged, MOTION_PROFILES) == 0) {
			motionHandler.setMotionChannels(SettingsHandler::getMotionChannels());
		//} else if(strcmp(settingThatChanged, "motionChannels") == 0) {
		// 	motionHandler.setMotionChannels(SettingsHandler::getGetMotionChannels()());
		} else if (strcmp(settingThatChanged, MOTION_ENABLED) == 0) {
			LogHandler::verbose(TagHandler::Main, "MOTION_ENABLED: %d", SettingsHandler::getMotionEnabled());
			motionHandler.setEnabled(SettingsHandler::getMotionEnabled());
		}
		// else if(strcmp(settingThatChanged, "motionAmplitudeGlobal") == 0)
		// 	motionHandler.setAmplitude(SettingsHandler::getGetMotionAmplitudeGlobal()());
		// else if(strcmp(settingThatChanged, "motionOffsetGlobal") == 0)
		// 	motionHandler.setOffset(SettingsHandler::getGetMotionOffsetGlobal()());
		// else if(strcmp(settingThatChanged, "motionPeriodGlobal") == 0)
		// 	motionHandler.setPeriod(SettingsHandler::getGetMotionPeriodGlobal()());
		// else if(strcmp(settingThatChanged, "motionUpdateGlobal") == 0)
		// 	motionHandler.setUpdate(SettingsHandler::getGetMotionUpdateGlobal()());
		// else if(strcmp(settingThatChanged, "motionPhaseGlobal") == 0)
		// 	motionHandler.setPhase(SettingsHandler::getGetMotionPhaseGlobal()());
		// else if(strcmp(settingThatChanged, "motionReversedGlobal") == 0)
		// 	motionHandler.setReverse(SettingsHandler::getGetMotionReversedGlobal()());
		// else if(strcmp(settingThatChanged, "motionAmplitudeGlobalRandom") == 0)
		// 	motionHandler.setAmplitudeRandom(SettingsHandler::getGetMotionAmplitudeGlobalRandom()());
		// else if(strcmp(settingThatChanged, "motionAmplitudeGlobalRandomMin") == 0)
		// 	motionHandler.setAmplitudeRandomMin(SettingsHandler::getGetMotionAmplitudeGlobalRandomMin()());
		// else if(strcmp(settingThatChanged, "motionAmplitudeGlobalRandomMax") == 0)
		// 	motionHandler.setAmplitudeRandomMax(SettingsHandler::getGetMotionAmplitudeGlobalRandomMax()());
		// else if(strcmp(settingThatChanged, "motionPeriodGlobalRandom") == 0)
		// 	motionHandler.setPeriodRandom(SettingsHandler::getGetMotionPeriodGlobalRandom()());
		// else if(strcmp(settingThatChanged, "motionPeriodGlobalRandomMin") == 0)
		// 	motionHandler.setPeriodRandomMin(SettingsHandler::getGetMotionPeriodGlobalRandomMin()());
		// else if(strcmp(settingThatChanged, "motionPeriodGlobalRandomMax") == 0)
		// 	motionHandler.setPeriodRandomMax(SettingsHandler::getGetMotionPeriodGlobalRandomMax()());
		// else if(strcmp(settingThatChanged, "motionOffsetGlobalRandom") == 0)
		// 	motionHandler.setOffsetRandom(SettingsHandler::getGetMotionOffsetGlobalRandom()());
		// else if(strcmp(settingThatChanged, "motionOffsetGlobalRandomMin") == 0)
		// 	motionHandler.setOffsetRandomMin(SettingsHandler::getGetMotionOffsetGlobalRandomMin()());
		// else if(strcmp(settingThatChanged, "motionOffsetGlobalRandomMax") == 0)
		// 	motionHandler.setOffsetRandomMax(SettingsHandler::getGetMotionOffsetGlobalRandomMax()());
		// else if(strcmp(settingThatChanged, "motionRandomChangeMin") == 0)
		// 	motionHandler.setMotionRandomChangeMin(SettingsHandler::getGetMotionRandomChangeMin()());
		// else if(strcmp(settingThatChanged, "motionRandomChangeMax") == 0)
		// 	motionHandler.setMotionRandomChangeMax(SettingsHandler::getGetMotionRandomChangeMax()());
	}
	else if (voiceHandler && profile == SettingProfile::Voice)
	{
		if (strcmp(settingThatChanged, "voiceMuted") == 0)
		{
			voiceHandler->setMuteMode(settingsFactory->getVoiceMuted());
		}
		else if (strcmp(settingThatChanged, "voiceVolume") == 0)
		{
			voiceHandler->setVolume(settingsFactory->getVoiceVolume());
		}
		else if (strcmp(settingThatChanged, "voiceWakeTime") == 0)
		{
			voiceHandler->setWakeTime(settingsFactory->getVoiceWakeTime());
		}
	}
	else if (buttonHandler && profile == SettingProfile::Button)
	{
		if (strcmp(settingThatChanged, "bootButtonCommand") == 0)
			buttonHandler->updateBootButtonCommand(settingsFactory->getBootButtonCommand());
		else if (strcmp(settingThatChanged, "analogButtonCommands") == 0)
		{
			buttonHandler->updateAnalogButtonCommands(SettingsHandler::getButtonSets());
		}
		else if (strcmp(settingThatChanged, "buttonAnalogDebounce") == 0)
		{
			buttonHandler->updateAnalogDebounce(settingsFactory->getButtonAnalogDebounce());
		}
	}
	else if (profile == SettingProfile::ChannelRanges)
	{ // TODO add channe; specific updates when moving to its own save...maybe...
		motionHandler.updateChannelRanges();
	}
}
void loadI2CModules(bool displayEnabled, bool batteryEnabled, bool voiceEnabled)
{
#if BUILD_DISPLAY
	if (displayEnabled)
	{
		LogHandler::debug(TagHandler::Main, "Start Display task");
		auto displayStatus = xTaskCreatePinnedToCore(
			DisplayHandler::startLoop,	  /* Function to implement the task */
			"DisplayTask",				  /* Name of the task */
			configMINIMAL_STACK_SIZE * 4, /* Stack size in words used to be 5000 */
			displayHandler,				  /* Task input parameter */
			1,							  /* Priority of the task */
			&displayTask,				  /* Task handle. */
			APP_CPU_NUM);				  /* Core where the task should run */
		if (displayStatus != pdPASS)
		{
			LogHandler::error(TagHandler::Main, "Could not start display task.");
		}
	}
#endif
	if (batteryEnabled)
	{
		batteryHandler = new BatteryHandler();
		if (batteryHandler->setup())
		{
			LogHandler::debug(TagHandler::Main, "Start Battery task");
			auto batteryStatus = xTaskCreatePinnedToCore(
				BatteryHandler::startLoop, /* Function to implement the task */
				"BatteryTask",			   /* Name of the task */
				configMINIMAL_STACK_SIZE,  /* Stack size in words used to be 4028 */
				batteryHandler,			   /* Task input parameter */
				1,						   /* Priority of the task */
				&batteryTask,			   /* Task handle. */
				APP_CPU_NUM);			   /* Core where the task should run */
			if (batteryStatus != pdPASS)
			{
				LogHandler::error(TagHandler::Main, "Could not start battery task.");
			}
			batteryHandler->setMessageCallback(batteryVoltageCallback);
		}
	}
	if (voiceEnabled)
	{
		voiceHandler = new VoiceHandler();
		voiceHandler->setMessageCallback(TCodeCommandCallback);
		if (voiceHandler->setup())
		{
			LogHandler::debug(TagHandler::Main, "Start Voice task");
			auto voiceStatus = xTaskCreatePinnedToCore(
				VoiceHandler::startLoop,  /* Function to implement the task */
				"VoiceTask",			  /* Name of the task */
				configMINIMAL_STACK_SIZE, /* Stack size in words used to be 4028 */
				voiceHandler,			  /* Task input parameter */
				1,						  /* Priority of the task */
				&voiceTask,				  /* Task handle. */
				APP_CPU_NUM);			  /* Core where the task should run */
			if (voiceStatus != pdPASS)
			{
				LogHandler::error(TagHandler::Main, "Could not start voice task.");
			}
		}
	}
}
void setup()
{

	// setCpuFrequencyMhz(240);

	// see if we can use the onboard led for status
	// https://github.com/kriswiner/ESP32/blob/master/PWM/ledcWrite_demo_ESP32.ino
	// digitalWrite(5, LOW);// Turn off on-board blue led

	Serial.begin(115200);
    Serial.printf("Startup DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));

	LogHandler::setLogLevel(LogLevel::INFO);
	LogHandler::setMessageCallback(logCallBack);

	Serial.println();
	LogHandler::info(TagHandler::Main, "Firmware version: %s", FIRMWARE_VERSION_NAME);
	// LogHandler::info(TagHandler::Main, "Esp arduino version: %s", ESP_ARDUINO_VERSION_STR);
	LogHandler::info(TagHandler::Main, "ESP IDF version: %s", esp_get_idf_version());
	uint32_t chipId = 0;
	for (int i = 0; i < 17; i = i + 8)
	{
		chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
	LogHandler::info(TagHandler::Main, "ESP32 Chip model = %s Rev %d", ESP.getChipModel(), ESP.getChipRevision());
	LogHandler::info(TagHandler::Main, "This chip has %d cores", ESP.getChipCores());
	LogHandler::info(TagHandler::Main, "Chip ID: %u", chipId);
	Serial.println();

	// esp_log_level_set("*", ESP_LOG_VERBOSE);
	// LogHandler::debug("main", "this is verbose");
	// LogHandler::debug("main", "this is debug");
	// LogHandler::info("main", "this is info");
	// LogHandler::warning("main", "this is warning");
	// LogHandler::error("main", "this is error");

	if (!LittleFS.begin(true))
	{
		LogHandler::error(TagHandler::Main, "An Error has occurred while mounting LittleFS");
		setupSucceeded = false;
		return;
	}
    LogHandler::debug(TagHandler::Main, "LittleFS DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));

	// LogHandler::setLogLevel(LogLevel::DEBUG);
	settingsFactory = SettingsFactory::getInstance();
	settingsFactory->setMessageCallback(settingChangeCallback);
	if (!settingsFactory->init())
	{
		LogHandler::error(TagHandler::Main, "Failed to load settings...");
		return;
	}
    LogHandler::debug(TagHandler::Main, "Settings factory  DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
	// LogHandler::setLogLevel(LogLevel::DEBUG);

	const PinMap *pinMap = settingsFactory->getPins();

	SettingsHandler::init();
	SettingsHandler::setMessageCallback(settingChangeCallback);
    LogHandler::debug(TagHandler::Main, "Settings handler DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));

#if BLE_TCODE
	settingsFactory->getValue(BLE_ENABLED, bleEnabled);
	
	//bleEnabled = true;
#endif
#if BLUETOOTH_TCODE
	settingsFactory->getValue(BLUETOOTH_ENABLED, bluetoothEnabled);
	
	//bluetoothEnabled = true;
#endif

#if WIFI_TCODE
	if ((!bluetoothEnabled && !bleEnabled) || COEXIST)
		wifi.setWiFiStatusCallback(wifiStatusCallBack);
#endif

	// Get ConfigurationSettings
	bool fanControlEnabled = FAN_CONTROL_ENABLED_DEFAULT;
	settingsFactory->getValue(FAN_CONTROL_ENABLED, fanControlEnabled);

	// Cached (Requires reboot)
	MotorType motorType;
	BoardType boardType;
	DeviceType deviceType;
	settingsFactory->getValue(MOTOR_TYPE_SETTING, motorType);
	settingsFactory->getValue(BOARD_TYPE_SETTING, boardType);
	settingsFactory->getValue(DEVICE_TYPE, deviceType);
	SettingsHandler::channelMap.init(settingsFactory->getTcodeVersion(), motorType, deviceType);

	// bool lubeEnabled;
	// bool feedbackTwist;
	// bool analogTwist;
	bool bootButtonEnabled;
	bool buttonSetsEnabled;

	// settingsFactory->getValue(FEEDBACK_TWIST, feedbackTwist);
	// settingsFactory->getValue(ANALOG_TWIST, analogTwist);
	settingsFactory->getValue(BOOT_BUTTON_ENABLED, bootButtonEnabled);
	settingsFactory->getValue(BUTTON_SETS_ENABLED, buttonSetsEnabled);

	bool batteryLevelEnabled;
	bool voiceEnabled;
	settingsFactory->getValue(BATTERY_LEVEL_ENABLED, batteryLevelEnabled);
	settingsFactory->getValue(VOICE_ENABLED, voiceEnabled);

	bool displayEnabled;
	settingsFactory->getValue(DISPLAY_ENABLED, displayEnabled);
	char Display_I2C_AddressString[DISPLAY_I2C_ADDRESS_LEN] = {0};
	settingsFactory->getValue(DISPLAY_I2C_ADDRESS, Display_I2C_AddressString, DISPLAY_I2C_ADDRESS_LEN);
	int Display_I2C_Address = (int)strtol(Display_I2C_AddressString, NULL, 0);

	systemCommandHandler = new SystemCommandHandler();
	systemCommandHandler->registerExternalCommandCallback(TCodePassthroughCommandCallback);
    LogHandler::debug(TagHandler::Main, "System command handler DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));

#if MOTOR_TYPE == 0
	if (settingsFactory->getTcodeVersion() == TCodeVersion::v0_3)
	{
		motorHandler = new ServoHandler0_3();
	}
	else if (settingsFactory->getTcodeVersion() == TCodeVersion::v0_4)
	{
		motorHandler = new ServoHandler0_4();
	}
#if !DEBUG_BUILD && TCODE_V2
	// else if(settingsFactory->getTcodeVersion() == TCodeVersion::v0_2)
	// 	motorHandler = new ServoHandler0_2();
#endif
	else
	{
		LogHandler::error(TagHandler::Main, "Invalid TCode version: %ld", settingsFactory->getTcodeVersion());
		return; // TODO: this stops apmode and not what we want
	}
#elif MOTOR_TYPE == 1
	if (settingsFactory->getTcodeVersion() == TCodeVersion::v0_3)
	{
		motorHandler = new BLDCHandler0_3();
	}
	else if (settingsFactory->getTcodeVersion() == TCodeVersion::v0_4)
	{
		motorHandler = new BLDCHandler0_4();
	}
#else
	LogHandler::error(TagHandler::Main, "Invalid motor type defined!");
	return;
#endif

	motorHandler->setMessageCallback(TCodeCommandCallback);
    LogHandler::debug(TagHandler::Main, "Motor handler DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
	// SystemCommandHandler::registerOtherCommandCallback(TCodeCommandCallback);

#if BUILD_TEMP
	bool sleeveTempEnabled;
	bool internalTempEnabled;
	int8_t heaterChannel = pinMap->heaterChannel();
	int heaterFrequency = heaterChannel > -1 ? pinMap->getChannelFrequency(pinMap->heaterChannel()) : ESP_TIMER_FREQUENCY_DEFAULT;
	int heaterResolution;
	float heaterThreshold;
	int8_t caseFanChannel = pinMap->caseFanChannel();
	int caseFanFrequency = caseFanChannel > -1 ? pinMap->getChannelFrequency(pinMap->caseFanChannel()) : ESP_TIMER_FREQUENCY_DEFAULT;
	int caseFanResolution;
	settingsFactory->getValue(TEMP_SLEEVE_ENABLED, sleeveTempEnabled);
	settingsFactory->getValue(TEMP_INTERNAL_ENABLED, internalTempEnabled);
	settingsFactory->getValue(HEATER_RESOLUTION, heaterResolution);
	settingsFactory->getValue(HEATER_THRESHOLD, heaterThreshold);
	settingsFactory->getValue(CASE_FAN_RESOLUTION, caseFanResolution);
	if (sleeveTempEnabled || internalTempEnabled)
	{
		temperatureHandler = new TemperatureHandler();
		temperatureHandler->setup(internalTempEnabled,
								  sleeveTempEnabled,
								  pinMap->sleeveTemp(),
								  pinMap->internalTemp(),
								  pinMap->heater(),
								  heaterChannel,
								  pinMap->caseFan(),
								  caseFanChannel,
								  heaterFrequency,
								  heaterResolution,
								  fanControlEnabled,
								  caseFanFrequency,
								  caseFanResolution);
		temperatureHandler->setMessageCallback(tempChangeCallBack);
		temperatureHandler->setStateChangeCallback(tempStateChangeCallBack);
		LogHandler::debug(TagHandler::Main, "Start temperature task");
		auto tempStartStatus = xTaskCreatePinnedToCore(
			TemperatureHandler::startLoop, /* Function to implement the task */
			"TempTask",					   /* Name of the task */
			static_cast<uint16_t>(configMINIMAL_STACK_SIZE * 2.5),  /* Stack size in words used to be 5000 */
			temperatureHandler,			   /* Task input parameter */
			1,							   /* Priority of the task */
			&temperatureTask,			   /* Task handle. */
			APP_CPU_NUM);				   /* Core where the task should run */
		if (tempStartStatus != pdPASS)
		{
			LogHandler::error(TagHandler::Main, "Could not start temperature task.");
		}
    	LogHandler::debug(TagHandler::Main, "Temp DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
	}

#endif
#if BUILD_DISPLAY
	displayHandler = new DisplayHandler();
	if (displayEnabled)
	{
		displayHandler->setup(Display_I2C_Address, fanControlEnabled, pinMap->displayReset());
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
    LogHandler::debug(TagHandler::Main, "Display DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
#endif

#if BLE_TCODE
	if (bleEnabled)
	{
		startBLETCode();
	}
	else
	{
		BLEHandler::disable();
	}
    LogHandler::debug(TagHandler::Main, "BLE DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
#else
	esp_bt_controller_mem_release(ESP_BT_MODE_BTDM)
#endif
#if BLUETOOTH_TCODE
	if (bluetoothEnabled)
	{
		startBlueTooth();
	}
	else
	{
		BluetoothHandler::disable();
	}
    LogHandler::debug(TagHandler::Main, "Bluetooth DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
#else
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
#endif

#if BLE_TCODE || BLUETOOTH_TCODE
	if (WIFI_TCODE && !COEXIST && (bluetoothEnabled || bleEnabled))
	{
		WifiHandler::disable();
    	LogHandler::debug(TagHandler::Main, "Wifi disable DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
	}
#endif

#if WIFI_TCODE
	if ((!bluetoothEnabled && !bleEnabled) || COEXIST)
	{
		char ssid[SSID_LEN];
		char wifiPass[WIFI_PASS_LEN];
		bool staticIP;
		char localIP[IP_ADDRESS_LEN];
		char gateway[IP_ADDRESS_LEN];
		char subnet[IP_ADDRESS_LEN];
		char dns1[IP_ADDRESS_LEN];
		char dns2[IP_ADDRESS_LEN];

		settingsFactory->getValue(SSID_SETTING, ssid, SSID_LEN);
		settingsFactory->getValue(WIFI_PASS_SETTING, wifiPass, WIFI_PASS_LEN);
		settingsFactory->getValue(STATICIP, staticIP);
		settingsFactory->getValue(LOCALIP, localIP, IP_ADDRESS_LEN);
		settingsFactory->getValue(GATEWAY, gateway, IP_ADDRESS_LEN);
		settingsFactory->getValue(SUBNET, subnet, IP_ADDRESS_LEN);
		settingsFactory->getValue(DNS1, dns1, IP_ADDRESS_LEN);
		settingsFactory->getValue(DNS2, dns2, IP_ADDRESS_LEN);
		if (strcmp(wifiPass, WIFI_PASS_DONOTCHANGE_DEFAULT) != 0 && strlen(ssid))
		{
			displayPrint("Setting up wifi...");
			LogHandler::info(TagHandler::Main, "Setting up wifi...");
			displayPrint("Connecting to: ");
			LogHandler::info(TagHandler::Main, "Connecting to: %s", ssid);
			displayPrint(ssid);
			if (wifi.connect(ssid, wifiPass))
			{
// 				String ipaddress = wifi.ip().toString();
// 				displayPrint("Connected IP: " + ipaddress);
// 				LogHandler::info(TagHandler::Main, "Connected IP: %s", ipaddress.c_str());
// #if BUILD_DISPLAY
// 				displayHandler->setLocalIPAddress(wifi.ip());
// #endif
				if (!startUDPTCode(settingsFactory->getUdpServerPort()))
				{
					LogHandler::error(TagHandler::Main, "Error starting UDP server!");
					return;
				}
    			LogHandler::debug(TagHandler::Main, "UDP DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
				startNetworking(false,
						 settingsFactory->getWebServerPort(),
						 settingsFactory->getUdpServerPort(),
						 settingsFactory->getHostname(),
						 settingsFactory->getFriendlyName());
			}
		}
		else
		{
			startConfigMode(
				settingsFactory->getWebServerPort(),
				settingsFactory->getUdpServerPort(),
				settingsFactory->getHostname(),
				settingsFactory->getFriendlyName());
		}
	}
#endif
	// otaHandler.setup();
	displayPrint("Setting up motor");
	motorHandler->setup();
    LogHandler::debug(TagHandler::Main, "Motor DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
	motionHandler.setup(settingsFactory->getTcodeVersion());
    LogHandler::debug(TagHandler::Main, "Motion handler DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
	loadI2CModules(displayEnabled, batteryLevelEnabled, voiceEnabled);
    LogHandler::debug(TagHandler::Main, "I2C DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));

	if (bootButtonEnabled || buttonSetsEnabled)
	{
		buttonHandler = new ButtonHandler();
		buttonHandler->init(settingsFactory->getButtonAnalogDebounce(),
							settingsFactory->getBootButtonCommand(),
							SettingsHandler::buttonSets);
	}

	setupSucceeded = true;
	LogHandler::debug(TagHandler::Main, "Setup finished");
	SettingsHandler::printFree();
}

// Main loop functions/////////////////////////////////////////////////
void readTCode(String &tcode)
{
	if (motorHandler)
	{
		motorHandler->read(tcode);
		tcode.clear();
	}
}

void readTCode(char *tcode, int len)
{
	if (motorHandler)
	{
		motorHandler->read(tcode, len);
		tcode[0] = {0};
	}
}

void processButton()
{
	if (buttonHandler)
	{
		buttonHandler->read(buttonCommand);
		if (buttonCommand)
		{
			char command[MAX_COMMAND];
			systemCommandHandler->process(buttonCommand, command);
			if (strlen(command) > 0)
			{
				readTCode(command, strlen(command));
			}
		}
	}
}

void getTCodeInput()
{
	if (Serial.available() > 0)
	{
		serialData = Serial.readStringUntil('\n');
		if(!serialData.isEmpty())
			serialData += '\n';
	}
	else if (serialData.length())
	{
		serialData.clear();
	}
	if (systemCommandHandler)
	{
		systemCommandHandler->getTCode(commandTCodeData);
	}
#if BLUETOOTH_TCODE
	if (btHandler && btHandler->isConnected() && btHandler->available() > 0)
	{
		bluetoothData = btHandler->readStringUntil('\n');
	}
#endif
#if WIFI_TCODE
	if (webSocketHandler)
	{
		benchStart(1);
		webSocketHandler->getTCode(webSocketData);
		benchFinish("Websocket get", 1);
	}
	if (udpHandler)
	{
		benchStart(2);
		udpHandler->read(udpData);
		benchFinish("Udp get", 2);
	}
#endif
#if BLE_TCODE
	if (bleHandler)
	{
		bleHandler->read(bleData);
	}
#endif
}

void processCommand()
{
	// Read and process tcode $ and # commands
	if (serialData.length() > 0)
	{
		if (systemCommandHandler && systemCommandHandler->isCommand(serialData.c_str()))
		{
			// systemCommandHandler->process(serialData.c_str());
			readTCode(serialData);
		}
	}
#if BLUETOOTH_TCODE
	if (bluetoothData.length() > 0)
	{
		if (systemCommandHandler && systemCommandHandler->isCommand(bluetoothData.c_str()))
		{
			// systemCommandHandler->process(bluetoothData.c_str());
			readTCode(bluetoothData);
		}
	}
#endif
#if WIFI_TCODE
	else if (strlen(udpData) > 0 && systemCommandHandler && systemCommandHandler->isCommand(udpData))
	{
		// systemCommandHandler->process(udpData);
		readTCode(udpData, strlen(udpData));
	}
	else if (strlen(webSocketData) > 0 && systemCommandHandler && systemCommandHandler->isCommand(webSocketData))
	{
		// systemCommandHandler->process(webSocketData);
		readTCode(webSocketData, strlen(webSocketData));
	}
#endif
}

void processMotionHandlerMovement()
{
	motionHandler.getMovement(movement, MAX_COMMAND);
	if (strlen(movement) > 0)
	{
		LogHandler::verbose(TagHandler::MainLoop, "motion handler writing: %s", movement);
		readTCode(movement, strlen(movement));
	}
}

void loop()
{
	// if(setupSucceeded && SettingsHandler::getSaving()) {
	// 	motorHandler->execute();
	// 	vTaskDelay(250/portTICK_PERIOD_MS);
	// 	return;
	// }
	// LogHandler::verbose(TagHandler::MainLoop, "Enter loop ############################################");
	tcodeV2Recieved = false;
	benchStart(0);
	if (SettingsHandler::restartRequired > -1 || restarting)
	{ // check the flag here to determine if a restart is required
		if (SettingsHandler::restartRequired <= 0 && !restarting)
		{
			LogHandler::info(TagHandler::Main, "Restarting ESP");
			ESP.restart();
			restarting = true;
		}
		else
		{
			LogHandler::info(TagHandler::Main, "Restarting ESP in: %ld", SettingsHandler::restartRequired);
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		SettingsHandler::restartRequired--;
	}
#if BUILD_TEMP
	else if (temperatureHandler && temperatureHandler->isMaxTempTriggered())
	{
		char stop[7] = "DSTOP\n";
		readTCode(stop, 7);
		LogHandler::error(TagHandler::Main, "Internal temp has reached maximum user set. Main loop disabled! Restart system to enable the loop.");
		temperatureHandler->setFanState();
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
#endif
	else
	{
		if (setupSucceeded)
		{
			// otaHandler.handle();

			getTCodeInput(); // Must be executed first!

			processButton();

			processCommand();

			if (!SettingsHandler::getMotionPaused())
			{
				dStopped = false;
				benchStart(3);
				if (SettingsHandler::getMotionEnabled())
				{ // Motion overrides all other input
					processMotionHandlerMovement();
				}
				else if (strlen(commandTCodeData) > 0)
				{
					LogHandler::verbose(TagHandler::MainLoop, "system command tcode writing: %s", commandTCodeData);
					readTCode(commandTCodeData, strlen(commandTCodeData));
				}
				else if (serialData.length() > 0)
				{
					LogHandler::verbose(TagHandler::MainLoop, "serial writing: %s", serialData.c_str());
					readTCode(serialData);
#if WIFI_TCODE == 1
				}
				else if (strlen(webSocketData) > 0)
				{
					LogHandler::verbose(TagHandler::MainLoop, "webSocket writing: %s", webSocketData);
					readTCode(webSocketData, strlen(webSocketData));
				}
				else if (!SettingsHandler::apMode && strlen(udpData) > 0)
				{
					benchStart(6);
					LogHandler::verbose(TagHandler::MainLoop, "udp writing: %s", udpData);
					readTCode(udpData, strlen(udpData));
					benchFinish("Udp write", 6);
#endif
#if BLE_TCODE
				}
				else if (strlen(bleData) > 0)
				{
					LogHandler::verbose(TagHandler::MainLoop, "BLE writing: %s", bleData);
					readTCode(bleData, strlen(bleData));
#endif
#if BLUETOOTH_TCODE
				}
				else if (bluetoothData.length() > 0)
				{
					LogHandler::verbose(TagHandler::MainLoop, "bluetooth writing: %s", bluetoothData.c_str());
					readTCode(bluetoothData);
#endif
				}
				benchFinish("Input check", 3);
			}
			else if (!dStopped)
			{ // All motion is paused execute stop.
				// movement[0] = {0};
				// udpData[0] = {0};
				// webSocketData[0] = {0};
				// serialData.clear();
				char stop[7] = "DSTOP\n";
				readTCode(stop, 7);
				dStopped = true;
				tcodeV2Recieved = false;
#if BLE_TCODE
				// bleData = {0};
#endif
#if BLUETOOTH_TCODE
				// bluetoothData.clear();
#endif
			}

			benchStart(4);
			if (motorHandler)
				motorHandler->execute();
			benchFinish("Execute", 4);

#if BUILD_TEMP
			benchStart(5);
			if (temperatureHandler && temperatureHandler->isRunning())
			{
				temperatureHandler->setHeaterState();
				temperatureHandler->setFanState();
			}
			benchFinish("Temp check", 5);
#endif
		}
	}
	if (!setupSucceeded)
	{
		LogHandler::error(TagHandler::Main, "There was an issue in setup");
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	} else {
        //xTaskDelayUntil(&pxPreviousWakeTime, 10/portTICK_PERIOD_MS);
	}

	benchFinish("Main loop", 0);
}
