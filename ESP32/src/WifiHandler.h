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
#include "BLEHandler.h"

class WifiHandler 
{
  public:
    ~WifiHandler() 
    {
      if(onApEventID != 0)
        WiFi.removeEvent(onApEventID);
    }
    static bool isConnected() 
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
	int8_t RSSI()
	{
		return WiFi.RSSI();
	}
	static int8_t getRSSI()
	{
		return WiFi.RSSI();
	}
	static bool apMode()
	{
		return _apMode;
	}
    bool connect(char ssid[100], char pass[100]) 
    {
	      _apMode = false;
        WiFi.disconnect(true, true);
        //Serial.println("Setting mode");
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.setHostname("TCodeESP32");
      if (SettingsHandler::staticIP) 
      {
            Serial.println("Setting static IP settings: ");
            Serial.print(SettingsHandler::localIP);
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
      Serial.println("Setting up wifi");
      Serial.print("Mac: ");
      Serial.println(mac());
      Serial.print("Establishing connection to ");
      Serial.print(ssid);
      if(pass[0] == '\0')
          WiFi.begin(ssid);
      else
          WiFi.begin(ssid, pass);
      int connectStartTimeout = millis() + connectTimeOut;
      while (!isConnected() && millis() < connectStartTimeout) 
      {
        delay(1000);
        Serial.print(".");
      }
      if (millis() >= connectStartTimeout) 
      {
        Serial.println("Wifi timed out connection to AP");
        WiFi.disconnect(true, true);
        return false;
      }
      IPAddress ipAddress = ip();
      Serial.println();
      Serial.print("Connected: IP: ");
      Serial.println(ipAddress);
        _apMode = false;
      return true;
    }

    void WiFiEvent(WiFiEvent_t event, system_event_info_t info){
      switch(event){
        case SYSTEM_EVENT_STA_START:
          Serial.println("Station Mode Started");
          break;
        case SYSTEM_EVENT_STA_GOT_IP:
          Serial.println("Connected to :" + String(WiFi.SSID()));
          Serial.println(WiFi.localIP());
          break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("Disconnected from station, attempting reconnection");
          WiFi.reconnect();
          break;
        case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
          Serial.println("WPS Successfull, stopping WPS and connecting to: " + String(WiFi.SSID()));
          WiFi.begin();
          break;
        case SYSTEM_EVENT_STA_WPS_ER_FAILED:
          Serial.println("WPS Failed, retrying");
          break;
        case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
          Serial.println("WPS Timedout, retrying");
          break;
        case SYSTEM_EVENT_STA_WPS_ER_PIN:
          break;
        case SYSTEM_EVENT_AP_STACONNECTED:
          if(_apMode)
          {
            _bleHandler->stop(); // If a client connects to the ap stop the BLE to save memory.
          }
          break;
        case SYSTEM_EVENT_AP_STADISCONNECTED:
          if(_apMode)
          {
            // _bleHandler->setup(); //Didnt get called for some reason. No time to debug. Just restart the esp.
          }
          break;
        default:
          break;
      }
    }

    bool startAp(BLEHandler* bleHandler) {
      _bleHandler = bleHandler;
      WiFi.disconnect(true, true);
      WiFi.mode(WIFI_AP);
      //WiFi.setHostname("TCodeESP32");
      WiFi.softAP(ssid);
      Serial.print("Mac: ");
      Serial.println(mac());
      delay(100);
      onApEventID = WiFi.onEvent([this](WiFiEvent_t event, system_event_info_t info) {
                    this->WiFiEvent(event, info);
                  });

      //WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
      // Set your Static IP address
      IPAddress local_IP(192, 168, 1, 1);
      IPAddress subnet(255, 255, 255, 0);
      // Set your Gateway IP address
      IPAddress gateway(192, 168, 1, 254);
      if (!WiFi.softAPConfig(local_IP, gateway, subnet)) 
	    {
        Serial.println("AP Failed to configure");
        return false;
      }
	    _apMode = true;
      Serial.print("Wifi APMode IP: ");
      Serial.println(WiFi.softAPIP());
      return true;
    }
    private: 
    const char *ssid = "TCodeESP32Setup";
    const char *password = "12345678";
    int connectTimeOut = 10000;
    int onApEventID = 0;
    BLEHandler* _bleHandler;
	static int8_t _rssi;
	static bool _apMode;
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
bool WifiHandler::_apMode = false;