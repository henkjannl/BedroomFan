/*******************************************************************
    A telegram bot for ESP32 that controls a
    fan to cool down the bedroom on warm summer days
 *******************************************************************/

#include "data.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// ============== TYPES ==============
enum FanStatus_t 
// ============ CONSTANTS ============
const bool FORMAT_SPIFFS_IF_FAILED=false;
const uint8_t RELAY_PIN = 18;
const unsigned long BOT_MTBS = 1000; // mean time between scan messages

// ======== GLOBAL VARIABLES =========
tConfig config; // Configuration data, lives as JSON file in SPIFFS

WiFiMulti wifiMulti;
WiFiClientSecure secured_client;

UniversalTelegramBot bot(config.botToken, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done

// ======== HELPER FUNCTIONS =========

void messageWithKeyboard(String& chat_id, String message) {
  String keyboardJson = "[[\"/on\", \"/off\"],[\"/timer20\"],[\"/timer60\"],[\"/timer120\"]]";
  bot.sendMessageWithReplyKeyboard(chat_id, message, "", keyboardJson, true);            
}

void handleNewMessages(int numNewMessages)
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    if (text == "/on")
    {
      digitalWrite(RELAY_PIN, HIGH); 
      messageWithKeyboard(chat_id, "Fan switched on");
      //bot.sendMessage(chat_id, "Fan is ON", "");
      //String keyboardJson = "[[\"/on\", \"/off\"],[\"/status\"]]";
      //bot.sendMessageWithReplyKeyboard(chat_id, "Fan is on", "", keyboardJson, true);            
    }

    if (text == "/off")
    {
      digitalWrite(RELAY_PIN, LOW); 
      messageWithKeyboard(chat_id, "Fan switched off");
      //bot.sendMessage(chat_id, "Fan is OFF", "");
      //String keyboardJson = "[[\"/on\", \"/off\"],[\"/status\"]]";
      //bot.sendMessageWithReplyKeyboard(chat_id, "Fan is off", "", keyboardJson, true);            
    }

    if (text == "/start")
    {
      messageWithKeyboard(chat_id, "Welcome!");
      //String welcome = "Welcome to the bedroom fan controller, " + from_name + ".\n";
      //welcome += "/on  : to switch the fan ON\n";
      //welcome += "/off : to switch the fan OFF\n";
      //bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}


// ======== MAIN FUNCTIONS =========
void setup()
{
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT); 
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
      Serial.println("SPIFFS Mount Failed");
  }

  config.load(); 

  // Submit accesspoint data from config to the wifi station
  for (auto& accessPoint : config.AccessPoints) {
    Serial.print("Adding wifi access point ");
    Serial.println(accessPoint.ssid);
    wifiMulti.addAP(accessPoint.ssid.c_str(), accessPoint.password.c_str());
  }

  // Connect to wifi
  Serial.print("Connecting Wifi");
  while(wifiMulti.run()!=WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.printf("\nConnected to %s\n", WiFi.SSID());

  // Sync the time
  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(300);
    now = time(nullptr);
  }
  Serial.println(now);
  Serial.print("BotName ");
  Serial.print(config.botName);
  Serial.print(" token ");
  Serial.println(config.botToken);

  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  bot.updateToken(config.botToken);
}

void loop()
{
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}
