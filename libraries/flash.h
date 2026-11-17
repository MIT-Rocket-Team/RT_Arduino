#include "Arduino.h"
#include "SPI.h"

class flash {
    public:
        flash(SPIClass* SPI, SPISettings settings, int cs);
        void begin();
        void programPage(uint8_t* data, uint32_t adr);
        void readPage(uint8_t* data, uint32_t adr);
        void sectorErase(uint32_t adr);
        bool isBusy();
        void eraseFlash();
        uint8_t readStatusRegister();
        void writeEnable();
        
    private:
        SPIClass* _SPI;
        SPISettings _settings;
        int _cs;
};