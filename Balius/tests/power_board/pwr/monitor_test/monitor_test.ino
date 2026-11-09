#include <INA232.h>
#include <Wire.h>

#define EN0 PB0 //28V
#define EN1 PB1 //5V
#define EN2 PB3 //3V
#define EN3 PB4 //8.4V Digital VTX
#define EN4 PB5 //8.4V Servo

int16_t enables[] = {EN2, EN1, EN3, EN4, EN0}; //3V, 5V, 8.4V Digital VTX, 8.4V Servo, 28V


TwoWire Wire1(PA6,PA7);
TwoWire Wire2(PC14,PB6);


INA232 monitor3v3(&Wire2, 0b1000001);
INA232 monitor3v(&Wire2, 0b1000000);
INA232 monitor5v(&Wire2, 0b1000010);
INA232 monitor8v4d(&Wire1, 0b1000001);
INA232 monitor28v(&Wire1, 0b1000000);
INA232 monitor8v4s(&Wire1, 0b1000010);

INA232* monitors[] = {&monitor3v, &monitor3v3, &monitor5v, &monitor8v4d, &monitor8v4s, &monitor28v};

void setup() {
  // put your setup code here, to run once:
  Serial.setTx(PA2);
  Serial.setRx(PA3);
  Serial.begin(115200);
  Serial.println("BOOT");


  // put your setup code here, to run once:
  for (int i = 0; i < 5; i++) {
    pinMode(enables[i], OUTPUT);
    digitalWrite(enables[i], 1);
  }
  /*Wire.setSDA(PA6);
  Wire.setSCL(PA7);
  Wire.begin();*/

  Wire1.begin();
  Wire2.begin();

  for (uint8_t i = 0; i < 6; i++) {
    monitors[i]->begin();
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  for (uint8_t i = 0; i < 6; i++) {
    Serial.print("Monitor ");
    Serial.print(i);
    Serial.println(":");
    Serial.print("Current: ");
    Serial.println(monitors[i]->current());
    Serial.print("Voltage: ");
    Serial.println(monitors[i]->voltage());
    Serial.println();
  }
  
  delay(1000);
}
