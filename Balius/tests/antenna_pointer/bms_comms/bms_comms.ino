#include <Wire.h>

Uart antennaSer(PA1, PC14);

void setup() {
  antennaSer.begin(115200);
  delay(100);
  antennaSer.println("BMS I2C scan");

  Wire.setSCL(PB6);
  Wire.setSDA(PB7);
  Wire.begin();

  byte found = 0;
  for (byte address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    if (error == 0) {
      antennaSer.print("Found I2C device at 0x");
      if (address < 0x10) antennaSer.print('0');
      antennaSer.println(address, HEX);
      ++found;
    }
  }

  if (found == 0) {
    antennaSer.println("No I2C devices found");
  }
  antennaSer.println("Scan complete");
}

void loop() {
  delay(1000);
}
