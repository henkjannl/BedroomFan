#include <Arduino.h>

/*******************************************************************
    A telegram bot for ESP32 that controls a
    fan to cool down the bedroom on warm summer days
 *******************************************************************/
#include "version.h"
#include "myCredentials.h"
#include "eventlog.h"
#include "timer.h"
#include "fancontrol.h"
#include "wifi_connect.h"
#include "telegram.h"


/*
MyCredentials.h and MyCredentials.cpp are not included since they are in .gitignore
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

void setup()
{
  delay(500);
  Serial.begin(115200);
  delay(500);

  setupWifi();
  setupFan();
  setupTelegram();

  addToEventLog( String("Bedroom fan started. Software version ") + bf_version);
  Serial.println("Init completed");
}

void loop() {
  loopWifi();
  loopFan();
  loopTelegram();
  delay(500);
}
