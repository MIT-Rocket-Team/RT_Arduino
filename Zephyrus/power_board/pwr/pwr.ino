#include <SoftwareSerial.h>
#include <Wire.h>
#include <INA232.h>
#include <myTypes.h>

#define EN0 PB0
#define EN1 PB1
#define EN2 PB3
#define EN3 PB4
#define EN4 PB5

SoftwareSerial bmsSer(PB8, PB7);
HardwareSerial debugSer(PA3, PA2);
HardwareSerial FCSer(PA1, PA0);

TwoWire Wire1(PA6,PA7);
TwoWire Wire2(PC14,PB6);


INA232 monitor3v3(&Wire2, 0b1000001);
INA232 monitor3v(&Wire2, 0b1000000);
INA232 monitor5v(&Wire2, 0b1000010);
INA232 monitor7v4(&Wire1, 0b1000001);
INA232 monitor28v(&Wire1, 0b1000000);
INA232 monitor8v4(&Wire1, 0b1000010);

INA232 monitors[] = {monitor3v, monitor3v3, monitor5v, monitor7v4, monitor8v4, monitor28v};

pwrBoardData powerPkt;

uint8_t tmp[15];

int16_t enables[] = {EN0, EN1, EN2};

void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i < 3; i++) {
    pinMode(enables[i], OUTPUT);
    digitalWrite(enables[i], 1);
  }
  bmsSer.begin(9600);
  FCSer.begin(115200);
  debugSer.begin(115200);
  debugSer.println("BOOT");

  Wire1.begin();
  Wire2.begin();

  for (uint8_t i = 0; i < 6; i++) {
    monitors[i].begin();
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  while (bmsSer.available() >= 17) {
    if (bmsSer.read() == 0xAA) {
      bmsSer.readBytes(tmp, 15);
      if (bmsSer.read() == calcChecksum(tmp, 15)) {
        memcpy(&powerPkt.BMS, tmp, 15);    
      } 
    }
  }

  for (uint8_t i = 0; i < 6; i++) {
    powerPkt.currents[i] = monitors[i].currentRaw();
    powerPkt.voltages[i] = monitors[i].voltageRaw();
  }

  FCSer.write(0xAA);
  FCSer.write((uint8_t*) &powerPkt, sizeof(powerPkt));
  FCSer.write(calcChecksum((uint8_t*) &powerPkt, sizeof(powerPkt)));
  delay(100);
}

uint8_t calcChecksum(uint8_t* p, uint8_t len) {
  uint8_t ret = 0;
  for (uint8_t i = 0; i < 15; i++) {
    ret += *(p + i);
  }
  return ret;
}