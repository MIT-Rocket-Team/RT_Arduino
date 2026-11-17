#include "Arduino.h"
#include "SPI.h"
#include "gyro.h"
#include "math.h"

gyro::gyro(SPIClass* SPI, SPISettings settings, int cs) {
    _SPI = SPI;
    _settings = settings;
    _cs = cs;
}

void gyro::begin() {
    _SPI->begin();
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, 1);
}

uint8_t gyro::readReg(uint8_t reg) {
    digitalWrite(_cs, 0);
    _SPI->beginTransaction(_settings);
    _SPI->transfer(READ | reg);
    uint8_t data = _SPI->transfer(0x00);
    _SPI->endTransaction();
    digitalWrite(_cs, 1);
    return data;
}

void gyro::_writeReg(byte reg, byte val) {
    delay(10);
    digitalWrite(_cs, 0);
    _SPI->beginTransaction(_settings);
    _SPI->transfer(reg);
    _SPI->transfer(val);
    _SPI->endTransaction();
    digitalWrite(_cs, 1);
}

void gyro::config() {
    _writeReg(0x6B, 0x81); //"To initialize the sensor, perform a reset and let the IAM-20380 select the best clock source by setting the register PWR_MGMT_1 (address 0x6B) to 0x81"
    delay(500);
    _writeReg(0x19, 0x03); //250 Hz
    _writeReg(0x1A, 0x03); //DPPF corner freq 41 Hz
    _writeReg(0x1B, 0x10); //1000 dps
}

void gyro::update() {
    digitalWrite(_cs, 0);
    _SPI->beginTransaction(_settings);
    _SPI->transfer(READ | GYRO_XOUT_H);
    _rawX = _SPI->transfer16(0x00);
    _rawY = _SPI->transfer16(0x00);
    _rawZ = _SPI->transfer16(0x00);
    _SPI->endTransaction();
    digitalWrite(_cs, 1);

    uint32_t now = micros();
    _roll += getDpsX() * -1.0 * (now - _lastUpdate) / 1000000.0; //Define roll as positive clockwise looking down at rocket
    _pitch += getDpsY() * (now - _lastUpdate) / 1000000.0;
    _yaw += getDpsZ() * (now - _lastUpdate) / 1000000.0;
    _angleFromVertical = acos(cos(_pitch * PI / 180.0) * cos(_yaw * PI / 180)) * 180.0 / PI;
    _lastUpdate = now;
}

int16_t gyro::getRawX() {
    return _rawX;
}

int16_t gyro::getRawY() {
    return _rawY;
}

int16_t gyro::getRawZ() {
    return _rawZ;
}

float gyro::getDpsX() {
    return _rawX * 0.03051757812 + GYRO_OFFSET_ROLL_DPS;
}

float gyro::getDpsY() {
    return _rawY * 0.03051757812 - GYRO_OFFSET_PITCH_DPS;
}

float gyro::getDpsZ() {
    return _rawZ * 0.03051757812 - GYRO_OFFSET_YAW_DPS;
}

float gyro::getRoll() {
    return _roll;
}

float gyro::getRollRate() {
    return getDpsX() * -1.0;
}

float gyro::getPitch() {
    return _pitch;
}

float gyro::getYaw() {
    return _yaw;
}

float gyro::getAngleFromVertical() {
    return _angleFromVertical;
}

void gyro::zeroRollPitchYaw() {
    _roll = 0;
    _pitch = 0;
    _yaw = 0;
}