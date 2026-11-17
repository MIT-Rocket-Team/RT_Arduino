#include "Arduino.h"
#include "SPI.h"
#include "flash.h"

flash::flash(SPIClass* SPI, SPISettings settings, int cs) {
    _SPI = SPI;
    _settings = settings;
    _cs = cs;
}

uint8_t flash::readStatusRegister() {
    digitalWrite(_cs, 0);
    _SPI->beginTransaction(_settings);
    _SPI->transfer(0x05);
    uint8_t status = _SPI->transfer(0x00);
    _SPI->endTransaction();
    digitalWrite(_cs, 1);
    return status;
}
void flash::begin() {
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, 1);
}

void flash::programPage(uint8_t* data, uint32_t adr) {
    writeEnable();
    digitalWrite(_cs, 0);
    _SPI->beginTransaction(_settings);
    _SPI->transfer(0x02);
    _SPI->transfer(adr >> 16);
    _SPI->transfer(adr >> 8);
    _SPI->transfer(adr);
    _SPI->transfer(data, 512);
    _SPI->endTransaction();
    digitalWrite(_cs, 1);
    writeEnable();
}

void flash::readPage(uint8_t* data, uint32_t adr) {
    digitalWrite(_cs, 0);
    _SPI->beginTransaction(_settings);
    _SPI->transfer(0x03);
    _SPI->transfer(adr >> 16);
    _SPI->transfer(adr >> 8);
    _SPI->transfer(adr);
    _SPI->transfer(data, 512);
    _SPI->endTransaction();
    digitalWrite(_cs, 1);
}

void flash::sectorErase(uint32_t adr) {
    writeEnable();
    digitalWrite(_cs, 0);
    _SPI->beginTransaction(_settings);
    _SPI->transfer(0xD8);
    _SPI->transfer(adr >> 16);
    _SPI->transfer(adr >> 8);
    _SPI->transfer(adr);
    _SPI->endTransaction();
    digitalWrite(_cs, 1);
    writeEnable();
}

void flash::writeEnable() {
    digitalWrite(_cs, 0);
    _SPI->beginTransaction(_settings);
    _SPI->transfer(0x06);
    _SPI->endTransaction();
    digitalWrite(_cs, 1);
}

bool flash::isBusy() {
    digitalWrite(_cs, 0);
    _SPI->beginTransaction(_settings);
    _SPI->transfer(0x05);
    uint8_t status = _SPI->transfer(0x00);
    _SPI->endTransaction();
    digitalWrite(_cs, 1);
    return (status & 0x01);
} 

void flash::eraseFlash() {
    writeEnable();
    digitalWrite(_cs, 0);
    _SPI->beginTransaction(_settings); 
    _SPI->transfer(0xC7);
    _SPI->endTransaction();
    digitalWrite(_cs, 1);
}