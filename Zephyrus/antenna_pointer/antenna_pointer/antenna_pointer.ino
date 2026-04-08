#include "SPI.h"
#include "DRV8452.h"

// SPI pin assignments for the STM32 board connected to both DRV8452 drivers.
#define MOSI PA7
#define MISO PA6
#define SCLK PA5

// Azimuth driver control pins.
#define CS_AZ PB0
#define SLP_AZ PB1
#define EN_AZ PB2

// Elevation driver control pins.
#define CS_EL PB6
#define SLP_EL PB7
#define EN_EL PB8

// Ignore tiny command changes smaller than this many degrees.
#define POINT_ERR 1

// Shared SPI bus used by both motor drivers.
SPIClass SPI_3(MOSI, MISO, SCLK, -1);
SPISettings settings(1000000, MSBFIRST, SPI_MODE1);

// Two independent driver instances, one per axis.
DRV8452 drv_azimuth(&SPI_3, settings, CS_AZ, SLP_AZ, EN_AZ);
DRV8452 drv_elevation(&SPI_3, settings, CS_EL, SLP_EL, EN_EL);

// Previous commanded targets, used to detect when a new move is requested.
float p_azimuth_target;
float p_elevation_target;

// Target angles received over serial, in degrees.
float elevation_target;
float azimuth_target;

// Cached copies of the old targets so small command noise can be rejected.
float old_a_target;
float old_e_target;

// Desired positions in motor steps.
int elevation_target_steps;
int azimuth_target_steps;

// Current positions in motor steps.
int elevation_steps;
int azimuth_steps;

// Currently unused.
float error_tolerance = 0.05;

// Timestamp of the most recent step pulse on each axis.
long last_step_elevation;
long last_step_azimuth;

// Delay between successive steps, in microseconds.
// Smaller interval means faster stepping.
unsigned long el_interval = 3600;
unsigned long az_interval = 3600;

// Number of steps taken since the current acceleration ramp started.
unsigned long newStepsAz = 0;
unsigned long newStepsEl = 0;

// Absolute elapsed time along the acceleration curve at the previous step.
unsigned long pastTAz = 0;
unsigned long pastTEl = 0;

// Persistent copies of the current time point on each acceleration ramp.
unsigned long newTAz = 0;
unsigned long newTEl = 0;

// Track the direction of the current move so a reversal can restart the ramp.
int last_az_direction = 0;
int last_el_direction = 0;

void setup() {
  // Serial link to the host computer / radio controller.
  Serial.begin(115200);

  // Initialize the shared SPI bus before touching the motor drivers.
  SPI_3.begin();

  // Bring both drivers out of reset / sleep and configure their SPI state.
  drv_azimuth.setup();
  drv_elevation.setup();

  // Set motor currents for hold and stepping.
  // Hold current is slightly lower to reduce heating at rest.
  drv_azimuth.setHoldCurrentLimit(2.5);
  drv_azimuth.setStepCurrentLimit(2.75);
  drv_elevation.setHoldCurrentLimit(2.5);
  drv_elevation.setStepCurrentLimit(2.75);

  // Initialize step timestamps so the first loop iteration has a valid baseline.
  last_step_elevation = micros();
  last_step_azimuth = micros();
}

void loop() {
  // Every command packet is 11 bytes long:
  //   byte 0  : sync header 0xAA
  //   byte 1  : command ID
  //   bytes 2-5 : azimuth float
  //   bytes 6-9 : elevation float
  //   byte 10 : checksum over bytes 1-9
  if (Serial.available() >= 11) {
    if(Serial.peek() == 0xAA){
      uint8_t data[11];

      // Pull one whole packet from the serial buffer.
      Serial.readBytes((char*)data, 11);

      // Verify the simple additive checksum before acting on the packet.
      uint8_t checksum = 0;
      for(int i = 1; i < 10; i++){
        checksum += data[i];
      }
      if(checksum == data[10]){
        // Save the last commanded targets so target changes can be detected.
        p_azimuth_target = azimuth_target;
        p_elevation_target = elevation_target;
        switch (data[1]){
          case 0x00:
            // Absolute pointing command: decode two floats from the payload.
            old_a_target = azimuth_target;
            old_e_target = elevation_target;
            memcpy(&azimuth_target, &data[2], 4); //0 - 360 deg

            // Coordinate convention for azimuth is inverted relative to input.
            azimuth_target *= -1.0; 
            memcpy(&elevation_target, &data[6], 4); // -90 to 90 deg

            // Reject tiny changes so noise or float jitter does not restart motion.
            azimuth_target = (abs(azimuth_target - old_a_target) > POINT_ERR ? azimuth_target : old_a_target);
            elevation_target = abs(elevation_target - old_e_target) > POINT_ERR ? elevation_target : old_e_target;
            break;
          case 0x01:
            // Manual jog up in elevation.
            elevation_target += 5;
            break;
          case 0x02:
            // Manual jog down in elevation.
            elevation_target -= 5;
            break;
          case 0x03:
            // Manual jog positive in azimuth.
            azimuth_target += 5;
            break;
          case 0x04:
            // Manual jog negative in azimuth.
            azimuth_target -= 5;
            break;
          case 0x05:
            // Re-zero the software position estimate and commanded targets.
            elevation_steps = 0;
            azimuth_steps = 0;
            elevation_target = 0.0;
            azimuth_target = 0.0;
            break;
        }
      }

      // Debug output.
      Serial.println(elevation_target);      

      // Convert target angles into motor step counts.
      // 1 / 1.8 converts degrees to motor full steps per revolution.
      // 50.0 is the gearbox / drive ratio.
      // 4.0 reflects the quarter-degree scaling used elsewhere in this ramp math.
      azimuth_target_steps = round(azimuth_target * (1/1.8) * 50.0 * 4.0);
      elevation_target_steps = round(elevation_target * (1/1.8) * 50.0 * 4.0);

      // For azimuth, wrap to the shortest equivalent path instead of always
      // taking the long way around the circle.
      shortest_path();

      if (abs(p_azimuth_target - azimuth_target) >= 1) {
        int az_direction = 0;
        if (azimuth_target_steps > azimuth_steps) {
          az_direction = 1;
        } else if (azimuth_target_steps < azimuth_steps) {
          az_direction = -1;
        }

        // Keep the current ramp when extending a move in the same direction.
        // Restart only if the axis is idle or the command reverses direction.
        bool az_is_idle = (azimuth_steps == azimuth_target_steps);
        bool az_reversing = (az_direction != 0) && (last_az_direction != 0) && (az_direction != last_az_direction);
        if (az_is_idle || az_reversing) {
          newStepsAz = 0;
          pastTAz = 0;
          newTAz = 0;
          az_adjust();
        }
        last_az_direction = az_direction;
      }

      if (abs(p_elevation_target - elevation_target) >= 1) {
        int el_direction = 0;
        if (elevation_target_steps > elevation_steps) {
          el_direction = 1;
        } else if (elevation_target_steps < elevation_steps) {
          el_direction = -1;
        }

        // Apply the same rule to elevation.
        bool el_is_idle = (elevation_steps == elevation_target_steps);
        bool el_reversing = (el_direction != 0) && (last_el_direction != 0) && (el_direction != last_el_direction);
        if (el_is_idle || el_reversing) {
          newStepsEl = 0;
          pastTEl = 0;
          newTEl = 0;
          el_adjust();
        }
        last_el_direction = el_direction;
      }
    } else {
      // Discard bytes until we realign to the next packet header.
      Serial.read();
    }
  }

  // Azimuth stepping is driven by elapsed time, not by blocking delays.
  // When enough time has passed, emit one step toward the target.
  if(micros() - last_step_azimuth >= az_interval){
    if(azimuth_steps < azimuth_target_steps){
      drv_azimuth.fullStep(true);
      azimuth_steps ++;
      newStepsAz ++;
      last_az_direction = 1;

      // Record when this step occurred.
      last_step_azimuth = micros();

      // Update the interval while accelerating, but stop once it gets small.
      if (az_interval > 1000) { az_adjust(); };
    } else if(azimuth_steps > azimuth_target_steps){
      drv_azimuth.fullStep(false);
      azimuth_steps --;
      newStepsAz ++;
      last_az_direction = -1;
      last_step_azimuth = micros();
      if (az_interval > 1000) { az_adjust(); };
    }
  }

  // Elevation uses the same non-blocking step scheduler.
  if(micros() - last_step_elevation >= el_interval){
    if(elevation_steps < elevation_target_steps){
      drv_elevation.fullStep(true);
      elevation_steps ++;
      newStepsEl ++;
      last_el_direction = 1;
      last_step_elevation = micros();
      if (el_interval > 1000) { el_adjust(); };
    } else if(elevation_steps > elevation_target_steps){
      drv_elevation.fullStep(false);
      elevation_steps --;
      newStepsEl ++;
      last_el_direction = -1;
      last_step_elevation = micros();
      if (el_interval > 1000) { el_adjust(); };
    }
  }
  
}

void shortest_path(){
  // The azimuth axis can wrap all the way around.
  // If the requested target is more than 180 degrees away, add or subtract
  // one full revolution so the platform turns the shorter direction.
  while(abs(azimuth_target_steps - azimuth_steps) > 4 * 5000){ //if we are > 180 deg away, we can do better
    if(azimuth_target_steps - azimuth_steps > 0){
      azimuth_target_steps -= 4 * 10000; //decrease 360deg
    } else {
      azimuth_target_steps += 4 * 10000; //increase 360deg
    }
  }
}

/*
Given constant angular acceleration, max deg (t) = 0.5 * a * (t^2)
To find the next time, we solve for t: t = sqrt(2 * max deg / a)

From spreadsheet: with 2 Nm motor toque our max angular acceleration is 1255 deg/s^2
So t = sqrt(2 * max deg / 1255)
*/

void az_adjust() {
  // Compute the total elapsed time to reach the next step on the
  // acceleration curve, then subtract the previous total to get the
  // interval for just the next step.
  if (newStepsAz == 0) {
    newTAz = 0;
  }

  newTAz = sqrt((2.0 * ((newStepsAz + 1)/ 4.0) / (1255.0 / 1.5))) * 1000000.0;
  az_interval = newTAz - pastTAz;
  pastTAz = newTAz;
}

void el_adjust() {
  // Same acceleration-ramp calculation for elevation.
  if (newStepsEl == 0) {
    newTEl = 0;
  }

  newTEl = sqrt((2.0 * ((newStepsEl + 1)/ 4.0) / (1255.0 / 1.5))) * 1000000.0;
  el_interval = newTEl - pastTEl;
  pastTEl = newTEl;
}
