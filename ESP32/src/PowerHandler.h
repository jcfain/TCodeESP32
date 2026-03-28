#ifndef POWER_HANDLER_H
#define POWER_HANDLER_H

#include <MCP4018.h>

class PowerHandler {
    public:
        PowerHandler() {
            m_settingsFactory = SettingsFactory::getInstance();

        }

        bool setup() {
            Wire.begin();
            if (!SettingsHandler::waitForI2CDevices(MCP4018_ADDRESS)) {
                LogHandler::error(Tags::Battery, "MCP4018 not found on I2C bus.");
                return false;
            }
            long timeout = millis() + 10000;
            while(!mcp4018.begin()) {
                LogHandler::error(Tags::Battery, "Failed to initialize MCP4018. Retrying...");
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                if(millis() > timeout) {
                    LogHandler::error(Tags::Battery, "Detecting MCP4018 timed out. Exit.");
                    return false;
                }
            }
            return true;
        }

        void startLoop()
        {
            while (true)
            {
                mcp4018.
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
        }
    private:
        SettingsFactory* m_settingsFactory;
        static MCP4018 mcp4018(I2C1);
        static uint16_t enable_pin = 13;
        static uint16_t pd_config_pins[3] = {27, 14, 12};
};

#endif // POWER_HANDLER_H
