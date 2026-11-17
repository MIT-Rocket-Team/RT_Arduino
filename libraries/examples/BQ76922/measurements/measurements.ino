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
}

void loop() {
  // put your main code here, to run repeatedly:
  for(byte i = 1; i <= 10; i++) {
    Serial.print("Voltage at pin ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(bms.cellVoltage(i));
    Serial.println(" mV");
  }

  Serial.print("Stack voltage: ");
  Serial.print(bms.stackVoltage());
  Serial.println(" userV (default cV)");

  Serial.print("Current: ");
  Serial.print(bms.current());
  Serial.println(" userA (defualt mA)");

  Serial.print("Temperature: ");
  Serial.print(bms.temp());
  Serial.println(" C");

  Serial.println();
  delay(5000);
}
