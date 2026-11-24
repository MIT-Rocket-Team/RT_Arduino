// Match the UART, SPI bus, and chip-select pins used by the flight-computer
// CC1200 simple_tx test.
Uart debugSer(PC7, PC6);

#include "SPI.h"
#include "CC1200.h"
#include <IWatchdog.h>
#include <stdio.h>

#define MOSI  PD7
#define MISO  PB4
#define SCLK  PA5
#define CC_CS PA2

static constexpr uint8_t RX_PACKET_LENGTH = 3;

SPIClass SPI_3(MOSI, MISO, SCLK, -1);
SPISettings settings(1000000, MSBFIRST, SPI_MODE0);
CC1200 cc(&SPI_3, settings, CC_CS, PB0);

static uint8_t packet[RX_PACKET_LENGTH];
static uint32_t lastHeartbeatMs = 0;
static char logBuffer[80];

// Use a bounded HAL write for diagnostics. Arduino's buffered Uart::write()
// can wait forever for TX-space if its completion interrupt is lost.
void debugWrite(const char *text, size_t length) {
  (void)HAL_UART_Transmit(debugSer.getHandle(),
                          reinterpret_cast<uint8_t *>(const_cast<char *>(text)),
                          length, 25);
}

void debugWrite(const char *text) {
  debugWrite(text, strlen(text));
}

// Write one standard CC1200 configuration register over the already-active
// SPI transaction used by the CC1200 library.
void writeRadioRegister(uint8_t address, uint8_t value) {
  digitalWrite(CC_CS, LOW);
  SPI_3.transfer(address);
  SPI_3.transfer(value);
  digitalWrite(CC_CS, HIGH);
}

void setup() {
  const bool watchdogReset = IWatchdog.isReset(true);
  IWatchdog.begin(3000000);  // reboot if the main loop stops for 3 seconds

  debugSer.begin(115200);
  SPI_3.begin();
  cc.begin();
  delay(100);

  debugWrite("CC1200 simple RX\r\n");
  if (watchdogReset) {
    debugWrite("Recovered from a main-loop stall (watchdog reset)\r\n");
  }
  const int partNumberLength = snprintf(logBuffer, sizeof(logBuffer),
                                        "Part number: 0x%02X\r\n", cc.partnum());
  if (partNumberLength > 0) debugWrite(logBuffer, (size_t)partNumberLength);

  // Use the same modem and frequency setup as simple_tx, then listen for its
  // three-byte "ABC" payloads.
  cc.simpleConfig();
  // Weak/out-of-range frames often fail CRC. Discard those frames in hardware
  // and make bad packets return to IDLE so the loop can restart RX cleanly.
  writeRadioRegister(0x001D, 0x80);  // FIFO_CFG.CRC_AUTOFLUSH
  writeRadioRegister(0x002A, 0x08);  // RFEND_CFG0.TERM_ON_BAD_PACKET_EN
  cc.Rx(RX_PACKET_LENGTH);
  debugWrite("Listening for 3-byte packets...\r\n");
}

void loop() {
  const uint8_t radioState = cc.status();
  const uint8_t fifoStatus = cc.avail();
  const uint8_t available = fifoStatus & 0x7F;  // bit 7 indicates FIFO overflow

  // Follow Zephyrus's recovery flow and also catch a FIFO overflow that may
  // have changed state between the status and FIFO reads.
  if (radioState == 6 || (fifoStatus & 0x80)) {
    debugWrite("RX FIFO error; restarting radio\r\n");
    cc.flushRx();
    cc.Rx(RX_PACKET_LENGTH);
  } else if (radioState == 0 && available == RX_PACKET_LENGTH) {
    // In simpleConfig the radio returns to IDLE after a good packet. Read only
    // in IDLE so we never interrupt a packet that is still arriving.
    cc.read(packet, RX_PACKET_LENGTH);
    const float rssiDbm = cc.rssiDBm();
    char payload[RX_PACKET_LENGTH + 1];
    for (uint8_t i = 0; i < RX_PACKET_LENGTH; ++i) {
      if (packet[i] >= 32 && packet[i] <= 126) {
        payload[i] = (char)packet[i];
      } else {
        payload[i] = '.';
      }
    }
    payload[RX_PACKET_LENGTH] = '\0';

    const int rssiTenths = (int)(rssiDbm * 10.0f + (rssiDbm < 0 ? -0.5f : 0.5f));
    const int rssiWhole = rssiTenths / 10;
    const int rssiFraction = abs(rssiTenths % 10);
    const int lineLength = snprintf(logBuffer, sizeof(logBuffer),
                                    "RX: %s  RSSI: %d.%d dBm\r\n",
                                    payload, rssiWhole, rssiFraction);
    if (lineLength > 0) debugWrite(logBuffer, (size_t)lineLength);

    cc.Rx(RX_PACKET_LENGTH);
  } else if (radioState == 0) {
    // Discard any incomplete/stale bytes left by a rejected packet before
    // re-entering RX. This keeps the fixed-length FIFO aligned to a packet.
    if (available != 0) {
      cc.flushRx();
    }
    cc.Rx(RX_PACKET_LENGTH);
  }

  const uint32_t now = millis();
  if (now - lastHeartbeatMs >= 1000) {
    lastHeartbeatMs = now;
    const int lineLength = snprintf(logBuffer, sizeof(logBuffer),
                                    "RX alive; state=%u FIFO=%u\r\n",
                                    radioState, available);
    if (lineLength > 0) debugWrite(logBuffer, (size_t)lineLength);
  }

  // Refresh only after the loop finishes. If an SPI or serial operation stalls,
  // the watchdog resets the MCU and setup() reinitializes the CC1200.
  IWatchdog.reload();
}
