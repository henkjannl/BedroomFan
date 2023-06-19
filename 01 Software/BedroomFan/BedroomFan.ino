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

To do:
  Sync time every few days
*/

#include "MyCredentials.h"
#include "eventlog.h"
#include "timer.h"
#include "fancontrol.h"
#include "wifi.h"
#include "telegram.h"
#include "ota.h"

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
