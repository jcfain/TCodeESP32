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
#include <esp_wifi.h>
enum class WiFiStatus {
  CONNECTED,
  DISCONNECTED
};
enum class WiFiReason {
  UNKNOWN,
  AUTH,
  NO_AP,
  AP_MODE
};
using WIFI_STATUS_FUNCTION_PTR_T = void (*)(WiFiStatus status, WiFiReason reason);
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
    bool connect(char ssid[32], char pass[63]) 
    {
      LogHandler::info(_TAG, "Setting up wifi");
	      _apMode = false;
        WiFi.disconnect(true, true);
        //Serial.println("Setting mode");
        if(onApEventID != 0)
          WiFi.removeEvent(onApEventID);
        onApEventID = WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
                      this->WiFiEvent(event, info);
                    });
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.setHostname("TCodeESP32");
      if (SettingsHandler::staticIP) 
      {
        LogHandler::info(_TAG, "Setting static IP settings: %s", SettingsHandler::localIP);
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
      LogHandler::info(_TAG, "Mac: %s", mac().c_str());
      LogHandler::info(_TAG, "Establishing connection to %s", ssid);
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
        LogHandler::error(_TAG, "Wifi timed out connection to AP");
        WiFi.disconnect(true, true);
        return false;
      }
      IPAddress ipAddress = ip();
	    LogHandler::info(_TAG, "Connected IP: %s", ip().toString().c_str());
      _apMode = false;
      return true;
    }

    void dispose() {
      //WiFi.disconnect(true, true);
      //esp_http_client_cleanup( WiFi ); // dismiss the TCP stack
      esp_wifi_disconnect();            // break connection to AP
      esp_wifi_stop();                 // shut down the wifi radio
      //esp_wifi_deinit();              // release wifi resources
    }

    void WiFiEvent(arduino_event_id_t event, arduino_event_info_t info){
      switch(event){
        case ARDUINO_EVENT_WIFI_STA_START:
          LogHandler::info(_TAG, "Station Mode Started");
          break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
          LogHandler::info(_TAG, "Connected to: %s", WiFi.SSID().c_str());
          LogHandler::info(_TAG, "IP Address: %s", WiFi.localIP().toString().c_str());
          break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: 
        {
          LogHandler::warning(_TAG, "Disconnected from station, attempting reconnection");
          LogHandler::info(_TAG, "Reason: %u", lastReason);
          uint8_t reason = info.wifi_sta_disconnected.reason;
          if(reason == WIFI_REASON_NO_AP_FOUND) {
            LogHandler::info(_TAG, "WIFI_REASON_NO_AP_FOUND");
            lastReason = reason;
          } else if(reason == WIFI_REASON_AUTH_FAIL || reason == WIFI_REASON_CONNECTION_FAIL) {
            lastReason = reason;
          } else if(reason == WIFI_REASON_BEACON_TIMEOUT || reason == WIFI_REASON_HANDSHAKE_TIMEOUT) {
            LogHandler::info(_TAG, "WIFI_REASON_BEACON_TIMEOUT or WIFI_REASON_HANDSHAKE_TIMEOUT");
          } else if(reason == WIFI_REASON_AUTH_EXPIRE) {
            LogHandler::info(_TAG, "WIFI_REASON_AUTH_EXPIRE");
          } else {
            LogHandler::info(_TAG, "Unknown reason %u", lastReason);
          }
          WiFi.reconnect();
          break;
        }
        case ARDUINO_EVENT_WIFI_STA_STOP:
          LogHandler::error(_TAG, "Station Mode Stopped: %u", info.wifi_sta_disconnected.reason);
          if(lastReason == WIFI_REASON_NO_AP_FOUND) {
            LogHandler::info(_TAG, "WIFI_REASON_NO_AP_FOUND");
            if(wifiStatus_callback)
              wifiStatus_callback(WiFiStatus::DISCONNECTED, WiFiReason::NO_AP);
          } else if(lastReason == WIFI_REASON_AUTH_FAIL || lastReason == WIFI_REASON_CONNECTION_FAIL) {
            if(wifiStatus_callback)
              wifiStatus_callback(WiFiStatus::DISCONNECTED, WiFiReason::AUTH);
          } else {
            if(wifiStatus_callback)
              wifiStatus_callback(WiFiStatus::DISCONNECTED, WiFiReason::UNKNOWN);
          }
            lastReason = 0;
          break;
        case ARDUINO_EVENT_WPS_ER_SUCCESS:
          LogHandler::info(_TAG, "WPS Successfull, stopping WPS and connecting to: %s", WiFi.SSID().c_str());
          WiFi.begin();
          break;
        case ARDUINO_EVENT_WPS_ER_FAILED:
          LogHandler::error(_TAG, "WPS Failed, retrying");
          WiFi.reconnect();
          break;
        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
          LogHandler::error(_TAG, "WPS Timedout, retrying");
          WiFi.reconnect();
          break;
        case ARDUINO_EVENT_WPS_ER_PIN:
          LogHandler::debug(_TAG, "ARDUINO_EVENT_WPS_ER_PIN");
          break;
        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
          LogHandler::debug(_TAG, "ARDUINO_EVENT_WIFI_AP_STACONNECTED");
            if(wifiStatus_callback)
              wifiStatus_callback(WiFiStatus::CONNECTED, WiFiReason::AP_MODE);
          // if(_apMode)
          // {
          //   if(_bleHandler)
          //     _bleHandler->stop(); // If a client connects to the ap stop the BLE to save memory.
          //   if(_btHandler)
          //     _btHandler->stop();
          // }
          break;
        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
          LogHandler::debug(_TAG, "ARDUINO_EVENT_WIFI_AP_STADISCONNECTED");
            if(wifiStatus_callback)
              wifiStatus_callback(WiFiStatus::DISCONNECTED, WiFiReason::AP_MODE);
          if(_apMode)
          {
            // _bleHandler->setup(); //Didnt get called for some reason. No time to debug. Just restart the esp.
          }
          break;
        default:
          break;
      }
    }

    bool startAp() {
      WiFi.disconnect(true, true);
      WiFi.mode(WIFI_AP);
      //WiFi.setHostname("TCodeESP32");
      WiFi.softAP(ssid);
      LogHandler::info(_TAG, "Mac: %s", mac().c_str());
      delay(100);
      if(onApEventID != 0)
        WiFi.removeEvent(onApEventID);
      onApEventID = WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
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
        LogHandler::error(_TAG, "AP Failed to configure");
        return false;
      }
	    _apMode = true;
      LogHandler::info(_TAG, "Wifi APMode IP: %s", WiFi.softAPIP().toString().c_str());//TODO: fix this.. 
      return true;
    }
	void setWiFiStatusCallback(WIFI_STATUS_FUNCTION_PTR_T f)
	{
		wifiStatus_callback = f == nullptr ? 0 : f;
	}
private: 
    WIFI_STATUS_FUNCTION_PTR_T wifiStatus_callback;
    const char* _TAG = "WIFI";
    const char *ssid = "TCodeESP32Setup";
    const char *password = "12345678";
    int connectTimeOut = 10000;
    int onApEventID = 0;
    static int8_t _rssi;
    static bool _apMode;
    uint8_t lastReason;
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