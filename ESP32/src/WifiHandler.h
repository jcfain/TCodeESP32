/* MIT License

Copyright (c) 2020 Jason C. Fain

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

#include <WiFi.h>

class WifiHandler 
{
  
  public:
    bool isConnected() 
    {
      return WiFi.isConnected();
    }
    
    IPAddress ip() 
    {
      return WiFi.localIP();
    }
    String mac() 
    {
      return WiFi.macAddress();
    }

    bool connect(char ssid[100], char pass[100]) 
    {
        WiFi.disconnect(true);
        //Serial.println("Setting mode");
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.setHostname("TCodeESP32");
		if (SettingsHandler::staticIP) 
		{
        	Serial.print("Setting static IP settings:");
        	Serial.println(SettingsHandler::localIP);
			uint8_t ipAddress[4];
			sscanf(SettingsHandler::localIP, "%u.%u.%u.%u", &ipAddress[0], &ipAddress[1], &ipAddress[2], &ipAddress[3]);
			uint8_t gateway[4];
			sscanf(SettingsHandler::gateway, "%u.%u.%u.%u", &gateway[0], &gateway[1], &gateway[2], &gateway[3]);
			uint8_t subnet[4];
			sscanf(SettingsHandler::subnet, "%u.%u.%u.%u", &subnet[0], &subnet[1], &subnet[2], &subnet[3]);
			uint8_t dns1[4];
			sscanf(SettingsHandler::dns1, "%u.%u.%u.%u", &dns1[0], &dns1[1], &dns1[2], &dns1[3]);
			uint8_t dns2[4];
			sscanf(SettingsHandler::dns2, "%u.%u.%u.%u", &dns2[0], &dns2[1], &dns2[2], &dns2[3]);
			WiFi.config(IPAddress(ipAddress), IPAddress(gateway), IPAddress(subnet), IPAddress(dns1), IPAddress(dns2));
		}
        Serial.print("Establishing connection to");
        Serial.println(ssid);
        WiFi.begin(ssid, pass);
        int connectStartTimeout = millis() + connectTimeOut;
        while (!isConnected() && millis() < connectStartTimeout) 
        {
			delay(1000);
			Serial.print(".");
        }
		if (millis() > connectStartTimeout) 
		{
			Serial.println("Wifi timed out connection to AP");
			return false;
		}
		IPAddress ipAddress = ip();
		Serial.println("");
		Serial.print("Connected: IP: ");
		Serial.println(ipAddress);
		return true;
    }

    bool startAp() {
      WiFi.disconnect(true);
      WiFi.mode(WIFI_AP);
      //WiFi.setHostname("TCodeESP32");
      WiFi.softAP(ssid);
      delay(100);
      //WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
      // Set your Static IP address
      IPAddress local_IP(192, 168, 1, 1);
      IPAddress subnet(255, 255, 255, 0);
      // Set your Gateway IP address
      IPAddress gateway(192, 168, 1, 254);
      if (!WiFi.softAPConfig(local_IP, gateway, subnet)) 
	  {
			Serial.println("STA Failed to configure");
			return false;
      }
      Serial.print("Wifi started in APMode: ");
      Serial.println(WiFi.softAPIP());
      return true;
    }

    private: 
    const char *ssid = "TCodeESP32Setup";
    const char *password = "12345678";
    int connectTimeOut = 10000;
  //  String translateEncryptionType(wifi_auth_mode_t encryptionType) {
  //    switch (encryptionType) {
  //      case (WIFI_AUTH_OPEN):
  //        return "Open";
  //      case (WIFI_AUTH_WEP):
  //        return "WEP";
  //      case (WIFI_AUTH_WPA_PSK):
  //        return "WPA_PSK";
  //      case (WIFI_AUTH_WPA2_PSK):
  //        return "WPA2_PSK";
  //      case (WIFI_AUTH_WPA_WPA2_PSK):
  //        return "WPA_WPA2_PSK";
  //      case (WIFI_AUTH_WPA2_ENTERPRISE):
  //        return "WPA2_ENTERPRISE";
  //    }
  //  }
};
