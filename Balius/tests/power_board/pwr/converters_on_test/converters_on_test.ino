#define EN0 PB0 //28V
#define EN1 PB1 //5V
#define EN2 PB3 //3V
#define EN3 PB4 //8.4V Digital VTX
#define EN4 PB5 //8.4V Servo

int16_t enables[] = {EN2, EN1, EN3, EN4, EN0}; //3V, 5V, 8.4V Digital VTX, 8.4V Servo, 28V

void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i < 5; i++) {
    pinMode(enables[i], OUTPUT);
    digitalWrite(enables[i], 1);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
