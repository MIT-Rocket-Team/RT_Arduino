#include <Wire.h>
#include "BQ76952.h"

Uart antennaSer(PA1, PC14);
BQ76952 bms(&Wire, BQ76952_DEFAULT_ADDR);

void setup() {
  antennaSer.begin(115200);
  delay(100);
  antennaSer.println("BQ7695202 transistor-on test");

  Wire.setSCL(PB6);
  Wire.setSDA(PB7);
  bms.begin();

  antennaSer.println("Entering config mode");
  bms.enterConfigMode();

  antennaSer.println("Disabling protections");
  bms.disableProtections();

  antennaSer.println("Configuring DFETOFF");
  bms.dfetoffConfig();

  antennaSer.println("Enabling FETs and charge pump");
  bms.enableFet();
  bms.chargePumpEnable();
  bms.exitConfigMode();

  antennaSer.println("Configuration complete");
}

void loop() {
  antennaSer.print("FET status: 0x");
  antennaSer.println(bms.fetStatus(), HEX);
  delay(1000);
}
