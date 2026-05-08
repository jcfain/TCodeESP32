

class NetworkHandler : public TaskHandler::Task
{
private:
	const bool &apMode;
	const int &port;
	const int &udpPort;
	const char *hostname;
	const char *friendlyName;
	MDNSHandler mdnsHandler;
	HTTPBase *webHandler = nullptr;
	WebSocketBase* webSocketHandler = nullptr;

public:
	NetworkHandler(const bool &apMode, const int &port, const int &udpPort, const char *hostname, const char *friendlyName)
		: apMode(apMode), port(port), udpPort(udpPort), hostname(hostname), friendlyName(friendlyName)
	{
	}
	void setup() override
	{
		LogHandler::info(Tags::Network, "Setting up network handler");
		// Network setup code here
	}

	void loop() override
	{
		// Network handling code here
	}

	void start()
	{
		if ((MODULE_CURRENT != ModuleType::WROOM32 || (!bluetoothEnabled && !bleEnabled)) && !webHandler)
		{
			displayPrint("Starting web server");
#if !SECURE_WEB
			webHandler = new WebHandler();
			webSocketHandler = new WebSocketHandler();
#else
			LogHandler::debug(Tags::Main, "Start https task");
			webHandler = new HTTPSHandler();
			webSocketHandler = new SecureWebSocketHandler();
			TaskHandler::global().add(static_cast<TaskHandler::Task*>(webHandler));
#endif
			webHandler->setup(port, webSocketHandler, apMode);
			LogHandler::debug(Tags::Main, "Web DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
		}
		else
		{
			displayPrint("WebServer disabled");
			LogHandler::info(Tags::Main, "WebServer disabled due to bluetooth and chip model");
		}
		if (!apMode)
		{ // mdns breaks apmode?
			mdnsHandler.setup(hostname, friendlyName, port, udpPort);
			LogHandler::debug(Tags::Main, "MDNS DRAM heaps free %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
		}
	}
}