#pragma once

/*
  telegram_ctbot.h

  Drop-in replacement for your previous AsyncTelegram2-based telegram.h.

  - Uses CTBot instead of AsyncTelegram2
  - Uses CTBotInlineKeyboard for inline keyboards
  - Fixes callback-data collisions for clock OFF buttons
  - Fixes a keyboard construction bug (the "Main menu" button for the clock keyboard was being added to the wrong keyboard)
  - Fixes navigation logic: Clock ON/OFF buttons now open the clock edit keyboard (instead of bouncing back to Settings)

  Notes:
  - CTBot does not have the same "edit message" API as AsyncTelegram2; this implementation replies with a fresh message.
  - "Download event log": instead of sending a file, this sends the log contents as text (chunked to Telegram's 4096 char limit).
*/

#include <ArduinoJson.h>
#include <CTBot.h>
#include <map>
#include <FS.h>
#include <SPIFFS.h>

#include "fancontrol.h"

using namespace std;

// ======== DEFINES ==================
// Main menu
#define CB_FAN_ON     "cbFanOn"
#define CB_FAN_OFF    "cbFanOff"
#define CB_FAN_CLOCK  "cbFanClock"
#define CB_TMR_20MIN  "cb20min"
#define CB_TMR_1HR    "cb1hr"
#define CB_TMR_4HRS   "cb4hrs"
#define CB_SETTINGS   "cbSettings"

// Settings menu
#define CB_SET_CLOCK   "cbSetClock"
#define CB_EVENTLOG   "cbEventLog"
#define CB_EVENTCLR   "cbEventClr"
#define CB_STATUS     "cbStatus"
#define CB_MAIN       "cbMain"

// Clock edit menu (clock_on)
#define CB_CLK_ON_MHR     "cbClkOnMHr"   // Clock ON minus hour
#define CB_CLK_ON_PHR     "cbClkOnPHr"   // Clock ON plus hour
#define CB_CLK_ON_M15     "cbClkOnM15"   // Clock ON minus 15 minutes
#define CB_CLK_ON_P15     "cbClkOnP15"   // Clock ON plus 15 minutes

// Clock edit menu (clock_off)
#define CB_CLK_OFF_MHR    "cbClkOffMHr"  // Clock OFF minus hour
#define CB_CLK_OFF_PHR    "cbClkOffPHr"  // Clock OFF plus hour
#define CB_CLK_OFF_M15    "cbClkOffM15"  // Clock OFF minus 15 minutes
#define CB_CLK_OFF_P15    "cbClkOffP15"  // Clock OFF plus 15 minutes


// ======== CONSTANTS ================
const char EMOTICON_WELCOME[]   = { 0xf0, 0x9f, 0x99, 0x8b, 0xe2, 0x80, 0x8d, 0xe2, 0x99, 0x80, 0xef, 0xb8, 0x8f, 0x00 };
const char EMOTICON_STOP[]      = { 0xf0, 0x9f, 0x9b, 0x91, 0x00 };
const char EMOTICON_WIND[]      = { 0xf0, 0x9f, 0x92, 0xa8, 0x00 };
const char EMOTICON_HOURGLASS[] = { 0xe2, 0x8f, 0xb3, 0x00 };
const char EMOTICON_FINISH[]    = { 0xf0, 0x9f, 0x8f, 0x81, 0x00 };
const char EMOTICON_EVENTLOG[]  = { 0xf0, 0x9f, 0x93, 0x9d, 0x00 };
const char EMOTICON_CLEAR[]     = { 0xf0, 0x9f, 0x97, 0x91, 0x00 };     // Garbage bin
const char EMOTICON_SETTINGS[]  = { 0xe2, 0x9a, 0x99, 0xef, 0xb8, 0x8f, 0x00 };
const char EMOTICON_STATUS[]    = { 0xf0, 0x9f, 0xa9, 0xba, 0x00 };     // Stethoscope
const char EMOTICON_MAIN[]      = { 0xf0, 0x9f, 0x94, 0x99, 0x00 };     // Back arrow
const char EMOTICON_CLOCK[]     = { 0xf0, 0x9f, 0x95, 0x90, 0x00 };     // Clock
const char EMOTICON_VERSION[]   = { 0xf0, 0x9f, 0xa7, 0xa0, 0x00 };  // 🧠

// ======== TYPES ================
enum keyboard_t { kbMain, kbSettings, kbClock };

// ======== GLOBALS =================
CTBot myBot;

CTBotInlineKeyboard mainKeyboard;
CTBotInlineKeyboard settingsKeyboard;
CTBotInlineKeyboard clockKeyboard;

keyboard_t currentKeyboard = kbMain;

static std::map<int64_t, int32_t> lastMessageId;

// Map keyboard enum -> object
std::map<keyboard_t, CTBotInlineKeyboard*> KEYBOARDS = {
  { kbMain,     &mainKeyboard     },
  { kbSettings, &settingsKeyboard },
  { kbClock,    &clockKeyboard    },
};

// ======== HELPERS =================
static void sendOrEdit(int64_t chatId, const String& text, CTBotInlineKeyboard* kbd = nullptr) {
  int32_t& msgId = lastMessageId[chatId];

  Serial.print("sendOrEdit messageID ");
  Serial.print(msgId);
  Serial.print("chatID ");
  Serial.print(chatId);
  Serial.print((kbd) ? "kbd " : "no kbd ");
  Serial.print("text ");
  Serial.println(text);

  if (msgId > 0) {
    if (kbd) myBot.editMessageText(chatId, msgId, text, *kbd);
    else     myBot.editMessageText(chatId, msgId, text);
  } else {
    if (kbd) msgId = myBot.sendMessage(chatId, text, *kbd);
    else     msgId = myBot.sendMessage(chatId, text);
  }
}

static String convertToHexString(String input) {
  String result = "{ ";
  for (int i = 0; i < input.length(); i++) {
    result += String("0x") + String((int)input[i], HEX) + ", ";
  }
  result += "0x00 };";
  result.toLowerCase();
  Serial.println(input + ": " + result);
  return result;
}

static void sendMessageToKeyUser(String msg) {
  // keep same behavior: always attach the main keyboard
  myBot.sendMessage((uint32_t)userid, msg, mainKeyboard);
}

static void sendLongMessage(uint32_t chatId, const String &text, CTBotInlineKeyboard *kbd = nullptr) {
  // Telegram text message max is 4096 chars.
  const size_t MAXLEN = 4096;
  size_t start = 0;

  while (start < text.length()) {
    size_t chunkLen = min(MAXLEN, text.length() - start);
    String chunk = text.substring(start, start + chunkLen);

    if (kbd && start == 0) myBot.sendMessage(chatId, chunk, *kbd);
    else                  myBot.sendMessage(chatId, chunk);

    start += chunkLen;
  }
}

// ======== KEYBOARD BUILDERS =======
static void buildMainKeyboard() {
  mainKeyboard.flushData();

  String btn;

  btn = String(EMOTICON_WIND) + " Fan on";
  mainKeyboard.addButton(btn, CB_FAN_ON, CTBotKeyboardButtonQuery);

  btn = String(EMOTICON_CLOCK) + " Clock";
  mainKeyboard.addButton(btn, CB_FAN_CLOCK, CTBotKeyboardButtonQuery);

  btn = String(EMOTICON_STOP) + " Fan off";
  mainKeyboard.addButton(btn, CB_FAN_OFF, CTBotKeyboardButtonQuery);
  mainKeyboard.addRow();

  btn = String(EMOTICON_HOURGLASS) + " 20 min";
  mainKeyboard.addButton(btn, CB_TMR_20MIN, CTBotKeyboardButtonQuery);

  btn = String(EMOTICON_HOURGLASS) + " 1 hour";
  mainKeyboard.addButton(btn, CB_TMR_1HR, CTBotKeyboardButtonQuery);

  btn = String(EMOTICON_HOURGLASS) + " 4 hours";
  mainKeyboard.addButton(btn, CB_TMR_4HRS, CTBotKeyboardButtonQuery);
  mainKeyboard.addRow();

  btn = String(EMOTICON_SETTINGS) + " Settings";
  mainKeyboard.addButton(btn, CB_SETTINGS, CTBotKeyboardButtonQuery);
  mainKeyboard.addRow();
}

static void buildSettingsKeyboard() {
  settingsKeyboard.flushData();

  String btn;

  btn = String(EMOTICON_CLOCK) + " Set clock time";
  settingsKeyboard.addButton(btn, CB_SET_CLOCK, CTBotKeyboardButtonQuery);

  btn = String(EMOTICON_STATUS) + " Status";
  settingsKeyboard.addButton(btn, CB_STATUS, CTBotKeyboardButtonQuery);

  settingsKeyboard.addRow();

  btn = String(EMOTICON_EVENTLOG) + " Download event log";
  settingsKeyboard.addButton(btn, CB_EVENTLOG, CTBotKeyboardButtonQuery);

  btn = String(EMOTICON_CLEAR) + " Clear event log";
  settingsKeyboard.addButton(btn, CB_EVENTCLR, CTBotKeyboardButtonQuery);
  settingsKeyboard.addRow();

  btn = String(EMOTICON_MAIN) + " Main menu";
  settingsKeyboard.addButton(btn, CB_MAIN, CTBotKeyboardButtonQuery);
  settingsKeyboard.addRow();
}

static void buildClockKeyboard() {
  clockKeyboard.flushData();

  String btn;

  // Row 1: ON hour +/-
  btn = "< ON hr";
  clockKeyboard.addButton(btn, CB_CLK_ON_MHR, CTBotKeyboardButtonQuery);
  btn = "ON hr >";
  clockKeyboard.addButton(btn, CB_CLK_ON_PHR, CTBotKeyboardButtonQuery);
  clockKeyboard.addRow();

  // Row 2: ON 15 +/-
  btn = "< ON 15";
  clockKeyboard.addButton(btn, CB_CLK_ON_M15, CTBotKeyboardButtonQuery);
  btn = "ON 15 >";
  clockKeyboard.addButton(btn, CB_CLK_ON_P15, CTBotKeyboardButtonQuery);
  clockKeyboard.addRow();

  // Row 3: OFF hour +/-
  btn = "< OFF hr";
  clockKeyboard.addButton(btn, CB_CLK_OFF_MHR, CTBotKeyboardButtonQuery);
  btn = "OFF hr >";
  clockKeyboard.addButton(btn, CB_CLK_OFF_PHR, CTBotKeyboardButtonQuery);
  clockKeyboard.addRow();

  // Row 4: OFF 15 +/-
  btn = "< OFF 15";
  clockKeyboard.addButton(btn, CB_CLK_OFF_M15, CTBotKeyboardButtonQuery);
  btn = "OFF 15 >";
  clockKeyboard.addButton(btn, CB_CLK_OFF_P15, CTBotKeyboardButtonQuery);
  clockKeyboard.addRow();

  // Back to main menu  (FIX: was added to settingsKeyboard in the old code)
  btn = String(EMOTICON_MAIN) + " Main menu";
  clockKeyboard.addButton(btn, CB_MAIN, CTBotKeyboardButtonQuery);
  clockKeyboard.addRow();
}

static void buildAllKeyboards() {
  buildMainKeyboard();
  buildSettingsKeyboard();
  buildClockKeyboard();
}

static String StatusMessage() {
  String result;
  char item[80];
  float remainingSeconds;

  switch (fanMode) {
    case fsOn:
      result = String(EMOTICON_WIND) + " Fan is permanently switched on";
      break;
    case fsOff:
      result = String(EMOTICON_STOP) + " Fan is permanently switched off";
      break;
    case fsTimer:
      remainingSeconds = fanTimer.remaining() / 1000.0f;
      if (remainingSeconds >= 60)
        snprintf(item, sizeof(item), " Fan will switch off after %d minutes", int(remainingSeconds / 60));
      else
        snprintf(item, sizeof(item), " Fan will switch off after %d seconds", int(remainingSeconds));
      result = String(EMOTICON_HOURGLASS) + item;
      break;
    case fsClock:
      result =  String(EMOTICON_CLOCK) + " Fan is switched on from "  + clock_on.to_String() +
        String(" to ") + clock_off.to_String() + String(". It is currently ") + (fanIsOn() ? "on." : "off.");
      break;
  }

  return result;
}

static String clockStatus() {
  String result;
  result  = String("Fan on from ")  + clock_on.to_String() + " until " + clock_off.to_String();
  return result;
}

// ======== CALLBACK / COMMAND HANDLING =======
static void handleCallback(const TBMessage &msg) {
  String newMessage;
  String userName = msg.sender.firstName + " " + msg.sender.lastName;

  const String &cb = msg.callbackQueryData;
  uint8_t time_changed = 0;

  newMessage = "";
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char buf[32];
    strftime(buf, sizeof(buf), "%H:%M ", &timeinfo);
    newMessage = String(buf);
  }

  // MAIN actions
  if (cb == CB_FAN_ON) {
    setFanModeOn();
    addToEventLogfile(String("Fan switched on by ") + userName);
    switchOnFan();
    currentKeyboard = kbMain;
  }
  else if (cb == CB_FAN_OFF) {
    setFanModeOff();
    addToEventLogfile(String("Fan switched off by ") + userName);
    switchOffFan();
    currentKeyboard = kbMain;
  }
  else if (cb == CB_FAN_CLOCK) {
    addToEventLogfile(String("Fan switched to clock mode by ") + userName);
    setFanModeClock();
    currentKeyboard = kbMain;
  }
  else if (cb == CB_TMR_20MIN) {
    addToEventLogfile(String("Fan switched on for 20 minutes by ") + userName);
    setFanModeTimer(tdTimer20);
    currentKeyboard = kbMain;
  }
  else if (cb == CB_TMR_1HR) {
    addToEventLogfile(String("Fan switched on for 1 hour by ") + userName);
    setFanModeTimer(tdTimer60);
    currentKeyboard = kbMain;
  }
  else if (cb == CB_TMR_4HRS) {
    addToEventLogfile(String("Fan switched on for 4 hours by ") + userName);
    setFanModeTimer(tdTimer240);
    currentKeyboard = kbMain;
  }
  else if (cb == CB_SETTINGS) {
    newMessage += String(EMOTICON_SETTINGS) + " Settings menu\n";
    currentKeyboard = kbSettings;
  }

  // SETTINGS actions
  else if (cb == CB_SET_CLOCK) {
    currentKeyboard = kbClock;
  }
  else if (cb == CB_STATUS) {
    newMessage += String(EMOTICON_VERSION) + String("Software version: ") + bf_version + "\n";
    newMessage += wifiConnectedTo() + "\n";
    currentKeyboard = kbSettings;
  }
  else if (cb == CB_EVENTLOG) {
    newMessage += String(EMOTICON_EVENTLOG) + " Event log:\n";
    addToEventLogfile(String("Event log requested by ") + userName);

    // Send the log as text. (If you prefer file upload, we can switch once you confirm your CTBot version supports it.)
    File file = SPIFFS.open(EVT_LOG_FILE, "r");
    if (file) {
      while (file.available()) {
        newMessage += char(file.read());
      }
      file.close();
    } else {
      newMessage += "Can't open the event log file\n";
    }
    currentKeyboard = kbSettings;
  }
  else if (cb == CB_EVENTCLR) {
    newMessage += String(EMOTICON_EVENTLOG) + " Event log cleared\n";
    newEventLogfile();
    addToEventLogfile(String("Event log cleared by ") + userName);
    currentKeyboard = kbSettings;
  }
  else if (cb == CB_MAIN) {
    currentKeyboard = kbMain;
  }

  // Clock edit actions
  else if (cb == CB_CLK_ON_MHR)  { clock_on. add_minutes(-60); time_changed=1; }
  else if (cb == CB_CLK_ON_PHR)  { clock_on. add_minutes( 60); time_changed=1; }
  else if (cb == CB_CLK_ON_M15)  { clock_on. add_minutes(-15); time_changed=1; }
  else if (cb == CB_CLK_ON_P15)  { clock_on. add_minutes( 15); time_changed=1; }
  else if (cb == CB_CLK_OFF_MHR) { clock_off.add_minutes(-60); time_changed=2; }
  else if (cb == CB_CLK_OFF_PHR) { clock_off.add_minutes( 60); time_changed=2; }
  else if (cb == CB_CLK_OFF_M15) { clock_off.add_minutes(-15); time_changed=2; }
  else if (cb == CB_CLK_OFF_P15) { clock_off.add_minutes( 15); time_changed=2; }

  else {
    newMessage = "Command not recognized";
  }
  // end of callback recognition


  // Common actions if the time was changed
  if(time_changed>0) {

    addToEventLogfile(String("Clock time changed by ") + userName);
    currentKeyboard = kbClock;

    if(time_changed==1) {
      if (clock_on.minutes_after_midnight >= clock_off.minutes_after_midnight) {
        clock_on.minutes_after_midnight = clock_off.minutes_after_midnight-15;
      }
    } else if(time_changed==2) {
      if (clock_off.minutes_after_midnight <= clock_on.minutes_after_midnight) {
        clock_off.minutes_after_midnight = clock_on.minutes_after_midnight+15;
      }
    }
    newMessage += String(EMOTICON_CLOCK) + " " + clockStatus();
  }
  else {
    newMessage += StatusMessage();
  }

  // Answer the callback query (Telegram UI spinner)
  myBot.endQuery(msg.callbackQueryID, "OK");

  // Send response with current keyboard
  CTBotInlineKeyboard *kbd = KEYBOARDS[currentKeyboard];

  if (cb == CB_EVENTLOG) {
    // event log can be long -> chunk it (first chunk includes keyboard)
    sendLongMessage((uint32_t)msg.sender.id, newMessage, kbd);
  } else {
    sendOrEdit((uint32_t)msg.sender.id, newMessage, kbd);
  }

  Serial.println(newMessage);
}

// ======== PUBLIC API =======

static void setupTelegram() {
  // You can also call myBot.wifiConnect(ssid, pass) elsewhere; here we just setup token.
  myBot.enableUTF8Encoding(true);    // keeps emojis sane
  myBot.setTelegramToken(String(token));

  buildAllKeyboards();
  currentKeyboard = kbMain;

  // Send welcome message
  TBMessage msg;
  int64_t chatId = (int64_t)userid;   // private chat assumption
  String text = String(EMOTICON_WELCOME) + " Welcome!\n" + StatusMessage();
  lastMessageId[chatId] = myBot.sendMessage(chatId, text, *KEYBOARDS[currentKeyboard]);
}

static void loopTelegram() {
  TBMessage msg;

  if (myBot.getNewMessage(msg)) {

    // security: ignore messages not from your configured user
    if ((int64_t)msg.sender.id != userid) {
      // Optional: silently ignore or send a message
      return;
    }

    if (msg.messageType == CTBotMessageText) {
      String tgReply = msg.text;
      Serial.print("Text message received: ");
      Serial.println(tgReply);

      if (tgReply == "/start") {
        String text = String(EMOTICON_WELCOME) + " Welcome!\n" + StatusMessage();
        currentKeyboard = kbMain;
        myBot.sendMessage((uint32_t)msg.sender.id, text, *KEYBOARDS[currentKeyboard]);
      }
      else if (tgReply == "/status") {
        currentKeyboard = kbMain;
        myBot.sendMessage((uint32_t)msg.sender.id, StatusMessage(), *KEYBOARDS[currentKeyboard]);
      }
      else if (tgReply.startsWith("/hex ")) {
        String payload = tgReply.substring(5);
        String text = String("const char EMOTICON[] = ") + convertToHexString(payload);
        currentKeyboard = kbMain;
        myBot.sendMessage((uint32_t)msg.sender.id, text, *KEYBOARDS[currentKeyboard]);
      }
      else {
        // echo + main keyboard
        currentKeyboard = kbMain;
        myBot.sendMessage((uint32_t)msg.sender.id, String("Unknown command: ") + tgReply, *KEYBOARDS[currentKeyboard]);
      }
    }
    else if (msg.messageType == CTBotMessageQuery) {
      handleCallback(msg);
    }
  }
}
