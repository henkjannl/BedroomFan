#ifndef DATA_H
#define DATA_H

#include <Arduino.h>
#include "esp_system.h"
#include "FS.h"
#include "SPIFFS.h"
#include "time.h"
#include <list>
#include <string>
#include <WiFi.h>
#include <U8g2lib.h>
#include <ArduinoJson.h>

using namespace std;

// ======== CONSTANTS ================
const char *CONFIG_FILE = "/config.jsn";

// ======== TYPES ================
struct tAccessPoint {
  String ssid;
  String password;
}; // tAccesspoint

class tConfig {
  public:
    list<tAccessPoint> AccessPoints;  

    String botName;
    String botUserName;
    String botToken;
    String botChatID;
  
    void load() {    
      Serial.println("Loading config");
      StaticJsonDocument<1024> doc;
      File file = SPIFFS.open(CONFIG_FILE);
      DeserializationError error = deserializeJson(doc, file);

      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }
      
      Serial.println("Parsing JSON");
      for (JsonObject elem : doc["AccessPoints"].as<JsonArray>()) {
        tAccessPoint AccessPoint;
        AccessPoint.ssid=elem["SSID"].as<String>();
        AccessPoint.password=elem["password"].as<String>();
        AccessPoints.push_back(AccessPoint);
      }

      botName=    doc["BotName"    ].as<String>();
      botUserName=doc["BotUsername"].as<String>();
      botToken=   doc["BotToken"   ].as<String>();
      botChatID=  doc["ChatID"     ].as<String>();

      Serial.println(botToken);
    }
}; // tConfig

#endif // DATA_H
