#pragma once

#include <Arduino.h>

extern const char* EVT_LOG_FILE;
extern const bool FORMAT_SPIFFS_IF_FAILED;

void removeEventLogfile();
void newEventLogfile();
void addToEventLogfile(const String& event);
