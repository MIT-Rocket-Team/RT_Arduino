Uart debugSer(PC7, PC6);

#include "SPI.h"
#include "CC1200.h"

#define MOSI PD7
#define MISO PB4
#define SCLK PA5

#define IMU_CS   PD0
#define BARO_CS  PD1
#define FLASH_CS PD2
#define CC_CS    PD3
#define VIB_CS   PD4
#define LOAD_CS  PD5

uint8_t otherPins[5] = {IMU_CS, FLASH_CS, BARO_CS, VIB_CS, LOAD_CS};

SPIClass SPI_3(MOSI, MISO, SCLK, -1);
SPISettings settings(1000000, MSBFIRST, SPI_MODE0);

uint8_t pwrPin = PA2;

CC1200 cc(&SPI_3, settings, CC_CS, pwrPin);

void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i < 5; i++) {
    pinMode(otherPins[i], OUTPUT);
    digitalWrite(otherPins[i], 1);
  }
  debugSer.begin(115200);
  SPI_3.begin();
  cc.begin();
  delay(100);
  debugSer.println(cc.partnum());
  debugSer.println(cc.status());
  cc.simpleConfig();
  //cc.sineWave();
}

void loop() {
  cc.Tx((uint8_t*)"ABC", 3);
  while(cc.status() != 2) {}
  delayMicroseconds(5);
  debugSer.println(cc.outputPowerDBm());
  delay(100);
}

