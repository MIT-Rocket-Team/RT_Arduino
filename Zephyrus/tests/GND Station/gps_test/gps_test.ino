HardwareSerial gpsSer(PB12, PB13);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  gpsSer.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (gpsSer.available()) {
    Serial.write(gpsSer.read());
  }
}
