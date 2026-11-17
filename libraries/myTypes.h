#ifndef MY_TYPES_H
#define MY_TYPES_H

enum State {
  GROUND_TESTING,
  PRE_FLIGHT,
  FLIGHT,
  APOGEE,
  MAIN,
  END
};

enum PyroStatus {
  PYRO_FAILURE,
  PYRO_UNCONNECTED,
  PYRO_CONNECTED,
  PYRO_SUCCESS
};

typedef struct __attribute__((packed))
{
  int16_t cell1;
  int16_t cell2;
  int16_t cell3;
  int16_t stackVoltage;
  int16_t current;
  float temp;
  uint8_t fetStatus;
  uint8_t protectionStatus;
  uint8_t protectionsEnabled;
} bmsData;

typedef struct __attribute__((packed))
{
  bool protectionsEnabled = true;
  bool screwSwitchEnabled = true;
} bmsCommands;

typedef struct __attribute__((packed))
{
  bmsData BMS;
  int16_t voltages[6];
  int16_t currents[6];
} pwrBoardData;

typedef struct __attribute__((packed))
{
  bmsCommands BMS;
  bool convertersEnabled[6] = {true, true, true, true, true, true}; //3V, 3.3V, 5V, 7.4V, 8.4V, 28V
} pwrCommands;

typedef struct __attribute__((packed))
{
  PyroStatus statuses[6];
  uint8_t armed[6];
  uint8_t fired[6];
  float resistances[6];
} pyroData;

typedef struct {
  float altitude;
  float vel_z;
  float accel_z;
  bool apogeeReached;
} AirbrakesData;

#endif