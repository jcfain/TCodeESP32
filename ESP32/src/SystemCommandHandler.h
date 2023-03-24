
#include <Arduino.h>
#include "SettingsHandler.h"
#include "utils.h"

class SystemCommandHandler {
    public: 
    static bool restartRequired;
    static bool process(const char* in) {
        
	if(strpbrk("$", in) != nullptr) {
		LogHandler::debug("main", "Enter TCode Command callback %s", in);
		// Commands with out values
		if(strstr(in, "$help") != nullptr) {
			printCommandHelp();
			return true;
		}
		if(strstr(in, "$save") != nullptr) {
            SettingsHandler::save();
			Serial.println("Settings saved!");
			return true;
		}
		if(strstr(in, "$defaultAll") != nullptr) {
            SettingsHandler::defaultAll();
			Serial.println("All settings reset to default! You must restart for the changes to take effect.");
			return true;
		}

		if(strstr(in, "$restart") != nullptr) {
            restart();
			return true;
		}

		// Commands with values
		int indexofDelim = getposition(in, strlen(in), ':');
		if(indexofDelim == -1) {
			LogHandler::error("TCodeCommandCallback", "Invalid command format: missing colon, correct format is $<command>:<value>");
		    return false;
		}
		char* value = substr(in, indexofDelim +1, strlen(in));
        if(!strlen(value)) {
			LogHandler::error("TCodeCommandCallback", "Invalid command format: missing value, correct format is $<command>:<value>");
		    return false;
        }
		if(strstr(in, "$wifi-ssid") != nullptr) {
			if(strlen(in) > sizeof(SettingsHandler::ssid)) {
				LogHandler::error("TCodeCommandCallback", "Invalid command: SSID max length is: %ld", strlen(SettingsHandler::ssid));
		        return false;
			}
			strcpy(SettingsHandler::ssid, value);
			Serial.printf("SSID changed to: %s\n", SettingsHandler::ssid);
			Serial.println("Execute the command '$save' to store the new value otherwise the value will reset upon reboot.");
			Serial.println("Restart is required after save");
			return true;
		}
		if(strstr(in, "$wifi-pass") != nullptr) {
			if(strlen(in) > sizeof(SettingsHandler::wifiPass)) {
				LogHandler::error("TCodeCommandCallback", "Invalid command: Wifi password max length is: %ld", strlen(SettingsHandler::wifiPass));
		        return false;
			}
			strcpy(SettingsHandler::wifiPass, value);
			Serial.printf("Wifi password changed to a password of %ld length\n", strlen(SettingsHandler::wifiPass));
			Serial.println("Execute the command '$save' to store the new value otherwise the value will reset upon reboot.");
			Serial.println("Restart is required after save");
			return true;
		}
		if(strstr(in, "$log-level") != nullptr) {
            char * pEnd;
            int valueInt = String(value).toInt();
            if(valueInt > (int)LogLevel::VERBOSE) {
				LogHandler::error("TCodeCommandCallback", "Invalid value: %ld. Valid log levels are 0-4", valueInt);
                return false;
            }
            SettingsHandler::logLevel = (LogLevel)valueInt;
			Serial.printf("Log level changed to: %ld\n", valueInt);
			Serial.println("Execute the command '$save' to store the new value otherwise the value will reset upon reboot.");
			return true;
		}
		LogHandler::error("TCodeCommandCallback", "Invalid command: %s", in);
		printCommandHelp();
		return false;
    }
	return false;
}

static void restart() {
    LogHandler::info("SystemCommandHandler", "Schedule device restart...");
    //ESP.restart();
    restartRequired = true;
    delay(5000);
}

private: 
static void printCommandHelp() {
	Serial.println("");
	Serial.println("");
	Serial.println("");
	Serial.println("");
	Serial.println("Available commands:");
	Serial.println("$help ---------------- Print this.");
	Serial.println("$save ---------------- Flush ALL settings to disk.");
	Serial.println("$defaultAll ---------- Reset ALL settings to default");
	Serial.println("$restart ------------- Restart the esp");
	Serial.println("$wifi-ssid:value ----- Change the wifi ssid.");
	Serial.println("$wifi-pass:value ----- Change the wifi password.");
	Serial.println("$log-level:value ----- Change the log level.");
	Serial.println("Log level values: ");
	Serial.println("         0 -- error");
	Serial.println("         1 -- info");
	Serial.println("         2 -- warning");
	Serial.println("         3 -- debug");
	Serial.println("         4 -- verbose");
}
};

bool SystemCommandHandler::restartRequired = false;