

class NetworkHandler : public Task
{
private:
    const bool &apMode;
    const int &port;
    const int &udpPort;
    const char *hostname;
    const char *friendlyName;
    MDNSHandler mdnsHandler;
    HTTPBase *webHandler = nullptr;
    WebSocketBase *webSocketHandler = nullptr;
    TaskHandle_t httpsTask;
    const char* _TAG = TagHandler::NetworkHandler;
public:
    NetworkHandler(const bool &apMode, const int &port, const int &udpPort, const char *hostname, const char *friendlyName)
        : apMode(apMode), port(port), udpPort(udpPort), hostname(hostname), friendlyName(friendlyName)
    {
    }
    void setup() override {
        LogHandler::info(_TAG, "Setting up network handler");
        // Network setup code here
    }

    void loop() override {
        // Network handling code here
    }

    void start()
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
}