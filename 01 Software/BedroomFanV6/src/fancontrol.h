#pragma once

#include <Arduino.h>
#include "time.h"
#include <WiFi.h>

#include "clock.h"
#include "timer.h"

// ======== CONSTANTS ================
extern const uint8_t RELAY_PIN;
extern const bool C_ON;
extern const bool C_OFF;

// ======== TYPES ================
enum tFanMode { fsOn, fsOff, fsTimer, fsClock };
enum tTimerDuration  { tdTimer20, tdTimer60, tdTimer240 };

// ======== GLOBALS ================
extern tFanMode fanMode;
extern tTimerDuration timerDuration;
extern milliSecTimer fanTimer;

extern TimeOfDay clock_on;
extern TimeOfDay clock_off;

// ======== FUNCTIONS ================
void switchOnFan();  // Switch on fan, do not change mode
void switchOffFan(); // Switch off fan, do not change mode
bool fanIsOn();      // Return true if fan is currently on

void setFanModeOn();
void setFanModeOff();
void setFanModeClock();
void setFanModeTimer(tTimerDuration duration);

void setupFan();
void loopFan();
