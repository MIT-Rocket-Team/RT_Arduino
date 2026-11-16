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
// ADcmXL1021-1 requires SPI mode 3, MSB first, in 16-bit words.
SPISettings settings(1000000, MSBFIRST, SPI_MODE3);
ADcmXL1021 sensor(&SPI_3, settings, VIB_CS);

void setup() {
  debugSer.begin(115200);
  delay(100);
  debugSer.println("ADcmXL1021-1 SPI test");

  // Keep the other devices sharing SPI_3 deselected.
  for (uint8_t i = 0; i < 5; i++) {
    pinMode(otherPins[i], OUTPUT);
    digitalWrite(otherPins[i], 1);
  }

  sensor.begin();
}

void loop() {
  uint16_t productId = sensor.readProductId();

  debugSer.print("Product ID: 0x");
  if (productId < 0x1000) debugSer.print("0");
  if (productId < 0x0100) debugSer.print("0");
  if (productId < 0x0010) debugSer.print("0");
  debugSer.println(productId, HEX);

  if (productId == ADcmXL1021::EXPECTED_PRODUCT_ID) {
    debugSer.println("SPI read PASS");
  } else {
    debugSer.println("SPI read FAIL; expected 0x03FD");
  }

  delay(1000);
}
