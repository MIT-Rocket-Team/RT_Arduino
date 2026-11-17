#include "Arduino.h"
#include "rollcontrol.h"
#include "math.h"

rollcontrol::rollcontrol() {

}

void rollcontrol::atmosphere(float h_m) {
    h_m = h_m + LAUNCH_ALT;

    if (h_m < 0.0)      h_m = 0.0;
    if (h_m > 36600.0)  h_m = 36600.0;

    float g0 = 9.80665;
    float R  = 287.05287;
    float gamma = 1.4;

    // Layers: (h_base, h_top, lapse_rate)
    float layers[4][3] = {
        {0.0,     11000.0, -0.0065},
        {11000.0, 20000.0,  0.0},
        {20000.0, 32000.0,  0.0010},
        {32000.0, 47000.0,  0.0028}
    };

    T = LAUNCH_TEMP;  // K
    float P = 101325.0; // Pa (model starts from sea-level)

    for (int i = 0; i < 4; i++) {
        float h_base = layers[i][0];
        float h_top  = layers[i][1];
        float L      = layers[i][2];

        if (h_m <= h_top) {
            if (L == 0.0) {
                P *= exp(-g0 * (h_m - h_base) / (R * T));
            } else {
                float T_new = T + L * (h_m - h_base);
                P *= pow((T_new / T), (-g0 / (R * L)));
                T = T_new;
            }
            rho = P / (R * T);
            a = sqrt(gamma * R * T);
            return;
        }

        // Advance to top of layer
        if (L == 0.0) {
            P *= exp(-g0 * (h_top - h_base) / (R * T));
        } else {
            float T_new = T + L * (h_top - h_base);
            P *= pow((T_new / T), (-g0 / (R * L)));
            T = T_new;
        }
    }

    rho = P / (R * T);
    a = sqrt(gamma * R * T);
}

float rollcontrol::CMx_alpha(float mach) {
    if (mach < 1) {
        return 2.47539;
    }
    float p1 = 2.34725224807287;
    float p2 = 1.04024379248907;
    float p3 = 1.49368646930894;
    float p4 = 0.779528950818373;
    return p1*exp(-p2*(mach - p3)) + p4;
}

float rollcontrol::Jxx_of_t(float t) {
    if (t <= 0.0)  return Jxx0;
    if (t >= t_b)  return Jxxf;
    return Jxx0 + (t / t_b) * (Jxxf - Jxx0);
}

float rollcontrol::Gd(float v, float CMx_alpha_val, float Jxx) {
    if (Jxx == 0.0) {  // Catch for somehow passing in zero (should never happen)
        return 1.0;
    }
    return (rho * v * v * CMx_alpha_val) / (2.0 * Jxx);
}

void rollcontrol::Gd_star() {
    atmosphere(h_star);
    v_star = M_star * a;
    CMx_alpha_val = CMx_alpha(M_star);
    float Jxx_star = Jxx_of_t(t_b);
    Gd_star_val = Gd(v_star, CMx_alpha_val, Jxx_star);
}

float rollcontrol::K_servo(float v, float mach) {
    float CMxa = CMx_alpha(mach);
    float tau_alpha_proxy =  rho * v * v * CMxa;
    float deprate =  5.5830e-07;
    return 1.0 - deprate * tau_alpha_proxy;
}

void rollcontrol::begin() {
    Gd_star();
}

void rollcontrol::update(float t, float h, float v, float roll, float roll_rate) {
    float v_eff = (v >= V_MIN) ? v : V_MIN;
    v_eff = (v_eff <= V_MAX) ? v_eff : V_MAX;
    atmosphere(h);
    float mach = (a > 0.0) ? v_eff/a : 0.0;
    float CMx_a = CMx_alpha(mach);
    float Jxx = Jxx_of_t(t);
    float Gd_val = Gd(v_eff, CMx_a, Jxx);

    float e = -roll;
    float dedt = -roll_rate;
    float K_0 = KP * e + KD * dedt;

    float unscaledAngle = K_0 * Gd_star_val/Gd_val;
    if (unscaledAngle > 10.0) unscaledAngle = 10.0;
    if (unscaledAngle < -10.0) unscaledAngle = -10.0;

    angle = unscaledAngle * 1.0 / K_servo(v_eff, mach);
}

float rollcontrol::getAngle() {
    return angle;
}
