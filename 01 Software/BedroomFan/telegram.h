#pragma once

#include <AsyncTelegram2.h>
#include <map>
#include "fancontrol.h"

using namespace std;

// ======== DEFINES ==================
// Main menu
#define CB_FAN_ON     "cbFanOn"
#define CB_FAN_OFF    "cbFanOff"
#define CB_TMR_20MIN  "cb20min"
#define CB_TMR_1HR    "cb1hr"
#define CB_TMR_4HRS   "cb4hrs"
#define CB_SETTINGS   "cbSettings"

// Settings menu
#define CB_EVENTLOG   "cbEventLog"
#define CB_EVENTCLR   "cbEventClr"
#define CB_STATUS     "cbStatus"
#define CB_MAIN       "cbMain"

// ======== CONSTANTS ================
const char EMOTICON_WELCOME[]   = { 0xf0, 0x9f, 0x99, 0x8b, 0xe2, 0x80, 0x8d, 0xe2, 0x99, 0x80, 0xef, 0xb8, 0x8f, 0x00 };
const char EMOTICON_STOP[]      = { 0xf0, 0x9f, 0x9b, 0x91, 0x00 };
const char EMOTICON_WIND[]      = { 0xf0, 0x9f, 0x92, 0xa8, 0x00 };
const char EMOTICON_HOURGLASS[] = { 0xe2, 0x8f, 0xb3, 0x00 };
const char EMOTICON_FINISH[]    = { 0xf0, 0x9f, 0x8f, 0x81, 0x00 };
const char EMOTICON_EVENTLOG[]  = { 0xf0, 0x9f, 0x93, 0x9d, 0x00 };
const char EMOTICON_CLEAR[]     = { 0xf0, 0x9f, 0x97, 0x91, 0x00 };     // Garbage bin
const char EMOTICON_SETTINGS[]  = { 0xe2, 0x9a, 0x99, 0xef, 0xb8, 0x8f, 0x00 };
const char EMOTICON_STATUS[]    = { 0xf0, 0x9f, 0xa9, 0xba, 0x00 }; // Staethoscope
const char EMOTICON_MAIN[]      = { 0xf0, 0x9f, 0x94, 0x99, 0x00 }; // Back arrow

// ======== TYPES ================
enum keyboard_t { kbMain, kbSettings };

// ======== GLOBAL VARIABLES ================
WiFiClientSecure client;  
AsyncTelegram2 myBot(client);
InlineKeyboard mainKeyboard, settingsKeyboard;
keyboard_t currentKeyboard = kbMain;

std::map<keyboard_t, InlineKeyboard* > KEYBOARDS = { 
  { kbMain,             &mainKeyboard             },
  { kbSettings,         &settingsKeyboard         },
};

// ======== FUNCTIONS ================

String convertToHexString( String input ) {
  String result = "{ ";
  for( int i=0; i<input.length(); i++ ) {
    result += String("0x") + String( (int) input[i], HEX ) + ", ";
  }
  result+="0x00 };";
  result.toLowerCase();
  Serial.println(input + ": " + result);
  return result;
}

void sendMessageToKeyUser( String msg ) {
  myBot.sendTo(userid, msg.c_str(), mainKeyboard.getJSON() );  
}

String StatusMessage() {
  String result;
  char item[80];
  float remainingSeconds;

  switch(fanStatus) {
    case fsOn:
      result = String( EMOTICON_WIND ) + " Fan is switched on";
    break;
    
    case fsOff:
      result = String( EMOTICON_STOP ) + " Fan is switched off";
    break;
    
    case fsTimer:
      remainingSeconds = fanTimer.remaining()/1000;

      if(remainingSeconds>=60)
        snprintf( item, sizeof(item), " Fan will switch off after %d minutes", int(remainingSeconds/60) );
      else
        snprintf( item, sizeof(item), " Fan will switch off after %d seconds", int(remainingSeconds) );

      result = String( EMOTICON_HOURGLASS ) + item;
    break;
  }

  return result;
};  

// Callback functions definition for inline keyboard buttons
void onQueryMain( const TBMessage &queryMsg ) {
  String newMessage = "";
  String userName = queryMsg.sender.firstName + " " + queryMsg.sender.lastName;

  // To do: prevent a lot of switching if telegram receives a lot of pending messages after switching on
  if( queryMsg.callbackQueryData == CB_FAN_ON ) {
    newMessage = String(EMOTICON_WIND) + " Fan is switched on without end time.";
    addToEventLogfile( String("Fan switched on by ") + userName );
    switchOnFan();
  }
  else if( queryMsg.callbackQueryData == CB_FAN_OFF ) {
    newMessage = String(EMOTICON_STOP) + " Fan is switched off.";
    addToEventLogfile( String("Fan switched off by ") + userName );
    switchOffFan();
  }
  else if( queryMsg.callbackQueryData == CB_TMR_20MIN ) {
    newMessage = String(EMOTICON_HOURGLASS) + " Fan is switched on for 20 minutes.";
    addToEventLogfile( String("Fan switched on for 20 minutes by ") + userName );
    setFanTimer(tdTimer20);
  }
  else if( queryMsg.callbackQueryData == CB_TMR_1HR ) {
    newMessage = String(EMOTICON_HOURGLASS) + " Fan is switched on for 1 hour.";
    addToEventLogfile( String("Fan switched on for 1 hour by ") + userName );
    setFanTimer(tdTimer60);
  }
  else if( queryMsg.callbackQueryData == CB_TMR_4HRS ) {
    newMessage = String(EMOTICON_HOURGLASS) + " Fan is switched on for 4 hours.";
    addToEventLogfile( String("Fan switched on for 4 hours by ") + userName );
    setFanTimer(tdTimer240);
  }
  else if( queryMsg.callbackQueryData == CB_SETTINGS ) {
    newMessage = String(EMOTICON_SETTINGS) + " Switching to settings menu.";
    currentKeyboard = kbSettings;
  }
  else newMessage = "Command not recognized";

  myBot.editMessage(queryMsg.chatId, queryMsg.messageID, newMessage, *KEYBOARDS[currentKeyboard] );
  Serial.println( newMessage );
}

void onQuerySettings(const TBMessage &queryMsg) {
  String newMessage;
  String userName = queryMsg.sender.firstName + " " + queryMsg.sender.lastName;
  char item[80];
  struct tm * timeinfo;

  if( queryMsg.callbackQueryData == CB_STATUS ) {
    newMessage = String("Software version: ") + version;
  }  
  else if( queryMsg.callbackQueryData == CB_EVENTLOG ) {
    newMessage = "Download event log";
    addToEventLogfile( String("Event log requested by ") + userName );
    File file = SPIFFS.open(EVT_LOG_FILE, "r");
    if (file) {
      myBot.sendDocument(queryMsg, file, file.size(), AsyncTelegram2::DocumentType::CSV, file.name());
      file.close();
    }
    else
      Serial.println("Can't open the event log file");
  }  
  else if( queryMsg.callbackQueryData == CB_EVENTCLR ) {
    newMessage = "Event log cleared";
    newEventLogfile();
    addToEventLogfile( String("Event log cleared by ") + userName );
  }  
  else if( queryMsg.callbackQueryData == CB_MAIN ) {
    newMessage = "Back to main menu";
    currentKeyboard = kbMain;
  }
  else newMessage = "Command not recognized";

  newMessage += "\n" + StatusMessage();  
  myBot.editMessage( queryMsg.chatId, queryMsg.messageID, newMessage, *KEYBOARDS[ currentKeyboard ] );
  Serial.println( newMessage );
}; // onQuerySettings

void addInlineKeyboard() {
  String btntext;

  // Add buttons for main keyboard
  btntext=String(EMOTICON_WIND)   + " Fan on";       
  mainKeyboard.addButton(btntext.c_str(), CB_FAN_ON,     KeyboardButtonQuery, onQueryMain);
  btntext=String(EMOTICON_STOP)  + " Fan off";      
  mainKeyboard.addButton(btntext.c_str(), CB_FAN_OFF,    KeyboardButtonQuery, onQueryMain);
  mainKeyboard.addRow();

  btntext=String(EMOTICON_HOURGLASS) + " 20 min";    
  mainKeyboard.addButton(btntext.c_str(), CB_TMR_20MIN,  KeyboardButtonQuery, onQueryMain);
  btntext=String(EMOTICON_HOURGLASS) + " 1 hour";    
  mainKeyboard.addButton(btntext.c_str(), CB_TMR_1HR,    KeyboardButtonQuery, onQueryMain);
  btntext=String(EMOTICON_HOURGLASS) + " 4 hours";      
  mainKeyboard.addButton(btntext.c_str(), CB_TMR_4HRS,   KeyboardButtonQuery, onQueryMain);
  mainKeyboard.addRow();

  btntext=String(EMOTICON_SETTINGS) + " Settings";   
  mainKeyboard.addButton(btntext.c_str(), CB_SETTINGS,   KeyboardButtonQuery, onQueryMain);
  mainKeyboard.addRow();

  // Add buttons for settings keyboard
  btntext=String(EMOTICON_EVENTLOG) + " Download event log";   
  settingsKeyboard.addButton(btntext.c_str(), CB_EVENTLOG, KeyboardButtonQuery, onQuerySettings);
  settingsKeyboard.addRow();

  btntext=String(EMOTICON_CLEAR) + " Clear event log";   
  settingsKeyboard.addButton(btntext.c_str(), CB_EVENTCLR, KeyboardButtonQuery, onQuerySettings);
  settingsKeyboard.addRow();

  btntext=String(EMOTICON_STATUS) + " Status";   
  settingsKeyboard.addButton(btntext.c_str(), CB_STATUS, KeyboardButtonQuery, onQuerySettings);
  settingsKeyboard.addRow();

  btntext=String(EMOTICON_MAIN) + " Main menu";   
  settingsKeyboard.addButton(btntext.c_str(), CB_MAIN, KeyboardButtonQuery, onQuerySettings);
  settingsKeyboard.addRow();

  // Add both inline keyboards to the bot
  myBot.addInlineKeyboard(&mainKeyboard);
  myBot.addInlineKeyboard(&settingsKeyboard);
}

void setupTelegram() {
  
  // Initialize Telegram
  Serial.println("Setup Telegram");
  client.setCACert(telegram_cert);

  // Set the Telegram bot properties
  myBot.setUpdateTime(1000);
  myBot.setTelegramToken(token);

  // Check if all things are ok
  Serial.print("Test Telegram connection... ");
  myBot.begin() ? Serial.println("Bot OK") : Serial.println("Bot not OK");
  Serial.print("Bot name: @");
  Serial.println(myBot.getBotName());

  addInlineKeyboard();
  
  // Try to set the default commands
  myBot.deleteMyCommands();

  if ( myBot.setMyCommands("/start", "start conversation") ) {
    Serial.println("myBot.setMyCommands successful");
  }
  else {
    Serial.println("myBot.setMyCommands not successful");
  }

  String cmdList;
  myBot.getMyCommands(cmdList);
  Serial.print("Result of get my commands:");
  Serial.println(cmdList);

  String text = String(EMOTICON_WELCOME) + " Welcome";
  myBot.sendTo(userid, text.c_str(), mainKeyboard.getJSON() );

  Serial.println("End of setup loop");
};


void loopTelegram() {
  // a variable to store telegram message data
  TBMessage msg;

  // if there is an incoming message...
  if (myBot.getNewMessage(msg)) {
    // check what kind of message I received
    String tgReply;
    MessageType msgType = msg.messageType;
    
    switch (msgType) {
      case MessageText :
        // received a text message
        tgReply = msg.text;
        Serial.print("Text message received: ");
        Serial.println(tgReply);

        if (tgReply.equalsIgnoreCase("/start")) {          
          String text = String(EMOTICON_WELCOME) + " Welcome!";
          myBot.sendMessage(msg, text.c_str(), mainKeyboard);      
          Serial.printf("Start command received from %d\n", msg.chatId);    
        }        
        else {
          // write back feedback message and show a hint
          String text = String("const char EMOTICON[] = ") + convertToHexString( msg.text );
          myBot.sendMessage(msg, text.c_str(), mainKeyboard);
        }
        break;
        
        case MessageQuery:
          // Handled by message query callback functions
          break;
        
        default:
          break;
    }
  }
}
