#include "BQ76942.h"
#include "Wire.h"

BQ76922 bms;

void setup() {
  // put your setup code here, to run once:
  Serial.setTx(PA9);
  Serial.setRx(PA10);
  Serial.begin(115200);
  Wire.setSCL(PB6);
  Wire.setSDA(PB7);
  bms.begin();
  Serial.print("Device Number: ");
  Serial.println(bms.devNum());
  Serial.print("Firmware Version: ");
  Serial.println(bms.fwVersion()); 
}

void loop() {
  // put your main code here, to run repeatedly:
    
}
