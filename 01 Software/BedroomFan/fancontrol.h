#pragma once

#include <Arduino.h>
#include "esp_system.h"
#include "time.h"
#include <list>
#include <string>
#include <WiFi.h>

using namespace std;

// ======== CONSTANTS ================
const uint8_t RELAY_PIN = 18;

// ======== TYPES ================
enum tFanStatus { fsOn, fsOff, fsTimer };
enum tTimerDuration  { tdTimer20, tdTimer60, tdTimer240 };

// ======== GLOBALS ================
tFanStatus fanStatus = fsOff;
tFanStatus prevFanStatus = fsOn;
tTimerDuration timerDuration  = tdTimer20;
milliSecTimer fanTimer = milliSecTimer(  20*60*1000, false ); // Switch off fan if timer is set

// ======== FUNCTIONS ================

void switchOnFan() {
  fanStatus=fsOn;
  digitalWrite(RELAY_PIN, HIGH); 
}

void switchOffFan() {
  fanStatus=fsOff;
  digitalWrite(RELAY_PIN, LOW); 
}

void setFanTimer(tTimerDuration duration) {

  switch(duration) {
    case tdTimer20:
      fanTimer.interval = 20*60*1000;
    break;

    case tdTimer60:
      fanTimer.interval = 60*60*1000;
    break;
    
    case tdTimer240:
      fanTimer.interval = 240*60*1000;
    break;  
  }

  fanTimer.reset();
  timerDuration=duration;
  fanStatus=fsTimer;
  digitalWrite(RELAY_PIN, HIGH); 
}

void setupFan() {
  pinMode(RELAY_PIN, OUTPUT); 
  switchOffFan();
}

void loopFan() {

  // Switch off fan if timer has lapsed
  if( (fanStatus==fsTimer) and (fanTimer.lapsed()) ) {
    switchOffFan();
    addToEventLogfile( String("Timer lapsed. Fan switching off") );
  }

}
