#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>

/*
EXAMPLE USAGE:

#include <iostream>

int main() {
    TimeOfDay t1(7, 30);

    t1.parse("08:05");

    std::cout << "Stored: " << t1.to_string() << "\n";

    if (t1.is_due(9, 0)) {
        std::cout << "Time has been reached.\n";
    }
}
*/

struct TimeOfDay {
    int minutes_after_midnight = 0;

    // Constructor from hours and minutes
    TimeOfDay(int hours, int minutes) {
        minutes_after_midnight = hours * 60 + minutes;
    }

    // Default constructor
    TimeOfDay() = default;

    void add_minutes(int mins) {
        minutes_after_midnight = (minutes_after_midnight + mins) % (24 * 60);
    }

    // Parse "h:mm" or "hh:mm"
    bool parse(const std::string& s) {
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
        return true;
    }

    // Return true if (now >= stored time)
    bool is_due(int now_hours, int now_minutes) const {
        int now = now_hours * 60 + now_minutes;
        return now >= minutes_after_midnight;
    }

    // Optional: return string like "hh:mm"
    String to_String() const {
        int h = minutes_after_midnight / 60;
        int m = minutes_after_midnight % 60;
        return String(h) + ":" + (m < 10 ? "0" : "") + String(m);
    }

    std::string to_string() const {
        int h = minutes_after_midnight / 60;
        int m = minutes_after_midnight % 60;

        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << h
            << ":"
            << std::setw(2) << std::setfill('0') << m;
        return oss.str();
    }
};
