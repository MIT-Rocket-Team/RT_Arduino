#include "Arduino.h"
#include "SPI.h"
#include "ADXL357.h"

ADXL357::ADXL357(SPIClass* SPI, SPISettings settings, int cs) {
    _SPI = SPI;
    _settings = settings;
    _cs = cs;
}

void ADXL357::setup(){
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, 1);
    _writeReg(0x28, 0b00000011);
    _writeReg(0x2D, 0b00000000);
    _writeReg(0x2C, 0b10000011);
}

float ADXL357::getAccelX(){
    return _accelX;
}

float ADXL357::getAccelY(){
    return _accelY;
}

float ADXL357::getAccelZ(){
    return _accelZ;
}

int32_t ADXL357::getRawX() {
    return _accelXraw;
}

int32_t ADXL357::getRawY() {
    return _accelYraw;
}

int32_t ADXL357::getRawZ() {
    return _accelZraw;
}

uint8_t ADXL357::_readReg(byte reg) {
    digitalWrite(_cs, 0);
    _SPI->beginTransaction(_settings);
    _SPI->transfer((reg << 1) | 1);
    uint8_t data = _SPI->transfer(0x00);
    _SPI->endTransaction();
    digitalWrite(_cs, 1);
    return data;
}

bool ADXL357::_dataReady(){
    uint8_t data = _readReg(0x04);
    return data & 1;
}

void ADXL357::update(State rocketState) {
    if(_dataReady()){
        uint8_t data[10];
        data[0] = (0x08 << 1) | 1;
        digitalWrite(_cs, 0);
        _SPI->beginTransaction(_settings);
        _SPI->transfer(data, 10);
        _SPI->endTransaction();
        digitalWrite(_cs, 1);

        _accelXraw = ((int32_t)data[1] << 12) |
               ((int32_t)data[2] << 4)  |
               ((int32_t)(data[3] & 0xF0) >> 4);
        _accelXraw = ((int32_t) (_accelXraw << 12)) >> 12;
        _accelX = _accelXraw / 12800.0 * 9.80665 * 1.060;
        _accelYraw = ((int32_t)data[4] << 12) |
               ((int32_t)data[5] << 4)  |
               ((int32_t)(data[6] & 0xF0) >> 4);
        _accelYraw = ((int32_t) (_accelYraw << 12)) >> 12;
        _accelY = _accelYraw / 12800.0 * 9.80665;
        _accelZraw = ((int32_t)data[7] << 12) |
               ((int32_t)data[8] << 4)  |
               ((int32_t)(data[9] & 0xF0) >> 4);
        _accelZraw = ((int32_t) (_accelZraw << 12)) >> 12;
        _accelZ = _accelZraw / 12800.0 * 9.80665;

        _verticalAccelMinusGravity = _accelX - 9.8065;

        uint32_t now = micros();
        //Only integrate if above threshold in pre-flight
        if (rocketState == PRE_FLIGHT) {
            if (_verticalAccelMinusGravity > ACCEL_PREFLIGHT_INTEGRATION_THRESHOLD) {
                _integratedVelo += _verticalAccelMinusGravity * (now - _lastUpdate) / 1000000.0;
            }
        } else {
            _integratedVelo += _verticalAccelMinusGravity * (now - _lastUpdate) / 1000000.0;
        }
        _lastUpdate = now;
    }
}

void ADXL357::_writeReg(byte reg, byte val) {
    _SPI->beginTransaction(_settings);
    digitalWrite(_cs, 0);
    _SPI->transfer(reg << 1);
    _SPI->transfer(val);
    digitalWrite(_cs, 1);
    _SPI->endTransaction();
}

float ADXL357::getVerticalAccelMinusGravity() {
    return _verticalAccelMinusGravity;
}

float ADXL357::getIntegratedVelo() {
    return _integratedVelo;
}

void ADXL357::zeroIntegratedVelo() {
    _integratedVelo = 0;
}