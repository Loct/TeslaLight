#include "Arduino.h"
#include "Car.h"
#include <mcp_can.h>


Car::Car() {
}

void Car::init(byte v_pin, byte c_pin) {
  _VCAN = new MCP_CAN(v_pin);
  _CCAN = new MCP_CAN(c_pin);
  if (_CCAN->begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
    _CCAN->init_Mask(0, 0, 0x07FF0000);
    _CCAN->init_Filt(0, 0, 0x02730000);
    _CCAN->init_Filt(1, 0, 0x03990000);
    _CCAN->setMode(MCP_NORMAL);
    _c_enabled = true;
  }
  if (_VCAN->begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
    _v_enabled = true;
    _VCAN->init_Mask(0, 0, 0x07FF0000);
    _VCAN->init_Filt(0, 0, 0x03F50000);
    _VCAN->init_Mask(1, 0, 0x07FF0000);
    _VCAN->init_Filt(1, 0, 0x01020000);
    _VCAN->init_Filt(2, 0, 0x01030000);
    _VCAN->init_Filt(3, 0, 0x02E10000);
    _VCAN->init_Filt(4, 0, 0x01180000);
    _VCAN->setMode(MCP_NORMAL);
  }
}

void Car::_processAutopilot(unsigned char len, unsigned char data[]) {
  blindSpotLeft = ((data[0] >> 4) & 0x03) > 0;
  blindSpotRight = ((data[0] >> 6) & 0x03) > 0;
  blindSpotLeftAlert = ((data[0] >> 4) & 0x03) > 1;
  blindSpotRightAlert = ((data[0] >> 6) & 0x03) > 1;

  byte ap = (data[0] & 0x0F);
  autosteerOn = ap == 3 || ap == 4 || ap == 5;

  byte ho = ((data[5] >> 2) & 0x0F);
  handsOn = ho == 1;
  handsOnRequired = ho > 0;
  handsOnWarning = ho > 2;
  handsOnAlert = ho > 3;
}

void Car::_processLights(unsigned char len, unsigned char data[]) {
  turningRightLight = (data[0] & 0x08) == 0x08;
  turningRight = (data[0] & 0x04) == 0x04;
  turningLeftLight = (data[0] & 0x02) == 0x02;
  turningLeft = (data[0] & 0x01) == 0x01;
}

void Car::_processRightDoors(unsigned char len, unsigned char data[]) {
  doorOpen[0] = data[0] & 0x01;
  doorOpen[2] = data[0] & 0x10;
  doorHandlePull[0] = data[1] & 0x04;
}

void Car::_processLeftDoors(unsigned char len, unsigned char data[]) {
  doorOpen[1] = data[0] & 0x01;
  doorOpen[3] = data[0] & 0x10;
  doorHandlePull[1] = data[1] & 0x04;
}

void Car::_processGear(unsigned char len, unsigned char data[]) {
  gear = (Gear)((data[2] >> 5) & 0x07);
  if (gear == 0x07)
    gear = (Gear)GEAR_UNKNOWN;
}

void Car::process() {
  if (_v_enabled && _VCAN->checkReceive()) {
    long unsigned int rxId;
    unsigned char len = 0;
    unsigned char rxBuf[8];
    _VCAN->readMsgBuf(&rxId, &len, rxBuf);
    if (rxId == 0x3F5) {
      _processLights(len, rxBuf);
    } else if (rxId == 0x103) {
      _processRightDoors(len, rxBuf);
    } else if (rxId == 0x102) {
      _processLeftDoors(len, rxBuf);
    } else if (rxId == 0x118) {
       _processGear(len, rxBuf);
    }
  }

  if (_c_enabled && _CCAN->checkReceive()) {
    long unsigned int rxId;
    unsigned char len = 0;
    unsigned char rxBuf[8];
    _CCAN->readMsgBuf(&rxId, &len, rxBuf);  // Read data: len = data length, buf = data byte(s)

    if (rxId == 0x399) {
      _processAutopilot(len, rxBuf);
    }
  }
}