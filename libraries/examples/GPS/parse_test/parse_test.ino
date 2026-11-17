#include <GPS.h>
//COMMENT THIS TO USE HARDWARE SERIAL
#define USE_USB_SER

#ifdef USE_USB_SER
#define GPS_SER Serial
#else
Uart GPS_SER(PA1, PA0);
#endif

GPS gps(&GPS_SER, 0);

unsigned long last;

void setup() {
  // put your setup code here, to run once:
  gps.setup();
}

void loop() {
  // put your main code here, to run repeatedly:
  gps.updateAndParse();
  if(millis() - last > 1000) {
    last = millis();
    GPS_SER.print("fix: ");
    GPS_SER.println(gps.getFix());
    GPS_SER.print("lat: ");
    GPS_SER.println(gps.getLatitude());
    GPS_SER.print("long: ");
    GPS_SER.println(gps.getLongitude());
    GPS_SER.print("Alt: ");
    GPS_SER.println(gps.getAltitude());
    GPS_SER.print("VDOP: ");
    GPS_SER.println(gps.getVDOP());
    GPS_SER.print("PDOP: ");
    GPS_SER.println(gps.getPDOP());
    GPS_SER.print("HDOP: ");
    GPS_SER.println(gps.getHDOP());
  }
}
