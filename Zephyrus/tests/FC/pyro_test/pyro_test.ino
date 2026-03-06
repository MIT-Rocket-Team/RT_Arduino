#include <pyro.h>

HardwareSerial debugSer(PC7, PC6);


pyro myPyro;

void setup() {
  debugSer.begin(115200);
  debugSer.println("Starting Pyro Test");
  myPyro.begin();
  /*
  myPyro.arm(0);
  myPyro.fire(0);
  delay(1000):
  myPyro.off(0);
  */
}

void loop() {
    for (int i = 0; i < 6; i++) {
        debugSer.print("Pyro ");
        debugSer.print(i);
        debugSer.print(" Connected: ");
        debugSer.print(myPyro.connected(i));
        debugSer.print(" Resistance: ");
        debugSer.println(myPyro.resistance(i));
        
    }
    debugSer.println();
    delay(20000);
}
