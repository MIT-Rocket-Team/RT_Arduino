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

bool direction = true;

SPIClass SPI_3(MOSI, MISO, SCLK, -1);
SPISettings settings(1000000, MSBFIRST, SPI_MODE1);
DRV8452 drv_azimuth(&SPI_3, settings, CS_AZ, SLP_AZ, EN_AZ);
DRV8452 drv_elevation(&SPI_3, settings, CS_EL, SLP_EL, EN_EL);

void setup(){
  delay(100);
  SPI_3.begin();
  drv_azimuth.setup();
  drv_elevation.setup();
}

void loop(){
  for(float i = 0.0; i < 360; i += 1.8){
      drv_azimuth.fullStep(direction);
      drv_elevation.fullStep(direction);
      delayMicroseconds(20000);
  }

  direction = !direction;
}