#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <Arduino.h>

#include "clock.h"

TimeOfDay::TimeOfDay(int hours, int minutes) {
    minutes_after_midnight = hours * 60 + minutes;
    range_check();
}

void TimeOfDay::range_check() {
    if (minutes_after_midnight < 0)
        minutes_after_midnight = 0;
    else if (minutes_after_midnight >= 24 * 60)
        minutes_after_midnight = (24 * 60) - 1;
}

void TimeOfDay::add_minutes(int mins) {
    minutes_after_midnight = (minutes_after_midnight + mins) % (24 * 60);
    range_check();
}

// Parse "h:mm" or "hh:mm"
bool TimeOfDay::parse(const std::string& s) {
    int h, m;
    char colon;

    std::istringstream iss(s);
    if (!(iss >> h >> colon >> m))
        return false;
    if (colon != ':')
        return false;
    if (h < 0 || h > 23 || m < 0 || m > 59)
        return false;

    minutes_after_midnight = h * 60 + m;
    range_check();
    return true;
}

// Return true if (now >= stored time)
bool TimeOfDay::is_due(int now_hours, int now_minutes) const {
    int now = now_hours * 60 + now_minutes;
    return now >= minutes_after_midnight;
}

// Optional: return string like "hh:mm"
String TimeOfDay::to_String() const {
    int h = minutes_after_midnight / 60;
    int m = minutes_after_midnight % 60;
    return String(h) + ":" + (m < 10 ? "0" : "") + String(m);
}

std::string TimeOfDay::to_string() const {
    int h = minutes_after_midnight / 60;
    int m = minutes_after_midnight % 60;

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << h
        << ":"
        << std::setw(2) << std::setfill('0') << m;
    return oss.str();
}
