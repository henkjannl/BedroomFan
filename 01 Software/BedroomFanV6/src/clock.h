#pragma once

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

    TimeOfDay(int hours, int minutes);
    // Default constructor
    TimeOfDay() = default;

    void range_check();
    void add_minutes(int mins);
    bool parse(const std::string& s); // Parse "h:mm" or "hh:mm"
    bool is_due(int now_hours, int now_minutes) const;

    String to_String() const; // Optional: return string like "hh:mm"
    std::string to_string() const;
};
