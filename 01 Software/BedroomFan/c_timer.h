#pragma once

#include <Arduino.h>
#include "esp_system.h"
#include "time.h"
#include <list>
#include <string>
#include <WiFi.h>

using namespace std;

// ======== CONSTANTS ================
const unsigned long MS_PER_SEC  =      1000; 
const unsigned long MS_PER_MIN  = 60 * MS_PER_SEC; 
const unsigned long MS_PER_HOUR = 60 * MS_PER_MIN; 
const unsigned long MS_PER_DAY  = 24 * MS_PER_HOUR; 
const unsigned long MS_PER_WEEK =  7 * MS_PER_DAY; 

// ======== TYPES ================

class milliSecTimer {
  public:
    unsigned long previous;
    unsigned long interval;
    bool autoReset; // Beware, reset happens only when calling lapsed(), and interval is not accurate
    
    // Constructor
    milliSecTimer(unsigned long interval, bool autoReset = true) {
      this->previous = millis();
      this->interval = interval;
      this->autoReset = autoReset;
    }
    
    void reset() { previous = millis(); }
      
    bool lapsed() {
      bool result = (millis() - previous >= interval );
      if(result and autoReset) reset(); // interval could be made more accurate with % operator 
      return result;
    }

    unsigned long remaining() {
      return interval - (millis() - previous);
    }
};
