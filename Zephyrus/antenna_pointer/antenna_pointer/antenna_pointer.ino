#include "SPI.h"
#include "DRV8452.h"

#define MOSI PA7
#define MISO PA6
#define SCLK PA5
#define CS_AZ PB0
#define SLP_AZ PB1
#define EN_AZ PB2
#define CS_EL PB6
#define SLP_EL PB7
#define EN_EL PB8

#define POINT_ERR 1

SPIClass SPI_3(MOSI, MISO, SCLK, -1);
SPISettings settings(1000000, MSBFIRST, SPI_MODE1);

DRV8452 drv_azimuth(&SPI_3, settings, CS_AZ, SLP_AZ, EN_AZ);
DRV8452 drv_elevation(&SPI_3, settings, CS_EL, SLP_EL, EN_EL);

float elevation_target;
float azimuth_target;
float old_a_target;
float old_e_target;
int elevation_target_steps;
int azimuth_target_steps;
int elevation_steps;
int azimuth_steps;
float error_tolerance = 0.05;
long last_step_elevation;
long last_step_azimuth;
unsigned long elInterval = 3600;

void setup() {
  Serial.begin(115200);
  SPI_3.begin();
  drv_azimuth.setup();
  drv_elevation.setup();
  drv_azimuth.setHoldCurrentLimit(2.5);
  drv_azimuth.setStepCurrentLimit(3.5);
  drv_elevation.setHoldCurrentLimit(2.5);
  drv_elevation.setStepCurrentLimit(3.5);
  last_step_elevation = micros();
  last_step_azimuth = micros();
}

void loop() {
  if (Serial.available() >= 11) {
    if(Serial.peek() == 0xAA){
      uint8_t data[11];
      Serial.readBytes((char*)data, 11);
      uint8_t checksum = 0;
      for(int i = 1; i < 10; i++){
        checksum += data[i];
      }
      if(checksum == data[10]){
        switch (data[1]){
          case 0x00:
            old_a_target = azimuth_target;
            old_e_target = elevation_target;
            memcpy(&azimuth_target, &data[2], 4); //0 - 360 deg 
            memcpy(&elevation_target, &data[6], 4); // -90 to 90 deg
            azimuth_target = abs(azimuth_target - old_a_target) > POINT_ERR ? azimuth_target : old_a_target;
            elevation_target = abs(elevation_target - old_e_target) > POINT_ERR ? elevation_target : old_e_target;
            break;
          case 0x01:
            elevation_target += 5;
            break;
          case 0x02:
            elevation_target -= 5;
            break;
          case 0x03:
            azimuth_target += 5;
            break;
          case 0x04:
            azimuth_target -= 5;
            break;
          case 0x05:
            elevation_steps = 0;
            azimuth_steps = 0;
            elevation_target = 0.0;
            azimuth_target = 0.0;
            break;
        }
      }


      Serial.println(elevation_target);      
      azimuth_target_steps = round(azimuth_target * (1/1.8) * 50.0);
      elevation_target_steps = round(elevation_target * (1/1.8) * 50.0);
      shortest_path();

      el_interval = 10000;
      
    } else {
      Serial.read();
    }
  }

  if(micros() - last_step_azimuth >= 3600){
    if(azimuth_steps < azimuth_target_steps){
      drv_azimuth.fullStep(true);
      azimuth_steps ++;
      last_step_azimuth = micros();
    } else if(azimuth_steps > azimuth_target_steps){
      drv_azimuth.fullStep(false);
      azimuth_steps --;
      last_step_azimuth = micros();
    }
  }
  if(micros() - last_step_elevation >= 3600){
    if(elevation_steps < elevation_target_steps){
      drv_elevation.fullStep(true);
      elevation_steps ++;
      last_step_elevation = micros();
      computeStepInterval(el_interval);
    } else if(elevation_steps > elevation_target_steps){
      drv_elevation.fullStep(false);
      elevation_steps --;
      last_step_elevation = micros();
      computeStepInterval(el_interval);
    }
  }
  
}

void shortest_path(){
  while(abs(azimuth_target_steps - azimuth_steps) > 5000){
    if(azimuth_target_steps - azimuth_steps > 0){
      azimuth_target_steps -= 10000;
    } else {
      azimuth_target_steps += 10000;
    }
  }
}

unsigned long computeStepInterval(unsigned long currentInterval) {
  if (currentInterval > 3600) {
    return currentInterval - 100;
  } else {
    return currentInterval
  }
}