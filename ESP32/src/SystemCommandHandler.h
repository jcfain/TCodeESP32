#pragma once

#include <Arduino.h>
#include "SettingsHandler.h"
#include "utils.h"
#include "TagHandler.h"

class SystemCommandHandler {
public: 
    static bool restartRequired;
    static bool process(const char* in) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
		if(strpbrk("$", in) != nullptr || strpbrk("#", in) != nullptr) {
			LogHandler::debug(_TAG, "Enter TCode Command callback %s", in);
			// Commands with out values
			if(isCommand(in, "#help")) {
				return execute([]() -> bool {
					printCommandHelp();
					return true;
				});
			}

			if(isCommand(in, "$save")) {
				return execute([]() -> bool {
					SettingsHandler::save();
					Serial.println("Settings saved!");
					return true;
				});
			}

			if(isCommand(in, "$defaultAll")) {
				return execute([]() -> bool {
					SettingsHandler::defaultAll();
					Serial.println("All settings reset to default!");
					return true;
				}, false, true);
			}

			if(isCommand(in, "#restart")) {
				return execute([]() -> bool {
					restart();
					return true;
				});
			}

			if(isCommand(in, "#clear-log-include")) {
				return execute([]() -> bool {
					LogHandler::clearIncludes();
					Serial.println("Tags cleared");
					return true;
				}, true);
			}

			if(isCommand(in, "#clear-log-exclude")) {
				return execute([]() -> bool {
					LogHandler::clearExcludes();
					Serial.println("Tags filters cleared");
					return true;
				}, true);
			}

			if(isCommand(in, "#motion-enable")) {
				return validateBool("Motion", true, SettingsHandler::getMotionEnabled(), [](bool value) -> bool {
					SettingsHandler::setMotionEnabled(value);
					return true;
				}, true);
			}
			if(isCommand(in, "#motion-disable")) {
				return validateBool("Motion", false, SettingsHandler::getMotionEnabled(), [](bool value) -> bool {
					SettingsHandler::setMotionEnabled(value);
					return true;
				}, true);
			}
			if(isCommand(in, "#motion-toggle")) {
				return execute([]() -> bool {
					SettingsHandler::setMotionEnabled(!SettingsHandler::getMotionEnabled());
					Serial.println(SettingsHandler::getMotionEnabled() ? "Motion enabled" : "Motion disabled");
					return true;
				}, true);
			}
			
			// if(isCommand(in, "$motion-period-random-on")) {
			// 	return validateBool("Motion amplitude random", true, SettingsHandler::getMotionPeriodGlobalRandom(), [](bool value) -> bool {
			// 		SettingsHandler::setMotionPeriodGlobalRandom(value);
			// 		return true;
			// 	}, true);
			// }
			// if(isCommand(in, "$motion-period-random-off")) {
			// 	return validateBool("Motion amplitude random", false, SettingsHandler::getMotionPeriodGlobalRandom(), [](bool value) -> bool {
			// 		SettingsHandler::setMotionPeriodGlobalRandom(value);
			// 		return true;
			// 	}, true);
			// }
			// if(isCommand(in, "$motion-amplitude-random-on")) {
			// 	return validateBool("Motion amplitude random", true, SettingsHandler::getMotionAmplitudeGlobalRandom(), [](bool value) -> bool {
			// 		SettingsHandler::setMotionAmplitudeGlobalRandom(value);
			// 		return true;
			// 	}, true);
			// }
			// if(isCommand(in, "$motion-amplitude-random-off")) {
			// 	return validateBool("Motion amplitude random", false, SettingsHandler::getMotionAmplitudeGlobalRandom(), [](bool value) -> bool {
			// 		SettingsHandler::setMotionAmplitudeGlobalRandom(value);
			// 		return true;
			// 	}, true);
			// }
			// if(isCommand(in, "$motion-offset-random-on")) {
			// 	return validateBool("Motion offset random", true, SettingsHandler::getMotionOffsetGlobalRandom(), [](bool value) -> bool {
			// 		SettingsHandler::setMotionOffsetGlobalRandom(value);
			// 		return true;
			// 	}, true);
			// }
			// if(isCommand(in, "$motion-offset-random-off")) {
			// 	return validateBool("Motion offset random", false, SettingsHandler::getMotionOffsetGlobalRandom(), [](bool value) -> bool {
			// 		SettingsHandler::setMotionOffsetGlobalRandom(value);
			// 		return true;
			// 	}, true);
			// }

			// Commands with values
			int indexofDelim = getposition(in, strlen(in), ':');
			if(indexofDelim == -1) {
				Serial.println("Invalid command format: missing colon, correct format is $|#<command>:<value>");
				xSemaphoreGive(xMutex);
				return false;
			}
			const char* value = substr(in, indexofDelim +1, strlen(in));
			if(!strlen(value)) {
				Serial.println("Invalid command format: missing value, correct format is $|#<command>:<value>");
				xSemaphoreGive(xMutex);
				return false;
			}
			LogHandler::verbose(_TAG, "Value command: %s", value);
			//Start value commands
			if(isCommand(in, "#wifi-ssid")) {
				return validateMaxLength("Wifi SSID", value, sizeof(SettingsHandler::ssid), false, [](const char* value) -> bool {
					strcpy(SettingsHandler::ssid, value);
					return true;
				}, true, true); 
			}
			if(isCommand(in, "#wifi-pass")) {
				return validateMaxLength("Wifi password", value, sizeof(SettingsHandler::wifiPass), true, [](const char* value) -> bool {
					strcpy(SettingsHandler::wifiPass, value);
					return true;
				}, true, true); 
			}
			if(isCommand(in, "#log-level")) {
				return executeValue(value, [](const char* value) -> bool {
					int valueInt = getInt(value);
					if(valueInt > (int)LogLevel::VERBOSE) {
						LogHandler::error(_TAG, "Invalid value: %ld. Valid log levels are 0-4", valueInt);
						return false;
					}
					SettingsHandler::logLevel = static_cast<LogLevel>(valueInt);
					LogHandler::setLogLevel(static_cast<LogLevel>(valueInt));
					Serial.printf("Log level changed to: %ld\n", valueInt);
					return true;
				}, true);
			}
			if(isCommand(in, "#add-log-include")) {
				return executeValue(value, [](const char* value) -> bool {
					if(!LogHandler::addInclude(value)) {
					Serial.printf("Tag already exists: %s\n", value);
						return false;
					}
					Serial.printf("Log level tag added: %s\n", value);
					return true;
				}, true);
			}
			if(isCommand(in, "#remove-log-include")) {
				return executeValue(value, [](const char* value) -> bool {
					if(!LogHandler::removeInclude(value)) {
					Serial.printf("Tag did not exist: %s\n", value);
						return false;
					}
					Serial.printf("Log level tag removed: %s\n", value);
					return true;
				}, true);
			}
			if(isCommand(in, "#add-log-exclude")) {
				return executeValue(value, [](const char* value) -> bool {
					if(!LogHandler::addExclude(value)) {
					Serial.printf("Tag filter already exists: %s\n", value);
						return false;
					}
					Serial.printf("Log level tag filter added: %s\n", value);
					return true;
				}, true);
			}
			if(isCommand(in, "#remove-log-exclude")) {
				return executeValue(value, [](const char* value) -> bool {
					if(!LogHandler::removeExclude(value)) {
						Serial.printf("Tag filter did not exist: %s\n", value);
						return false;
					}
					Serial.printf("Log level tag filter removed: %s\n", value);
					return true;
				}, true);
			}

			if(isCommand(in, "#motion-profile-name")) {
				return validateMaxLength("Motion profile name", value, maxMotionProfileNameLength, false, [](const char* value) -> bool {
					SettingsHandler::setMotionProfileName(value);
					return true;
				}, true);
			}
			// if(isCommand(in, "$motion-update")) {
			// 	return validateGreaterThanNegativeOne("Motion update global", value, [](int valueInt) -> bool {
			// 		SettingsHandler::setMotionUpdateGlobal(valueInt);
			// 		return true;
			// 	}, true);
			// }
			// if(isCommand(in, "$motion-period")) {
			// 	return validateGreaterThanNegativeOne("Motion period global", value, [](int valueInt) -> bool {
			// 		SettingsHandler::setMotionPeriodGlobal(valueInt);
			// 		return true;
			// 	}, true);
			// }
			// if(isCommand(in, "$motion-amplitude")) {
			// 	return validateGreaterThanNegativeOne("Motion amplitude global", value, [](int valueInt) -> bool {
			// 		SettingsHandler::setMotionAmplitudeGlobal(valueInt);
			// 		return true;
			// 	}, true);
			// }
			// if(isCommand(in, "$motion-offset")) {
			// 	return validateGreaterThanNegativeOne("Motion offset global", value, [](int valueInt) -> bool {
			// 		SettingsHandler::setMotionOffsetGlobal(valueInt);
			// 		return true;
			// 	}, true);
			// }
			// if(isCommand(in, "$motion-phase")) {
			// 	return validateGreaterThanNegativeOne("Motion phase global", value, [](int valueInt) -> bool {
			// 		SettingsHandler::setMotionPhaseGlobal(valueInt);
			// 		return true;
			// 	}, true);
			// }
			// if(isCommand(in, "$motion-reverse")) {
			// 	return validateGreaterThanNegativeOne("Motion reverse global", value, [](int valueInt) -> bool {
			// 		SettingsHandler::setMotionReversedGlobal(valueInt);
			// 		return true;
			// 	}, true);
			// }
			if(isCommand(in, "#motion-set-profile")) {
				return validateGreaterThanZero("Motion profile", value, [](int valueInt) -> bool {
					int profileAsIndex = valueInt - 1;
					if(profileAsIndex > maxMotionProfileCount) {
						LogHandler::error(_TAG, "Motion profile %ld does not exist", profileAsIndex);
						return false;
					}
					SettingsHandler::setMotionProfile(profileAsIndex);
					return true;
				});
			}
			Serial.printf("Invalid command: %s\n", in);
			//printCommandHelp();
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
	static bool isCommand(const char* in, const char* command) {
		return strstr(in, command) != nullptr;
	}
	static int getInt(const char* value) {
		return (int)(String(value).toInt());
	}

	static bool execute(bool (*function)(), bool isSaveRequired = false, bool isRestartRequired = false) {
		bool subValidate = function();
		if(subValidate) {
			printComplete(isRestartRequired, isSaveRequired);
		}
		xSemaphoreGive(xMutex);
		return subValidate;
	}

	static bool executeValue(const char* value, bool (*function)(const char* value), bool isSaveRequired = false, bool isRestartRequired = false) {
		bool subValidate = function(value);
		if(subValidate) {
			printComplete(isRestartRequired, isSaveRequired);
		}
		xSemaphoreGive(xMutex);
		return subValidate;
	}

	static bool validateBool(const char* name, bool value, bool currentValue, bool (*function)(bool), bool isSaveRequired = false, bool isRestartRequired = false) {
		if(value && currentValue) {
			Serial.println("Already on!");
			xSemaphoreGive(xMutex);
			return false;
		}
		if(!value && !currentValue) {
			Serial.println("Already off!");
			xSemaphoreGive(xMutex);
			return false;
		}
		bool subValidate = function(value);
		if(subValidate) {
			printNewState(name, value);
			printComplete(isRestartRequired, isSaveRequired);
		}
		xSemaphoreGive(xMutex);
		return subValidate;
	}

	static bool validateGreaterThanZero(const char* name, const char* value, bool (*function)(int), bool isSaveRequired = false, bool isRestartRequired = false) {
		int valueInt = getInt(value);
		if(valueInt < 1) {
			Serial.printf("Invalid value: %ld.", valueInt);
			xSemaphoreGive(xMutex);
			return false;
		}
		bool subValidate = function(valueInt);
		if(subValidate) {
			printNewState(name, valueInt);
			printComplete(isRestartRequired, isSaveRequired);
		}
		xSemaphoreGive(xMutex);
		return subValidate;
	}
	static bool validateGreaterThanNegativeOne(const char* name, const char* value, bool (*function)(int), bool isSaveRequired = false, bool isRestartRequired = false) {
		int valueInt = getInt(value);
		if(valueInt < 0) {
			Serial.printf("Invalid value: %ld.", valueInt);
			xSemaphoreGive(xMutex);
			return false;
		}
		bool subValidate = function(valueInt);
		if(subValidate) {
			printNewState(name, valueInt);
			printComplete(isRestartRequired, isSaveRequired);
		}
		xSemaphoreGive(xMutex);
		return subValidate;
	}

	static bool validateMaxLength(const char* name, const char* value, int maxLen, bool valueSensitive, bool (*function)(const char*), bool isSaveRequired = false, bool isRestartRequired = false) {
		if(strlen(value) > maxLen) {
			Serial.printf("Invalid command: %s max length is: %ld\n", name, maxLen);
			xSemaphoreGive(xMutex);
			return false;
		}
		bool subValidate = function(value);
		if(subValidate) {
			if(!valueSensitive)
				printNewState(name, value);
			else
				Serial.printf("%s changed to a value of %ld length\n", name, strlen(value));
			printComplete(isRestartRequired, isSaveRequired);
		}
		xSemaphoreGive(xMutex);
		return subValidate;
	}

	static void printNewState(const char* name, const char* newValue) {
		Serial.printf("%s changed to: %s\n", name, newValue);
	}
	static void printNewState(const char* name, int newValue) {
		Serial.printf("%s changed to: %ld\n", name, newValue);
	}
	static void printNewState(const char* name, bool newValue) {
		Serial.printf("%s %s\n", name, newValue ? "enabled" : "disabled");
	}
	static void printNewState(const char* name, float newValue) {
		Serial.printf("%s changed to: %f\n", name, newValue);
	}
	static void printComplete(bool isRestartRequired, bool isSaveRequired) {
		if(isSaveRequired)
			Serial.println("Execute the command '$save' to store the new value otherwise the value will reset upon reboot.");
		if(isRestartRequired)
			Serial.println("Restart is required after save");
	}
	static void printCommandHelp() {
		Serial.println("");
		Serial.println("");
		Serial.println("");
		Serial.println("");
		Serial.println("Available commands:");
		Serial.println("$help ------------------------- Print this.");
		Serial.println("$save ------------------------- Flush ALL settings to disk.");
		Serial.println("$defaultAll ------------------- Reset ALL settings to default");
		Serial.println("#restart ---------------------- Restart the esp");
		Serial.println("");
		Serial.println("Wifi:");
		Serial.println("$wifi-ssid:value -------------- Change the wifi ssid.");
		Serial.println("$wifi-pass:value -------------- Change the wifi password.");
		Serial.println("");
		Serial.println("Log:");
		Serial.println("$log-level:value -------------- Change the log level.");
		Serial.println("    Log level values: ");
		Serial.println("         0 -- error");
		Serial.println("         1 -- info");
		Serial.println("         2 -- warning");
		Serial.println("         3 -- debug");
		Serial.println("         4 -- verbose");
		Serial.println("$add-log-include:value --------- Add a log tag to include");
		Serial.println("$remove-log-include:value ------ Remove a log tag to include");
		Serial.println("$clear-log-include ------------- Clear all included log tags");
		Serial.println("$add-log-exclude:value --------- Add a log tag to exclude");
		Serial.println("$remove-log-exclude:value ------ Remove a log tag to exclude");
		Serial.println("$clear-log-exclude ------------- Clear all excluded log tags");
		Serial.println("");
		Serial.println("Motion generator:");
		Serial.println("#motion-enable ----------------- Enable motion generator");
		Serial.println("#motion-disable ---------------- Disable motion generator");
		Serial.println("#motion-set-profile:value ------ Set the current profile");
		Serial.printf("    Motion profile values: 1-%ld\n", maxMotionProfileCount);
		Serial.println("#motion-toggle ----------------- Toggle motion generator");
		// Serial.println("$motion-period-random-on ------- Period random on for the current profile");
		// Serial.println("$motion-period-random-off ------ Period random off for the current profile");
		// Serial.println("$motion-amplitude-random-on ---- Amplitude random on for the current profile");
		// Serial.println("$motion-amplitude-random-off --- Amplitude random off for the current profile");
		// Serial.println("$motion-offset-random-on ------- Offset random on for the current profile");
		// Serial.println("$motion-offset-random-off ------ Offset random off for the current profile");
		// Serial.println("$motion-period:value ----------- Set period for the current profile");
		// Serial.println("$motion-update:value ----------- Set update rate for the current profile");
		// Serial.println("$motion-amplitude:value -------- Set amplitude for the current profile");
		// Serial.println("$motion-offset:value ----------- Set offset for the current profile");
		// Serial.println("$motion-phase:value ------------ Set phase for the current profile");
		// Serial.println("$motion-reverse:value ---------- Set reverse for the current profile");
	}
};

SemaphoreHandle_t SystemCommandHandler::xMutex = xSemaphoreCreateMutex();
const char* SystemCommandHandler::_TAG = TagHandler::SystemCommandHandler;
bool SystemCommandHandler::restartRequired = false;