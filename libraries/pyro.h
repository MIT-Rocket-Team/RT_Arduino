#include "Arduino.h"
#include "myTypes.h"

#define PYRO_FIRE_DURATION 250

class pyro {
    public:
        pyro(uint8_t numPyros, uint8_t firePins[], uint8_t adca[], uint8_t adcb[], uint8_t sens[]);
        void begin();
        bool connected(uint8_t channel);
        float resistance(uint8_t channel);
        void arm(uint8_t channel);
        void fire(uint8_t channel);
        void off(uint8_t channel);
        bool isArmed(uint8_t channel);
        bool isFired(uint8_t channel);
        uint16_t getPyrosStatus();
        PyroStatus getPyroStatus(uint8_t channel);
        void update(State rocketState);
        void _setPyroStatus(uint8_t channel, PyroStatus val);

    private:
        bool _armed[16];
        bool _fired[16];
        uint32_t _firedTimes[16];
        uint32_t _pyroStatus;
        uint8_t _numPyros;
        uint8_t* _firePins;
        uint8_t* _adca;
        uint8_t* _adcb;
        uint8_t* _sens;
};
