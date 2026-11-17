#ifndef ADCMXL1021_H
#define ADCMXL1021_H

#include "Arduino.h"
#include "SPI.h"

class ADcmXL1021 {
public:
    static constexpr uint8_t PRODUCT_ID_REGISTER = 0x56;
    static constexpr uint8_t OUT_BUF_REGISTER = 0x12;
    static constexpr uint16_t EXPECTED_PRODUCT_ID = 0x03FD;
    static constexpr uint16_t FFT_BIN_COUNT = 2048;
    static constexpr uint8_t FFT_RECORD_COUNT = 10;

    ADcmXL1021(SPIClass* spi, SPISettings settings, int chipSelect);
    void begin();
    uint16_t readRegister(uint8_t lowerByteAddress);
    uint16_t readProductId();

    // Configure a single-rate MFFT capture with Hanning window. Rate code 4
    // is 27.5 kSPS (6.71 Hz/bin); FIR selection 3 is the 10 kHz low-pass.
    // FIR selections: 0 none, 1/2/3 LP 1/5/10 kHz, 4/5/6 HP 1/5/10 kHz.
    bool configureFFT(uint8_t sampleRateCode = 1, uint8_t firSelection = 3,
                      uint8_t fftAverages = 8);

    // Start one manually triggered FFT using the current REC_CTRL settings.
    // Returns immediately; poll fftReady() from the application loop.
    bool calculateFFT();

    // Select MFFT mode with "store all records" and start one FFT.
    // Returns immediately. Flash holds at most 10 spectral records.
    bool calculateAndSaveFFT();

    // One-shot readiness check; does not wait or delay.
    bool fftReady();

    // Copy the most recently completed FFT from volatile RAM (2048 bins).
    bool readCurrentFFT(uint16_t* data);

    // Load flash record index 0..9 and copy its 2048 unsigned bins to data.
    // Caller must provide an array with at least FFT_BIN_COUNT elements.
    bool retrieveFFT(uint16_t* data, uint8_t index);

private:
    void writeRegister(uint8_t lowerByteAddress, uint16_t value);
    void writeByte(uint8_t address, uint8_t value);
    SPIClass* _spi;
    SPISettings _settings;
    int _chipSelect;
};

#endif
