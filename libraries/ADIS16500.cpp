#include "Arduino.h"
#include "SPI.h"
#include "ADIS16500.h"

#define ACCEL_PREFLIGHT_INTEGRATION_THRESHOLD 10
#define ACCEL_BURST_INTEGRATION 0 // basically chages if the burst is getting accel data or vel + delta angle


ADIS16500::ADIS16500(SPIClass* SPI, SPISettings settings, int cs) {
    _SPI = SPI;
    _settings = settings;
    _cs = cs;
}

void ADIS16500::setup() {
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, 1);
    _SPI->begin();
    _SPI->beginTransaction(_settings);
    delay(1);
}

float ADIS16500::getAccelX() {
    return _accelX;
}

float ADIS16500::getAccelY() {
    return _accelY;
}

float ADIS16500::getAccelZ() {
    return _accelZ;
}

int32_t ADIS16500::getRawX() {
    return _accelXraw;
}

int32_t ADIS16500::getRawY() {
    return _accelYraw;
}

int32_t ADIS16500::getRawZ() {
    return _accelZraw;
}

float ADIS16500::getVelX() {
    return _velX;
}

float ADIS16500::getVelY() {
    return _velY;
}

float ADIS16500::getVelZ() {
    return _velZ;
}

float ADIS16500::getDeltaVelX() {
    return _deltaVelX;
}

float ADIS16500::getDeltaVelY() {
    return _deltaVelY;
}

float ADIS16500::getDeltaVelZ() {
    return _deltaVelZ;
}

float ADIS16500::getGyroX() {
    return _gyroX;
}

float ADIS16500::getGyroY() {
    return _gyroY;
}

float ADIS16500::getGyroZ() {
    return _gyroZ;
}

float ADIS16500::getDeltaGyroX() {
    return _deltaGyroX;
}

float ADIS16500::getDeltaGyroY() {
    return _deltaGyroY;
}

float ADIS16500::getDeltaGyroZ() {
    return _deltaGyroZ;
}

float ADIS16500::getVerticalAccelMinusGravity() {
    return _verticalAccelMinusGravity;
}

// all ADIS16500 regs are on page 0 (< 0x80) so no PAGE_ID writes like adis.c does
// each 16 bit frame gets its own CS pulse, read data comes back on the next frame
uint16_t ADIS16500::_frame(uint16_t tx) {
    digitalWrite(_cs, 0);
    _SPI->beginTransaction(_settings);
    uint16_t rx = _SPI->transfer16(tx);
    _SPI->endTransaction();
    digitalWrite(_cs, 1);
    delayMicroseconds(16); // tSTALL
    return rx;
}

uint16_t ADIS16500::_readReg(byte reg) {
    _frame((reg & 0x7F) << 8);
    return _frame(0x0000);
}

uint32_t ADIS16500::_readS32(byte reg) {
    // reg is the _LOW address, upper word lives at reg + 2
    _frame(((reg + 2) & 0x7F) << 8);
    uint16_t high = _frame((reg & 0x7F) << 8);
    uint16_t low = _frame(0x0000);
    return ((uint32_t)high << 16) | low;
}

void ADIS16500::_writeReg(byte reg, byte val) {
    _frame(((0x80 | reg) << 8) | val);
}

bool ADIS16500::_dataReady() {
    // no-OS has no DR helper, DATA_CNTR (0x22) increments on every new sample
    uint16_t cntr = _readReg(0x22);
    if (cntr == _lastDataCntr) return false;
    _lastDataCntr = cntr;
    return true;
}

// 32 bit scales from the ADIS16500 datasheet (16 bit LSB / 2^16 or 2^15 / 2^16)
#define GYRO_SCALE (0.1 / 65536.0)                       // deg/s, 0.1 deg/s per 16 bit LSB
#define ACCEL_SCALE (1.25e-3 * 9.80665 / 65536.0)        // m/s^2, 1.25 mg per 16 bit LSB
#define DELTA_ANG_SCALE (2160.0 / 2147483648.0)          // deg, +-2160 deg full scale
#define DELTA_VEL_SCALE (400.0 / 2147483648.0)           // m/s, +-400 m/s full scale

void ADIS16500::update(State rocketState) {
    if(_dataReady()){
        _gyroX = (int32_t)_readS32(0x04) * GYRO_SCALE;
        _gyroY = (int32_t)_readS32(0x08) * GYRO_SCALE;
        _gyroZ = (int32_t)_readS32(0x0C) * GYRO_SCALE;

        _accelXraw = (int32_t)_readS32(0x10);
        _accelYraw = (int32_t)_readS32(0x14);
        _accelZraw = (int32_t)_readS32(0x18);
        _accelX = _accelXraw * ACCEL_SCALE;
        _accelY = _accelYraw * ACCEL_SCALE;
        _accelZ = _accelZraw * ACCEL_SCALE;

        _deltaGyroX = (int32_t)_readS32(0x24) * DELTA_ANG_SCALE;
        _deltaGyroY = (int32_t)_readS32(0x28) * DELTA_ANG_SCALE;
        _deltaGyroZ = (int32_t)_readS32(0x2C) * DELTA_ANG_SCALE;

        _deltaVelX = (int32_t)_readS32(0x30) * DELTA_VEL_SCALE;
        _deltaVelY = (int32_t)_readS32(0x34) * DELTA_VEL_SCALE;
        _deltaVelZ = (int32_t)_readS32(0x38) * DELTA_VEL_SCALE;

        // same as ADXL357, X is up
        _verticalAccelMinusGravity = _accelX - 9.80665;

        uint32_t now = micros();
        float dt = (now - _lastUpdate) / 1000000.0;
        //Only integrate if above threshold in pre-flight
        if (rocketState != PRE_FLIGHT || _verticalAccelMinusGravity > ACCEL_PREFLIGHT_INTEGRATION_THRESHOLD) {
            _velX += _verticalAccelMinusGravity * dt;
            _velY += _accelY * dt;
            _velZ += _accelZ * dt;
        }
        _lastUpdate = now;
    }
}
