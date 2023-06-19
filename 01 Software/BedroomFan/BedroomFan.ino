/*******************************************************************
    A telegram bot for ESP32 that controls a
    fan to cool down the bedroom on warm summer days
 *******************************************************************/

const String version = "2.0";

/* 
Version history
1.0 First working version using UniversalTelegramBot
2.0 Upgraded to ASyncTelegram2
    milliSecondTimer class used for timing of fan and wifi reconnect
    Emojis as constants
    Over the air updates
    Event logger implemented
    Sync clock with time server every few days
*/

#include "MyCredentials.h"
#include "eventlog.h"
#include "timer.h"
#include "fancontrol.h"
#include "wifi.h"
#include "telegram.h"
#include "ota.h"

/* 
MyCredentials.h is not included since it is in .gitignore
This file contains all private user specific data
The template for this file is:

      #pragma once

      #include <map>

      // Password to upload software through wireless port
      #define OTApassword "********"

      // Telegram token for the bedroom fan bot
      const char* token =  "##########:aaaaaaaaaaa-bbbbbbbbbbbbbbbbbbbbbbb";

      // Telegram user ID for the user to be notified on startup
      int64_t userid = ########;

      // WiFi access points
      std::map< String , String > ACCESS_POINTS { 
        { "SSID-1", "pwd1" }, 
        { "SSID-2", "pwd2" }, 
        { "SSID-3", "pwd3" } 
      };

      // Timezone where the device is located
      // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
      #define localTimezone "CET-1CEST,M3.5.0,M10.5.0/3"
*/

// ============== TYPES ==============

// ============ CONSTANTS ============

// ======== MAIN FUNCTIONS =========
void setup()
{
  Serial.begin(115200);

  SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED); // SPIFFS needed for event log

  delay(500);

  setupWifi();
  setupOTA();
  setupFan();
  setupTelegram();

  addToEventLogfile( String("Bedroom fan started. Software version ") + version );

  Serial.println("Init completed");
}

void loop() {

  loopWifi();
  loopOTA();
  loopFan();
  loopTelegram();
  
  delay(500);
}
