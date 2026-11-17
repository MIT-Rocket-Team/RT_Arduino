#include "Arduino.h"
#include "GPS.h"

uint8_t GPS_10HZ[14] = {0xB5,0x62,0x06,0x08,0x06,0x00,0x64,0x00,0x01,0x00,0x01,0x00,0x7A,0x12};
uint8_t GPS_UBX_ENABLE[16] = {0xB5,0x62,0x06,0x01,0x08,0x00,0x01,0x07,0x00,0x01,0x00,0x00,0x00,0x00,0x18,0xE1};
uint8_t GPS_SERIAL_CONFIG[28] = {0xB5,0x62,0x06,0x00,0x14,0x00,0x01,0x00,0x00,0x00,0xD0,0x08,0x00,0x00,0x00,0xC2,0x01,0x00,0x07,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0xBE,0x72};
uint8_t GPS_CONFIG_UPDATE[9] = {0xB5,0x62,0x06,0x00,0x01,0x00,0x01,0x08,0x22};

GPS::GPS(Uart* gpsSer){
  _gpsSer = gpsSer;
}

void GPS::begin() {
  _gpsSer->begin(9600);
  _gpsSer->write(GPS_10HZ, 14);
  _gpsSer->write(GPS_UBX_ENABLE, 16);
  _gpsSer->write(GPS_SERIAL_CONFIG, 28);
  _gpsSer->write(GPS_CONFIG_UPDATE, 9);
  _gpsSer->flush();
  _gpsSer->begin(115200);
}

void GPS::update() {
  bool endLoop = false;
  while (_gpsSer->available() >= 6 && !endLoop) {
    if (!_headerValid) {
      if (_gpsSer->peek() == 0xB5) {
        _headerValid = _validateHeader();
      } else {
        _gpsSer->read();
      }
    } else {
      if(_gpsSer->available() >= 94) {
        _readPacket();
        if (_validateChecksum()) {
          memcpy(&_pkt, _buf + 4, 92);
          if (getHeight() > _maxAlt && getFixType() == 3) {
            _maxAlt = getHeight();
          }
        }
        _headerValid = false;
      } else {
        endLoop = true;
      }
      
    }
  }
}

bool GPS::_validateHeader() {
  if (_gpsSer->read() != 0xB5) return false;
  if (_gpsSer->read() != 0x62) return false;
  if (_gpsSer->read() != 0x01) return false;
  if (_gpsSer->read() != 0x07) return false;
  if (_gpsSer->read() != 0x5C) return false;
  if (_gpsSer->read() != 0x00) return false;
  return true;
}

void GPS::_readPacket() {
  _gpsSer->readBytes(_buf + 4, 94);
  _buf[0] = 0x01;
  _buf[1] = 0x07;
  _buf[2] = 0x5C;
  _buf[3] = 0x00;
}

bool GPS::_validateChecksum() {
  uint8_t ck_a = 0, ck_b = 0;
  for (int i = 0; i < 96; i++) {
    ck_a += _buf[i];
    ck_b += ck_a;
  }
  return ck_a == _buf[96] && ck_b == _buf[97];
}

void GPS::zeroAlt() {
  _heightOffset = _pkt.height;
  _maxAlt = 0;
}

float GPS::getMaxAlt() {
  return _maxAlt;
}
