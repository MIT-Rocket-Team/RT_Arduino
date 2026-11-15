#include "SPI.h"
#include "flash.h"

#define MOSI PD7
#define MISO PB4
#define SCLK PA5

#define IMU_CS   PD0
#define BARO_CS  PD1
#define FLASH_CS PD2
#define CC_CS    PD3
#define VIB_CS   PD4
#define LOAD_CS  PD5

uint8_t otherPins[5] = {IMU_CS, BARO_CS, CC_CS, VIB_CS, LOAD_CS};

Uart debugSer(PC7, PC6);

uint8_t testWritePage[512];
uint8_t testReadPage[512];

SPIClass SPI_3(MOSI, MISO, SCLK, -1);
SPISettings settings(1000000, MSBFIRST, SPI_MODE0);

flash myFlash(&SPI_3,settings, FLASH_CS);
int counter = 0;
void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i < 5; i++) {
    pinMode(otherPins[i], OUTPUT);
    digitalWrite(otherPins[i], 1);
  }
  SPI_3.begin();
  debugSer.begin(115200);
  delay(1000);
  myFlash.begin();
  debugSer.println("Erasing flash sector 0");
  myFlash.sectorErase(0);
    while(myFlash.isBusy()) {
        debugSer.print(".");
        delay(100);
    }
  debugSer.println("Programming flash");
  for (int i = 0; i < 512; i++) {
    testWritePage[i] = i;
  }
  myFlash.programPage((uint8_t*) testWritePage, 0);
  while(myFlash.isBusy()) {}
  debugSer.println("Reading flash");
  myFlash.readPage((uint8_t*) testReadPage, 0);
  debugSer.println("Data read from flash:");
  for (int i = 0; i < 512; i++) {
    debugSer.print(testReadPage[i], HEX);
    debugSer.print(" ");
  }
  debugSer.println();
}

void loop() {

}