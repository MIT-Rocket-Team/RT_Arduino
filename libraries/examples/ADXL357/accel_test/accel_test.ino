#include "SPI.h"
#include "ADXL357.h"

#define MOSI PA7
#define MISO PA6
#define SCLK PA5
#define CS PB0

SPIClass SPI_3(MOSI, MISO, SCLK, -1);
SPISettings settings(1000000, MSBFIRST, SPI_MODE0);

ADXL357 accel(&SPI_3, settings, CS);

void setup() {
  Serial.begin(115200);
  SPI_3.begin();
  accel.setup();
}

void loop() {
  Serial.println("X: " + String(accel.getAccelX()) + " // Y: " + String(accel.getAccelY()) + " // Z: " + String(accel.getAccelZ()));
  accel.update();
  delay(1000);
}
