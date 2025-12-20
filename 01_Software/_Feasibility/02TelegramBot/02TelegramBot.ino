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
enum tFanStatus { fsOn, fsOff, fsTimer };
enum tFanCommand { fcOn, fcOff, fcTimer20, fcTimer60, fcTimer120, fcNone };

// ============ CONSTANTS ============
const bool FORMAT_SPIFFS_IF_FAILED=false;
const uint8_t RELAY_PIN = 18;
const unsigned long BOT_MTBS = 1000; // mean time between scan messages
const char* const commands[] = {"💡 Fan on", "🛑 Fan off", "⏳ 20 min", "⏳ 1 hour", "⏳ 2 hours" };
//const char* const commands[] = {"/U0001F4A1 Fan on", "/U0001F6D1 Fan off", "/U000023F3 20 min", "/U000023F3 1 hour", "/U000023F3 2 hours" };

// ======== GLOBAL VARIABLES =========
tConfig config; // Configuration data, lives as JSON file in SPIFFS
tFanStatus fanStatus;
volatile int timerCountDown;
volatile unsigned int applicationRunning;

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

WiFiMulti wifiMulti;
WiFiClientSecure secured_client;

UniversalTelegramBot bot(config.botToken, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done

// ======== HELPER FUNCTIONS =========

void messageWithKeyboard(String& chat_id, String message) {
  String keyboardJson = "[[\"" + String(commands[fcOn]) + "\", \"" + String(commands[fcOff]) + "\"], [\"" + String(commands[fcTimer20]) + "\", \"" + String(commands[fcTimer60]) + "\", \"" + String(commands[fcTimer120]) + "\"]]";
  
  bot.sendMessageWithReplyKeyboard(chat_id, message, "Markdown", keyboardJson, true);            
}

void handleNewMessages(int numNewMessages) {
  tFanCommand command=fcNone; // Buffer commands until all messages have been processed, then execute last command

  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == String(commands[fcOn])) {
      messageWithKeyboard(chat_id, "Fan switched on");
      command=fcOn;
    }

    if (text == String(commands[fcOff])) {
      messageWithKeyboard(chat_id, "Fan switched off");
      command=fcOff;
    }

    if (text == String(commands[fcTimer20])) {
      messageWithKeyboard(chat_id, "Fan will switch off after 20 minutes");
      command=fcTimer20;
    }

    if (text == String(commands[fcTimer60])) {
      messageWithKeyboard(chat_id, "Fan will switch off after 1 hour");
      command=fcTimer60;
    }

    if (text == String(commands[fcTimer120])) {
      messageWithKeyboard(chat_id, "Fan will switch off after 2 hours");
      command=fcTimer120;
    }

    // Forgive users for using capitals
    text.toLowerCase();

    if (text == "/start")
    {
      messageWithKeyboard(chat_id, "Welcome!");
    }

    if (text == "/status")
    {
      String answer="Ventilator status: *";
      if      (fanStatus==fsOff) answer+="Off*\n";
      else if (fanStatus==fsOn ) answer+="On*\n";
      else                       answer+="Timer*\n";

      if(fanStatus==fsTimer) answer+="Timer has *"+ String(1.0*timerCountDown/60, 2)+"* minutes left\n";
      
      answer+= "Application running for *" + String(1.0*applicationRunning/3600, 3)+"* hours\n";
      answer+= "Chat ID *" + String(chat_id) + "*\n";
      answer+= "Chat ID in config *" + String(config.botChatID) + "*\n";
      answer+= "Free heap *" + String(esp_get_free_heap_size()) + "* bytes\n";
      answer+= "Minimum heap *" + String(esp_get_minimum_free_heap_size()) + "* bytes\n";
      
      messageWithKeyboard(chat_id, answer);
    } // message=status
  } // i<numMessages

  // Only the last command for the fan is executed to prevent frying the relay after loss of connection
  switch(command) {
    case fcNone:
    break;
    
    case fcOn:
      digitalWrite(RELAY_PIN, HIGH); 
      fanStatus=fsOn;
    break;
    
    case fcOff:
      digitalWrite(RELAY_PIN, LOW); 
      fanStatus=fsOff;
    break;
    
    case fcTimer20:
      digitalWrite(RELAY_PIN, HIGH); 
      fanStatus=fsTimer;
      timerCountDown=20*60;
    break;
    
    case fcTimer60:
      digitalWrite(RELAY_PIN, HIGH); 
      fanStatus=fsTimer;
      timerCountDown=60*60;
    break;    

    case fcTimer120:
      digitalWrite(RELAY_PIN, HIGH); 
      fanStatus=fsTimer;
      timerCountDown=120*60;
    break;    
  }
}
 
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  timerCountDown--;
  applicationRunning++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void connect() {
  Serial.print("Connecting Wifi");
  while(wifiMulti.run()!=WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.printf("\nConnected to %s\n", WiFi.SSID());
}

// ======== MAIN FUNCTIONS =========
void setup()
{
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT); 
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) Serial.println("SPIFFS Mount Failed");

  config.load(); 

  fanStatus=fsOff;

  // Setup the timer
  applicationRunning=0;
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);

  // Submit accesspoint data from config to the wifi station
  for (auto& accessPoint : config.AccessPoints) {
    Serial.print("Adding wifi access point ");
    Serial.println(accessPoint.ssid);
    wifiMulti.addAP(accessPoint.ssid.c_str(), accessPoint.password.c_str());
  }

  // Connect to wifi
  connect();
  
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
  Serial.println("\nTime sync completed");

  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  bot.updateToken(config.botToken);
  messageWithKeyboard(config.botChatID, "\u270B The bedroom ventilator has started");
  
  Serial.println("Init completed");
}

void loop() {

  // Check if the timer is expired
  if( (fanStatus==fsTimer) & (timerCountDown<=0) ) {
    fanStatus=fsOff;
    digitalWrite(RELAY_PIN, LOW); 
  }

  // Check if we are still connected
  if (WiFi.status()!=WL_CONNECTED) connect();

  // Check telegram for incoming messages
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}
