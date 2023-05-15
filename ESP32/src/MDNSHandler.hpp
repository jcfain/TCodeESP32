
#pragma once
#include <ESPmDNS.h>
#include "SettingsHandler.h"
class MDNSHandler {
    public:
        void setup(char* hostName, char* friendlyName) {
            startMDNS(hostName, friendlyName);
        }
        void stop() {
            if(MDNSInitialized)
            {
                 MDNS.end();
                 MDNSInitialized = false;
            }
        }
       private: 
       bool MDNSInitialized = false;
        void startMDNS(char* hostName, char* friendlyName)
        {
            if(MDNSInitialized)
                MDNS.end();
            Serial.print("hostName: ");
            Serial.println(hostName);
            Serial.print("friendlyName: ");
            Serial.println(friendlyName);
            if (!MDNS.begin(hostName)) {
                printf("MDNS Init failed");
                return;
            }
            MDNS.setInstanceName(friendlyName);
            MDNS.addService("http", "tcp", 80);
            MDNS.addService("https", "tcp", 443);
            MDNS.addService("tcode", "udp", SettingsHandler::udpServerPort);
            MDNSInitialized = true;
        }
  };