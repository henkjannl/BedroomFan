/*********************************************************************************************
 *
 *  This project is intended to control a 220V bedroom fan from a Telegram group
 *
 *  Step 1 of the feasibility:
 *  Using an M5 Stack and some cables to check if we can get the relay switch toggling
 *
 *  Setup:
 *
 *  M5 stack with GPIO 18 as digital output
 *  one channel of a 4 channel stepper motor controller to boost the 3.3V output to 5V
 *  TONGLING 5VDC to 250VAC relay to switch the fan
 *
 **********************************************************************************************/

#include <M5Stack.h>
// The setup() function runs once each time the micro-controller starts

void setup() {
  // don't init lcd, SD and I2C, do initialize serial
  M5.begin(false /*LCDEnable*/, false /*SDEnable*/, true /*SerialEnable*/ ,false /*I2CEnable*/);

  Serial.println("Fan off"); 
  //Serial.begin(115200); 
  //M5.Power.begin();
  pinMode(18, OUTPUT);
}

// Add the main program code into the continuous loop() function
void loop() {
  // update button state
  M5.update();
  digitalWrite(18, HIGH);
  Serial.println("Fan on"); 
  delay(1000);
  digitalWrite(18, LOW); 
  Serial.println("Fan off"); 
  delay(1000);  
}
