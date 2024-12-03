#include "Car.h"
#include <Adafruit_NeoPixel.h>

#define LEDPIN    12
#define NUMPIXELS 154

const int vCanPin = 5;
const int cCanPin = 21;

int ticker = 0;

Car car;

Adafruit_NeoPixel* _strip;

void setup() {
  Serial.begin(921600);
  Serial.println();
  car.init(vCanPin, cCanPin);
  _strip = new Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);
  _strip->begin();
  _strip->clear();
  _strip->setBrightness(40);
}

void loop() {
  car.process();
  _strip->clear();
  if (ticker == 0 ) {
    _strip->setBrightness(30);
  }

  bool changedLeft = false;
  bool changedRight = false;
  _strip->clear();
  if (car.blindSpotLeft) {
    changedLeft = true;
    setBlindSpot(0, NUMPIXELS/2);
  }
  if (car.blindSpotRight) {
    changedRight = true;
    setBlindSpot(NUMPIXELS/2, NUMPIXELS);
  }
  if (car.turningLeftLight) {
    changedLeft = true;
    setBlink(0, NUMPIXELS/2 - ticker, car.blindSpotLeft);
  }
  if (car.turningRightLight) {
    changedRight = true;
    setBlink(NUMPIXELS/2, NUMPIXELS/2 + ticker, car.blindSpotRight);
  }
  if (!changedLeft) {
    if (car.handsOnRequired || car.handsOnAlert || car.handsOnWarning) {
      setAutopilotWarn(0, NUMPIXELS/2, ticker);
    } else {
      setDefault(0, NUMPIXELS/2);
    }
  }
  if (!changedRight) {
    if (car.handsOnRequired || car.handsOnAlert || car.handsOnWarning) {
      setAutopilotWarn(NUMPIXELS/2, NUMPIXELS, ticker);
    } else {
      setDefault(NUMPIXELS/2, NUMPIXELS);
    }
  }
  ticker++;
  if (ticker == NUMPIXELS) {
    ticker = 0;
  }
  _strip->show();
  delay(10);
}

void setBlindSpot(int start, int end) {
  for(int i=start; i<end; i++) {
    _strip->setPixelColor(i, Adafruit_NeoPixel::Color(150, 150, 0));
  }
}

void setBlink(int start, int end, bool blindSpot) {
  uint8_t green = 150;
  uint8_t red = 0;
  uint8_t blue = 0;
  if (blindSpot) {
    green = 0;
    red = 150;
  }
  
  for(int i=start; i<end; i++) {
    _strip->setPixelColor(i, Adafruit_NeoPixel::Color(red, green, blue));
  }
}

void setAutopilotWarn(int start, int end, int ticker) {
  for(int i=start; i<end; i++) {
    _strip->setPixelColor(i, Adafruit_NeoPixel::Color(0,0,ticker));
  }
}

void setDefault(int start, int end) {
    for(int i=start; i<end; i++) {
    _strip->setPixelColor(i, Adafruit_NeoPixel::Color(0,0,150));
  }
}