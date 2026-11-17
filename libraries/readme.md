# **RT Firmware Libs**

## Structure
Arduino code is at the top level. For other files, see the directory `.other`, in which the following can be found:

- `docs/system_architecture`: documentation regarding the structure of the code
- `src/java.sim.nocompile/*`: java code for the OR simulator at [`MDNich/ActiveControl_MIT_RktTeam`](https://github.com/MDNich/ActiveControl_MIT_RktTeam). It will not compile in this repository, and it is purely for reference purposes.


See [this diagram](https://github.com/MIT-Rocket-Team/RT_Firmware_Libs/blob/master/.other/docs/system_architecture/UML_FC_sim.pdf) for an overview of the simulation clases (updated continually).


## Arduino Libraries

---

### Accelerometer Library

#### `ADXL357(SPIClass* SPI, SPISettings settings, int cs)`
constructor
#### `void setup()`
Set range to +/- 40G, Set LPF to 125Hz and ODR to 500HZ, begin measurements
#### `void update()`
updates accelerometer values if available
#### `float getAccelX()`
returns x axis acceleration in m/s^2
#### `float getAccelY()`
returns y axis acceleration in m/s^2
#### `float getAccelZ()`
returns z axis acceleration in m/s^2

---

### GPS library

#### `GPS(Uart* gpsSer)`
hardware serial constructor
#### `GPS(USBSerial* gpsSer)`
USB serial constructor
#### `void setup()`
begins serial port
#### `void updateAndParse()`
moves any available serial data into the data buffer, and if any data is available, parses the buffer
#### `float getPDOP()`
returns PDOP
#### `float getVDOP()`
returns VDOP
#### `float getHDOP()`
returns HDOP
#### `float getLatitude()`
returns latitude
#### `float getLongitude()`
returns longitude
#### `float getAltitude()`
returns altitude in meters
#### `bool getFix()`
returns true if GPS fix; false otherwise

---

### **Roll Control Library**

#### `rollControl()`
constructor
#### `void setup()`
starts the servo drive signal to the zero (+offset) point
#### `void updateRollControl(ServoState state, float angle, float velo, float roll, float rollRate)`
updates the servo control mode, rocket velocity for scaling, and angle (if in SERVO_ANGLE mode).

`SERVO_STATE` is either `SERVO_ZERO`, `SERVO_ANGLE`, or `SERVO_PD`
#### `uint16_t getServo6Us()`
returns time in us the servo 6 drive signal is a logic high

conversion to degrees is `(us - 1500) / 500 * 60`
#### `uint16_t getServo7Us()`
returns time in us the servo 7 drive signal is a logic high

conversion to degrees is `(us - 1500) / 500 * 60`
