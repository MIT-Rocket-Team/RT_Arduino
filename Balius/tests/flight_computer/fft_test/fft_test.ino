#include <SPI.h>
#include <ADcmXL1021.h>

Uart debugSer(PC7, PC6);

#define MOSI PD7
#define MISO PB4
#define SCLK PA5

#define IMU_CS   PD0
#define BARO_CS  PD1
#define FLASH_CS PD2
#define CC_CS    PD3
#define VIB_CS   PD4
#define ADC_CS   PD5

const uint8_t otherPins[] = {IMU_CS, BARO_CS, FLASH_CS, CC_CS, ADC_CS};

SPIClass SPI_3(MOSI, MISO, SCLK, -1);
SPISettings settings(1000000, MSBFIRST, SPI_MODE3);
ADcmXL1021 sensor(&SPI_3, settings, VIB_CS);

// Set true to save each FFT to on-chip flash; false keeps it in volatile RAM.
constexpr bool SAVE_TO_FLASH = false;
constexpr uint8_t FLASH_RECORD_INDEX = 0;
constexpr uint8_t FFT_SAMPLE_RATE_CODE = 4; // 27.5 kSPS; 6.71 Hz/bin.
constexpr uint32_t SAMPLE_RATE_HZ = 27500;
constexpr uint32_t MIN_CAPTURE_TIME_MS = 200;
constexpr uint32_t CAPTURE_TIMEOUT_MS = 5000;

uint16_t fftBins[ADcmXL1021::FFT_BIN_COUNT];
bool captureStarted = false;
bool captureReported = false;
bool sensorReady = false;
uint32_t captureStartedAt = 0;
uint32_t lastAttemptAt = 0;

void streamFFT() {
  debugSer.print("FFT_BEGIN,");
  debugSer.print(SAVE_TO_FLASH ? "FLASH," : "RAM,");
  debugSer.print(SAVE_TO_FLASH ? FLASH_RECORD_INDEX : -1);
  debugSer.print(',');
  debugSer.print(SAMPLE_RATE_HZ);
  debugSer.print(',');
  debugSer.println(ADcmXL1021::FFT_BIN_COUNT);

  for (uint16_t i = 0; i < ADcmXL1021::FFT_BIN_COUNT; ++i) {
    debugSer.print("FFT_BIN,");
    debugSer.print(i);
    debugSer.print(',');
    debugSer.println(fftBins[i]);
  }
  debugSer.println("FFT_END");
}

void printRegister(const char* name, uint8_t address) {
  debugSer.print(name);
  debugSer.print("=0x");
  const uint16_t value = sensor.readRegister(address);
  if (value < 0x1000) debugSer.print('0');
  if (value < 0x0100) debugSer.print('0');
  if (value < 0x0010) debugSer.print('0');
  debugSer.println(value, HEX);
}

void setup() {
  debugSer.begin(115200);
  delay(100);
  debugSer.println("ADcmXL1021 FFT test");

  for (uint8_t i = 0; i < sizeof(otherPins) / sizeof(otherPins[0]); ++i) {
    pinMode(otherPins[i], OUTPUT);
    digitalWrite(otherPins[i], HIGH);
  }

  sensor.begin();
  if (sensor.readProductId() != ADcmXL1021::EXPECTED_PRODUCT_ID) {
    debugSer.println("ERROR: ADcmXL1021 product ID mismatch");
    return;
  }
  if (!sensor.configureFFT(FFT_SAMPLE_RATE_CODE, 3, 8)) {
    debugSer.println("ERROR: FFT configuration failed or device is busy");
    return;
  }
  sensorReady = true;

  debugSer.print("Mode: ");
  debugSer.println(SAVE_TO_FLASH ? "FLASH" : "RAM");
  printRegister("REC_CTRL", 0x1A);
  printRegister("DIAG_STAT", 0x3C);
  debugSer.println("Ready to capture");
}

void loop() {
  if (!sensorReady) return;
  const uint32_t now = millis();

  if (!captureStarted && !captureReported && (uint32_t)(now - lastAttemptAt) >= 1000) {
    lastAttemptAt = now;
    const bool started = SAVE_TO_FLASH ? sensor.calculateAndSaveFFT() : sensor.calculateFFT();
    if (started) {
      captureStarted = true;
      captureStartedAt = now;
      debugSer.println("FFT capture started");
    }
    else {
      debugSer.println("ERROR: FFT capture start failed");
      printRegister("REC_CTRL", 0x1A);
      printRegister("DIAG_STAT", 0x3C);
    }
  }

  if (captureStarted && (uint32_t)(now - captureStartedAt) >= MIN_CAPTURE_TIME_MS && sensor.fftReady()) {
    debugSer.println("FFT capture complete; reading bins");
    const bool readOk = SAVE_TO_FLASH
        ? sensor.retrieveFFT(fftBins, FLASH_RECORD_INDEX)
        : sensor.readCurrentFFT(fftBins);
    captureStarted = false;
    captureReported = true;
    if (readOk) {
      streamFFT();
      debugSer.println("FFT stream complete");
    } else {
      debugSer.println("ERROR: FFT readback failed");
    }
  }

  if (captureStarted && (uint32_t)(now - captureStartedAt) >= CAPTURE_TIMEOUT_MS) {
    debugSer.println("ERROR: FFT capture timed out");
    printRegister("REC_CTRL", 0x1A);
    printRegister("DIAG_STAT", 0x3C);
    captureStarted = false;
    captureReported = true;
  }

  delay(10); // Nonblocking capture polling at approximately 100 Hz.
}
