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


#include <WiFiUdp.h>
#include <string>
#include <ArduinoJson.h>
#include "SettingsHandler.h"


class Udphandler 
{
  public:
    void setup(int localPort) 
    {
      wifiUdp.begin(localPort);
      Serial.println("UDP Listening");
      udpInitialized = true;
    }

    char* read() 
    {
      if (!udpInitialized) 
      {
        return nullptr;
      }
      // if there's data available, read a packet
      int packetSize = wifiUdp.parsePacket();
      if (packetSize) 
      {
//          Serial.print("Received packet of size ");
//          Serial.println(packetSize);
//          Serial.print("From ");
//          IPAddress remoteIp = Udp.remoteIP();
//          Serial.print(remoteIp);
//          Serial.print(", port ");
//          Serial.println(Udp.remotePort());
      
        // read the packet into packetBufffer
        int len = wifiUdp.read(packetBuffer, 255);
        if (len > 0) 
        {
          packetBuffer[len] = 0;
        }
        //Serial.println("packetBuffer");
        //Serial.println(packetBuffer);
        //send a reply, to the IP address and port that sent us the packet we received
        if (strcmp(packetBuffer, "password") == 0) 
        {
          Serial.println("Handshake received");
          wifiUdp.beginPacket(wifiUdp.remoteIP(), 54000);
          int i = 0;
          while (ReplyBuffer[i] != 0)
            wifiUdp.write((uint8_t)ReplyBuffer[i++]);
          wifiUdp.endPacket();
        } 
        else if (strpbrk(packetBuffer, jsonIdentifier) != nullptr) 
        {
          //Serial.println("json");

          const size_t readCapacity = JSON_ARRAY_SIZE(5) + 5*JSON_OBJECT_SIZE(2) + 100;

          StaticJsonDocument<readCapacity> doc;
          //DynamicJsonDocument doc(readCapacity);
          DeserializationError error = deserializeJson(doc, packetBuffer);
          if (error) {
              Serial.println(F("Failed to read udp jsonobject, using default configuration"));
              return nullptr;
          }
          JsonArray arr = doc.as<JsonArray>();
          char buffer[300] = "";
          for(JsonObject repo: arr) 
          { 
            const char* channel = repo["Channel"];
            int value = repo["Value"];
            if(channel != nullptr && value != 0) 
            {
              if(buffer[0] == '\0') 
              {
                //Serial.println("tcode empty");
                strcpy(buffer, channel);
              } 
              else 
              {
                strcat(buffer, channel);
              }
              char integer_string[7];
              sprintf(integer_string, "%d", SettingsHandler::calculateRange(channel, value));
              //Serial.print("integer_string");
              //Serial.println(integer_string);
              strcat (buffer, integer_string);
              char speed_string[5];
              sprintf(speed_string, "%d", SettingsHandler::speed);
              strcat (buffer, "S");
              strcat (buffer, speed_string);
              strcat (buffer, " ");
              //Serial.print("buffer");
              //Serial.println(buffer);
            }
          }
          strcat(buffer, "\n");
          char* tcode = new char[strlen(buffer) + 1];
          strcpy(tcode, buffer);
          // Serial.print("tcode: ");
          // Serial.println(tcode);
          return tcode;
        } 
        char* tcode = new char[strlen(packetBuffer) + 1];
        strcpy(tcode, packetBuffer);
        // Serial.print("tcode: ");
        // Serial.println(tcode);
        return tcode;
      }
      return nullptr;
    }
    
/*     void listen(int localPort)
    {
        if(udp.listen(localPort)) {
        IPAddress ip = wifi.ip();
        String mac = wifi.mac();
        Serial.print("UDP Listening on IP: ");
        Serial.println(ip);
        Serial.print("MAC: ");
        Serial.println(mac);
        udp.onPacket([](AsyncUDPPacket packet) {
//            Serial.print("UDP Packet Type: ");
//            Serial.print(packet.isBroadcast()?"Broadcast":packet.isMulticast()?"Multicast":"Unicast");
//            Serial.print(", From: ");
//            Serial.print(packet.remoteIP());
//            Serial.print(":");
//            Serial.print(packet.remotePort());
//            Serial.print(", To: ");
//            Serial.print(packet.localIP());
//            Serial.print(":");
//            Serial.print(packet.localPort());
//            Serial.print(", Length: ");
//            Serial.print(packet.length());
            Serial.print(", Data: ");
            Serial.write(packet.data(), packet.length());
            Serial.println();
            //reply to the client
            packet.printf("Got %u bytes of data", packet.length());
            char* receivedData;
            receivedData = (char*)packet.data();
            if (strcmp(receivedData, "password") == 0) {
              packet.print("connected");
            } else if (strchr(receivedData, '{') != NULL) {
              Serial.write("JSON stuff here");
            } else {
              Serial.write(packet.read());
            }
          });
      }
    } */
    
  private: 
    WiFiUDP wifiUdp;
    bool udpInitialized = false;
    char packetBuffer[255];; //buffer to hold incoming packet
    char ReplyBuffer[10] = "connected";       // a string to send back
    char jsonIdentifier[2] = "{";
};
