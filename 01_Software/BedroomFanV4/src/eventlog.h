#pragma once

#include "FS.h"
#include "SPIFFS.h"
#include <AsyncTelegram2.h>

#define EVT_LOG_FILE "/EventLog.csv"

#define FORMAT_SPIFFS_IF_FAILED true

void removeEventLogfile() {
  SPIFFS.remove( EVT_LOG_FILE );
}

void newEventLogfile() {

  if( SPIFFS.exists( EVT_LOG_FILE ) ) removeEventLogfile();

  File file = SPIFFS.open(EVT_LOG_FILE, FILE_APPEND);
  file.print( F("Date, Time, Event\n" ) );
  file.close();
}

void addToEventLogfile( String event ) {
  time_t rawtime;
  struct tm * timeinfo;
  char item[32];

  if( !SPIFFS.exists( EVT_LOG_FILE ) ) newEventLogfile();

  time (&rawtime);
  timeinfo = localtime (&rawtime);

  snprintf( item, sizeof(item), "%04d-%02d-%02d, %02d:%02d:%02d, ", 
    1900+timeinfo->tm_year, 
    1+timeinfo->tm_mon, 
    timeinfo->tm_mday,
    timeinfo->tm_hour, 
    timeinfo->tm_min, 
    timeinfo->tm_sec );
  String content = String( item ) + event + "\n";

  File file = SPIFFS.open(EVT_LOG_FILE, FILE_APPEND);
  file.print( content );
  file.close();
} 

