#pragma once

#include <Arduino.h>

constexpr size_t EVENTLOG_SIZE = 20;

void addToEventLog(const String& event);
void addToEventLogf(const char *fmt, ...);
String getEventLogAsString();
void clearEventLog();
