#include "eventLog.h"
#include <time.h>

// Ring buffer
static String eventLog[EVENTLOG_SIZE];
static size_t writeIndex = 0;
static size_t eventCount = 0;

// Format timestamp: YYYY-MM-DD HH:MM:SS
static String timeStamp() {
  time_t now;
  struct tm timeinfo;
  char buf[24];

  time(&now);
  localtime_r(&now, &timeinfo);

  snprintf(buf, sizeof(buf),
           "%04d-%02d-%02d %02d:%02d:%02d",
           1900 + timeinfo.tm_year,
           1 + timeinfo.tm_mon,
           timeinfo.tm_mday,
           timeinfo.tm_hour,
           timeinfo.tm_min,
           timeinfo.tm_sec);

  return String(buf);
}

void addToEventLog(const String& event) {
  // Compose entry (bounded)
  eventLog[writeIndex] = timeStamp() + " - " + event;

  writeIndex = (writeIndex + 1) % EVENTLOG_SIZE;
  if (eventCount < EVENTLOG_SIZE) {
    eventCount++;
  }
}

void addToEventLogf(const char *fmt, ...) {
  char buf[128];

  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  addToEventLog(String(buf));
}

String getEventLogAsString() {
  String result;
  result.reserve(512);  // avoid reallocs (tweak if needed)

  // Oldest → newest
  size_t start =
    (eventCount < EVENTLOG_SIZE) ? 0 : writeIndex;

  for (size_t i = 0; i < eventCount; i++) {
    size_t idx = (start + i) % EVENTLOG_SIZE;
    result += eventLog[idx];
    if (i + 1 < eventCount) result += '\n';
  }

  return result;
}

void clearEventLog() {
  for (size_t i = 0; i < EVENTLOG_SIZE; i++) {
    eventLog[i].clear();
  }
  writeIndex = 0;
  eventCount = 0;
}
