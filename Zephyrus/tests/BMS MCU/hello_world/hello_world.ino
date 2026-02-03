HardwareSerial debugSer(PC7, PC6);

void setup() {
  // put your setup code here, to run once:
  debugSer.begin(115200);
  debugSer.println("BOOT");
}

void loop() {
  // put your main code here, to run repeatedly:
  debugSer.println("HELLO WORLD!");
  delay(1000);
}
