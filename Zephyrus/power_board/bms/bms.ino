#include "BQ76922.h"
#include "Wire.h"
#include "myTypes.h"

HardwareSerial pwrSer(PA1, PA0);

BQ76922 bms;

bmsData pkt;
bmsCommands newCommand;
bmsCommands currentCommand;

uint8_t tmp[sizeof(newCommand)];

void setup() {
  // put your setup code here, to run once:
  pwrSer.begin(9600);
  Serial.setTx(PA2);
  Serial.setRx(PA3);
  Serial.begin(115200);
  Serial.println("BOOT");
  Wire.setSCL(PA7);
  Wire.setSDA(PA6);
  bms.begin();
  delay(100);
  Serial.println("BMS BEGIN");
  int16_t dummy = bms.cellVoltage(0); //Chip locks up without a read first
  bms.enterConfigMode();
  Serial.println("ENTER CONFIG");
  bms.cellConfig(3);
  bms.minCellVoltage(3.3);
  bms.dfetoffConfig(0x06);
  bms.cellSC20mV();
  bms.cellUVandSC();
  bms.enableFet();
  bms.chargePumpEnable();
  bms.exitConfigMode();
}


void loop() {
  // put your main code here, to run repeatedly:
  pkt.cell1 = bms.cellVoltage(1);
  pkt.cell2 = bms.cellVoltage(2);
  pkt.cell3 = bms.cellVoltage(5);
  pkt.stackVoltage = bms.stackVoltage();
  pkt.current = bms.current();
  pkt.temp = bms.temp();
  pkt.fetStatus = bms.fetStatus();
  pkt.protectionStatus = bms.safetyStatusA();
  pkt.protectionsEnabled = bms.enabledProtectionsA();

  pwrSer.write(0xAA);
  pwrSer.write((uint8_t*) &pkt, sizeof(pkt));
  pwrSer.write(calcChecksum((uint8_t*) &pkt, sizeof(pkt)));
  delay(100);

  if (currentCommand.protectionsEnabled != newCommand.protectionsEnabled) {
    if (newCommand.protectionsEnabled) {
      bms.cellUVandSC();
    } else {
      bms.disableProtections();
    }
    currentCommand.protectionsEnabled = newCommand.protectionsEnabled;
  }

  if (currentCommand.screwSwitchEnabled != newCommand.screwSwitchEnabled) {
    if (newCommand.screwSwitchEnabled) {
      bms.dfetoffConfig(0x06);
    } else {
      bms.dfetoffConfig(0x00);
    }
    currentCommand.screwSwitchEnabled = newCommand.screwSwitchEnabled;
  }

  while (pwrSer.available() >= sizeof(newCommand) + 2) {
    if (pwrSer.read() == 0xAA) {
      pwrSer.readBytes(tmp, sizeof(newCommand));
      if (pwrSer.read() == calcChecksum(tmp, sizeof(newCommand))) {
        memcpy(&newCommand, tmp, sizeof(newCommand));    
      } 
    }
  }

}

uint8_t calcChecksum(uint8_t* p, uint8_t len) {
  uint8_t ret = 0;
  for (uint8_t i = 0; i < len; i++) {
    ret += *(p + i);
  }
  return ret;
}