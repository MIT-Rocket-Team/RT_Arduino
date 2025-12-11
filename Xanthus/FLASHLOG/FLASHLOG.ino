#include <SoftwareSerial.h>
#include <SPI.h>

#define MOSI PB5
#define MISO PB4
#define SCLK PB3
#define FLASH_CS PB12

uint32_t pkts;

uint8_t pkt[128];

SoftwareSerial mySerial(PB9, PB8);


SPIClass SPI_3(MOSI, MISO, SCLK, -1);
SPISettings settings(1000000, MSBFIRST, SPI_MODE0);

void setup() {
  // put your setup code here, to run once:
  SPI_3.begin();
  SPI_3.beginTransaction(settings);
  pinMode(PB12, OUTPUT);
  digitalWrite(PB12, 1);
  pinMode(PB13, OUTPUT);
  digitalWrite(PB13, 1);
  pinMode(PB15, OUTPUT);
  digitalWrite(PB15, 1);
  pinMode(PB14, OUTPUT);
  digitalWrite(PB14, 1);
  mySerial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  while(!mySerial.available()) {};
  pkts = mySerial.parseInt();
  while (mySerial.available()) {mySerial.read();}
  for(int i = 0; i < pkts; i++) {
    while (isFlashBusy()) {}
      read(pkt, i * 128);
      mySerial.write(0xAB);
      mySerial.write(0xAB);
      mySerial.write(pkt, 128);
  }
}

bool isFlashBusy() {
  digitalWrite(FLASH_CS, LOW);
  SPI_3.transfer(0x05);
  uint8_t status = SPI_3.transfer(0x00);
  digitalWrite(FLASH_CS, HIGH);
  return (status & 0x01);
}

void read (uint8_t* buf, int adr) {
  digitalWrite(FLASH_CS, 0);
  SPI_3.transfer(0x03);
  SPI_3.transfer(adr >> 16);
  SPI_3.transfer(adr >> 8);
  SPI_3.transfer(adr);
  SPI_3.transfer(buf, 128);
  digitalWrite(FLASH_CS, 1);
}