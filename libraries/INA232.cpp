#include "Arduino.h"
#include "INA232.h"

INA232::INA232(TwoWire* wire, byte adr) {
    _Wire = wire;
    _adr = adr;
}

void INA232::begin() {
    _Wire->begin();
    _Wire->beginTransmission(_adr);
    _Wire->write(0x00);
    _Wire->write(0x51);
    _Wire->write(0x27);
    _Wire->endTransmission();
}

int16_t INA232::_readReg(uint8_t reg) {
    _Wire->beginTransmission(_adr);
    _Wire->write(reg);
    _Wire->endTransmission(false);
    _Wire->requestFrom(_adr, 2);
    _Wire->readBytes(_buf, 2);
    return (_buf[0] << 8) | _buf[1];
}

float INA232::voltage() {
    return _readReg(0x02) * 0.0016;
}

float INA232::current() {
    return _readReg(0x01) * 0.000625;
}

int16_t INA232::voltageRaw() {
    return _readReg(0x02);
}

int16_t INA232::currentRaw() {
    return _readReg(0x01);
}