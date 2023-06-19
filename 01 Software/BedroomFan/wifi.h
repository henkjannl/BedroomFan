#pragma once

#include <WiFi.h>
#include <WiFiMulti.h>
#include <map>

#include "timer.h"          // milliSecTimer
#include "MyCredentials.h"  // This file contains private data such as wifi passwords

// ======== CONSTANTS ================
const uint32_t CONNECT_TIMEOUT_MS = 10*MS_PER_SEC;

// ======== GLOBALS ================
milliSecTimer reconnectTimer1 = milliSecTimer(  1*MS_PER_MIN, false ); // Reconnect attempt 1 after 1 min
milliSecTimer reconnectTimer2 = milliSecTimer(  2*MS_PER_MIN, false ); // Reconnect attempt 2 after 2 min
milliSecTimer reconnectTimer3 = milliSecTimer(  5*MS_PER_MIN, false ); // Reconnect attempt 3 after 5 min
milliSecTimer restartTimer    = milliSecTimer( 10*MS_PER_MIN, false ); // Restart ESP after 10 min

milliSecTimer syncTimeTimer   = milliSecTimer(  3*MS_PER_DAY, true  ); // Sync time every 3 days

WiFiMulti wifiMulti;

void syncTime() {
  // Sync time with NTP
  Serial.println("Sync clock with timeserver");
  configTzTime(localTimezone, "time.google.com", "time.windows.com", "pool.ntp.org");
  
  struct tm timeinfo;
  
  if( getLocalTime(&timeinfo) ){
    Serial.println("  Got the time from NTP");  
    syncTimeTimer.reset();
    addToEventLogfile( "Time synched" );
  } else {
    Serial.println("  Failed to obtain time");
  }
}

void setupWifi() {
  // Initialize Wifi
  Serial.println( "Initialize WiFi" );
  WiFi.mode(WIFI_STA);

  // Add wifi access points 
  for (const auto &ap : ACCESS_POINTS ) {
    //Serial.printf( "%s %s\n", ap.first.c_str(), ap.second.c_str() );
    wifiMulti.addAP( ap.first.c_str(), ap.second.c_str() );
  }

  Serial.println("Connecting Wifi...");
  if(wifiMulti.run( CONNECT_TIMEOUT_MS ) == WL_CONNECTED) {
    Serial.printf( "WiFi connected to SSID %s signal strength %ddB\n", WiFi.SSID().c_str(), WiFi.RSSI() );
  }
  else {
    Serial.println("WiFi not connected yet");
  }

  WiFi.setAutoReconnect(true);

  syncTimeTimer.previous = 0; // Time was not synched yet
  syncTime();
};

void loopWifi() {
  static bool wifiConnectionReported = true;
  static uint8_t reconnectReport = 0;
  uint8_t reconnectAttempt = 0;
  char item[80];
  struct tm timeinfo;
  
  bool wifiConnected = ( WiFi.status() == WL_CONNECTED );

  // Report if connection status is changed
  if( wifiConnected != wifiConnectionReported ) {

    if( wifiConnected ) {
      snprintf( item, sizeof(item), "WiFi connected to SSID %s. Signal strength %ddB.", WiFi.SSID().c_str(), WiFi.RSSI() );
    } else {
      snprintf( item, sizeof(item), "WiFi connection is lost." );
    }
    addToEventLogfile( item );
    Serial.println( item );

    wifiConnectionReported = wifiConnected;
  }

  // Reset all timers if wifi is connected
  if ( wifiConnected ) {
      reconnectReport = 0;
      reconnectTimer1.reset();
      reconnectTimer2.reset();
      reconnectTimer3.reset();
      restartTimer.reset();
  }

  reconnectAttempt = 0;
  if( reconnectTimer1.lapsed() ) reconnectAttempt = 1;
  if( reconnectTimer2.lapsed() ) reconnectAttempt = 2;
  if( reconnectTimer3.lapsed() ) reconnectAttempt = 3;

  if( reconnectAttempt > reconnectReport ) {

    if( reconnectAttempt<3 ) {
      // Try to reconnect to the previous network
      WiFi.disconnect();
      WiFi.reconnect();
    } else {
      // Try to connect to the strongest available network
      wifiMulti.run( CONNECT_TIMEOUT_MS );
    }

    snprintf( item, sizeof(item), "%s attempt to reconnect WiFi after %d attempts", 
      ( WiFi.status() == WL_CONNECTED) ? "Successful" : "Unsuccessful", reconnectAttempt );

    addToEventLogfile( item );
    reconnectReport = reconnectAttempt;
  } // reconnectAttempt > reconnectReport

  if( restartTimer.lapsed() ) {
    // After three unsuccessful attempts, restart the ESP
    addToEventLogfile( "Restarting ESP due to loss of WiFi connection" );
    ESP.restart();
  };

  // If time was not yet synched, but wifi is enabled, try synching with timeserver
  if ( (WiFi.status() == WL_CONNECTED) and syncTimeTimer.lapsed() ) syncTime();

}; // loopWifi()
