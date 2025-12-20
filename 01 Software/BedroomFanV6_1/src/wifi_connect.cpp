#include "wifi_connect.h"

#include <WiFi.h>
#include <WiFiMulti.h>
#include <time.h>

#include "eventLog.h"
#include "timer.h"          // milliSecTimer
#include "myCredentials.h"  // ACCESS_POINTS, localTimezone

// ======== CONSTANTS =================
constexpr uint32_t CONNECT_TIMEOUT_BOOT = 10 * MS_PER_SEC;
constexpr uint32_t CONNECT_TIMEOUT_LOOP = 500;

// ======== STATE =====================
struct WifiState {
  bool wifiReportedConnected = true;
  bool clockSynced = false;
  uint8_t reconnectReported = 0;

  milliSecTimer reconnect1 { 1 * MS_PER_MIN, false };
  milliSecTimer reconnect2 { 2 * MS_PER_MIN, false };
  milliSecTimer reconnect3 { 5 * MS_PER_MIN, false };
  milliSecTimer restart    {10 * MS_PER_MIN, false };
  milliSecTimer syncClock  { 3 * MS_PER_DAY, true };

  WiFiMulti multi;
};

static WifiState wifi;

// ======== CLOCK SYNC =================
static void syncClockIfNeeded() {
  if (WiFi.status() != WL_CONNECTED) {
    wifi.clockSynced = false;
    return;
  }

  if (wifi.clockSynced && !wifi.syncClock.lapsed()) return;

  Serial.println("Sync clock with NTP");
  configTzTime(localTimezone,
               "time.google.com",
               "time.windows.com",
               "pool.ntp.org");

  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 2000)) {
    wifi.clockSynced = true;
    wifi.syncClock.reset();
    addToEventLog("Time synced via NTP");
    Serial.println("  Time sync OK");
  } else {
    Serial.println("  Time sync failed");
  }
}

// ======== SETUP ======================
void setupWifi() {
  Serial.println("Initialize WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  for (const auto &ap : ACCESS_POINTS) {
    wifi.multi.addAP(ap.first.c_str(), ap.second.c_str());
  }

  Serial.println("Connecting WiFi...");
  if (wifi.multi.run(CONNECT_TIMEOUT_BOOT) == WL_CONNECTED) {
    Serial.printf("WiFi connected to %s, RSSI %d dBm\n",
                  WiFi.SSID().c_str(),
                  WiFi.RSSI());
  } else {
    Serial.println("WiFi not connected yet");
  }

  syncClockIfNeeded();
}

// ======== LOOP =======================
void loopWifi() {
  // Just proceed if WiFi is bad, to at least keep the clock running
  // we only lose Telegram functionality then
  syncClockIfNeeded();

  // const bool connected = (WiFi.status() == WL_CONNECTED);

  // if (connected != wifi.wifiReportedConnected) {
  //   char msg[96];

  //   if (connected) {
  //     const char* ssid =
  //       WiFi.SSID().length() ? WiFi.SSID().c_str() : "<unknown>";
  //     snprintf(msg, sizeof(msg),
  //              "WiFi connected to %s, RSSI %d dBm",
  //              ssid, WiFi.RSSI());
  //   } else {
  //     snprintf(msg, sizeof(msg), "WiFi connection lost");
  //     wifi.clockSynced = false;
  //   }

  //   addToEventLog(msg);
  //   // Serial.println(msg);

  //   wifi.wifiReportedConnected = connected;
  // }

  // if (connected) {
  //   wifi.reconnectReported = 0;
  //   wifi.reconnect1.reset();
  //   wifi.reconnect2.reset();
  //   wifi.reconnect3.reset();
  //   wifi.restart.reset();

  //   syncClockIfNeeded();
  //   return;
  // }

  // uint8_t attempt = 0;
  // if (wifi.reconnect3.lapsed())      attempt = 3;
  // else if (wifi.reconnect2.lapsed()) attempt = 2;
  // else if (wifi.reconnect1.lapsed()) attempt = 1;

  // if (attempt > wifi.reconnectReported) {
  //   if (attempt < 3) {
  //     WiFi.disconnect();
  //     WiFi.reconnect();
  //   } else {
  //     wifi.multi.run(CONNECT_TIMEOUT_LOOP);
  //   }

  //   // char msg[96];
  //   // snprintf(msg, sizeof(msg),
  //   //          "%s WiFi reconnect attempt %u",
  //   //          (WiFi.status() == WL_CONNECTED) ? "Successful" : "Unsuccessful",
  //   //          attempt);

  //   // addToEventLog(msg);
  //   // Serial.println(msg);

  //   wifi.reconnectReported = attempt;
  // }

  // if (wifi.reconnectReported >= 3 && wifi.restart.lapsed()) {
  //   addToEventLog("Restarting ESP due to prolonged WiFi loss");
  //   Serial.println("Restarting ESP");
  //   ESP.restart();
  // }
}

String wifiConnectedTo() {
  if (WiFi.status() != WL_CONNECTED) {
    return String("WiFi not connected");
  }

  return String("Connected to ") + WiFi.SSID() +
         String(", RSSI ") + String(WiFi.RSSI()) + String(" dBm");
}
