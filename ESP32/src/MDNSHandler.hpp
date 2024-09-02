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

#pragma once
#if ESP8266 == 1
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#else
#include <ESPmDNS.h>
#endif
#include "SettingsHandler.h"
// #include "LogHandler.h"
class MDNSHandler {
    public:
        void setup(const char* hostName, const char* friendlyName, const int udpPort, const uint8_t webPort = 0, const uint8_t securePort = 0) {
            if(!MDNSInitialized)
                startMDNS(hostName, friendlyName, webPort, udpPort);
        }
        void stop() {
            if(MDNSInitialized)
            {
                 MDNS.end();
                 MDNSInitialized = false;
            }
        }
        private: 
        const char* _TAG = TagHandler::MdnsHandler;
        bool MDNSInitialized = false;
        void startMDNS(const char* hostName, const char* friendlyName, const int udpPort, const uint8_t webPort = 0, const uint8_t securePort = 0)
        {
            LogHandler::info(_TAG, "Setting up MDNS");
            if(MDNSInitialized)
                MDNS.end();
            if (!MDNS.begin(hostName)) {
                printf("MDNS Init failed");
                return;
            }
            MDNS.setInstanceName(friendlyName);
            if(webPort) 
                MDNS.addService("http", "tcp", webPort);
            if(securePort)
                MDNS.addService("https", "tcp", securePort);
            if(webPort || securePort) {
                char hostLen = strlen(hostName) + 7;
                char domainName[hostLen];
                sprintf(domainName, "%s.local", hostName);
                SettingsHandler::printWebAddress(domainName);
            }
            MDNS.addService("tcode", "udp", udpPort);
            MDNSInitialized = true;
        }
  };