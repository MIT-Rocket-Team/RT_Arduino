#include "Arduino.h"
#include "Wire.h"

#define DEFAULT_ADDR 0x08

class BQ76922 {
    public:
        BQ76922(TwoWire* wire = &Wire, byte adr = DEFAULT_ADDR);
        void begin();
        int16_t cellVoltage(byte cell);
        int16_t stackVoltage();
        int16_t current();
        float temp();
        void fullAccess();
        bool dfetoffConfig(byte config = 0x86);
        bool ddsgConfig(byte config = 0x2A);
        bool daConfig(byte config = 0x0A);
        bool cellConfig(byte numCells);
        byte fetStatus();
        bool enableFet();
        void enterConfigMode();
        void exitConfigMode();
        bool disableProtections();
        bool cellUVOnly();
        bool minCellVoltage(float voltage = 3);
        bool OTPcheck();
        void OTPdebug();
        bool fullAccessCheck();
        bool writeOTP();
        void statusReadout();
        uint16_t statusHex();
        bool chargePumpEnable();
        bool cellUVandSC();
        bool cellSC20mV();
        uint8_t safetyStatusA();
        uint8_t enabledProtectionsA();

    private:
        TwoWire* _Wire;
        byte _adr;
        byte _buf[256];
        byte _bufLen;
        void _dirCmdR(byte cmd, byte len);
        bool _subCmdR(unsigned int cmd);
        void _subCmdW(unsigned int cmd, byte* data, byte len);
        void _writeByte(byte data);
        void _writeSubCmdAdr(uint16_t data, bool send = false);
        bool _writeMem(unsigned int cmd, byte* data, byte len);
        void _subCmdWByte(uint16_t cmd, byte data);
        void _subCmdWBytes(uint16_t cmd, uint16_t data);
        bool _writeMemByte(uint16_t adr, byte data);
        bool _writeMemBytes(uint16_t adr, uint16_t data);

};
