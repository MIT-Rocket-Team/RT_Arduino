#include <Arduino.h>
#include <math.h>
#include <stddef.h>
#include <myTypes.h>

/* ------------------ Compile-time constants ------------------ */
#define AIRBRAKES_N_MEASUREMENTS 20
#define AIRBRAKES_MEASUREMENT_FREQ_HZ 5
#define AIRBRAKES_SIMULATION_T_APOG 35.0f
#define DEBUG_AIRBRAKES_ON 1
#define AIRBRAKES_START_TIME 13.0f
#define FLAG_DYNAMIC_DESIRED_ALTITUDE true
#define SIM_PREDICTED_ALTITUDE 5046.0f

/* ------------------ Measurements types ------------------ */
typedef struct {
  float velocityMeasurement;
  float timeStamp;
} AirbrakesVelocityMeasurement;

typedef struct {
  float accelerationMeasurement;
  float timeStamp;
} AirbrakesAccelerationMeasurement;

/* ------------------ Controller state ------------------ */
typedef enum {
  DISABLED = 0,
  PREP,
  PREPROCESS,
  WAIT_FOR_START,
  CONTROLLING_RAMP,
  CONTROLLING_PLATEAU,
  DONE,
  INFEASIBLE
} airbrakesState;

class airbrakes {
public:
  airbrakes();

  void begin();
  void update(float currentTime, const AirbrakesData& status);

  float getDeployment() const;
  airbrakesState getState() const;

private:

  /* constants */
  const float g = 9.81f;

  float mass = 34.15380231015455f;
  float rho = 0.736115423712237f;
  float airbrakesCd = 1.28f;
  float rocketCd = 0.4843927669074317f;
  float a_ref = 0.019289796351014733f;
  float a_max = 0.0066f;
  float fudge_factor = 3.2f;
  float fudge_factor_2 = 3.5f;

  float EARLIEST_AIRBRAKES_PREP_TIME = 4.0f;
  float START_AIRBRAKES_PREP_VEL = 400.0f;
  float START_AIRBRAKES_PREPROC_TIME = 12.5f;
  float AIRBRAKES_TIME_DELAY = 1.0f;
  float AIRBRAKES_T_APOG_FUDGEDIFF = 1.5f;

  int roundToHowMuch = 100;

  float t_apog = 35.5f;
  float coeffA = -0.0154397511f;
  float coeffB = -0.3379534959f;

  float alt0 = 0.0f;
  float predictedAlt = 0.0f;
  float desiredDeltaX = 0.0f;

  float airbrakesCtrlStartTime = 1e10f;
  float A0_req = 0.0f;

  float Astar = 0.0f;
  float patchingAltitude = 0.0f;
  float velContribFudge = 1.0f;
  float cFudge = 0.825f;
  float K = 1;

  float lastA = 0;
  float lastDeltaA = 0;
  float lastDeltaH = 0;
  float lastHf = 0;
  float lastI = 0;
  
  uint32_t lastMeasurementTimeMs = 0;
  float desiredAlt=4550.0f;

  airbrakesState state;
  float deployment;

  AirbrakesAccelerationMeasurement accelData[AIRBRAKES_N_MEASUREMENTS];
  AirbrakesVelocityMeasurement velData[AIRBRAKES_N_MEASUREMENTS];

  int datIndex;
  int counter;

  /* helpers */
  float maxf(float a, float b);
  float minf(float a, float b);

  float p4(float x);
  float p5(float x);
  float p6(float x);
  float p7(float x);
  float p8(float x);
  float p9(float x);
  float p10(float x);

  float pow5f_fast(float x);
  float pow10f_fast(float x);

  float accelModel(float t, float a, float custom_t_apog);
  float getR2fromFit_accel(const AirbrakesAccelerationMeasurement *data,
                           size_t n,
                           float a,
                           float custom_t_apog);

  int argmax(const float *arr, size_t n);

  bool inverse2x2Matrix(const float A[2][2], float Ainv[2][2]);

  float reqDeployedAreaAirbrakes(float t_0, float deltaX);
  float computeFinalAltitude_Conrad(float A, float h0, float v0);
  float computeK(float Astar, float h0, float v0);

  void setAirbrakesServo(float deployedFraction);

  bool shouldStartAirbrakesControlPrep(float t, const AirbrakesData& s);
  bool shouldStartAirbrakesControlPreprocess(float t, const AirbrakesData& s);

  void handleState(float t, const AirbrakesData& status);
};