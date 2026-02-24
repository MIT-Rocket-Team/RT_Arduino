HardwareSerial cereal(PA1,PA0);
HardwareSerial debugSer(PA3,PA2);


void setup() {
  // put your setup code here, to run once:
  cereal.begin(115200);
  debugSer.begin(115200);
  cereal.println("BOOT2");
  debugSer.println("BOOT2 (DEBUG)");
}

void loop() {
  // put your main code here, to run repeatedly:
  cereal.println("hello world. lol <2>");
  while(cereal.available()) {
    debugSer.write(cereal.read());
  }
  delay(10);
}
