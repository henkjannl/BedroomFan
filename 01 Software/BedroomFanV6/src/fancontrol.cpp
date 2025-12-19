#include "fanControl.h"
#include "esp_system.h"
#include <list>
#include <string>
#include "timer.h"
#include "eventLog.h"

using namespace std;

// ======== CONSTANTS ================
const uint8_t RELAY_PIN = 18;
const bool C_ON = LOW;
const bool C_OFF = HIGH;

// ======== GLOBALS ================
bool fan_on = true;
tFanMode fanMode = fsClock;
tTimerDuration timerDuration  = tdTimer20;
milliSecTimer fanTimer = milliSecTimer(20*60*1000, false);

TimeOfDay clock_on (16, 30);
TimeOfDay clock_off(22, 00);

// ======== FUNCTIONS ================
void switchOnFan() {
  fan_on = true;
  digitalWrite(RELAY_PIN, C_ON);
}

void switchOffFan() {
  fan_on = false;
  digitalWrite(RELAY_PIN, C_OFF);
}

void setFanModeOn() {
  fanMode = fsOn;
  switchOnFan();
}

void setFanModeOff() {
  fanMode = fsOff;
  switchOffFan();
}

void setFanModeClock() {
  fanMode = fsClock;
}

void setFanModeTimer(tTimerDuration duration) {

  switch(duration) {
    case tdTimer20:  fanTimer.interval = 20*60*1000; break;
    case tdTimer60:  fanTimer.interval = 60*60*1000; break;
    case tdTimer240: fanTimer.interval = 240*60*1000; break;
  }

  fanTimer.reset();
  timerDuration = duration;
  fanMode = fsTimer;
  switchOnFan();
}

void setFanClockMode() {
  fanMode = fsClock;
}

void setupFan() {
  pinMode(RELAY_PIN, OUTPUT);
  setFanModeClock();
}

void loopFan() {

  if (fanMode == fsTimer && fanTimer.lapsed()) {
    switchOffFan();
    addToEventLogfile(String("Timer lapsed. Fan switching off"));
  }

  if (fanMode == fsClock) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;

    bool fan_must_be_on =
      clock_on.is_due(timeinfo.tm_hour, timeinfo.tm_min) &&
     !clock_off.is_due(timeinfo.tm_hour, timeinfo.tm_min);

    if (fan_must_be_on && !fanIsOn()) {
      switchOnFan();
      addToEventLogfile(String("Clock time reached. Fan switching on"));
    }

    if (!fan_must_be_on && fanIsOn()) {
      switchOffFan();
      addToEventLogfile(String("Clock time reached. Fan switching off"));
    }
  }
}

bool fanIsOn() {
  return fan_on;
}