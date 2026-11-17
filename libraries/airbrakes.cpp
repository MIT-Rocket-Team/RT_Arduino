#include "airbrakes.h"
#include "math.h"

/* ------------------ Constructor ------------------ */
airbrakes::airbrakes() {
  begin();
}

void airbrakes::begin() {
  state = DISABLED;
  deployment = 0.0f;
  datIndex = 0;
  counter = 0;
  patchingAltitude = 0;
}

/* ------------------ Public ------------------ */
void airbrakes::update(float t, const AirbrakesData& status) {
  handleState(t, status);
}

float airbrakes::getDeployment() const {
  return deployment;
}

airbrakesState airbrakes::getState() const {
  return state;
}

/* ------------------ Servo replacement ------------------ */
void airbrakes::setAirbrakesServo(float deployedFraction) {
  if (deployedFraction < 0.0f) deployedFraction = 0.0f;
  if (deployedFraction > 1.0f) deployedFraction = 1.0f;
  deployment = deployedFraction;
}

/* ------------------ Helpers ------------------ */
float airbrakes::maxf(float a, float b) {
  return (a > b) ? a : b;
}
float airbrakes::minf(float a, float b) {
  return (a < b) ? a : b;
}

/* power funcs */
float airbrakes::p4(float x){ float x2=x*x; return x2*x2; }
float airbrakes::p5(float x){ return p4(x)*x; }
float airbrakes::p6(float x){ float x3=x*x*x; return x3*x3; }
float airbrakes::p7(float x){ return p6(x)*x; }
float airbrakes::p8(float x){ float x4=p4(x); return x4*x4; }
float airbrakes::p9(float x){ return p8(x)*x; }
float airbrakes::p10(float x){ float x5=p5(x); return x5*x5; }

float airbrakes::pow5f_fast(float x){ return p5(x); }
float airbrakes::pow10f_fast(float x){ return p10(x); }

/* accel model */
float airbrakes::accelModel(float t, float a, float custom_t_apog) {
  float dt = t - custom_t_apog;
  return a * pow5f_fast(dt) - g;
}

float airbrakes::getR2fromFit_accel(const AirbrakesAccelerationMeasurement *data,
                                              size_t n,
                                              float a,
                                              float custom_t_apog) {
  float ss_res = 0.0f, ss_tot = 0.0f, sum_y = 0.0f;

  for (size_t i = 0; i < n; i++) {
    float y = data[i].accelerationMeasurement;
    float yh = accelModel(data[i].timeStamp, a, custom_t_apog);
    float r = y - yh;
    ss_res += r * r;
    sum_y += y;
  }

  float mean = sum_y / n;
  for (size_t i = 0; i < n; i++) {
    float d = data[i].accelerationMeasurement - mean;
    ss_tot += d * d;
  }

  if (ss_tot == 0.0f) return 0.0f;
  return 1.0f - (ss_res / ss_tot);
}

int airbrakes::argmax(const float *arr, size_t n) {
  float best = arr[0];
  int idx = 0;
  for (size_t i = 1; i < n; i++) {
    if (arr[i] > best) {
      best = arr[i];
      idx = i;
    }
  }
  return idx;
}

bool airbrakes::inverse2x2Matrix(const float A[2][2], float Ainv[2][2]) {
  float det = A[0][0]*A[1][1] - A[0][1]*A[1][0];
  if (fabsf(det) <= 1e-6f) return false;

  float f = 1.0f/det;
  Ainv[0][1] = -f*A[0][1];
  Ainv[1][0] = -f*A[1][0];
  Ainv[0][0] = f*A[1][1];
  Ainv[1][1] = f*A[0][0];
  return true;
}

/* ------------------ Physics ------------------ */
float airbrakes::reqDeployedAreaAirbrakes(float t_0, float deltaX) {
  float a = coeffA;
  float b = coeffB;
  float dt = (t_0 - t_apog);

  float a2=a*a,a3=a2*a,b2=b*b,b3=b2*b,g2=g*g,g3=g2*g;

  float term =
    (a3)*p10(dt)/10.0f +
    (a2*b)*p9(dt)/3.0f +
    (3*a*b2-3*a2*g)*p8(dt)/8.0f +
    (b3-6*a*b*g)*p7(dt)/7.0f +
    (a*g2-b2*g)*p6(dt)/2.0f +
    (3*b*g2)*p5(dt)/5.0f -
    (g3)*p4(dt)/4.0f;

  float xi = -term;

  float local_fudge = (deltaX > 40.0f) ? fudge_factor : fudge_factor_2;

  float denom = airbrakesCd * rho * xi;
  if (denom == 0.0f || a_max == 0.0f) return 0.0f;

  float a_0 = local_fudge * 2.0f * mass * g * deltaX / denom;
  float tentative = minf(maxf(0.0f, a_0 / a_max),1.0f);
  if (tentative == 0.0f) {
    tentative = 1.0f; // to have it deploy : )
  }
  return tentative;
}

float airbrakes::computeFinalAltitude_Conrad(float A, float h0, float v0) {
  float m = mass;
  float c = rho * rocketCd * a_ref / 2.0f;
  c *= cFudge;
  float alpha = rho * airbrakesCd * A / 2.0f;

  float hf = h0 + velContribFudge * m / (2.0f * (alpha + c)) *
    log((v0 * v0 * (alpha + c)) / g / m + 1.0f);

  return hf - 13.0f + patchingAltitude;
}

float airbrakes::computeK(float Astar, float h0, float v0) {
  float m = mass;
  float c = rho * rocketCd * a_ref / 2.0f;
  float alpha = rho * airbrakesCd * Astar / 2.0f;

  alpha /= 3.2f;

  float K0 = m/2 * (-1/((c+alpha)*(c+alpha))*log(v0*v0/g/m*(c+alpha)+1)
             + 1/(c+alpha)*v0*v0/g/m/(v0*v0/g/m*(c+alpha)+1));

  float ret = isnan(K0/2.0f) ? 1e10 : K0/2.0f;

  return ret;
}

/* ------------------ Start conditions ------------------ */
bool airbrakes::shouldStartAirbrakesControlPrep(float t, const AirbrakesData& s) {
  return (t > EARLIEST_AIRBRAKES_PREP_TIME) && (!s.apogeeReached) && (s.vel_z < START_AIRBRAKES_PREP_VEL);
}

bool airbrakes::shouldStartAirbrakesControlPreprocess(float t, const AirbrakesData& s) {
  return (t > START_AIRBRAKES_PREPROC_TIME) && (!s.apogeeReached);
}

/* ------------------ STATE MACHINE ------------------ */
void airbrakes::handleState(float t, const AirbrakesData& status) {

  if (state == DISABLED) {
    setAirbrakesServo(0.0f);
    state = shouldStartAirbrakesControlPrep(t, status) ? PREP : DISABLED;
    if (state == PREP) { datIndex = 0; counter = 0; }
  }

  else if (state == PREP) {
    uint32_t nowMs = millis();
    uint32_t periodMs = 1000 / AIRBRAKES_MEASUREMENT_FREQ_HZ;

    if (datIndex < AIRBRAKES_N_MEASUREMENTS &&
        (nowMs - lastMeasurementTimeMs) >= periodMs) {

        lastMeasurementTimeMs = nowMs;
        accelData[datIndex] = {status.accel_z, t};
        velData[datIndex]   = {status.vel_z, t};
        datIndex++;
    }

    if (datIndex >= AIRBRAKES_N_MEASUREMENTS) state = PREPROCESS;
    else state = shouldStartAirbrakesControlPreprocess(t, status) ? PREPROCESS : PREP;
  }

  else if (state == PREPROCESS) {

    const float t_apog_trials[3] = {34.0f,35.0f,36.0f};
    float R2[3]={0};

    for(int i=0;i<3;i++){
      float sum_num=0,sum_den=0;
      for(int j=0;j<datIndex;j++){
        float dt=accelData[j].timeStamp-t_apog_trials[i];
        sum_den+=pow10f_fast(dt);
        sum_num+=(accelData[j].accelerationMeasurement+g)*pow5f_fast(dt);
      }
      float a_coeff = (sum_den!=0)?(sum_num/sum_den):0;
      R2[i]=getR2fromFit_accel(accelData,AIRBRAKES_N_MEASUREMENTS,a_coeff,t_apog_trials[i]);
    }

    int best=argmax(R2,3);
    t_apog=t_apog_trials[best]+AIRBRAKES_T_APOG_FUDGEDIFF;

    float conrad=computeFinalAltitude_Conrad(0,status.altitude,status.vel_z);
    patchingAltitude=SIM_PREDICTED_ALTITUDE-conrad;
    predictedAlt=computeFinalAltitude_Conrad(0,status.altitude,status.vel_z);
    if (FLAG_DYNAMIC_DESIRED_ALTITUDE) {
      desiredAlt = floor(predictedAlt/100.0f)*100.0f;
    }
    desiredDeltaX=predictedAlt-desiredAlt;

    bool tooLate=t> AIRBRAKES_START_TIME-0.25f;
    airbrakesCtrlStartTime= tooLate ? t+AIRBRAKES_TIME_DELAY : AIRBRAKES_START_TIME;

    A0_req=reqDeployedAreaAirbrakes(airbrakesCtrlStartTime,desiredDeltaX);

    Astar=A0_req*a_max;
    lastA=Astar;
    K=computeK(Astar,status.altitude,status.vel_z);

    state=WAIT_FOR_START;
  }

  else if (state == WAIT_FOR_START) {
    if (t>=airbrakesCtrlStartTime) state=CONTROLLING_RAMP;

    if (status.apogeeReached){
      state=DONE;
      setAirbrakesServo(0);
    }
  }

  else if (state == CONTROLLING_RAMP) {

    if (t>=airbrakesCtrlStartTime) {
      float deployed=2.0f*A0_req*(t-airbrakesCtrlStartTime);
      setAirbrakesServo(deployed);
    }

    if (t>=airbrakesCtrlStartTime+0.5f){
      state=CONTROLLING_PLATEAU;
      setAirbrakesServo(A0_req);
    }

    if (status.apogeeReached){
      state=DONE;
      setAirbrakesServo(0);
    }
  }

  else if (state == CONTROLLING_PLATEAU) {

    float Ki=2.0f/K, Kp=1.0f/K;

    lastA=deployment*a_max;
    float hf=computeFinalAltitude_Conrad(lastA,status.altitude,status.vel_z);

    lastDeltaH=hf-desiredAlt;

    float I;
    if(((lastA/a_max>=1.0f)&&(lastDeltaH>=0))||((lastA/a_max<=1e-5)&&(lastDeltaH<0)))
      I=lastI;
    else
      I=lastI+2.0f/K*lastDeltaH;

    lastI=I;

    float nextA=Astar+(Kp*lastDeltaH+Ki*lastI);
    setAirbrakesServo(nextA/a_max);

    if(status.vel_z<=0||status.apogeeReached){
      state=DONE;
      setAirbrakesServo(0);
    }
  }
}