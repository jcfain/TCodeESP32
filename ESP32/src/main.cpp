/* MIT License

Copyright (c) 2026 Jason C. Fain

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
#if BLUETOOTH_TCODE
#include "bluetooth/BluetoothHandler.h"
#endif
#include "TCode/MotorHandler.h"

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
#include <esp_bt.h>

#include "network/WebSocketHandler.h"
#include "tasks/TaskHandler.h"

#include "sensors/BatteryHandler.h"
#include "PowerHandler.h"
#include "Motion/MotionHandler.hpp"
#include "sensors/VoiceHandler.hpp"
#include "sensors/ButtonHandler.hpp"

TickType_t pxPreviousWakeTime = millis();

// This has issues running with the webserver.
// OTAHandler otaHandler;
bool setupSucceeded = false;
bool restarting = false;
unsigned long restartAtMs = 0;
bool networkingBringupAttempted = false;
const unsigned long NETWORK_BRINGUP_DELAY_MS = 3000;

// =====================================================================
// FreeRTOS task / thread layout
// ---------------------------------------------------------------------
// Core 0 (PRO_CPU)
//   * "motor"           — pinned, prio MOTOR_TASK_PRIORITY (19), stack
//                         MOTOR_TASK_STACK_SIZE. Owns motor setup() +
//                         control loop. Drains motorCmdQueue each tick.
//   * "servoWiggle"     — short-lived, spawned by identifyServo() to
//                         pulse one servo for visual ID (see
//                         MotorHandler0_3/0_4 and ServoHandler0_4).
//
// Core 1 (APP_CPU)
//   * Arduino loop()    — cooperative poll() of every TaskHandler::Task
//                         registered via TaskHandler::global().add().
//                         Includes: SerialHandler, ButtonHandler,
//                         WifiHandler, UdpHandler, BatteryHandler,
//                         PowerHandler, OperatingModeHandler, WebHandler.
//   * WiFi/IP/AsyncTCP  — internal SDK tasks created by Arduino-ESP32.
//   * AsyncWebServer    — internal task created by ESPAsyncWebServer.
//
// Inter-core communication
//   * motorCmdQueue     — xQueue used by feedMotorCommand() so any
//                         core/task can submit a TCode command without
//                         touching motor-state directly.
// =====================================================================

// --- Motor task configuration ---
// Motor control runs on a dedicated FreeRTOS task pinned to PRO_CPU (Core 0).
// Priority must stay BELOW the WiFi/pp driver tasks (~23) so the WiFi stack
// can process packets, handle WPA key rotation, and service TCP ACKs.
// A priority of 19 still gives the motor task higher priority than most
// application tasks while letting the WiFi stack preempt when needed.
static const uint32_t MOTOR_TASK_STACK_SIZE = 16384;
static const UBaseType_t MOTOR_TASK_PRIORITY = 19;
static const BaseType_t MOTOR_TASK_CORE = PRO_CPU_NUM;
static const uint32_t MOTOR_CMD_QUEUE_SIZE = 32;
static const uint32_t MOTOR_CMD_MAX_LEN = 128;

struct MotorCommand
{
	char data[MOTOR_CMD_MAX_LEN];
	size_t len;
};
static QueueHandle_t motorCmdQueue = nullptr;
static volatile bool motorSetupComplete = false;

void displayPrint(const char *message);
void startNetworking(bool apMode, int webPort, int udpPort, const char *hostname, const char *friendlyName);
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
PowerHandler powerHandler;
HTTPBase *webHandler = nullptr;
WebSocketBase *webSocketHandler = nullptr;
bool networkingStarted = false;

// Motor handler - instantiated and initialized regardless of WiFi state
#if MOTOR_TYPE == 0
ServoHandler0_3 motorHandlerV03;
ServoHandler0_4 motorHandlerV04;
#elif MOTOR_TYPE == 1
BLDCHandler0_3 motorHandlerV03;
BLDCHandler0_4 motorHandlerV04;
#endif
MotorHandler *motorHandler = nullptr;

void displayPrint(const char *message)
{
	if (message && message[0] != '\0')
	{
		Serial.println(message);
		LogHandler::info(Tags::Main, "%s", message);
	}
}

void startNetworking(bool apMode, int webPort, int udpPort, const char *hostname, const char *friendlyName)
{
	(void)udpPort;
	(void)hostname;
	(void)friendlyName;

	if (networkingStarted)
	{
		return;
	}

	// Release Bluetooth controller memory if BLE is compiled in but not
	// used.  This returns ~60 KB of contiguous internal DRAM to the heap,
	// which is critical for the AsyncTCP task stack allocation under
	// WiFi+BLE coexistence heap fragmentation.
#if BLE_TCODE && !defined(BLE_ACTIVE)
	esp_err_t btRel = esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);
	if (btRel == ESP_OK)
	{
		LogHandler::info(Tags::Main, "Released BT controller memory (free heap: %u, max block: %u)",
						 ESP.getFreeHeap(), ESP.getMaxAllocHeap());
	}
	else
	{
		LogHandler::debug(Tags::Main, "BT mem release: %s (may already be released)", esp_err_to_name(btRel));
	}
#endif

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
	SettingsFactory *settingsFactory = SettingsFactory::getInstance();
	const int webPort = settingsFactory->getWebServerPort();
	const int udpPort = settingsFactory->getUdpServerPort();
	const char *hostname = settingsFactory->getHostname();
	const char *friendlyName = settingsFactory->getFriendlyName();

	char ssid[SSID_LEN] = {0};
	char pass[WIFI_PASS_LEN] = {0};
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

// Dedicated motor control task â€“ runs on PRO_CPU so it is never blocked
// by WiFi / networking / web-server work that lives on APP_CPU.
static void motorTaskFunc(void *param)
{
	MotorHandler *handler = static_cast<MotorHandler *>(param);
	LogHandler::info(Tags::Main, "Motor task starting on core %d", xPortGetCoreID());

	// Run motor hardware setup on this core (sensor, driver, FOC init)
	handler->setup();
	motorSetupComplete = true;
	LogHandler::info(Tags::Main, "Motor setup complete on core %d, entering control loop", xPortGetCoreID());

	MotorCommand cmd;
	for (;;)
	{
		// Service any pending hot-reattach (#reapply-pwm) before draining
		// the command queue so re-attached pins immediately accept writes.
		handler->serviceReapply();

		// Drain any queued TCode commands before each control cycle
		while (xQueueReceive(motorCmdQueue, &cmd, 0) == pdTRUE)
		{
			handler->read(cmd.data, cmd.len);
		}

		// Execute motor control (sensor read â†’ FOC â†’ move)
		handler->execute();

		// Yield for 1 tick (~1 ms) so the IDLE task can feed the watchdog
		// and lower-priority tasks on this core can run.
		vTaskDelay(1);
	}
}

// Thread-safe: enqueue a TCode command for the motor task.
// Called from serial / network handlers on any core.
void feedMotorCommand(const char *cmd, size_t len)
{
	if (!motorCmdQueue || !cmd || len == 0)
		return;

	MotorCommand motorCmd;
	size_t copyLen = (len < MOTOR_CMD_MAX_LEN - 1) ? len : (MOTOR_CMD_MAX_LEN - 1);
	memcpy(motorCmd.data, cmd, copyLen);
	motorCmd.data[copyLen] = '\0';
	motorCmd.len = copyLen;

	// Non-blocking send â€“ if the queue is full the command is dropped.
	xQueueSend(motorCmdQueue, &motorCmd, 0);
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

	SettingsFactory *settingsFactory = SettingsFactory::getInstance();
	const TCodeVersion tcodeVersion = settingsFactory->getTcodeVersion();
#if MOTOR_TYPE == 0
	motorHandler = (tcodeVersion == TCodeVersion::v0_3) ? static_cast<MotorHandler *>(&motorHandlerV03) : static_cast<MotorHandler *>(&motorHandlerV04);
#elif MOTOR_TYPE == 1
	motorHandler = (tcodeVersion == TCodeVersion::v0_3) ? static_cast<MotorHandler *>(&motorHandlerV03) : static_cast<MotorHandler *>(&motorHandlerV04);
#endif
	MotorHandler::setActive(motorHandler);
	LogHandler::info(Tags::Main, "Selected motor handler for TCode version: %s", settingsFactory->getTcodeVersionString());

	Serial.println("BOOT: SerialHandler::init");
	SerialHandler::init();

	TaskHandler::Manager &taskManager = TaskHandler::global();
	Serial.println("BOOT: Registering tasks");
	LogHandler::info(Tags::Main, "Initializing tasks");
	taskManager.add(&serialHandler); // Serial TCode ingress
	LogHandler::info(Tags::Main, "Serial handler initialized");
	taskManager.add(&buttonHandler); // Button input
	LogHandler::info(Tags::Main, "Button handler initialized");
	taskManager.add(&wifi); // Network connection state machine
	LogHandler::info(Tags::Main, "WiFi handler initialized");
	taskManager.add(&udpHandler); // TCode ingress/egress transport
	LogHandler::info(Tags::Main, "UDP handler initialized");
	taskManager.add(&batteryHandler); // Telemetry polling
	LogHandler::info(Tags::Main, "Battery handler initialized");
	taskManager.add(&powerHandler); // Analog rail telemetry polling
	LogHandler::info(Tags::Main, "Power handler initialized");

	batteryHandler.setMessageCallback([](const float& capacityRemainingPercentage, const float& capacityRemaining, const float& voltage, const float& temperature)
		{
			if (!webSocketHandler)
				return;
			char payload[256] = { 0 };
			snprintf(payload, sizeof(payload), "{\"batteryCapacityRemainingPercentage\":%.2f,\"batteryCapacityRemaining\":%.2f,\"batteryVoltage\":%.3f,\"batteryTemperature\":%.2f}",
				capacityRemainingPercentage, capacityRemaining, voltage, temperature);
			webSocketHandler->sendCommand("batteryStatus", payload);
		});
	powerHandler.setMessageCallback([](const char* payload)
		{
			if (!webSocketHandler || !payload)
				return;
			webSocketHandler->sendCommand("powerStatus", payload);
		});
	// Handles advanced fuctions (motor, ota, wifi, etc)
	Serial.println("BOOT: OperatingModeHandler::init");
	OperatingModeHandler::init();
	LogHandler::info(Tags::Main, "Operating mode handler initialized");

	// Create the command queue used by feedMotorCommand() from any core
	motorCmdQueue = xQueueCreate(MOTOR_CMD_QUEUE_SIZE, sizeof(MotorCommand));

	// Launch motor control on a dedicated FreeRTOS task pinned to PRO_CPU.
	// Motor setup() + execute() both run on that core so there is zero
	// contention with WiFi / networking work on APP_CPU.
	Serial.println("BOOT: Starting motor task on PRO_CPU");
	if (motorHandler)
	{
		BaseType_t rc = xTaskCreatePinnedToCore(
			motorTaskFunc,
			"motor",
			MOTOR_TASK_STACK_SIZE,
			motorHandler,
			MOTOR_TASK_PRIORITY,
			nullptr,
			MOTOR_TASK_CORE);
		if (rc != pdPASS)
		{
			LogHandler::error(Tags::Main, "Failed to create motor task!");
		}
	}
	else
	{
		LogHandler::error(Tags::Main, "Motor handler not initialized â€“ skipping motor task");
	}

	LogHandler::info(Tags::Main, "Tasks registered");
	Serial.println("BOOT: setup complete");
}

void loop()
{
	// Cooperatively poll all communication / sensor tasks on APP_CPU (Core 1).
	// Motor control runs on its own FreeRTOS task (PRO_CPU) â€“ nothing to do here.
	TaskHandler::global().tasks().poll();

	if (!networkingBringupAttempted && millis() >= NETWORK_BRINGUP_DELAY_MS)
	{
		networkingBringupAttempted = true;
		Serial.println("BOOT: network bring-up");
		ensureNetworkingAvailable();
	}

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
