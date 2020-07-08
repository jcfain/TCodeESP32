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

    void connect(char ssid[100], char pass[100]) 
    {
        WiFi.disconnect(true);
        //Serial.println("Setting mode");
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.setHostname("OSR2");
        Serial.printf("Establishing connection to  %s\n", ssid);
        WiFi.begin(ssid, pass);
        while (!isConnected()) 
        {
          delay(1000);
          Serial.print(".");
        }
        IPAddress ipAddress = ip();
      Serial.print("Connected: IP: ");
      Serial.println(ipAddress);
    }

    private: 
    
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
