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
  _strip->setBrightness(30);
  bool changedLeft = false;
  bool changedRight = false;
  _strip->clear();
  if (car.blindSpotLeft) {
    changedLeft = true;
    setBlindSpot(NUMPIXELS/2, NUMPIXELS);
  }
  if (car.blindSpotRight) {
    changedRight = true;
    setBlindSpot(NUMPIXELS/2 + 1, 0);
  }
  if (car.turningLeftLight) {
    changedLeft = true;
    setBlink(NUMPIXELS/2, NUMPIXELS/2 + ticker, car.blindSpotLeft);
  }
  if (car.turningRightLight) {
    changedRight = true;
    setBlink(NUMPIXELS/2, NUMPIXELS/2 - ticker, car.blindSpotRight);
  }
  if (!changedLeft) {
    if (car.handsOnRequired || car.handsOnAlert || car.handsOnWarning) {
      setAutopilotWarn(NUMPIXELS/2, NUMPIXELS, ticker);
    } else {
      setDefault(NUMPIXELS/2, NUMPIXELS);
    }
  }
  if (!changedRight) {
    if (car.handsOnRequired || car.handsOnAlert || car.handsOnWarning) {
      setAutopilotWarn(NUMPIXELS/2, 0, ticker);
    } else {
      setDefault(NUMPIXELS/2, 0);
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
  if (start > end) {
    for(int i=start; i>end; i--) {
      _strip->setPixelColor(i, Adafruit_NeoPixel::Color(150, 150, 0));
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