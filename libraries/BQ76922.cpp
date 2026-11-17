#include "Arduino.h"
#include "BQ76922.h"

BQ76922::BQ76922(TwoWire* wire, byte adr) {
    _Wire = wire;
    _adr = adr;
}

void BQ76922::begin() {
    _Wire->begin();
    delay(100);
    _Wire->beginTransmission(_adr);
    _Wire->endTransmission();
}

void BQ76922::_writeByte(byte data) {
    _Wire->beginTransmission(_adr);
    _Wire->write(data);
    _Wire->endTransmission();
}

void BQ76922::_writeSubCmdAdr(uint16_t data, bool send) {
    _Wire->beginTransmission(_adr);
    _Wire->write(0x3E);
    _Wire->write((byte) data);
    _Wire->write((byte) (data >> 8));
    if(send) {
        _Wire->endTransmission();
    }
}

void BQ76922::_dirCmdR(byte cmd, byte len) {
    _Wire->beginTransmission(_adr);
    _Wire->write(cmd);
    _Wire->endTransmission(false);
    _Wire->requestFrom(_adr, len);
    _Wire->readBytes(_buf, len);
}

bool BQ76922::_subCmdR(unsigned int cmd) { //See page 13 TRM
    _writeSubCmdAdr(cmd, true); //1. Write lower byte of subcommand to 0x3E, 2. Write upper byte of subcommand to 0x3F.
    
    bool complete = false;
    while(!complete) { //Continue reading 0x3E and 0x3F until it returns what was written originally
        _writeByte(0x3E);
        _Wire->requestFrom(_adr, 2); //3. Read 0x3E and 0x3F
        if((_Wire->read() == ((byte) cmd)) && (_Wire->read() == ((byte) (cmd >> 8)))) { //If this returns 0xFF, this indicates the subcommand has not completed operation yet
            complete = true;
        }
    }
    _writeByte(0x61); //4. Read the length of response from 0x61.
    _Wire->requestFrom(_adr, 1);
    _bufLen = _Wire->read() - 4; //0x61 provides the length of the buffer data plus 4
    _writeByte(0x40); //5. Read buffer starting at 0x40
    _Wire->requestFrom(_adr, _bufLen);
    _Wire->readBytes(_buf, _bufLen);
    //6. Read the checksum at 0x60 and verify it matches the data read
    byte checksum = (byte) cmd + (byte) (cmd >> 8); 
    for(int i = 0; i < _bufLen; i++) {
        checksum += _buf[i];
    }
    checksum = ~checksum;
    _writeByte(0x60);
    _Wire->requestFrom(_adr, 1);

    return _Wire->read() == checksum;
}

void BQ76922::_subCmdW(unsigned int cmd, byte* data, byte len) {
    byte checksum = (byte) cmd + (byte) (cmd >> 8);
    for(int i = 0; i < len; i++) {
        checksum += *(data + i);
    }
    checksum = ~checksum;
    _writeSubCmdAdr(cmd);
    _Wire->write(data, len);
    for(int i = 0; i < (32 - len); i++) {
        _Wire->write(0x00);
    }
    _Wire->write(checksum);
    _Wire->write(len + 4);
    _Wire->endTransmission();
}

bool BQ76922::_writeMem(unsigned int cmd, byte* data, byte len) {
    _subCmdW(cmd, data, len);
    /*Serial.println("HERE2");
    if(!_subCmdR(cmd)) {
        Serial.print("HERE3");
        return false;
    }
    byte _bufcpy[1];
    memcpy(_bufcpy, _buf, 1);
    Serial.print("HERE4");
    for(byte i = 0; i < len; i++) {
        if(*(data + i) != *(_bufcpy + i)) {
            Serial.println("RET FALSE");
            return false;
        }
    }
    Serial.println("RET TRUE");
    return true;
    Serial.print("BYTE: ");
    //Serial.print(_buf);
    //Serial.println(_buf[0]);*/
    return true;
}


void BQ76922::_subCmdWByte(uint16_t cmd, byte data) {
    byte checksum = (byte) cmd + (byte) (cmd >> 8);
    checksum += data;
    checksum = ~checksum;
    _writeSubCmdAdr(cmd);
    _Wire->write(data);
    for(int i = 0; i < 31; i++) {
        _Wire->write(0x00);
    }
    _Wire->write(checksum);
    _Wire->write(5); //len + 4
    _Wire->endTransmission();
}

void BQ76922::_subCmdWBytes(uint16_t cmd, uint16_t data) {
    byte checksum = (byte) cmd + (byte) (cmd >> 8);
    checksum += (byte) data + (byte) (data >> 8);
    checksum = ~checksum;
    _writeSubCmdAdr(cmd);
    _Wire->write((byte) data);
    _Wire->write((byte) (data >> 8));
    for(int i = 0; i < 30; i++) {
        _Wire->write(0x00);
    }
    _Wire->write(checksum);
    _Wire->write(6); //len + 4
    _Wire->endTransmission();
}

bool BQ76922::_writeMemByte(uint16_t adr, byte data) {
    _subCmdWByte(adr, data);
    _subCmdR(adr);
    //to-do: verify write
    return true;
}


bool BQ76922::_writeMemBytes(uint16_t adr, uint16_t data) {
    _subCmdWBytes(adr, data);
    _subCmdR(adr);
    //to-do: verify write
    return true;
}

bool BQ76922::chargePumpEnable() {
    _writeMemByte(0x9309, 0x01);
    //to-do: verify write
    return true;
}

void BQ76922::enterConfigMode() {
    _writeSubCmdAdr(0x0090, true);
    while (1) {
        _dirCmdR(0x12, 2);
        if (_buf[0] & 0x01) {
            return;
        }
        delay(1);
    }
}

void BQ76922::exitConfigMode() {
    _writeSubCmdAdr(0x0092, true);
    while (1) {
        _dirCmdR(0x12, 2);
        if (!(_buf[0] & 0x01)) {
            return;
        }
        delay(1);
    }
}


bool BQ76922::disableProtections() {
    _writeMemByte(0x9261, 0x00);

    /*if(_subCmdR(0x9261)) {
        Serial.println("SC SUCCESS");
    }
    Serial.print("BYTE 1: 0x");
    Serial.println(_buf[0]);
    Serial.print("BYTE 2: 0x");
    Serial.println(_buf[31]);*/

    //to-do: verify write
    return true;
}

bool BQ76922::enableFet() {

    
    //_writeSubCmdAdr(0x0020, true);
    //_writeSubCmdAdr(0x0090, true);
    //delay(1000);
    /*byte arr[2] = {0x10, 0x00};
    _writeMem(0x9343, arr, 2);
    if(_subCmdR(0x9343)) {
        Serial.println("SC SUCCESS");
    }
    Serial.print("BYTE 1: 0x");
    Serial.println(_buf[0]);
    Serial.print("BYTE 2: 0x");
    Serial.println(_buf[1]);
    _writeSubCmdAdr(0x0092, true);*/
    _writeMemBytes(0x9308, 0x1C);
    _writeMemBytes(0x9343, 0x10);
    
    //to-do: verify write
    return true;
}

byte BQ76922::fetStatus() {
    _dirCmdR(0x7F, 1);
    return _buf[0];
}

uint16_t BQ76922::statusHex() {
    _dirCmdR(0x12, 2);
    return (_buf[1] << 8) + _buf[0];
}

bool BQ76922::ddsgConfig(byte config) {
    /*_writeSubCmdAdr(0x0090, true);
    delay(1000);
    _dirCmdR(0x12,2);
    Serial.print("BYTE 1: 0x");
    Serial.println(_buf[0], HEX);
    Serial.print("BYTE 2: 0x");
    Serial.println(_buf[1], HEX);
    _writeMem(0x9302, (byte*) &config, 1);
    if(_subCmdR(0x9302)) {
        Serial.println("SC SUCCESS");
    }
    Serial.print("BYTE 1: 0x");
    Serial.println(_buf[0], HEX);
    _writeSubCmdAdr(0x0092, true);*/

    _writeMemByte(0x9302, config);

    //to-do: verify write
    return true;
}

bool BQ76922::dfetoffConfig(byte config) {
    /*_writeSubCmdAdr(0x0090, true);
    delay(1000);
    _writeMem(0x92FB, (byte*) &config, 1);
    _writeSubCmdAdr(0x0092, true);*/
    _writeMemByte(0x92FB, config);

    //to-do: verify write
    return true;
}

bool BQ76922::daConfig(byte config) { //0x0A: userV = mV, userA = cA, use die temp for cell temp protections
    _writeMemByte(0x9303, config);

    //to-do: verify write
    return true;
}

int16_t BQ76922::cellVoltage(byte cell) {
    _dirCmdR(0x14 + 2 * (cell - 1), 2);
    return (_buf[1] << 8) + _buf[0];
}

int16_t BQ76922::stackVoltage() {
    _dirCmdR(0x34, 2);
    return (_buf[1] << 8) + _buf[0];
}

int16_t BQ76922::current() {
    _dirCmdR(0x3A, 2);
    return (_buf[1] << 8) + _buf[0];
}

float BQ76922::temp() {
    _dirCmdR(0x68, 2);
    return (((_buf[1] << 8) + _buf[0]) / 10.0) - 273.15;
}

void BQ76922::fullAccess() {
    _Wire->beginTransmission(_adr);
    _Wire->write(0x3E);
    _Wire->write(0xFF);
    _Wire->write(0xFF);
    _Wire->endTransmission(false);
    _Wire->beginTransmission(_adr);
    _Wire->write(0x3E);
    _Wire->write(0xFF);
    _Wire->write(0xFF);
    _Wire->endTransmission();
}

bool BQ76922::OTPcheck() {
    _subCmdR(0x00A0);
    return _buf[0] & (1 << 7);
}

void BQ76922::OTPdebug() {
    _subCmdR(0x00A0);
    if (_buf[0] & 1) {
        Serial.println("STACK VOLTAGE TOO HIGH");
    } else if (_buf[0] & (1 << 1)) {
        Serial.println("STACK VOLTAGE TOO LOW");
    } else if (_buf[1] & (1 << 2)) {
        Serial.println("HIGH TEMP");
    } else if (_buf[1] & (1 << 3)) {
        Serial.println("DATA PROGRAMMED TOO MANY TIMES");
    } else if (_buf[1] & (1 << 4)) {
        Serial.println("SIGNATURE WRITTEN TOO MANY TIMES");
    } else if (_buf[1] & (1 << 4)) {
        Serial.println("NOT IN FULLACCESS AND CONFIG_UPDATE / OTP LOCK SET");
    }
}

void BQ76922::statusReadout() {
    _dirCmdR(0x12, 2);
    uint16_t status = (_buf[1] << 8) + _buf[0];
    
    if (status & 1) {
        Serial.println("Device is in CONFIG_UPDATE mode.");
    } else {
        Serial.println("Device is not in CONFIG_UPDATE mode.");
    }

    if (status & (1 << 1)) {
        Serial.println("Device is in PRECHARGE mode.");
    } else {
        Serial.println("Device is not in PRECHARGE mode.");
    }

    if (status & (1 << 2)) {
        Serial.println("SLEEP mode is allowed when other SLEEP conditions are met.");
    } else {
        Serial.println("SLEEP mode is disabled by the host.");
    }

    if (status & (1 << 3)) {
        Serial.println("Full reset has occurred since last exit of CONFIG_UPDATE and reconfiguration of any RAM settings is required.");
    } else {
        Serial.println("Full reset has not occurred since last exit of CONFIG_UPDATE mode.");
    }

    if (status & (1 << 4)) {
        Serial.println("Previous reset was caused by the watchdog timer.");
    } else {
        Serial.println("Previous reset was normal.");
    }

    if (status & (1 << 5)) {
        Serial.println("Device is actively performing a cell open-wire check.");
    } else {
        Serial.println("Device is not actively performing a cell open-wire check.");
    }

    if (status & (1 << 6)) {
        Serial.println("Writes to OTP are pending.");
    } else {
        Serial.println("No writes to OTP are pending.");
    }

    if (status & (1 << 7)) {
        Serial.println("Writes to OTP are blocked");
    } else {
        Serial.println("OTP writes are allowed.");
    }

    if (status & (3 << 8) == 0) {
        Serial.println("Device has not initialized yet.");
    } else if (status & (3 << 8) == 1) {
        Serial.println("Device is in FULLACCESS mode.");
    } else if (status & (3 << 8) == 2) {
        Serial.println("Device is in UNSEALED mode.");
    } else {
        Serial.println("Device is in SEALED mode.");
    }

    if (status & (1 << 10)) {
        Serial.println("FUSE pin was asserted by device or secondary protector at last sample.");
    } else {
        Serial.println("FUSE pin was not asserted by device or secondary protector at last sample.");
    }

    if (status & (1 << 11)) {
        Serial.println("At least one enabled safety fault is triggered.");
    } else {
        Serial.println("No safety fault is triggered.");
    }

    if (status & (1 << 12)) {
        Serial.println("At least one Permanent Fail fault has triggered.");
    } else {
        Serial.println("No Permanent Fail fault has triggered.");
    }

    if (status & (1 << 13)) {
        Serial.println("Shutdown due to command or pin is pending.");
    } else {
        Serial.println("Shutdown due to command or pin is not pending.");
    }

    if (status & (1 << 15)) {
        Serial.println("Device is in SLEEP mode.");
    } else {
        Serial.println("Device is not in SLEEP mode.");
    }

}

bool BQ76922::fullAccessCheck() {
    _dirCmdR(0x12, 2);
    return (_buf[1] & 0x03) == 0x01;
}

bool BQ76922::writeOTP() {
    _subCmdR(0x00A1);
    delay(1000);
    return _buf[0] == 0x80;
}

bool BQ76922::cellConfig(byte numCells) {
    unsigned int config;
    if(numCells == 3) {
        config = 0x0013;
    }
    _writeMemBytes(0x9304, config);

    //to-do: verify write
    return true;
}

bool BQ76922::cellUVOnly() {
    _writeMemByte(0x9261, 0x04);

    //to-do: verify write
    return true;
}

bool BQ76922::cellUVandSC() {
    _writeMemByte(0x9261, 0x84);

    //to-do: verify write
    return true;
}

bool BQ76922::cellSC20mV() {
    _writeMemByte(0x9286, 0x01);
    _writeMemByte(0x9287, 31);

    //to-do: verify write
    return true;
}

bool BQ76922::minCellVoltage(float voltage) {
    byte setting = (voltage / .0506) + 1; //round up for safety
    _writeMemByte(0x9275, setting);

    //to-do: verify write
    return true;
}

uint8_t BQ76922::safetyStatusA() {
    _dirCmdR(0x03, 1);
    return _buf[0];
}

uint8_t BQ76922::enabledProtectionsA() {
    _subCmdR(0x9261);
    return _buf[0];
}