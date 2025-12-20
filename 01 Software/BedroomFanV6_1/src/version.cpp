#include "version.h"

const String bf_version = "6.2";

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
6.0 Ported to CTBot library
    Limited clock times to 00:00 .. 23:45
    Prevented clock_on to be after clock_off
    Split application in header and cpp files
    Overwriting existing Telegram keyboard + message
6.1 Changed event logging
6.1 Cleaned up WiFi, to keep clock running even if WiFi is lost

To do:
 - check why group chats are not working
 - store settings in NVS
 - do not act on !getLocalTime(&timeinfo)) every loop
 - maybe backup error log once in a while to SPIFFS
*/
