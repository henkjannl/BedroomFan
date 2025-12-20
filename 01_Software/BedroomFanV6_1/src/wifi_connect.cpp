#include "wifi_connect.h"

#include <esp_wifi.h>
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
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);

  for (const auto &ap : ACCESS_POINTS) {
    wifi.multi.addAP(ap.first.c_str(), ap.second.c_str());
  }

  esp_err_t err = esp_wifi_set_max_tx_power(78);
  if (err != ESP_OK) {
    addToEventLog(String("TX power set failed: ") + String(err));
  }

  Serial.println("Connecting WiFi...");
  if (wifi.multi.run(CONNECT_TIMEOUT_BOOT) == WL_CONNECTED) {
    addToEventLog(String("WiFi connected to ") + WiFi.SSID() +
                  ", RSSI " + String(WiFi.RSSI()) + " dBm");
  } else {
    addToEventLog("WiFi not connected yet");
  }

  syncClockIfNeeded();
}

// ======== LOOP =======================
void loopWifi() {
  static unsigned long lastCheck = 0;
  const unsigned long CHECK_INTERVAL_MS = 10000;

  syncClockIfNeeded();

  if (millis() - lastCheck < CHECK_INTERVAL_MS) return;

  lastCheck = millis();

  if (WiFi.status() != WL_CONNECTED) {
    addToEventLog("WiFi disconnected, retrying...");
    wifi.multi.run();   // triggers scan + reconnect
  }
}

String wifiConnectedTo() {
  String result;

  if (WiFi.status() != WL_CONNECTED) {
    result = "WiFi not connected";
  } else {
    int8_t rssi = WiFi.RSSI();

    result = String("Connected to ") + WiFi.SSID() + " received signal strength ";

    if (rssi >= -40) {
      result+="●●●●●";
    } else if (rssi >= -50) {
      result+="●●●●○";
    } else if (rssi >= -60) {
      result+="●●●○○";
    } else if (rssi >= -70) {
      result+="●●○○○";
    } else if (rssi >= -80) {
      result+="●○○○○";
    } else {
      result+="○○○○○";
    }
  }

  return result;
}
