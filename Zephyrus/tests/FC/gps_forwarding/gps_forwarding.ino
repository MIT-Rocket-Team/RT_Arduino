HardwareSerial debugSer(PC7, PC6);
HardwareSerial gpsSer(PB12, PB13);

void setup() {
  // put your setup code here, to run once:
  debugSer.begin(115200);
  gpsSer.begin(9600);
  debugSer.println("BOOT");
}

void loop() {
  // put your main code here, to run repeatedly:
  //debugSer.println("HELLO WORLD!");
  //delay(1000);
  while (gpsSer.available()) {
    debugSer.write(gpsSer.read());
  }
}
