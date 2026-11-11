#include <SoftwareSerial.h>
#include <Wire.h>
#include <INA232.h>
#include <myTypes.h>

#define EN0 PB0 //28V
#define EN1 PB1 //5V
#define EN2 PB3 //3V
#define EN3 PB4 //8.4V Digital VTX
#define EN4 PB5 //8.4V Servo

SoftwareSerial bmsSer(PB8, PB7);
Uart debugSer(PA3, PA2);
Uart FCSer(PA1, PA0);

//Wire1, Wire2 swapped from schematic
TwoWire Wire1(PA6,PA7);
TwoWire Wire2(PC14,PB6);


INA232 monitor3v3(&Wire2, 0b1000001);
INA232 monitor3v(&Wire2, 0b1000000);
INA232 monitor5v(&Wire2, 0b1000010);
INA232 monitor8v4d(&Wire1, 0b1000001);
INA232 monitor28v(&Wire1, 0b1000000);
INA232 monitor8v4s(&Wire1, 0b1000010);

INA232* monitors[] = {&monitor3v, &monitor3v3, &monitor5v, &monitor8v4d, &monitor8v4s, &monitor28v};

pwrBoardData powerPkt;

pwrCommands command;
uint8_t tmpCommand[sizeof(command)];

uint8_t tmp[sizeof(bmsData)];

int16_t enables[] = {EN2, EN1, EN3, EN4, EN0}; //3V, 5V, 8.4V Digital VTX, 8.4V Servo, 28V

void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i < 5; i++) {
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
    monitors[i]->begin();
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  while (bmsSer.available() >= sizeof(bmsData) + 2) {
    if (bmsSer.read() == 0xAA) {
      bmsSer.readBytes(tmp, sizeof(bmsData));
      if (bmsSer.read() == calcChecksum(tmp, sizeof(bmsData))) {
        memcpy(&powerPkt.BMS, tmp, sizeof(bmsData));
      } 
    }
  }

  for (uint8_t i = 0; i < 6; i++) {
    powerPkt.currents[i] = monitors[i]->currentRaw();
    powerPkt.voltages[i] = monitors[i]->voltageRaw();
  }

  FCSer.write(0xAA);
  FCSer.write((uint8_t*) &powerPkt, sizeof(powerPkt));
  FCSer.write(calcChecksum((uint8_t*) &powerPkt, sizeof(powerPkt)));
  

  while (FCSer.available() >= sizeof(command) + 2) {
    if (FCSer.read() == 0xAA) {
      FCSer.readBytes(tmpCommand, sizeof(command));
      if (FCSer.read() == calcChecksum(tmpCommand, sizeof(command))) {
        memcpy(&command, tmpCommand, sizeof(command));    
      } 
    }
  }

  //debugSer.println(powerPkt.BMS.cell1);
  //debugSer.println(powerPkt.BMS.protectionsEnabled, BIN);

  for (uint8_t i = 0; i < 5; i++) {
    if (command.convertersEnabled[i]) {
      digitalWrite(enables[i], 1);
    } else {
      digitalWrite(enables[i], 0);
    }
  }

  bmsSer.write(0xAA);
  bmsSer.write((uint8_t*) &command.BMS, sizeof(command.BMS));
  bmsSer.write(calcChecksum((uint8_t*) &command.BMS, sizeof(command.BMS)));

  delay(100);
}

uint8_t calcChecksum(uint8_t* p, uint8_t len) {
  uint8_t ret = 0;
  for (uint8_t i = 0; i < len; i++) {
    ret += *(p + i);
  }
  return ret;
}