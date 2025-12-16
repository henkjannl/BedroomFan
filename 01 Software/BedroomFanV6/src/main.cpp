#include <Arduino.h>

/*******************************************************************
    A telegram bot for ESP32 that controls a
    fan to cool down the bedroom on warm summer days
 *******************************************************************/
const String version = "4.0";
const bool FORMAT_SPIFFS_IF_FAILED = true;

/*
To do:
sync clock
add clock to userinterface
*/

/*
Version history
1.0 First working version using UniversalTelegramBot
2.0 Upgraded to ASyncTelegram2
    milliSecondTimer class used for timing of fan and wifi reconnect
    Emojis as constants
    Over the air updates
    Event logger implemented
    Sync clock with time server every few days
3.0 Removed over the air updates`
4.0 Ported to PlatformIO
5.0 Included clock to switch on and off the fan at specific times
*/

#include <SPIFFS.h>

#include "myCredentials.h"
#include "eventlog.h"
#include "timer.h"
#include "fancontrol.h"
#include "wifi_connect.h"
#include "telegram.h"


/*
MyCredentials.h is not included since it is in .gitignore
This file contains all private user specific data
The template for this file is:

  #pragma once

  #include <map>

  // Telegram token for the bedroom fan bot
  const char* token =  "##########:aaaaaaaaaaa-bbbbbbbbbbbbbbbbbbbbbbb";

  // Telegram user ID for the user to be notified on startup
  int64_t userid = ########;

  // WiFi access points
  std::map< String , String > ACCESS_POINTS {
      { "SSID1",    "PASSWORD1" },
      { "SSID2",    "PASSWORD2" },
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
  setupFan();
  setupTelegram();

  addToEventLogfile( String("Bedroom fan started. Software version ") + version );

  Serial.println("Init completed");
}

void loop() {

  loopWifi();
  loopFan();
  loopTelegram();

  delay(500);
}
