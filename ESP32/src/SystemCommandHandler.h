
#include <Arduino.h>
#include "SettingsHandler.h"
#include "utils.h"
#include "TagHandler.h"

class SystemCommandHandler {
public: 
    static bool restartRequired;
    static bool process(const char* in) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
		if(strpbrk("$", in) != nullptr) {
			LogHandler::debug(_TAG, "Enter TCode Command callback %s", in);
			// Commands with out values
			if(strstr(in, "$help") != nullptr) {
				printCommandHelp();
				xSemaphoreGive(xMutex);
				return true;
			}
			if(strstr(in, "$save") != nullptr) {
				SettingsHandler::save();
				Serial.println("Settings saved!");
				xSemaphoreGive(xMutex);
				return true;
			}
			if(strstr(in, "$defaultAll") != nullptr) {
				SettingsHandler::defaultAll();
				Serial.println("All settings reset to default! You must restart for the changes to take effect.");
				xSemaphoreGive(xMutex);
				return true;
			}

			if(strstr(in, "$restart") != nullptr) {
				restart();
				xSemaphoreGive(xMutex);
				return true;
			}

			if(strstr(in, "$clear-log-include") != nullptr) {
				LogHandler::clearIncludes();
				Serial.println("Tags cleared");
				xSemaphoreGive(xMutex);
				return true;
			}

			if(strstr(in, "$clear-log-exclude") != nullptr) {
				LogHandler::clearExcludes();
				Serial.println("Tags filters cleared");
				xSemaphoreGive(xMutex);
				return true;
			}
			
			// Commands with values
			int indexofDelim = getposition(in, strlen(in), ':');
			if(indexofDelim == -1) {
				Serial.println("Invalid command format: missing colon, correct format is $<command>:<value>");
				xSemaphoreGive(xMutex);
				return false;
			}
			const char* value = substr(in, indexofDelim +1, strlen(in));
			if(!strlen(value)) {
				Serial.println("Invalid command format: missing value, correct format is $<command>:<value>");
				xSemaphoreGive(xMutex);
				return false;
			}
			if(strstr(in, "$wifi-ssid") != nullptr) {
				if(strlen(in) > sizeof(SettingsHandler::ssid)) {
					Serial.printf("Invalid command: SSID max length is: %ld\n", strlen(SettingsHandler::ssid));
					xSemaphoreGive(xMutex);
					return false;
				}
				strcpy(SettingsHandler::ssid, value);
				Serial.printf("SSID changed to: %s\n", SettingsHandler::ssid);
				Serial.println("Execute the command '$save' to store the new value otherwise the value will reset upon reboot.");
				Serial.println("Restart is required after save");
				xSemaphoreGive(xMutex);
				return true;
			}
			if(strstr(in, "$wifi-pass") != nullptr) {
				if(strlen(in) > sizeof(SettingsHandler::wifiPass)) {
					Serial.printf("Invalid command: Wifi password max length is: %ld\n", strlen(SettingsHandler::wifiPass));
					xSemaphoreGive(xMutex);
					return false;
				}
				strcpy(SettingsHandler::wifiPass, value);
				Serial.printf("Wifi password changed to a password of %ld length\n", strlen(SettingsHandler::wifiPass));
				Serial.println("Execute the command '$save' to store the new value otherwise the value will reset upon reboot.");
				Serial.println("Restart is required after save");
				xSemaphoreGive(xMutex);
				return true;
			}
			if(strstr(in, "$log-level") != nullptr) {
				char * pEnd;
				int valueInt = String(value).toInt();
				if(valueInt > (int)LogLevel::VERBOSE) {
					LogHandler::error(_TAG, "Invalid value: %ld. Valid log levels are 0-4", valueInt);
					xSemaphoreGive(xMutex);
					return false;
				}
				SettingsHandler::logLevel = (LogLevel)valueInt;
				Serial.printf("Log level changed to: %ld\n", valueInt);
				Serial.println("Execute the command '$save' to store the new value otherwise the value will reset upon reboot.");
				xSemaphoreGive(xMutex);
				return true;
			}
			if(strstr(in, "$add-log-include") != nullptr) {
				if(!LogHandler::addInclude(value)) {
					Serial.printf("Tag already exists: %s\n", value);
					xSemaphoreGive(xMutex);
					return false;
				}
				Serial.printf("Log level tag added: %s\n", value);
				Serial.println("Execute the command '$save' to store the new value otherwise the value will reset upon reboot.");
				xSemaphoreGive(xMutex);
				return true;
			}
			if(strstr(in, "$remove-log-include") != nullptr) {
				if(!LogHandler::removeInclude(value)) {
					Serial.printf("Tag did not exist: %s\n", value);
					xSemaphoreGive(xMutex);
					return false;
				}
				Serial.printf("Log level tag removed: %s\n", value);
				Serial.println("Execute the command '$save' to store the new value otherwise the value will reset upon reboot.");
				xSemaphoreGive(xMutex);
				return true;
			}
			if(strstr(in, "$add-log-exclude") != nullptr) {
				if(!LogHandler::addExclude(value)) {
					Serial.printf("Tag filter already exists: %s\n", value);
					xSemaphoreGive(xMutex);
					return false;
				}
				Serial.printf("Log level tag filter added: %s\n", value);
				Serial.println("Execute the command '$save' to store the new value otherwise the value will reset upon reboot.");
				xSemaphoreGive(xMutex);
				return true;
			}
			if(strstr(in, "$remove-log-exclude") != nullptr) {
				if(!LogHandler::removeExclude(value)) {
					Serial.printf("Tag filter did not exist: %s\n", value);
					xSemaphoreGive(xMutex);
					return false;
				}
				Serial.printf("Log level tag filter removed: %s\n", value);
				Serial.println("Execute the command '$save' to store the new value otherwise the value will reset upon reboot.");
				xSemaphoreGive(xMutex);
				return true;
			}
			Serial.printf("Invalid command: %s\n", in);
			printCommandHelp();
			xSemaphoreGive(xMutex);
			return false;
		}
		xSemaphoreGive(xMutex);
		return false;
	}

	static void restart() {
		Serial.println("Schedule device restart...");
		//ESP.restart();
		restartRequired = true;
		delay(1000);
	}

private: 
	static SemaphoreHandle_t xMutex;
	static const char* _TAG;
	static void printCommandHelp() {
		Serial.println("");
		Serial.println("");
		Serial.println("");
		Serial.println("");
		Serial.println("Available commands:");
		Serial.println("$help ----------------------- Print this.");
		Serial.println("$save ----------------------- Flush ALL settings to disk.");
		Serial.println("$defaultAll ----------------- Reset ALL settings to default");
		Serial.println("$restart -------------------- Restart the esp");
		Serial.println("$wifi-ssid:value ------------ Change the wifi ssid.");
		Serial.println("$wifi-pass:value ------------ Change the wifi password.");
		Serial.println("$log-level:value ------------ Change the log level.");
		Serial.println("    Log level values: ");
		Serial.println("         0 -- error");
		Serial.println("         1 -- info");
		Serial.println("         2 -- warning");
		Serial.println("         3 -- debug");
		Serial.println("         4 -- verbose");
		Serial.println("$add-log-include:value ------- Add a log tag to include");
		Serial.println("$remove-log-include:value ---- Remove a log tag to include");
		Serial.println("$clear-log-include ----------- Clear all included log tags");
		Serial.println("$add-log-exclude:value ------- Add a log tag to exclude");
		Serial.println("$remove-log-exclude:value ---- Remove a log tag to exclude");
		Serial.println("$clear-log-exclude ----------- Clear all excluded log tags");
	}
};

SemaphoreHandle_t SystemCommandHandler::xMutex = xSemaphoreCreateMutex();
const char* SystemCommandHandler::_TAG = TagHandler::SystemCommandHandler;
bool SystemCommandHandler::restartRequired = false;