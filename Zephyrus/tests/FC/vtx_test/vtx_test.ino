#include <vtx.h>

HardwareSerial VTX_SER(PA12);
HardwareSerial debugSer(PC7, PC6);

vtx myVTX(&VTX_SER);

void setup() {
  debugSer.begin(115200);
  debugSer.println("Starting VTX Test");
  myVTX.begin();
}

void loop() {
    myVTX.getSettings();
    VTX_SER.enableHalfDuplexRx();
    delay(1000);
    while(VTX_SER.available()) {
      debugSer.print(VTX_SER.read(), HEX);
      debugSer.print(" ");
    }
}
