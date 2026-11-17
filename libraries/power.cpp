#include "Arduino.h"
#include "power.h"

power::power(Uart* pwrSer){
  _pwrSer = pwrSer;
}

void power::begin() {
  _pwrSer->begin(115200);
}

void power::update() {
    while (_pwrSer->available() >= sizeof(_pkt) + 2) {
        if (_pwrSer->read() == 0xAA) {
            _pwrSer->readBytes(_tmp, sizeof(_pkt));
            if (_pwrSer->read() == _calcChecksum(_tmp)) {
                memcpy(&_pkt, _tmp, sizeof(_pkt));    
            } 
        }
    }
}

uint8_t power::_calcChecksum(uint8_t* p) {
  uint8_t ret = 0;
  for (uint8_t i = 0; i < sizeof(_pkt); i++) {
    ret += *(p + i);
  }
  return ret;
}

uint16_t power::getConverterVoltage(uint8_t converter) {
    return _pkt.voltages[converter];
}

uint16_t power::getConverterCurrent(uint8_t converter) {
    return _pkt.currents[converter];
}

uint16_t power::getCell1Voltage() {
    return _pkt.BMS.cell1;
}

uint16_t power::getCell2Voltage() {
    return _pkt.BMS.cell2;
}

uint16_t power::getCell3Voltage() {
    return _pkt.BMS.cell3;
}

uint16_t power::getTotalCurrent() {
    return _pkt.BMS.current;
}

uint8_t power::getProtectionStatus() {
    return _pkt.BMS.protectionStatus;
}

uint8_t power::getProtectionsEnabled() {
    return _pkt.BMS.protectionsEnabled;
}

float power::getTemp() {
    return _pkt.BMS.temp;
}
