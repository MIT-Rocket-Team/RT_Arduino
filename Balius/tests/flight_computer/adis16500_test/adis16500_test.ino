#include <ADIS16500.h>

Uart debugSer(PC7, PC6);

#define MOSI PD7
#define MISO PB4
#define SCLK PA5

#define IMU_CS   PD0
#define BARO_CS  PD1
#define FLASH_CS PD2
#define CC_CS    PD3
#define VIB_CS   PD4
#define LOAD_CS  PD5

constexpr uint8_t otherPins[] = {BARO_CS, FLASH_CS, CC_CS, VIB_CS, LOAD_CS};
constexpr uint16_t EXPECTED_PRODUCT_ID = 0x4074;

SPIClass SPI_3(MOSI, MISO, SCLK, -1);
// ADIS16500 requires mode 3, MSB first, 16-bit frames, and SCLK <= 2 MHz.
SPISettings settings(1000000, MSBFIRST, SPI_MODE3);
ADIS16500 imu(&SPI_3, settings, IMU_CS);

void setup() {
  debugSer.begin(115200);
  delay(100);
  debugSer.println("ADIS16500 SPI test");

  // Keep the other devices sharing SPI_3 deselected.
  for (uint8_t pin : otherPins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }

  imu.setup();
}

void loop() {
  const uint16_t productId = imu.readProductId();

  debugSer.print("Product ID: 0x");
  if (productId < 0x1000) debugSer.print("0");
  if (productId < 0x0100) debugSer.print("0");
  if (productId < 0x0010) debugSer.print("0");
  debugSer.println(productId, HEX);

  if (productId == EXPECTED_PRODUCT_ID) {
    debugSer.println("SPI read PASS");
  } else {
    debugSer.println("SPI read FAIL; expected 0x4074");
  }

  delay(1000);
}
