#pragma once
#include "InstanceHandler.h"

class InitHandler
{
public:
    static InitHandler* getInstance() 
    {
        static InitHandler instance;
        return &instance;
    }
    bool init() 
    {
        taskHandler = TaskHandler::getInstance();


        // setCpuFrequencyMhz(240);

        // see if we can use the onboard led for status
        // https://github.com/kriswiner/ESP32/blob/master/PWM/ledcWrite_demo_ESP32.ino
        // digitalWrite(5, LOW);// Turn off on-board blue led
        serialHandler = new SerialHandler();
        serialHandler->setup();
        LogHandler::init();
        LogHandler::info(m_TAG, "Startup DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));

        #if DEBUG_BUILD == 1
            LogHandler::setLogLevel(LogLevel::DEBUG);
        #else
            LogHandler::setLogLevel(LogLevel::INFO);
        #endif
        LogHandler::setMessageCallback(logCallBack);

        Serial.println();
        LogHandler::info(m_TAG, "Firmware version: %s", FIRMWARE_VERSION_NAME);
        // LogHandler::info(m_TAG, "Esp arduino version: %s", ESP_ARDUINO_VERSION_STR);
        LogHandler::info(m_TAG, "ESP IDF version: %s", esp_get_idf_version());
        uint32_t chipId = 0;
        for (int i = 0; i < 17; i = i + 8)
        {
            chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
        }
        LogHandler::info(m_TAG, "ESP32 Chip model = %s Rev %d", ESP.getChipModel(), ESP.getChipRevision());
        LogHandler::info(m_TAG, "This chip has %d cores", ESP.getChipCores());
        LogHandler::info(m_TAG, "Chip ID: %u", chipId);
        Serial.println();

        // esp_log_level_set("*", ESP_LOG_VERBOSE);
        // LogHandler::debug("main", "this is verbose");
        // LogHandler::debug("main", "this is debug");
        // LogHandler::info("main", "this is info");
        // LogHandler::warning("main", "this is warning");
        // LogHandler::error("main", "this is error");

        if (!LittleFS.begin(true))
        {
            LogHandler::error(m_TAG, "An Error has occurred while mounting LittleFS");
            return false;
        }
        LogHandler::debug(m_TAG, "LittleFS DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));

        settingsFactory = SettingsFactory::getInstance();
        settingsFactory->setMessageCallback(settingChangeCallback);

        if (!settingsFactory->initNetworking())
        {
            LogHandler::error(m_TAG, "Failed to load networking settings...");
            return false;
        }
        if(!initNetworking())
            return false;
        if (!settingsFactory->init())
        {
            LogHandler::error(m_TAG, "Failed to load settings...");
            return false;
        }
        LogHandler::debug(m_TAG, "Settings factory  DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
        LogHandler::setLogLevel(settingsFactory->getLogLevel());

        const PinMap *pinMap = settingsFactory->getPins();
        if(!pinMap)
        {
            LogHandler::warning(m_TAG, "No pin map defined");
            return false;
        }

        SettingsHandler::init();
        SettingsHandler::setMessageCallback(settingChangeCallback);
        LogHandler::debug(m_TAG, "Settings handler DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));

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
            wifi.setWiFiStatusCallback(std::bind(&InitHandler::wifiStatusCallBack, this, std::placeholders::_1, std::placeholders::_2));
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
        bool bootButtonEnabled = BOOT_BUTTON_ENABLED_DEFAULT;
        bool buttonSetsEnabled = BUTTON_SETS_ENABLED_DEFAULT;

        // settingsFactory->getValue(FEEDBACK_TWIST, feedbackTwist);
        // settingsFactory->getValue(ANALOG_TWIST, analogTwist);
        settingsFactory->getValue(BOOT_BUTTON_ENABLED, bootButtonEnabled);
        settingsFactory->getValue(BUTTON_SETS_ENABLED, buttonSetsEnabled);

        bool batteryLevelEnabled = BATTERY_LEVEL_ENABLED_DEFAULT;
        bool voiceEnabled = VOICE_ENABLED_DEFAULT;
        settingsFactory->getValue(BATTERY_LEVEL_ENABLED, batteryLevelEnabled);
        settingsFactory->getValue(VOICE_ENABLED, voiceEnabled);

        bool displayEnabled = DISPLAY_ENABLED_DEFAULT;
        settingsFactory->getValue(DISPLAY_ENABLED, displayEnabled);
        char Display_I2C_AddressString[DISPLAY_I2C_ADDRESS_LEN] = {0};
        settingsFactory->getValue(DISPLAY_I2C_ADDRESS, Display_I2C_AddressString, DISPLAY_I2C_ADDRESS_LEN);
        int Display_I2C_Address = (int)strtol(Display_I2C_AddressString, NULL, 0);

        systemCommandHandler = new SystemCommandHandler();
        systemCommandHandler->registerExternalCommandCallback(tcodePassthroughCommandCallback);
        LogHandler::debug(m_TAG, "System command handler DRAM heaps free %u", heap_caps_get_free_size(MALLOC_CAP_8BIT));

    #ifdef MOTOR_TYPE_SERVO
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
            LogHandler::error(m_TAG, "Invalid TCode version: %ld", settingsFactory->getTcodeVersion());
            return false; // TODO: this stops apmode and not what we want
        }
    #elif defined MOTOR_TYPE_BLDC
        if (settingsFactory->getTcodeVersion() == TCodeVersion::v0_3)
        {
            motorHandler = new BLDCHandler0_3();
        }
        else if (settingsFactory->getTcodeVersion() == TCodeVersion::v0_4)
        {
            motorHandler = new BLDCHandler0_4();
        }
    #else
        #error "Build error! Invalid motor type defined!"
    #endif

        motorHandler->setMessageCallback(tcodeCommandCallback);
        LogHandler::debug(m_TAG, "Motor handler DRAM heaps free %u", heap_caps_get_free_size(MALLOC_CAP_8BIT));
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
        int caseFanMaxPWM;
        settingsFactory->getValue(TEMP_SLEEVE_ENABLED, sleeveTempEnabled);
        settingsFactory->getValue(TEMP_INTERNAL_ENABLED, internalTempEnabled);
        settingsFactory->getValue(HEATER_RESOLUTION, heaterResolution);
        settingsFactory->getValue(HEATER_THRESHOLD, heaterThreshold);
        settingsFactory->getValue(CASE_FAN_RESOLUTION, caseFanResolution);
        settingsFactory->getValue(CASE_FAN_MAX_PWM, caseFanMaxPWM);
        if (sleeveTempEnabled || internalTempEnabled || fanControlEnabled)
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
                                    caseFanResolution,
                                    caseFanMaxPWM);
            temperatureHandler->setMessageCallback(tempChangeCallBack);
            temperatureHandler->setStateChangeCallback(tempStateChangeCallBack);
            LogHandler::debug(m_TAG, "Start temperature task");
            taskHandler->startTemperatureTask(temperatureHandler);
            LogHandler::debug(m_TAG, "Temp DRAM heaps free %u", heap_caps_get_free_size(MALLOC_CAP_8BIT));
        }

    #endif
    #if BUILD_DISPLAY
        if (displayEnabled)
        {
            displayHandler = new DisplayHandler();
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
        LogHandler::debug(m_TAG, "Display DRAM heaps free %u", heap_caps_get_free_size(MALLOC_CAP_8BIT));
    #endif
        motionHandler = new MotionHandler();
        motionHandler->setup(settingsFactory->getTcodeVersion());
        LogHandler::debug(m_TAG, "Motion handler DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
        loadI2CModules(displayEnabled, batteryLevelEnabled, voiceEnabled);
        LogHandler::debug(m_TAG, "I2C DRAM heaps free %u", heap_caps_get_free_size(MALLOC_CAP_8BIT));

        if (bootButtonEnabled || buttonSetsEnabled)
        {
            buttonHandler = new ButtonHandler();
            buttonHandler->init(settingsFactory->getButtonAnalogDebounce(),
                                settingsFactory->getBootButtonCommand(),
                                settingsFactory->getButtonSets());
        }
        
        // otaHandler.setup();
        displayPrint("Setting up motor");
        if(!motorHandler->setup())
        {
            return false;
        }
        LogHandler::debug(m_TAG, "Motor DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));

        LogHandler::debug(m_TAG, "Setup finished");
        LogHandler::printFree();
        m_initialized = true;
        return true;
    }


private:
    static inline SettingsFactory *settingsFactory = SettingsFactory::getInstance();
    TaskHandler* taskHandler;
    const char* m_TAG = TagHandler::InitHandler;
    //CallbackHandler* callbackHandler;
    bool m_initialized = false;
    bool bluetoothEnabled = BLUETOOTH_ENABLED_DEFAULT;
    bool bleEnabled = BLE_ENABLED_DEFAULT;
    // BLEConfigurationHandler* bleConfigurationHandler;
    // TcpHandler tcpHandler;
    void loadI2CModules(bool displayEnabled, bool batteryEnabled, bool voiceEnabled)
    {
#if BUILD_DISPLAY
        if (displayEnabled)
        {
            taskHandler->startDisplayTask(displayHandler);
        }
#endif
        if (batteryEnabled)
        {
            batteryHandler = new BatteryHandler();
            if (batteryHandler->setup())
            {
                taskHandler->startBatteryTask(batteryHandler);
                batteryHandler->setMessageCallback(batteryVoltageCallback);
            }
        }
        if (voiceEnabled)
        {
            voiceHandler = new VoiceHandler();
            if (voiceHandler->setup())
            {
                taskHandler->startVoiceTask(voiceHandler);
                voiceHandler->setMessageCallback(tcodeCommandCallback);
            }
        }
    }
    bool initNetworking() 
    {

    #if BLE_TCODE
        if (bleEnabled)
        {
            startBLETCode();
        }
        else
        {
            BLEHandler::disable();
        }
        LogHandler::debug(m_TAG, "BLE DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
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
        LogHandler::debug(m_TAG, "Bluetooth DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
    #else
        esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    #endif

    #if BLE_TCODE || BLUETOOTH_TCODE
        if (WIFI_TCODE && !COEXIST && (bluetoothEnabled || bleEnabled))
        {
            WifiHandler::disable();
            LogHandler::debug(m_TAG, "Wifi disable DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
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
                LogHandler::info(m_TAG, "Setting up wifi...");
                displayPrint("Connecting to: ");
                LogHandler::info(m_TAG, "Connecting to: %s", ssid);
                displayPrint(ssid);
                if (wifi.connect(settingsFactory->getHostname(), ssid, wifiPass))
                {
    // 				String ipaddress = wifi.ip().toString();
    // 				displayPrint("Connected IP: " + ipaddress);
    // 				LogHandler::info(m_TAG, "Connected IP: %s", ipaddress.c_str());
    // #if BUILD_DISPLAY
    // 				displayHandler->setLocalIPAddress(wifi.ip());
    // #endif


                    if (!startUDPTCode(settingsFactory->getUdpServerPort()))
                    {
                        LogHandler::error(m_TAG, "Error starting UDP server!");
                        return false;
                    }
                    LogHandler::debug(m_TAG, "UDP DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
                    startWebServer(false,
                            settingsFactory->getWebServerPort(),
                            settingsFactory->getUdpServerPort(),
                            settingsFactory->getHostname(),
                            settingsFactory->getFriendlyName());
                }
            }
            else
            {
                startAPMode(
                    settingsFactory->getWebServerPort(),
                    settingsFactory->getUdpServerPort(),
                    settingsFactory->getHostname(),
                    settingsFactory->getFriendlyName());
            }
        }
    #endif
        return true;
    }

#if WIFI_TCODE
    void startWebServer(const bool &apMode, const int &port, const int &udpPort, const char *hostname, const char *friendlyName)
    {
        if((MODULE_CURRENT != ModuleType::WROOM32 || (!bluetoothEnabled && !bleEnabled)) && !webHandler) 
        {
            displayPrint("Starting web server");
    #if !SECURE_WEB
            webHandler = new WebHandler();
            webSocketHandler = new WebSocketHandler();
    #else
            webHandler = new HTTPSHandler();
            webSocketHandler = new SecureWebSocketHandler();
            taskHandler->startHTTPSTask(webHandler);
    #endif
            webHandler->setup(port, webSocketHandler, apMode);
            LogHandler::printWebAddress(m_TAG, WiFi.softAPIP().toString().c_str(), port);
            LogHandler::debug(m_TAG, "Web DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
            if (!apMode) {// mdns breaks apmode?
                bool mdnsEnabled = MDNS_ENABLED_DEFAULT;
                settingsFactory->getValue(MDNS_ENABLED, mdnsEnabled);
                if(mdnsEnabled)
                {
                    mdnsHandler.setup(hostname, friendlyName, udpPort, port);
                    char hostLen = strlen(hostname) + 7;
                    char domainName[hostLen];
                    sprintf(domainName, "%s.local", hostname);
                    LogHandler::printWebAddress(m_TAG, domainName, port);
                    LogHandler::debug(m_TAG, "MDNS DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
                }
            }
        } else {
            displayPrint("WebServer disabled");
            LogHandler::info(m_TAG, "WebServer disabled due to bluetooth and chip model");
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
        if (bluetoothEnabled && !bluetoothHandler)
        {
            displayPrint("Starting Bluetooth serial");
            bluetoothHandler = new BluetoothHandler();
            bluetoothHandler->setup();
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

    void startAPMode(const int &webPort, const int &udpPort, const char *hostname, const char *friendlyName)
    {
#if WIFI_TCODE
        SettingsHandler::apMode = true;
        displayPrint("Starting in APMode");

        char pass[WIFI_PASS_LEN];
        bool hidden = AP_MODE_HIDDEN_DEFAULT;
        uint8_t channel = AP_MODE_CHANNEL_DEFAULT;

        char subnet[IP_ADDRESS_LEN];
        char gateway[IP_ADDRESS_LEN];

        settingsFactory->getValue(AP_MODE_PASS, pass, WIFI_PASS_LEN);
        settingsFactory->getValue(AP_MODE_SUBNET, subnet, IP_ADDRESS_LEN);
        settingsFactory->getValue(AP_MODE_GATEWAY, gateway, IP_ADDRESS_LEN);
        settingsFactory->getValue(AP_MODE_HIDDEN, hidden);
        settingsFactory->getValue(AP_MODE_CHANNEL, channel);
        if (wifi.startAp(hostname, settingsFactory->getAPModeSSID(), pass, channel, hidden, settingsFactory->getAPModeIP(), subnet, gateway))
        {
            displayPrint("APMode started");
            startWebServer(SettingsHandler::apMode, webPort, udpPort, hostname, friendlyName);
        }
        else
        {
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
            LogHandler::debug(m_TAG, "wifiStatusCallBack WiFiStatus::CONNECTED");
            if (reason == WiFiReason::AP_MODE)
            {
                LogHandler::debug(m_TAG, "wifiStatusCallBack WiFiReason::AP_MODE");
                // if(bleConfigurationHandler)
                //   bleConfigurationHandler->stop(); // If a client connects to the ap stop the BLE to save memory.
            }
        }
        else if(status == WiFiStatus::DISCONNECTED)
        {
            // wifi.dispose();
            // startApMode();
            LogHandler::debug(m_TAG, "wifiStatusCallBack Not connected");
            if (reason == WiFiReason::NO_AP || reason == WiFiReason::UNKNOWN)
            {
                LogHandler::debug(m_TAG, "wifiStatusCallBack WiFiReason::NO_AP || WiFiReason::UNKNOWN");
                startAPMode(
                    settingsFactory->getWebServerPort(),
                    settingsFactory->getUdpServerPort(),
                    settingsFactory->getHostname(),
                    settingsFactory->getFriendlyName());
            }
            else if (reason == WiFiReason::AUTH)
            {
                LogHandler::debug(m_TAG, "wifiStatusCallBack WiFiReason::AUTH");
                LogHandler::warning(m_TAG, "Connection auth failed: Resetting wifi password and restarting");
                settingsFactory->defaultValue(WIFI_PASS_SETTING);
                ESP.restart();
            }
            else if (reason == WiFiReason::AP_MODE)
            {
                LogHandler::debug(m_TAG, "wifiStatusCallBack WiFiReason::AP_MODE");
                // #ifdef !ESP32_DA
                // if(bleConfigurationHandler)
                // 	bleConfigurationHandler->setup();
                // #endif
            }
        } 
        else if(status == WiFiStatus::IP) 
        {
    #if BUILD_DISPLAY
            if(displayHandler) 
            {
                String ipaddress = wifi.ip().toString();
                displayPrint("Connected IP: " + ipaddress);
                displayHandler->setLocalIPAddress(wifi.ip());
            }
    #endif
        }
    }
    void displayPrint(String text)
    {
    #if BUILD_DISPLAY
        if(displayHandler)
            displayHandler->println(text);
    #endif
    }
    bool initialized() {
        return m_initialized;
    }
#endif

// TODO move to CallbackHandler or freertos queues///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
    // void TCodeCommandCallback(const char *in)
    // {

    //     if (systemCommandHandler->isCommand(in))
    //     {
    //         systemCommandHandler->process(in);
    //     }
    //     else
    //     {
    // #if BLUETOOTH_TCODE
    //         if (bluetoothHandler && bluetoothHandler->isConnected())
    //             bluetoothHandler->CommandCallback(in);
    // #endif
    // #if BLE_TCODE
    //         if (bleHandler && bleHandler->isConnected())
    //             bleHandler->send(in);
    // #endif
    // #if WIFI_TCODE
    //         if (webSocketHandler)
    //             webSocketHandler->send(in);
    //         if (udpHandler)
    //             udpHandler->send(in);
    // #endif
    //         serialHandler->send(in);
    //     }
    // }

    // void tcodePassthroughCommandCallback(const char *in)
    // {
    //     if (systemCommandHandler->isCommand(in))
    //     {
    //         // This seems wrong but since we are only calling this from one place its fine for now.
    //         char temp[strlen(in) + 2];
    //         temp[0] = {0};
    //         strcpy(temp, in);
    //         strcat(temp, "\n");
    // //////////////////////////////////////////////////////////////////////////////////////
    // #if BLUETOOTH_TCODE
    //         if (bluetoothHandler && bluetoothHandler->isConnected())
    //             bluetoothHandler->send(temp);
    // #endif
    // #if BLE_TCODE

    // #endif
    // #if WIFI_TCODE
    //         if (webSocketHandler)
    //             webSocketHandler->send(temp);
    //         if (udpHandler)
    //             udpHandler->send(temp);
    // #endif
    //         serialHandler->send(temp);
    //     }
    // }

    // void profileChangeCallback(uint8_t profile)
    // {
    // }

    // void logCallBack(const char *input, size_t length, LogLevel level)
    // {
    // #if WIFI_TCODE
    //     // if(webSocketHandler) {
    //     // 	webSocketHandler->sendLog(in, level);
    //     // }
    // #endif
    // }

    // #if BUILD_TEMP
    // void tempChangeCallBack(TemperatureType type, const char *message, float temp)
    // {
    // #if WIFI_TCODE
    //     if (webSocketHandler)
    //     {
    //         if (strpbrk(message, "{") == nullptr)
    //         {
    //             webSocketHandler->sendCommand(message);
    //         }
    //         else
    //         {
    //             if (type == TemperatureType::SLEEVE)
    //             {
    //                 webSocketHandler->sendCommand("sleeveTempStatus", message);
    //             }
    //             else
    //             {
    //                 webSocketHandler->sendCommand("internalTempStatus", message);
    //             }
    //         }
    //     }
    // #endif
    // #if BUILD_DISPLAY
    //     if (displayHandler)
    //     {
    //         if (type == TemperatureType::SLEEVE)
    //         {
    //             displayHandler->setSleeveTemp(temp);
    //         }
    //         else
    //         {
    //             displayHandler->setInternalTemp(temp);
    //         }
    //     }
    // #endif
    // }

    // void tempStateChangeCallBack(TemperatureType type, const char *state)
    // {
    // #if BUILD_DISPLAY
    //     if (displayHandler)
    //     {
    //         if (type == TemperatureType::SLEEVE)
    //         {
    //             LogHandler::verbose(m_TAG, "tempStateChangeCallBack heat: %s", state);
    //             displayHandler->setHeateState(state);
    //             if (temperatureHandler)
    //                 displayHandler->setHeateStateShort(temperatureHandler->getShortSleeveControlStatus(state));
    //         }
    //         else
    //         {
    //             LogHandler::verbose(m_TAG, "tempStateChangeCallBack fan: %s", state);
    //             displayHandler->setFanState(state);
    //         }
    //     }
    // #endif
    // }
    // #endif

    // void batteryVoltageCallback(float capacityRemainingPercentage, float capacityRemaining, float voltage, float temperature)
    // {
    // #if BUILD_DISPLAY
    //     if (displayHandler)
    //     {
    //         displayHandler->setBatteryInformation(capacityRemainingPercentage, voltage, temperature);
    //     }
    // #endif
    // #if WIFI_TCODE
    //     if (webSocketHandler)
    //     {
    //         String statusJson("{\"batteryCapacityRemaining\":\"" + String(capacityRemaining) + "\", \"batteryCapacityRemainingPercentage\":\"" + String(capacityRemainingPercentage) + "\", \"batteryVoltage\":\"" + String(voltage) + "\", \"batteryTemperature\":\"" + String(temperature) + "\"}");
    //         webSocketHandler->sendCommand("batteryStatus", statusJson.c_str());
    //     }
    // #endif
    // }

    // void settingChangeCallback(const SettingProfile &profile, const char *settingThatChanged)
    // {
    //     LogHandler::verbose(m_TAG, "settingChangeCallback: %s", settingThatChanged);
    //     if (profile == SettingProfile::System)
    //     {
    //         if (!strcmp(settingThatChanged, LOG_LEVEL_SETTING))
    //         {
                
    //             #if DEBUG_BUILD != 1
    //                 LogHandler::setLogLevel(settingsFactory->getLogLevel());
    //             #endif
    //         }
    //         else if (!strcmp(settingThatChanged, LOG_INCLUDETAGS))
    //         {
    //             LogHandler::setIncludes(settingsFactory->getLogIncludes());
    //         }
    //         else if (!strcmp(settingThatChanged, LOG_EXCLUDETAGS))
    //         {
    //             LogHandler::setExcludes(settingsFactory->getLogExcludes());
    //         }
    //     }
    //     else if (profile == SettingProfile::MotionProfile)
    //     {
    //         if (strcmp(settingThatChanged, MOTION_PROFILE_SELECTED_INDEX) == 0 || strcmp(settingThatChanged, MOTION_PROFILES) == 0) {
    //             motionHandler->setMotionChannels(SettingsHandler::getMotionChannels());
    //         //} else if(strcmp(settingThatChanged, "motionChannels") == 0) {
    //         // 	motionHandler->setMotionChannels(SettingsHandler::getGetMotionChannels()());
    //         } else if (strcmp(settingThatChanged, MOTION_ENABLED) == 0) {
    //             LogHandler::verbose(m_TAG, "MOTION_ENABLED: %d", SettingsHandler::getMotionEnabled());
    //             motionHandler->setEnabled(SettingsHandler::getMotionEnabled());
    //         }
    //         // else if(strcmp(settingThatChanged, "motionAmplitudeGlobal") == 0)
    //         // 	motionHandler->setAmplitude(SettingsHandler::getGetMotionAmplitudeGlobal()());
    //         // else if(strcmp(settingThatChanged, "motionOffsetGlobal") == 0)
    //         // 	motionHandler->setOffset(SettingsHandler::getGetMotionOffsetGlobal()());
    //         // else if(strcmp(settingThatChanged, "motionPeriodGlobal") == 0)
    //         // 	motionHandler->setPeriod(SettingsHandler::getGetMotionPeriodGlobal()());
    //         // else if(strcmp(settingThatChanged, "motionUpdateGlobal") == 0)
    //         // 	motionHandler->setUpdate(SettingsHandler::getGetMotionUpdateGlobal()());
    //         // else if(strcmp(settingThatChanged, "motionPhaseGlobal") == 0)
    //         // 	motionHandler->setPhase(SettingsHandler::getGetMotionPhaseGlobal()());
    //         // else if(strcmp(settingThatChanged, "motionReversedGlobal") == 0)
    //         // 	motionHandler->setReverse(SettingsHandler::getGetMotionReversedGlobal()());
    //         // else if(strcmp(settingThatChanged, "motionAmplitudeGlobalRandom") == 0)
    //         // 	motionHandler->setAmplitudeRandom(SettingsHandler::getGetMotionAmplitudeGlobalRandom()());
    //         // else if(strcmp(settingThatChanged, "motionAmplitudeGlobalRandomMin") == 0)
    //         // 	motionHandler->setAmplitudeRandomMin(SettingsHandler::getGetMotionAmplitudeGlobalRandomMin()());
    //         // else if(strcmp(settingThatChanged, "motionAmplitudeGlobalRandomMax") == 0)
    //         // 	motionHandler->setAmplitudeRandomMax(SettingsHandler::getGetMotionAmplitudeGlobalRandomMax()());
    //         // else if(strcmp(settingThatChanged, "motionPeriodGlobalRandom") == 0)
    //         // 	motionHandler->setPeriodRandom(SettingsHandler::getGetMotionPeriodGlobalRandom()());
    //         // else if(strcmp(settingThatChanged, "motionPeriodGlobalRandomMin") == 0)
    //         // 	motionHandler->setPeriodRandomMin(SettingsHandler::getGetMotionPeriodGlobalRandomMin()());
    //         // else if(strcmp(settingThatChanged, "motionPeriodGlobalRandomMax") == 0)
    //         // 	motionHandler->setPeriodRandomMax(SettingsHandler::getGetMotionPeriodGlobalRandomMax()());
    //         // else if(strcmp(settingThatChanged, "motionOffsetGlobalRandom") == 0)
    //         // 	motionHandler->setOffsetRandom(SettingsHandler::getGetMotionOffsetGlobalRandom()());
    //         // else if(strcmp(settingThatChanged, "motionOffsetGlobalRandomMin") == 0)
    //         // 	motionHandler->setOffsetRandomMin(SettingsHandler::getGetMotionOffsetGlobalRandomMin()());
    //         // else if(strcmp(settingThatChanged, "motionOffsetGlobalRandomMax") == 0)
    //         // 	motionHandler->setOffsetRandomMax(SettingsHandler::getGetMotionOffsetGlobalRandomMax()());
    //         // else if(strcmp(settingThatChanged, "motionRandomChangeMin") == 0)
    //         // 	motionHandler->setMotionRandomChangeMin(SettingsHandler::getGetMotionRandomChangeMin()());
    //         // else if(strcmp(settingThatChanged, "motionRandomChangeMax") == 0)
    //         // 	motionHandler->setMotionRandomChangeMax(SettingsHandler::getGetMotionRandomChangeMax()());
    //     }
    //     else if (voiceHandler && profile == SettingProfile::Voice)
    //     {
    //         if (strcmp(settingThatChanged, "voiceMuted") == 0)
    //         {
    //             voiceHandler->setMuteMode(settingsFactory->getVoiceMuted());
    //         }
    //         else if (strcmp(settingThatChanged, "voiceVolume") == 0)
    //         {
    //             voiceHandler->setVolume(settingsFactory->getVoiceVolume());
    //         }
    //         else if (strcmp(settingThatChanged, "voiceWakeTime") == 0)
    //         {
    //             voiceHandler->setWakeTime(settingsFactory->getVoiceWakeTime());
    //         }
    //     }
    //     else if (buttonHandler && profile == SettingProfile::Button)
    //     {
    //         if (strcmp(settingThatChanged, "bootButtonCommand") == 0)
    //             buttonHandler->updateBootButtonCommand(settingsFactory->getBootButtonCommand());
    //         else if (strcmp(settingThatChanged, "analogButtonCommands") == 0)
    //         {
    //             buttonHandler->updateAnalogButtonCommands(settingsFactory->getButtonSets());
    //         }
    //         else if (strcmp(settingThatChanged, "buttonAnalogDebounce") == 0)
    //         {
    //             buttonHandler->updateAnalogDebounce(settingsFactory->getButtonAnalogDebounce());
    //         }
    //     }
    //     else if (profile == SettingProfile::ChannelRanges)
    //     { 
    //         if (strcmp(settingThatChanged, CHANNEL_PROFILE) == 0) {
    //             // TODO add channe; specific updates when moving to its own save...maybe...
    //             motionHandler->updateChannelRanges();
    //         } else if (strcmp(settingThatChanged, "channelRangesEnabled") == 0) {
    //             webSocketHandler->sendCommand("channelRangesEnabled", SettingsHandler::getChannelRangesEnabled() ? "true" : "false");
    //         }
            
    //     }
    // }
};