#include "Car.h"
#include <Adafruit_NeoPixel.h>

#define LEDPIN    12
#define NUMPIXELS 154

uint8_t leftStart = (NUMPIXELS / 8) * 7;
uint8_t leftEnd = NUMPIXELS;

uint8_t rightStart = NUMPIXELS / 8;
uint8_t rightEnd = 0;

uint8_t tickerSpeed = NUMPIXELS / 2;

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
  _strip->setBrightness(30);
}

void loop() {
  car.process();
  _strip->clear();
  _strip->setBrightness(30);
  _strip->clear();
  if (car.handsOnRequired || car.handsOnAlert || car.handsOnWarning) {
    setAutopilotWarn(0, NUMPIXELS, ticker);
  } else {
    setDefault(0, NUMPIXELS);
  }
  if (car.blindSpotLeft) {
    setBlindSpot(leftStart, leftEnd);
  }
  if (car.blindSpotRight) {
    setBlindSpot(rightStart, rightEnd);
  }
  if (car.turningLeftLight) {
    setBlink(leftStart, leftStart + (ticker % tickerSpeed), car.blindSpotLeft);
  }
  if (car.turningRightLight) {
    setBlink(rightStart, rightStart - (ticker % tickerSpeed), car.blindSpotRight);
  }

  ticker++;
  if (ticker == NUMPIXELS) {
    ticker = 0;
  }
  _strip->show();
  delay(10);
}

void setBlindSpot(int start, int end) {
  if (start > end) {
    for(int i=start; i>end; i--) {
      _strip->setPixelColor(i - 1, Adafruit_NeoPixel::Color(150, 150, 0));
    }
  } else {
    for(int i=start; i<end; i++) {
      _strip->setPixelColor(i, Adafruit_NeoPixel::Color(150, 150, 0));
    }
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
  
  if (start > end) {
    for(int i=start; i>end; i--) {
      _strip->setPixelColor(i - 1, Adafruit_NeoPixel::Color(red, green, blue));
    }
  } else {
    for(int i=start; i<end; i++) {
      _strip->setPixelColor(i, Adafruit_NeoPixel::Color(red, green, blue));
    }
  }
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

void setDefault(int start, int end) {
  if (start > end) {
    for(int i=start; i>end; i--) {
      _strip->setPixelColor(i - 1, Adafruit_NeoPixel::Color(0,0,150));
    }
  } else {
    for(int i=start; i<end; i++) {
      _strip->setPixelColor(i, Adafruit_NeoPixel::Color(0,0,150));
    }
  }
}