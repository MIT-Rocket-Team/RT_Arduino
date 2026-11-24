Uart antennaSer(PA1, PC14);

void setup() {
  Serial.begin(115200);
  antennaSer.begin(115200);
  delay(100);
  Serial.println("Hello, world!");
  antennaSer.println("Hello, world!");
}

void loop() {
  while (antennaSer.available()) {
    antennaSer.write(antennaSer.read());
  }

  static unsigned long lastHello = 0;
  if (millis() - lastHello >= 1000) {
    lastHello = millis();
    Serial.println("Hello, world!");
    antennaSer.println("Hello, world!");
  }
}
