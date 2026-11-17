#include "Arduino.h"

#define LAUNCH_ALT 271.0f
#define LAUNCH_TEMP 290.0f

#define KP 0.08444f
#define KD 0.02111f

#define Jxx0 0.267f
#define Jxxf 0.241f
#define t_b 2.51f
#define M_star 1.24f
#define h_star 474.5736f
#define d_ref 0.2207f
#define n_tabs 2.0f

#define V_MIN 20.0f
#define V_MAX 1000.0f

class rollcontrol {
    public:
        rollcontrol();
        void atmosphere(float h_m);
        float CMx_alpha(float mach);
        float Jxx_of_t(float t);
        float Gd(float v, float CMx_alpha, float Jxx);
        void Gd_star();
        float K_servo(float v, float mach);
        void update(float t, float h, float v, float roll, float roll_rate);
        float getAngle();
        void begin();

    private:
        float T;
        float rho;
        float a;
        float Gd_star_val;
        float v_star;
        float rho_star;
        float T_star;
        float angle;
        float CMx_alpha_val;
};