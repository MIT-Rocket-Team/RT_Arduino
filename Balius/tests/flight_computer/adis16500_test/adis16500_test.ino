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

#define RST PA7
#define IMU_DR PA8

constexpr uint8_t otherPins[] = {BARO_CS, FLASH_CS, CC_CS, VIB_CS, LOAD_CS};
constexpr uint16_t EXPECTED_PRODUCT_ID = 0x4074;

SPIClass SPI_3(MOSI, MISO, SCLK, -1);
// ADIS16500 requires mode 3, MSB first, 16-bit frames, and SCLK <= 2 MHz.
SPISettings settings(1000000, MSBFIRST, SPI_MODE3);
ADIS16500 imu(&SPI_3, settings, IMU_CS);

volatile uint32_t imuDrRisingEdges = 0;

void onImuDrRisingEdge() {
  ++imuDrRisingEdges;
}

void setup() {
  pinMode(RST, OUTPUT);
  digitalWrite(RST, 1);
  debugSer.begin(115200);

  // Keep the other devices sharing SPI_3 deselected.
  for (uint8_t pin : otherPins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }

  // Hold CS inactive during the IMU power-on startup interval.
  pinMode(IMU_CS, OUTPUT);
  digitalWrite(IMU_CS, HIGH);
  pinMode(IMU_DR, INPUT);
  imu.setup();
  attachInterrupt(digitalPinToInterrupt(IMU_DR), onImuDrRisingEdge, RISING);

  delay(1000);
  noInterrupts();
  imuDrRisingEdges = 0;
  interrupts();
  debugSer.println("ADIS16500 SPI/DR test");
}

void loop() {
  static uint32_t lastReportMs = millis();
  static uint32_t lastEdgeCount = 0;
  const uint32_t now = millis();
  if ((uint32_t)(now - lastReportMs) < 1000) {
    delay(1);
    return;
  }

  const uint16_t productId = imu.readProductId();
  debugSer.print("Product ID: 0x");
  if (productId < 0x1000) debugSer.print("0");
  if (productId < 0x0100) debugSer.print("0");
  if (productId < 0x0010) debugSer.print("0");
  debugSer.println(productId, HEX);

  if (productId == EXPECTED_PRODUCT_ID) {
    debugSer.println("SPI read PASS");
    imu.update(GROUND_TESTING);

    debugSer.print("Accel (m/s^2): X=");
    debugSer.print(imu.getAccelX(), 6);
    debugSer.print(" Y=");
    debugSer.print(imu.getAccelY(), 6);
    debugSer.print(" Z=");
    debugSer.println(imu.getAccelZ(), 6);

    debugSer.print("Accel raw: X=");
    debugSer.print(imu.getRawX());
    debugSer.print(" Y=");
    debugSer.print(imu.getRawY());
    debugSer.print(" Z=");
    debugSer.println(imu.getRawZ());
  } else {
    debugSer.println("SPI read FAIL; expected 0x4074");
  }

  uint32_t edgeCount;
  noInterrupts();
  edgeCount = imuDrRisingEdges;
  interrupts();

  const uint32_t edgesThisInterval = edgeCount - lastEdgeCount;
  const uint32_t elapsedMs = now - lastReportMs;
  debugSer.print("DR rising edges/s: ");
  debugSer.println((edgesThisInterval * 1000U) / elapsedMs);

  lastEdgeCount = edgeCount;
  lastReportMs = now;
}
