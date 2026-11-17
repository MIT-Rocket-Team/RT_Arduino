#include "Arduino.h"
#include "SPI.h"

class DRV8452 {
    public:
        DRV8452(SPIClass* SPI, SPISettings settings, int cs, int slp, int en);
        void fullStep(bool forward);
        void setup();
        void setStepCurrentLimit(float current);
        void setHoldCurrentLimit(float current);
        float getStepCurrentLimit();
        float getHoldCurrentLimit();
        uint8_t status();
        
    private:
        SPIClass* _SPI;
        SPISettings _settings;
        int _cs;
        int _slp;
        int _en;
        void _writeReg(byte reg, byte val);
        uint8_t _readReg(byte reg);
};