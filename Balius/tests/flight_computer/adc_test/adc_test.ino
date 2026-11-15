#include <SPI.h>
#include <ADS1220.h>

Uart debugSer(PC7, PC6);

#define MOSI PD7
#define MISO PB4
#define SCLK PA5

#define IMU_CS   PD0
#define BARO_CS  PD1
#define FLASH_CS PD2
#define CC_CS    PD3
#define VIB_CS   PD4
#define ADC_CS   PD5

const uint8_t otherPins[] = {IMU_CS, BARO_CS, FLASH_CS, CC_CS, VIB_CS};

SPIClass SPI_3(MOSI, MISO, SCLK, -1);
// ADS1220 requires SPI mode 1 (CPOL=0, CPHA=1).
SPISettings settings(1000000, MSBFIRST, SPI_MODE1);
ADS1220 adc(&SPI_3, settings, ADC_CS);

uint32_t sampleNumber = 0;

void setup() {
  debugSer.begin(115200);
  delay(100);
  debugSer.println("ADS1220 test starting");

  // Keep every other device on the shared SPI bus deselected.
  for (uint8_t i = 0; i < sizeof(otherPins) / sizeof(otherPins[0]); ++i) {
    pinMode(otherPins[i], OUTPUT);
    digitalWrite(otherPins[i], HIGH);
  }

  adc.setup();
  delay(100);
  debugSer.println("ADC configured: AIN1-AIN2, gain 128, 175 SPS");
  debugSer.println("Printing signed 24-bit raw conversion codes:");
}

void loop() {
  adc.update();

  debugSer.print("Sample ");
  debugSer.print(sampleNumber++);
  debugSer.print(": ");
  debugSer.println(adc.getMeasurement());

  delay(100);
}
