#include "Arduino.h"
#include "SPI.h"
#include "ADS1220.h"

ADS1220::ADS1220(SPIClass* SPI, SPISettings settings, int cs) {
    _SPI = SPI;
    _settings = settings;
    _cs = cs;
}

void ADS1220::setup() {
    //Set cs pin as output and set it high
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, 1);

    //Begin _SPI, and run beginTransaction with _settings
    _SPI->begin();
    _SPI->beginTransaction(_settings);
    digitalWrite(_cs, 0);

    //Set AINP = AIN1, AINN = AIN2, gain = 128, PGA enabled (Configuration Register 0)
    _SPI->transfer(0b01000000); // WREG, register 0, 1 byte
    _SPI->transfer(0b00111110); // 0011 -> AINP = AIN1, AINN = AIN2; 111 -> gain = 128; 0 -> PGA enabled

    //Set the ADS1220 in continuous conversion mode in normal mode at 175SPS (Configuration Register 1)
    _SPI->transfer(0b01000100); // WREG, register 1, 1 byte
    _SPI->transfer(0b01100100); // 011 -> 175SPS; 00 -> normal mode; 1 -> continuous conversion mode; 0 -> disable temperature sensor; 0 -> burn-out current sources off

    //Set the ADS1220 to use "External reference selected using AIN0/REFP1 and AIN3/REFN1 inputs" as the voltage reference selection (Configuration Register 2)
    //Also, in Configuration Register 2, set Low-side power switch configuration to "Switch automatically closes when the START/SYNC command is sent and
    //opens when the POWERDOWN command is issued"
    _SPI->transfer(0b01001000); // WREG, register 2, 1 byte
    _SPI->transfer(0b10001000); // 10 -> external reference selected using AIN0/REFP1 and AIN3/REFN1 inputs; 00 -> not used in this mode; 1 -> switch automatically closes when the START/SYNC command is sent and opens when the POWERDOWN command is issued; 000 -> IDAC current off

    //Send the start command to the ADS1220 to begin conversions
    _SPI->transfer(0b00001000);

    digitalWrite(_cs, 1);
    _SPI->endTransaction();
}

void ADS1220::update() {
    _SPI->beginTransaction(_settings);
    digitalWrite(_cs, 0);

    //Send the RDATA command to the ADS1220 to read the conversion result
    _SPI->transfer(0b00010000);

    //Read the 3 bytes of data from the ADS1220 and store it in _measurement
    uint8_t data[3];
    data[0] = _SPI->transfer(0x00);
    data[1] = _SPI->transfer(0x00);
    data[2] = _SPI->transfer(0x00);

    digitalWrite(_cs, 1);
    _SPI->endTransaction();

    _measurement = ((int32_t)data[0] << 16) |
                   ((int32_t)data[1] << 8)  |
                   (int32_t)data[2];
    _measurement = ((int32_t) (_measurement << 8)) >> 8; // sign extend 24-bit result
}

int32_t ADS1220::getMeasurement() {
    return _measurement;
}