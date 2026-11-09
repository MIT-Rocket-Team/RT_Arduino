#include "BQ76922.h"
#include "Wire.h"

HardwareSerial pwrSer(PA1, PA0);
HardwareSerial debugSer(PA3, PA2);


BQ76922 bms;

void setup() {
  // put your setup code here, to run once:
  debugSer.begin(115200);
  debugSer.println("BOOT");
  Wire.setSCL(PA7);
  Wire.setSDA(PA6);
  bms.begin();
  delay(100);
  int16_t dummy = bms.cellVoltage(0); //Chip locks up without a read first
  bms.enterConfigMode();
  bms.disableProtections();
  bms.enableFet();
  bms.chargePumpEnable();
  bms.exitConfigMode();
}

void loop() {
  // put your main code here, to run repeatedly:
  pwrSer.println("TEST BMS -> PWR");
  while (BMS_Serial.available()) {
    debugSer.write(pwrSer.read());
  }
  delay(1000);
}
