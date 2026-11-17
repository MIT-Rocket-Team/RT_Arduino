#include "SPI.h"
#include "DRV8452.h"

#define MOSI PA7
#define MISO PA6
#define SCLK PA5
#define CS_AZ PB0
#define SLP_AZ PB1
#define EN_AZ PB2
#define CS_EL PB6
#define SLP_EL PB7
#define EN_EL PB8

SPIClass SPI_3(MOSI, MISO, SCLK, -1);
SPISettings settings(1000000, MSBFIRST, SPI_MODE1);
DRV8452 drv_azimuth(&SPI_3, settings, CS_AZ, SLP_AZ, EN_AZ);
DRV8452 drv_elevation(&SPI_3, settings, CS_EL, SLP_EL, EN_EL);

void setup() {
  Serial.begin(115200);
  SPI_3.begin();
  drv_azimuth.setup();
  drv_elevation.setup();
}

void loop() {
  Serial.println("Expected Current Limit: 3.5A");
  drv_azimuth.setCurrentLimit(3.5);
  drv_elevation.setCurrentLimit(3.5);

  Serial.print("Azimuth Hold Current Limit: ");
  Serial.print(drv_azimuth.getHoldCurrentLimit());
  Serial.println("A");

  Serial.print("Azimuth Step Current Limit: ");
  Serial.print(drv_azimuth.getStepCurrentLimit());
  Serial.println("A");

  Serial.print("Elevation Hold Current Limit: ");
  Serial.print(drv_elevation.getHoldCurrentLimit());
  Serial.println("A");

  Serial.print("Elevation Step Current Limit: ");
  Serial.print(drv_elevation.getStepCurrentLimit());
  Serial.println("A");
  Serial.println();
  delay(1000);
}