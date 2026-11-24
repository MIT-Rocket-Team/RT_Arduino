Uart debugSer(PC7, PC6);

#include "SPI.h"
#include "CC1200.h"

#define MOSI  PD7
#define MISO  PB4
#define SCLK  PA5
#define CC_CS PA2

static constexpr uint8_t TX_PACKET_LENGTH = 3;
static constexpr uint32_t TX_TIMEOUT_MS = 500;
static uint8_t testPacket[TX_PACKET_LENGTH] = {'A', 'B', 'C'};

SPIClass SPI_3(MOSI, MISO, SCLK, -1);
SPISettings settings(1000000, MSBFIRST, SPI_MODE0);
CC1200 cc(&SPI_3, settings, CC_CS, PB0);

void stopRadio() {
  digitalWrite(CC_CS, LOW);
  SPI_3.transfer(0x36);  // SIDLE
  digitalWrite(CC_CS, HIGH);
}

bool waitForTransmit() {
  const uint32_t startedMs = millis();
  bool sawTransmitState = false;

  while (millis() - startedMs < TX_TIMEOUT_MS) {
    const uint8_t state = cc.status();
    if (state == 2) {  // TX
      sawTransmitState = true;
    } else if (sawTransmitState) {
      return true;
    }
  }

  // Leave the radio in a known state if TX never starts or does not finish.
  stopRadio();
  cc.flushTx();
  return false;
}

void setup() {
  debugSer.begin(115200);
  SPI_3.begin();
  cc.begin();
  delay(100);

  debugSer.println("CC1200 simple TX");
  debugSer.print("Part number: 0x");
  debugSer.println(cc.partnum(), HEX);

  // Match the flight-computer simple_tx modem/frequency configuration.
  cc.simpleConfig();
  cc.sineWave();
}

void loop() {
  //(testPacket, TX_PACKET_LENGTH);
  delayMicroseconds(5);
  float outputPower = cc.outputPowerDBm();
  if (true) {
    debugSer.print("TX: ABC  output power: ");
    debugSer.print(outputPower, 1);
    debugSer.println(" dBm");
  } else {
    debugSer.println("TX timeout; radio recovered");
  }

  delay(100);
}
