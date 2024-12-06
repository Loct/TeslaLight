#include "Car.h"
#include <Adafruit_NeoPixel.h>

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
  car.init(vCanPin, cCanPin);
  _strip = new Adafruit_NeoPixel(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);
  _strip->begin();
}

void loop() {
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
  for(int i=start; i<end; i++) {
    _strip->setPixelColor(i, Adafruit_NeoPixel::Color(0,0,ticker));
  }
}

void setStrip(int start, int end, uint8_t r, uint8_t g, uint8_t b) {
  for(int i=start; i<end; i++) {
    _strip->setPixelColor(i, Adafruit_NeoPixel::Color(r,g,b));
  }
}