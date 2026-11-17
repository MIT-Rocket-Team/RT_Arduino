#include "Arduino.h"
#include "SPI.h"
#include "baro.h"

float dT, TEMP, P;
int64_t OFF, SENS;
const float P0 = 1013.25;

baro::baro(SPIClass* SPI, SPISettings settings, int cs) {
    _SPI = SPI;
    _settings = settings;
    _cs = cs;
}

void baro::begin(){
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, 1);
}

void baro::updateRawPress() {
    digitalWrite(_cs, 0);
    _SPI->beginTransaction(_settings);
    _SPI->transfer(0x42);
    _SPI->endTransaction();
    digitalWrite(_cs, 1);

    delayMicroseconds(1500);

    digitalWrite(_cs, 0);
    _SPI->beginTransaction(_settings);
    _SPI->transfer(0x00);
    _rawPress  = _SPI->transfer16(0x00) << 8;
    _rawPress += _SPI->transfer(0x00);
    _SPI->endTransaction();
    digitalWrite(_cs, 1);
}

void baro::updateRawTemp() {
    digitalWrite(_cs, 0);
    _SPI->beginTransaction(_settings);
    _SPI->transfer(0x52);
    _SPI->endTransaction();
    digitalWrite(_cs, 1);

    delayMicroseconds(1500);

    digitalWrite(_cs, 0);
    _SPI->beginTransaction(_settings);
    _SPI->transfer(0x00);
    _rawTemp  = _SPI->transfer16(0x00) << 8;
    _rawTemp += _SPI->transfer(0x00);
    _SPI->endTransaction();
    digitalWrite(_cs, 1);
}

uint32_t baro::getRawTemp() {
    return _rawTemp;
}

uint32_t baro::getRawPress() {
    return _rawPress;
}

float baro::getTemperature(){
    dT = (float)_rawTemp - ((float)C5)*((int)1<<8);
    TEMP = 2000.0 + dT * ((float)C6)/(float)((long)1<<23);
    return TEMP/100;
}

float baro::getPressure() {
    dT = (float)_rawTemp - ((float)C5)*((int)1<<8);
    TEMP = 2000.0 + dT * ((float)C6)/(float)((long)1<<23);
    OFF = (((int64_t)C2)*((long)1<<17)) + dT * ((float)C4)/((int)1<<6);
    SENS = ((float)C1)*((long)1<<16) + dT * ((float)C3)/((int)1<<7);
    float pa = (float)((float)_rawPress/((long)1<<15));
    float pb = (float)(SENS/((float)((long)1<<21)));
    float pc = pa*pb;
    float pd = (float)(OFF/((float)((long)1<<15)));
    P = pc - pd;
    return P/100;
}

float baro::getAltitude() {
    float h,t,p;
    t = getTemperature();
    p = getPressure();
    p = P0/p;
    h = 153.84615*(pow(p,0.19) - 1)*(t+273.15);
    return h;
}

uint16_t baro::getCalibrationConstant(uint8_t index) {
    digitalWrite(_cs, 0);
    _SPI->beginTransaction(_settings);
    _SPI->transfer(0b10100000 | (index << 1));
    uint16_t res = _SPI->transfer16(0x00);
    _SPI->endTransaction();
    digitalWrite(_cs, 1);
    return res;
}

float baro::getFilteredAltitude() {
    return _filteredAlt - _heightOffset;
}

void baro::updateAll() {
    updateRawTemp();
    updateRawPress();
    for(int8_t i = MAVG_SAMPLES - 2; i > - 1; i--) {
        _filteredAltSamples[i+1] = _filteredAltSamples[i];
    }
    _filteredAltSamples[0] = getAltitude();
    _filteredAlt = 0;
    for (uint8_t i = 0; i < 20; i++) {
        _filteredAlt += _filteredAltSamples[i];
    }
    _filteredAlt /= MAVG_SAMPLES;

    if (getFilteredAltitude() > _maxAlt) {
        _maxAlt = getFilteredAltitude();
    }
}

float baro::getMaxAlt() {
    return _maxAlt;
}

void baro::zeroAlt() {
    _heightOffset = getFilteredAltitude();
    _maxAlt = 0;
}

void baro::resetMaxAlt() {
    _maxAlt = 0;
}
