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

#include "utils.h"
#include <LittleFS.h>
#include <TCode.h>
#include "logging/LogHandler.h"
#include "settings/SettingsHandler.h"
#include "settings/FilesystemHandler.h"
#include "settings/OperatingModeHandler.h"
#include "messages/SystemCommandHandler.h"
#include "serial/SerialHandler.h"
#include "network/WifiHandler.h"
#include "sensors/TemperatureHandler.h"
#include "display/DisplayHandler.h"
#include "bluetooth/BluetoothHandler.h"
#include "tcode/MotorHandler.h"

#if MOTOR_TYPE == 0
#include "ServoHandler0_3.h"
#include "ServoHandler0_4.h"
#elif MOTOR_TYPE == 1
#include "BLDCHandler0_3.h"
#include "BLDCHandler0_4.h"
#endif

#include "network/UdpHandler.h"
#include "network/HTTP/HTTPBase.h"
#include "network/HTTP/WebSocketBase.h"
#if !SECURE_WEB
#include "network/WebHandler.h"
#else
#include "network/HTTP/HTTPSHandler.hpp"
#endif
#include "network/MDNSHandler.hpp"
// #include "OTAHandler.h"
#include "bluetooth/BLE/BLEHandler.hpp"

#include "network/WebSocketHandler.h"
#include "tasks/TaskHandler.h"

#include "sensors/BatteryHandler.h"
#include "motion/MotionHandler.hpp"
#include "sensors/VoiceHandler.hpp"
#include "sensors/ButtonHandler.hpp"

TickType_t pxPreviousWakeTime = millis();

// This has issues running with the webserver.
// OTAHandler otaHandler;
bool setupSucceeded = false;
bool restarting = false;
unsigned long restartAtMs = 0;

void displayPrint(const char* message);
void startNetworking(bool apMode, int webPort, int udpPort, const char* hostname, const char* friendlyName);
void ensureNetworkingAvailable();

extern WifiHandler wifi;

void startConfigMode(const int &webPort, const int &udpPort, const char *hostname, const char *friendlyName)
{
#if WIFI_TCODE
	SettingsHandler::apMode = true;
	displayPrint("Starting in APMode");

	char pass[WIFI_PASS_LEN];
	bool hidden = AP_MODE_HIDDEN_DEFAULT;
	uint8_t channel = AP_MODE_CHANNEL_DEFAULT;

	char subnet[IP_ADDRESS_LEN];
	char gateway[IP_ADDRESS_LEN];
	SettingsFactory *settingsFactory = SettingsFactory::getInstance();
	settingsFactory->getValue(AP_MODE_PASS, pass, WIFI_PASS_LEN);
	settingsFactory->getValue(AP_MODE_SUBNET, subnet, IP_ADDRESS_LEN);
	settingsFactory->getValue(AP_MODE_GATEWAY, gateway, IP_ADDRESS_LEN);
	settingsFactory->getValue(AP_MODE_HIDDEN, hidden);
	settingsFactory->getValue(AP_MODE_CHANNEL, channel);
	if (wifi.startAp(settingsFactory->getAPModeSSID(), pass, channel, hidden, settingsFactory->getAPModeIP(), subnet, gateway))
	{
		displayPrint("APMode started");
		startNetworking(SettingsHandler::apMode, webPort, udpPort, hostname, friendlyName);
	}
	else
	{
		displayPrint("APMode start failed");
	}
#endif
}

// These go on the stack (always allocate all necessary memory)
FilesystemHandler filesystemHandler;
SerialHandler serialHandler;
WifiHandler wifi;
UdpHandler udpHandler;
ButtonHandler buttonHandler;
BatteryHandler batteryHandler;
HTTPBase* webHandler = nullptr;
WebSocketBase* webSocketHandler = nullptr;
bool networkingStarted = false;

void displayPrint(const char* message)
{
	if (message && message[0] != '\0')
	{
		Serial.println(message);
		LogHandler::info(Tags::Main, "%s", message);
	}
}

void startNetworking(bool apMode, int webPort, int udpPort, const char* hostname, const char* friendlyName)
{
	(void)udpPort;
	(void)hostname;
	(void)friendlyName;

	if (networkingStarted)
	{
		return;
	}

	if (!webHandler)
	{
#if !SECURE_WEB
		webHandler = new WebHandler();
		webSocketHandler = new WebSocketHandler();
#else
		webHandler = new HTTPSHandler();
		webSocketHandler = new SecureWebSocketHandler();
#endif
	}

	if (webHandler && webSocketHandler)
	{
		webHandler->setup_http(webPort, webSocketHandler, apMode);
		networkingStarted = true;
		LogHandler::info(Tags::Main, "Configuration interfaces started on port %d (%s mode)", webPort, apMode ? "AP" : "STA");
	}
}

void ensureNetworkingAvailable()
{
#if WIFI_TCODE
	SettingsFactory* settingsFactory = SettingsFactory::getInstance();
	const int webPort = settingsFactory->getWebServerPort();
	const int udpPort = settingsFactory->getUdpServerPort();
	const char* hostname = settingsFactory->getHostname();
	const char* friendlyName = settingsFactory->getFriendlyName();

	char ssid[SSID_LEN] = { 0 };
	char pass[WIFI_PASS_LEN] = { 0 };
	settingsFactory->getValue(SSID_SETTING, ssid, sizeof(ssid));
	settingsFactory->getValue(WIFI_PASS_SETTING, pass, sizeof(pass));

	bool connectedToSta = false;
	if (strlen(ssid) > 0 && strcmp(ssid, SSID_DEFAULT) != 0)
	{
		displayPrint("Attempting WiFi STA connection");
		connectedToSta = wifi.connect(ssid, pass);
	}

	if (connectedToSta)
	{
		SettingsHandler::apMode = false;
		String staIp = wifi.ip().toString();
		LogHandler::info(Tags::Main, "WiFi connected, IP Address: %s", staIp.c_str());
		String staMsg = "WiFi connected: " + staIp;
		displayPrint(staMsg.c_str());
		startNetworking(false, webPort, udpPort, hostname, friendlyName);
		return;
	}

	displayPrint("Starting AP configuration mode");
	startConfigMode(webPort, udpPort, hostname, friendlyName);
	if (WifiHandler::apMode())
	{
		String apIp = WiFi.softAPIP().toString();
		LogHandler::info(Tags::Main, "Captive portal active, AP IP Address: %s", apIp.c_str());
		String apMsg = "Captive portal IP: " + apIp;
		displayPrint(apMsg.c_str());
	}

	if (!WifiHandler::apMode())
	{
		LogHandler::warning(Tags::Main, "Primary AP startup failed, retrying with defaults");
		if (wifi.startAp(AP_MODE_SSID_DEFAULT, AP_MODE_PASS_DEFAULT, AP_MODE_CHANNEL_DEFAULT, AP_MODE_HIDDEN_DEFAULT, AP_MODE_IP_DEFAULT, AP_MODE_SUBNET_DEFAULT, AP_MODE_GATEWAY_DEFAULT))
		{
			SettingsHandler::apMode = true;
			String apIp = WiFi.softAPIP().toString();
			LogHandler::info(Tags::Main, "Captive portal active, AP IP Address: %s", apIp.c_str());
			String apMsg = "Captive portal IP: " + apIp;
			displayPrint(apMsg.c_str());
			startNetworking(true, webPort, udpPort, hostname, friendlyName);
		}
	}
#endif
}

void setup()
{
	Serial.begin(115200);
	Serial.println("BOOT: setup entered");
	Serial.println();
	LogHandler::setLogLevel(LogLevel::INFO);
	LogHandler::info(Tags::Main, "Firmware version: %s", FIRMWARE_VERSION_NAME);
	uint32_t chipId = 0;
	for (int i = 0; i < 17; i = i + 8)
	{
		chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
	LogHandler::info(Tags::Main, "ESP32 Chip model = %s Rev %d", ESP.getChipModel(), ESP.getChipRevision());
	LogHandler::info(Tags::Main, "This chip has %d cores", ESP.getChipCores());
	LogHandler::info(Tags::Main, "Chip ID: %u", chipId);
	Serial.println();

	Serial.println("BOOT: FilesystemHandler::init");
	FilesystemHandler::init();
	Serial.println("BOOT: SettingsHandler::init");
	SettingsHandler::init();
	Serial.println("BOOT: SerialHandler::init");
	SerialHandler::init();

	TaskHandler::Manager& taskManager = TaskHandler::global();
	Serial.println("BOOT: Registering tasks");
	LogHandler::info(Tags::Main, "Initializing tasks");
	taskManager.critical(&serialHandler);     // Ensure serial commands always stay responsive
	LogHandler::info(Tags::Main, "Serial handler initialized");
	taskManager.critical(&buttonHandler);     // Command/input path
	LogHandler::info(Tags::Main, "Button handler initialized");
	taskManager.priority(&wifi);              // Network connection state machine
	LogHandler::info(Tags::Main, "WiFi handler initialized");
	taskManager.priority(&udpHandler);        // TCode ingress/egress transport
	LogHandler::info(Tags::Main, "UDP handler initialized");
	taskManager.auxiliary(&batteryHandler);   // Low-priority telemetry polling
	LogHandler::info(Tags::Main, "Battery handler initialized");
	// Handles advanced fuctions (motor, ota, wifi, etc)
	Serial.println("BOOT: OperatingModeHandler::init");
	OperatingModeHandler::init();
	LogHandler::info(Tags::Main, "Operating mode handler initialized");
	Serial.println("BOOT: taskManager.start");
	taskManager.start();
	LogHandler::info(Tags::Main, "Tasks started");
	Serial.println("BOOT: network bring-up");
	ensureNetworkingAvailable();
	Serial.println("BOOT: setup complete");
}

void loop()
{
	TaskHandler::global().update();

	if (SettingsHandler::restartRequired >= 0 && !restarting)
	{
		restarting = true;
		restartAtMs = millis() + (static_cast<unsigned long>(SettingsHandler::restartRequired) * 1000UL);
		LogHandler::info(Tags::Main, "Restart scheduled in %d second(s)", SettingsHandler::restartRequired);
	}

	if (restarting && millis() >= restartAtMs)
	{
		LogHandler::info(Tags::Main, "Restarting now");
		delay(50);
		ESP.restart();
	}
}
