#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include "data.h"

using namespace std;

// ======== GLOBAL HELPER FUNCTIONS =============
void connectToWiFi() {
  
  /* =========================================================
   *  The code below is way more complex than I'd wish
   *  but somehow I could not make wifiMulti work together 
   *  with FreeRTOS
   *  ======================================================== */
   
  //portENTER_CRITICAL(&connectionMux);
  //portENTER_CRITICAL(&dataAccessMux);

  for(auto accessPoint : config.AccessPoints) Serial.println(accessPoint.ssid.c_str());

  int n = WiFi.scanNetworks();
  Serial.printf("%d networks found\n", n);
  
  for (int i = 0; i < n; ++i) {
    for(auto accessPoint : config.AccessPoints) {
      if( WiFi.SSID(i) == String(accessPoint.ssid.c_str()) ) {
          Serial.printf("Connecting to %s\n", accessPoint.ssid.c_str());  
          WiFi.begin(accessPoint.ssid.c_str(), accessPoint.password.c_str());
          for(int j=0; j<35; j++) {
            if (WiFi.status() != WL_CONNECTED)
              {
                Serial.print(".");
                delay(500);
              }
            else {
              Serial.printf("\nWiFi connected to %s.\n", WiFi.SSID());

              // Based on the accesspoint, we know where we are.
              // the weather service can now determine the timezone and daylight saving time
              //portENTER_CRITICAL(&dataAccessMux);
              data.lat=accessPoint.lat;
              data.lon=accessPoint.lon;
              data.timezone=accessPoint.timezone;
              //portEXIT_CRITICAL(&dataAccessMux);  
              data.connected = true;

              break;
            } // not yet connected
            if(data.connected) break; // break j loop
        } // for j
      } // if ssid==ssid
      if(data.connected) break; // break accessPoint loop
    } // for accessPoint
    if(data.connected) break; // break i loop
  } // for i

  Serial.println("Connected");

  data.connected = (WiFi.status() == WL_CONNECTED);
  //portEXIT_CRITICAL(&dataAccessMux);  
  //portEXIT_CRITICAL(&connectionMux);
} // connectToWifi


void syncTime() {
  string url;
  
  //portENTER_CRITICAL(&connectionMux);

  HTTPClient http;

  url=string("http://worldtimeapi.org/api/timezone/") + data.timezone;
  Serial.printf("Retrieving time from %s\n", url.c_str());
  http.begin(url.c_str()); 
  
  int httpCode = http.GET();
  
  if(httpCode > 0) {    
    if(httpCode == HTTP_CODE_OK) {
      StaticJsonDocument<1024> doc;

      DeserializationError error = deserializeJson(doc, http.getString());

      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }

      //portENTER_CRITICAL(&dataAccessMux);
        //const char* abbreviation = doc["abbreviation"]; // "CEST"
        //const char* client_ip = doc["client_ip"]; // "80.100.167.45"
        //const char* datetime = doc["datetime"]; // "2021-04-05T14:28:36.119148+02:00"
        //int day_of_week = doc["day_of_week"]; // 1
        //int day_of_year = doc["day_of_year"]; // 95
        //bool dst = doc["dst"]; // true
        //const char* dst_from = doc["dst_from"]; // "2021-03-28T01:00:00+00:00"
        data.timeDSToffset = doc["dst_offset"]; // 3600
        //const char* dst_until = doc["dst_until"]; // "2021-10-31T01:00:00+00:00"
        data.timeZoneOffset = doc["raw_offset"]; // 3600
        //const char* timezone = doc["timezone"]; // "Europe/Amsterdam"
        //long unixtime = doc["unixtime"]; // 1617625716
        //const char* utc_datetime = doc["utc_datetime"]; // "2021-04-05T12:28:36.119148+00:00"
        //const char* utc_offset = doc["utc_offset"]; // "+02:00"
        //int week_number = doc["week_number"]; // 14
      //portEXIT_CRITICAL(&dataAccessMux);  
      Serial.printf("timezone: %d daylight %d\n", data.timeZoneOffset, data.timeDSToffset);

      configTime(data.timeZoneOffset, data.timeDSToffset, "pool.ntp.org");

      Serial.println("Configtime passed");
    
      //portENTER_CRITICAL(&dataAccessMux);
        data.syncTime = true;
      //portEXIT_CRITICAL(&dataAccessMux);  
    } // httpCode == HTTP_CODE_OK
    else {
      Serial.printf("http error returned: %d\n", httpCode);
    } // httpCode != HTTP_CODE_OK
  } // httpCode > 0
  else {
    Serial.printf("http error returned: %d\n", httpCode);
  } // httpCode <= 0
  
  //portEXIT_CRITICAL(&connectionMux);
  
} // syncTime


#endif // HELPERFUNCTIONS_H
