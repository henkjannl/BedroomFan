# Bedroom fan control
Bedroom ventilator controlled by Telegram.

The bedroom ventilator can be:
* permanently be switched on or off
* controlled by a timer, to automatically switch off after some time (20 minutes, 1 hour or 4 hours).
* controlled by the clock, to automatically switch on between an on and off time

The internal relay is normally closed, so if the ESP32 has no power, the ventilator will switch on.

<img src="05_readonly/photo1.png" alt="Fan control" height="250">
<img src="05_readonly/photo2.png" alt="Fan control" height="250">
<img src="05_readonly/photo3.png" alt="Fan control" height="250">

Currently it has no internal power supply for the ESP32, so the ESP has to be powered seperately by USB.
