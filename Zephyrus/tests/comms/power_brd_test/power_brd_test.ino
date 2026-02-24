HardwareSerial cereal(PB11,PB10);
HardwareSerial debugSer(PC7,PC6);


void setup() {
  // put your setup code here, to run once:
  cereal.begin(115200);
  debugSer.begin(115200);
  cereal.println("BOOT");
  debugSer.println("BOOT (DEBUG)");
}

void loop() {
  // put your main code here, to run repeatedly:
  cereal.println("hello world. lol <1>");
  while(cereal.available()) {
    debugSer.write(cereal.read());
  }
  delay(10);
}
