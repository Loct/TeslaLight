#include <AsyncTelegram2.h>
#include "Car.h"
#include <Adafruit_NeoPixel.h>
#include <time.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFi.h>

// Timezone definition
#define MYTZ "CET-1CEST,M3.5.0,M10.5.0/3"
#define UPDATER httpUpdate

WiFiClientSecure client;
AsyncTelegram2 myBot(client);

const char* ssid = "iPhone Loc";     // REPLACE mySSID WITH YOUR WIFI SSID
const char* pass = "jajajaja";     // REPLACE myPassword YOUR WIFI PASSWORD, IF ANY
//const char* token = "xxxxxxxxxxxxxx"; 
const char* token = "7606892401:AAFsTpzqW6k6KMxMGv795SgG9AiZclTr_QM";     // REPLACE myToken WITH YOUR TELEGRAM BOT TOKEN
  // REPLACE myToken WITH YOUR TELEGRAM BOT TOKEN
int64_t chat_id = 324153476;       // You can discover your own chat id, with "Json Dump Bot"

#define CANCEL  "CANCEL"
#define CONFIRM "FLASH_FW"

const char* firmware_version = __TIME__;

#define LEDPIN    12
#define NUMPIXELS 146

uint8_t leftStart = (NUMPIXELS / 2);
uint8_t leftEnd = NUMPIXELS;
uint8_t rightEnd = NUMPIXELS / 2;
uint8_t rightStart = 0;
const int vCanPin = 5;
const int cCanPin = 21;
uint8_t ticker = 0;
Car car;
Adafruit_NeoPixel* _strip;

void setup() {
  Serial.begin(115200);
  car.init(vCanPin, cCanPin);
  _strip = new Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);
  _strip->begin();
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
}

bool connected = false;

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    setupConnection();
    handleMessage();
  }

  car.process();
  _strip->clear();
  _strip->setBrightness(30);

  if (car.gear == GEAR_PARK || car.gear == GEAR_NEUTRAL) {
    setStrip(0, NUMPIXELS, 0, 0, 0);
  } else if (car.gear == GEAR_REAR) {
    setStrip(0, NUMPIXELS, 150, 150, 0);
  } else if (car.gear == GEAR_DRIVE) {
    setStrip(0, NUMPIXELS, 0, 0, 150);
  }

  if (car.handsOnAlert || car.handsOnWarning) {
    setAutopilotWarn(0, NUMPIXELS, ticker);
  }

  if (car.blindSpotLeft) {
    setStrip(leftStart, leftEnd, 150, 150, 0);
  }
  if (car.blindSpotRight) {
    setStrip(rightStart, rightEnd, 150, 150, 0);
  }
  if (car.turningLeftLight) {
    uint8_t green = 150;
    uint8_t red = 0;
    uint8_t blue = 0;
    if (car.blindSpotLeft) {
      green = 0;
      red = 150;
    }
    setStrip(leftStart, leftEnd, red, green, blue);
  }

  if (car.turningRightLight) {
    uint8_t green = 150;
    uint8_t red = 0;
    uint8_t blue = 0;
    if (car.blindSpotLeft) {
      green = 0;
      red = 150;
    }
    setStrip(rightStart, rightEnd, red, green, blue);
  }

  if (car.doorOpen[1] || car.doorOpen[3]) {
    setStrip(leftStart, leftEnd, 150, 150, 150);
  }
  if (car.doorOpen[0] || car.doorOpen[2]) {
    setStrip(rightStart, rightEnd, 150, 150, 150);
  }

  _strip->show();
  ticker++;
  if (ticker == NUMPIXELS) {
    ticker = 0;
  }
  delay(10);
}

void setAutopilotWarn(int start, int end, int ticker) {
  if (start > end) {
    for(int i=start; i>end; i--) {
      _strip->setPixelColor(i - 1, Adafruit_NeoPixel::Color(0,0,ticker));
    }
  } else {
    for(int i=start; i<end; i++) {
      _strip->setPixelColor(i, Adafruit_NeoPixel::Color(0,0,ticker));
    }
  }
}

void setStrip(int start, int end, uint8_t r, uint8_t g, uint8_t b) {
  if (start > end) {
    for(int i=start; i>end; i--) {
      _strip->setPixelColor(i - 1, Adafruit_NeoPixel::Color(r,g,b));
    }
  } else {
    for(int i=start; i<end; i++) {
      _strip->setPixelColor(i, Adafruit_NeoPixel::Color(r,g,b));
    }
  }
}

void setupConnection() {
  if (connected) {
    return;
  }
  // Set the Telegram bot properies
  myBot.setUpdateTime(2000);
  myBot.setTelegramToken(token);

  // Check if all things are ok
  Serial.print("\nTest Telegram connection... ");
  myBot.begin() ? Serial.println("OK") : Serial.println("NOK");

  char welcome_msg[128];
  snprintf(welcome_msg, 128, "BOT @%s online\n/help all commands avalaible.", myBot.getBotName());
  myBot.sendTo(chat_id, welcome_msg);

  // We have to handle reboot manually after sync with TG server
  UPDATER.rebootOnUpdate(false);
  connected = true;
}

void handleMessage() {
  // a variable to store telegram message data
  TBMessage msg;

  // if there is an incoming message...
  if (myBot.getNewMessage(msg)) {
    String tgReply;
    static String document;

    switch (msg.messageType) {
      case MessageDocument :
        document = msg.document.file_path;
        if (msg.document.file_exists) {

          // Check file extension of received document (firmware MUST be .bin)
          if (document.endsWith(".bin") > -1 ) {
            String report = "Start firmware update?\nFile name: "
                            + String(msg.document.file_name)
                            + "\nFile size: "
                            + String(msg.document.file_size);

            // Query user for flash confirmation
            InlineKeyboard confirmKbd;
            confirmKbd.addButton("FLASH", CONFIRM, KeyboardButtonQuery);
            confirmKbd.addButton("CANCEL", CANCEL, KeyboardButtonQuery);
            myBot.sendMessage(msg, report.c_str(), confirmKbd);
          }
        }
        else {
          myBot.sendMessage(msg, "File is unavailable. Maybe size limit 20MB was reached or file deleted");
        }
        break;

      case MessageQuery:
        // received a callback query message
        tgReply = msg.callbackQueryData;

        // User has confirmed flash start
        if (tgReply.equalsIgnoreCase(CONFIRM)) {
          myBot.endQuery(msg, "Start flashing... please wait (~30/60s)", true);
          handleUpdate(msg, document);
          document.clear();
        }
        // User has canceled the command
        else if (tgReply.equalsIgnoreCase(CANCEL)) {
          myBot.endQuery(msg, "Flashing canceled");
        }
        break;

      default:
        tgReply = msg.text;
        if (tgReply.equalsIgnoreCase("/version")) {
          String fw = "Version: " ;
          fw += firmware_version;
          myBot.sendMessage(msg, fw);
        }
        else {
          myBot.sendMessage(msg, "Send firmware binary file ###.bin to start update\n"
                            "/version for print the current firmware version\n");
        }
        break;
    }
  }
}


// Install firmware update
void handleUpdate(TBMessage msg, String &file_path) {
  // Create client for rom download
  WiFiClientSecure client;
  client.setInsecure();

  String report;
  Serial.print("Firmware path: ");
  Serial.println(file_path);

  // On a good connection the LED should flash regularly. On a bad connection the LED will be
  // on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
  // value is used to put the LED on. If the LED is on with HIGH, that value should be passed
  UPDATER.setLedPin(LED_BUILTIN, LOW);

  UPDATER.onProgress([](int cur, int total) {
    static uint32_t sendT;
    if (millis() - sendT > 1000) {
      sendT = millis();
      Serial.printf("Updating %d of %d bytes...\n", cur, total);
    }
  });
  
  t_httpUpdate_return ret = UPDATER.update(client, file_path);
  Serial.println("Update done!");
  client.stop();

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      report = "HTTP_UPDATE_FAILED Error (";
      report += UPDATER.getLastError();
      report += "): ";
      report += UPDATER.getLastErrorString();
      myBot.sendMessage(msg, report.c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      myBot.sendMessage(msg, "HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      myBot.begin();
      myBot.sendMessage(msg, "UPDATE OK.\nRestarting in few seconds...");
      // Wait until bot synced with telegram to prevent cyclic reboot
      while (!myBot.noNewMessage()) {
        Serial.print(".");
        delay(50);
      }
      ESP.restart();

      break;
    default:
      break;
  }

}