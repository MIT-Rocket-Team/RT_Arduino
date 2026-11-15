Uart debugSer(PC7, PC6);

#include "SPI.h"
#include "baro.h"

#define MOSI PD7
#define MISO PB4
#define SCLK PA5

#define IMU_CS   PD0
#define BARO_CS  PD1
#define FLASH_CS PD2
#define CC_CS    PD3
#define VIB_CS   PD4
#define LOAD_CS  PD5

uint8_t otherPins[5] = {IMU_CS, FLASH_CS, CC_CS, VIB_CS, LOAD_CS};

SPIClass SPI_3(MOSI, MISO, SCLK, -1);
SPISettings settings(1000000, MSBFIRST, SPI_MODE0);

baro myBaro(&SPI_3,settings, BARO_CS);

int counter = 0;

void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i < 5; i++) {
    pinMode(otherPins[i], OUTPUT);
    digitalWrite(otherPins[i], 1);
  }
  SPI_3.begin();
  myBaro.begin();
  delay(100);
  debugSer.begin(115200);
  debugSer.println("Calibration Values:");
  for (int i = 1; i <= 6; i++) {
    debugSer.print("C");
    debugSer.print(i);
    debugSer.print(": ");
    debugSer.println(myBaro.getCalibrationConstant(i), HEX);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  debugSer.println("\n--------------\n");
  debugSer.print("Data Packet #");
  debugSer.println(counter);
  counter += 1;

  myBaro.updateRawTemp();
  myBaro.updateRawPress();


  debugSer.print("Raw Temperature: ");
  debugSer.println(myBaro.getRawTemp());
  debugSer.print("Raw Pressure: ");
  debugSer.println(myBaro.getRawPress());
  debugSer.print("Temperature (C): ");
  debugSer.println(myBaro.getTemperature());
  debugSer.print("Altitude (m): ");
  debugSer.println(myBaro.getAltitude());
  debugSer.println("Calibration Values:");
  for (int i = 1; i <= 6; i++) {
    debugSer.print("C");
    debugSer.print(i);
    debugSer.print(": ");
    debugSer.println(myBaro.getCalibrationConstant(i), HEX);
  }
  delay(2000);
}