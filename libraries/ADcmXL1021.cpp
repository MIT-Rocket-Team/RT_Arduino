#include "ADcmXL1021.h"

ADcmXL1021::ADcmXL1021(SPIClass* spi, SPISettings settings, int chipSelect)
    : _spi(spi), _settings(settings), _chipSelect(chipSelect) {}

void ADcmXL1021::begin() {
    pinMode(_chipSelect, OUTPUT);
    digitalWrite(_chipSelect, HIGH);
    _spi->begin();

    // ADcmXL1021-1 startup time is specified as 205 ms after VDD reaches 3.0 V.
    delay(250);
}

uint16_t ADcmXL1021::readRegister(uint8_t lowerByteAddress) {
    _spi->beginTransaction(_settings);
    digitalWrite(_chipSelect, LOW);

    delayMicroseconds(1);

    // The ADcmXL interface uses 16-clock words. Send the read/address word
    // explicitly as bytes: R/W = 0, followed by the 7-bit register address.
    uint8_t address = lowerByteAddress & 0x7F;
    _spi->transfer16((uint16_t)(lowerByteAddress & 0x7F) << 8);
    digitalWrite(_chipSelect, HIGH);

    delayMicroseconds(20);
    
    digitalWrite(_chipSelect, LOW);
    uint16_t response = _spi->transfer16(0x0000);

    digitalWrite(_chipSelect, HIGH);
    _spi->endTransaction();

    delayMicroseconds(20);

    // The register readout is little-endian; return a host-order 16-bit word.
    return response;
}
uint16_t ADcmXL1021::readProductId() {
    return readRegister(PRODUCT_ID_REGISTER);
}

bool ADcmXL1021::configureFFT(uint8_t sampleRateCode, uint8_t firSelection,
                              uint8_t fftAverages) {
    if (sampleRateCode < 1 || sampleRateCode > 7 || firSelection > 6 ||
        fftAverages < 1) return false;
    if (!fftReady()) return false;

    // AVG_CNT[3:0] sets SR0's sample rate scale. The datasheet table maps
    // code 1 to 220 kSPS (53.8 Hz/bin for a 4096-sample record).
    uint16_t avgCnt = readRegister(0x3A);
    avgCnt = (uint16_t)((avgCnt & 0xFFF0) | sampleRateCode);
    writeRegister(0x3A, avgCnt);

    // FILT_CTRL[10:8] selects the preconfigured FIR coefficient bank.
    uint16_t filtCtrl = readRegister(0x38);
    filtCtrl = (uint16_t)((filtCtrl & ~0x0700) | ((uint16_t)firSelection << 8));
    writeRegister(0x38, filtCtrl);

    // FFT_AVG1 low byte controls SR0. One average gives the shortest capture.
    uint16_t fftAvg1 = readRegister(0x06);
    fftAvg1 = (uint16_t)((fftAvg1 & 0xFF00) | fftAverages);
    writeRegister(0x06, fftAvg1);

    // MFFT, Hanning window, SR0 enabled; leave record storage to the capture
    // call (RAM-only for calculateFFT, flash-all for calculateAndSaveFFT).
    uint16_t recCtrl = readRegister(0x1A);
    recCtrl = (uint16_t)((recCtrl & ~(0x3000 | 0x0F00 | 0x0003)) |
                         0x1000 | 0x0100);
    writeRegister(0x1A, recCtrl);
    return true;
}

// Register byte writes use one 16-clock command word per byte. The high bit
// marks a write; the remaining address bits select the register byte.
void ADcmXL1021::writeByte(uint8_t address, uint8_t value) {
    _spi->beginTransaction(_settings);
    digitalWrite(_chipSelect, LOW);
    delayMicroseconds(1);
    _spi->transfer16((uint16_t)(0x8000 | ((address & 0x7F) << 8) | value));
    digitalWrite(_chipSelect, HIGH);
    _spi->endTransaction();
    delayMicroseconds(20);
}

void ADcmXL1021::writeRegister(uint8_t lowerByteAddress, uint16_t value) {
    writeByte(lowerByteAddress, (uint8_t)(value & 0xFF));
    writeByte(lowerByteAddress + 1, (uint8_t)(value >> 8));
}

bool ADcmXL1021::fftReady() {
    // DIAG_STAT bit 7 is 1 when data is ready (BUSY is inactive).
    return (readRegister(0x3C) & 0x0080) != 0;
}

bool ADcmXL1021::calculateFFT() {
    const uint16_t recCtrl = readRegister(0x1A);
    if ((recCtrl & 0x0003) != 0x0000) return false; // Must be MFFT mode.

    // The caller polls readiness; send the start command without blocking.
    writeRegister(0x3E, 0x0800);
    return true;
}

bool ADcmXL1021::calculateAndSaveFFT() {
    // REC_CTRL[3:2] = 10 stores every completed record in flash;
    // REC_CTRL[1:0] = 00 selects manual FFT. Preserve other settings.
    uint16_t recCtrl = readRegister(0x1A);
    recCtrl = (uint16_t)((recCtrl & ~0x000F) | 0x0008);
    writeRegister(0x1A, recCtrl);

    writeRegister(0x3E, 0x0800); // Start capture; record storage is automatic.
    return true;
}

bool ADcmXL1021::readCurrentFFT(uint16_t* data) {
    if (data == nullptr) return false;

    // GLOB_CMD bit 10 resets BUF_PNTR to zero. OUT_BUF reads auto-increment it.
    writeRegister(0x3E, 0x0400);
    for (uint16_t i = 0; i < FFT_BIN_COUNT; ++i) {
        data[i] = readRegister(OUT_BUF_REGISTER);
    }
    return true;
}

bool ADcmXL1021::retrieveFFT(uint16_t* data, uint8_t index) {
    if (data == nullptr || index >= FFT_RECORD_COUNT) return false;

    // REC_PNTR[3:0] selects an FFT record; GLOB_CMD bit 13 loads it to SRAM.
    writeRegister(0x0C, index);
    writeRegister(0x3E, 0x2000);

    // Reset buffer pointer (GLOB_CMD bit 10), then read bins 0..2047.
    writeRegister(0x3E, 0x0400);
    for (uint16_t i = 0; i < FFT_BIN_COUNT; ++i) {
        data[i] = readRegister(OUT_BUF_REGISTER);
    }
    return true;
}
