#include "Arduino.h"
#include "Wire.h"

class INA232 {
    public:
        INA232(TwoWire* wire, byte adr);
        void begin();
        float voltage();
        float current();
        int16_t voltageRaw();
        int16_t currentRaw();

    private:
        TwoWire* _Wire;
        byte _adr;
        byte _buf[2];
        int16_t _readReg(uint8_t reg);
};
