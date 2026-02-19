HardwareSerial debugSer(PC7, PC6);

#include "SPI.h"
#include "ADXL357.h"
#include "gyro.h"

#define MOSI PD7
#define MISO PB4
#define SCLK PA5
#define CS PD1

SPIClass SPI_3(MOSI, MISO, SCLK, -1);
SPISettings settings(1000000, MSBFIRST, SPI_MODE0);

gyro g(&SPI_3, settings, CS);

void setup() {
  debugSer.begin(115200);
  g.begin();
  g.config();
}

void loop() {
  debugSer.println(g.gyro_x());
  debugSer.println(g.gyro_y());
  debugSer.println(g.gyro_z());
  debugSer.println();
  delay(1000);
}
