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


class tData {
  public:

    // Connection to internet
    bool connected;

    // Timezone and time
    bool syncTime;
    string timezone;
    int timeZoneOffset;
    int timeDSToffset;
    float lat, lon;

    // Weather info
    bool requestWeather;
    tDataQuality weatherAvailable;
    float outsideTemp;
    list<tPrecipitation> precipitation;
    char weatherIcon[12];
    time_t sunrise;
    time_t sunset;

    // Quote
    bool requestQuote;
    tDataQuality quoteAvailable;
    string quote;
  
    // System info
    UBaseType_t lightHighWaterMark;   // Unused stack for the measurement thread
    UBaseType_t screenHighWaterMark;  // Unused stack for the display thread
    UBaseType_t weatherHighWaterMark; // Unused stack for the weather thread
    UBaseType_t quoteHighWaterMark;   // Unused stack for the weather thread
    
    uint32_t lightAlive;
    uint32_t screenAlive;
    uint32_t keyboardAlive;
    uint32_t weatherAlive;
    uint32_t quoteAlive;

    uint32_t screenRedrawMillis;
    
    tData() {
      connected=false;
      syncTime = false;
      timezone="Europe/Amsterdam";
      timeZoneOffset=3600;
      timeDSToffset=0;
      lat=52.25319;
      lon=6.78546;

      weatherAvailable=dqUnavailable;

      quoteAvailable=dqUnavailable;
      requestQuote=false;

      lightAlive=0;
      screenAlive=0;
      keyboardAlive=0;
      weatherAlive=0;
    };

}; // tData

struct tAccessPoint {
  string ssid;
  string password;
  string timezone;
  float lat, lon;
}; // tAccesspoint

class tConfig {
  public:
    list<tAccessPoint> AccessPoints;  

    string botName;
    string botUserName;
    string botToken;
  
    void load() {    
      StaticJsonDocument<1024> doc;
      File input = SPIFFS.open(CONFIG_FILE);
      DeserializationError error = deserializeJson(doc, input);
            
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }
      
      for (JsonObject elem : doc["AccessPoints"].as<JsonArray>()) {
        tAccessPoint AccessPoint;
        AccessPoint.ssid=elem["SSID"].as<string>();
        AccessPoint.password=elem["password"].as<string>();
        AccessPoint.timezone=elem["timezone"].as<string>();
        AccessPoint.lat = elem["lat"]; // 52.25319, 56.25319
        AccessPoint.lon = elem["lon"]; // 6.78546, 6.78546
        AccessPoints.push_back(AccessPoint);    
      }

      botName=doc["BotName"].as<string>();
      botUserName=doc["BotUsername"].as<string>();
      botToken=doc["BotToken"].as<string>();
    }

    void Save() {
      // To do: loop over access points
    }
}; // tConfig

// ======== GLOBAL VARIABLES ============= 
tConfig config; // Configuration data, lives as JSON file in SPIFFS
tData data;     // Global datastructure, volatile

// Ensure only one process at a time writes to the global datastructure
portMUX_TYPE dataAccessMux = portMUX_INITIALIZER_UNLOCKED;

WiFiClientSecure secured_client;

// Ensure only one process at a time uses wifi
portMUX_TYPE connectionMux = portMUX_INITIALIZER_UNLOCKED;

#endif // DATA_H
