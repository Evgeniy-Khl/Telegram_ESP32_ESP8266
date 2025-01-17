/*
Complete project details at https://RandomNerdTutorials.com/telegram-control-esp32-esp8266-nodemcu-outputs/
*/
#include <SoftwareSerial.h>
#include <LittleFS.h>             //this needs to be first, or it all crashes and burns...
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#define MYPORT_TX 13
#define MYPORT_RX 12

#ifdef ESP32
  #include <LittleFS.h>
#endif
#include <UniversalTelegramBot.h> // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif
//define your default values here, if there are different values in config.json, they are overwritten.
char botToken[50] = "";  // your Bot Token (Get from Botfather);
char chatID [15] = "";   // your Chat ID (search for “IDBot” or open this link t.me/myidbot in your smartphone.)
EspSoftwareSerial::UART mySerial;
// Размер массива
const int ARRAY_SIZE = 30;
uint8_t dataToSend[ARRAY_SIZE] = {0};  // Массив для отправки
uint8_t receivedData[ARRAY_SIZE] = {0}; // Массив для приема
volatile bool dataReceived = false; // Флаг для прерывания

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);
// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const int ledPin = 2;
bool ledState = LOW;

//flag for saving data
bool shouldSaveConfig = false;

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != chatID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "/led_on to turn GPIO ON \n";
      welcome += "/led_off to turn GPIO OFF \n";
      welcome += "/state to request current GPIO state \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led_on") {
      bot.sendMessage(chat_id, "LED state set to ON", "");
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
    }
    
    if (text == "/led_off") {
      bot.sendMessage(chat_id, "LED state set to OFF", "");
      ledState = LOW;
      digitalWrite(ledPin, ledState);
    }
    
    if (text == "/state") {
      if (digitalRead(ledPin)){
        bot.sendMessage(chat_id, "LED is ON", "");
      }
      else{
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
    }
  }
}

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();
  mySerial.begin(19200, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);
  if (!mySerial) { // If the object did not initialize, then its configuration is invalid
    Serial.println("Invalid EspSoftwareSerial pin configuration, check config"); 
    // while (1) { // Don't continue with invalid configuration
    //   delay (1000);
    // }
  } 

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  //-----------------clean FS, for testing-----------------
  //LittleFS.format();
  //-------------------------------------------------------

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (LittleFS.begin()) {
    Serial.println("mounted file system");
    if (LittleFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = LittleFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);

 #if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        if ( ! deserializeError ) 
#else
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) 
#endif
        {
          Serial.println("\nparsed json");
          strcpy(botToken, json["botToken"]);
          strcpy(chatID, json["chatID"]);
        } else {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_botToken("botToken", "BOT token", botToken, 50);
  WiFiManagerParameter custom_chatID("chatID", "Chat ID", chatID, 11);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifiManager.addParameter(&custom_botToken);
  wifiManager.addParameter(&custom_chatID);

  //-----------reset settings - for testing--------------
  //wifiManager.resetSettings();
  //-----------------------------------------------------

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //read updated parameters
  strcpy(botToken, custom_botToken.getValue());
  strcpy(chatID, custom_chatID.getValue());
  Serial.println("The values in the file are: ");
  Serial.println("\tbotToken : " + String(botToken));
  Serial.println("\tchatID : " + String(chatID));
  bot.updateToken(botToken);
  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
 #if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
    DynamicJsonDocument json(1024);
#else
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
#endif
    json["botToken"] = botToken;
    json["chatID"] = chatID;

    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

#if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
    serializeJson(json, Serial);
    serializeJson(json, configFile);
#else
    json.printTo(Serial);
    json.printTo(configFile);
#endif
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  // Заполнение массива для отправки
  for (int i = 0; i < ARRAY_SIZE; i++) {
      dataToSend[i] = i; // Например, заполняем массив значениями от 0 до 29
  }

  // Отправка данных
  mySerial.write(dataToSend, ARRAY_SIZE);
  Serial.println("Data sent:");
  for (int i = 0; i < ARRAY_SIZE; i++) {
      Serial.print(dataToSend[i]);
      Serial.print(" ");
  }
  Serial.println();
}

void loop() {
  // Проверка наличия данных для приема
    if (mySerial.available() >= ARRAY_SIZE) {
        // Чтение массива из UART
        mySerial.readBytes(receivedData, ARRAY_SIZE);
        Serial.println("Data received:");
        for (int i = 0; i < ARRAY_SIZE; i++) {
            Serial.print(receivedData[i]);
            Serial.print(" ");
        }
        Serial.println();
    }
  
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

}