#include "BQ76942.h"
#include "Wire.h"

BQ76922 bms(&Wire, 0x08);

void setup() {
  // put your setup code here, to run once:
  Serial.setTx(PA9);
  Serial.setRx(PA10);
  Serial.begin(115200);
  Wire.setSCL(PB6);
  Wire.setSDA(PB7);
  bms.begin();
  delay(2000);
  Serial.print("OTP CHECKS");
  bms.fullAccess();
  delay(100);
  if(!bms.fullAccessCheck()) {
    Serial.println("FULL ACCESS FAILED");
  } else {
    Serial.println("FULL ACCESS SUCCESS");
  }
  bms.enterConfigMode();
  if(!bms.OTPcheck()) {
    bms.OTPdebug();
  } else {
    Serial.println("CONDITIONS GOOD FOR OTP");
  }

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1);
}
