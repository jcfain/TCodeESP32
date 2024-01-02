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