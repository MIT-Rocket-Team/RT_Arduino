#include "Arduino.h"
#include "SPI.h"

#define C1 0x9946
#define C2 0x921C
#define C3 0x5FED
#define C4 0x5B68
#define C5 0x85CB
#define C6 0x6C43

#define MAVG_SAMPLES 20

class baro {
    public:
        baro(SPIClass* SPI, SPISettings settings, int cs);
        void begin();
        void updateRawTemp();
        void updateRawPress();
        uint32_t getRawTemp();
        uint32_t getRawPress();
        uint16_t getCalibrationConstant(uint8_t index);
        float getTemperature();
        float getPressure();
        float getAltitude();
        float getFilteredAltitude();
        float getMaxAlt();
        void zeroAlt();
        void updateAll();
        void resetMaxAlt();
        
    private:
        SPIClass* _SPI;
        SPISettings _settings;
        int _cs;
        uint32_t _rawTemp, _rawPress;
        float _filteredAlt;
        float _filteredAltSamples[MAVG_SAMPLES];
        float _maxAlt;
        float _heightOffset;
};