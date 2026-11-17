#include "pyro.h"

pyro::pyro(uint8_t numPyros, uint8_t firePins[], uint8_t adca[], uint8_t adcb[], uint8_t sens[]) {
    _numPyros = numPyros;
    _firePins = firePins;
    _adca = adca;
    _adcb = adcb;
    _sens = sens;
    for (int i = 0; i < 16; i++) {
        _armed[i] = false;
        _fired[i] = false;
        _firedTimes[i] = 0;
    }
    _pyroStatus = 0;
}

void pyro::begin() {    
    for (int i = 0; i < _numPyros; i++) {
        pinMode(_firePins[i], OUTPUT);
        pinMode(_sens[i], INPUT);
    }
}

bool pyro::connected(uint8_t channel) {
    return digitalRead(_sens[channel]);
}

float pyro::resistance(uint8_t channel) {
    return 0.5 * (((float) analogRead(_adcb[channel])) / ((float)analogRead(_adca[channel])));
}

void pyro::arm(uint8_t channel) {
    _armed[channel] = true;
}

void pyro::fire(uint8_t channel) {
    if (_armed[channel]) {
        digitalWrite(_firePins[channel], HIGH);
        _firedTimes[channel] = millis();
        _fired[channel] = true;
        _armed[channel] = false;
    }
}

void pyro::off(uint8_t channel) {
    digitalWrite(_firePins[channel], LOW);
    _fired[channel] = false;
}

bool pyro::isArmed(uint8_t channel) {
    return _armed[channel];
}

bool pyro::isFired(uint8_t channel) {
    return _fired[channel];
}

uint16_t pyro::getPyrosStatus() {
    return _pyroStatus;
}

PyroStatus pyro::getPyroStatus(uint8_t channel) {
  return static_cast<PyroStatus> ((uint8_t) ((_pyroStatus >> (channel * 2)) & 0b11));
}

void pyro::update(State rocketState) {
    for (uint8_t i = 0; i < _numPyros; i++) {
        //turn off pyros if they have been on for > PYRO_FIRE_DURATION
        if (_fired[i] && millis() - _firedTimes[i] > PYRO_FIRE_DURATION) {
            off(i);
            //If pyro no longer connected and didn't come unconnected earlier, then it's a success
            if (!connected(i) && getPyroStatus(i) != PYRO_FAILURE) {
                _setPyroStatus(i, PYRO_SUCCESS);
            } else {
                _setPyroStatus(i, PYRO_FAILURE);
            }
        }
        //Update pyro state
        //in ground testing, we only care if pyro is connected or disconnected
        if (rocketState == GROUND_TESTING) {
            if (connected(i)) {
                _setPyroStatus(i, PYRO_CONNECTED);
            } else {
                _setPyroStatus(i, PYRO_UNCONNECTED);
            }
        } else {
            //Otherwise, an unconnected pyro is a failure unless it has been successfully fired
            if (!connected(i) && getPyroStatus(i) != PYRO_SUCCESS && !_fired[i]) {
                _setPyroStatus(i, PYRO_FAILURE);
            }
        }
    }
    
}

void pyro::_setPyroStatus(uint8_t channel, PyroStatus val) {
  uint16_t mask = 0b11 << (channel * 2);
  _pyroStatus &= ~mask;
  _pyroStatus |= (val & 0b11) << (channel * 2);
}