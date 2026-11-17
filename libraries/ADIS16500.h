#ifndef ADIS16500_H
#define ADIS16500_H

#include "Arduino.h"
#include "SPI.h"
#include "myTypes.h"

#define ACCEL_PREFLIGHT_INTEGRATION_THRESHOLD 10
#define ACCEL_BURST_INTEGRATION 0 // basically chages if the burst is getting accel data or vel + delta angle

class ADIS16500 {
    public:
        ADIS16500(SPIClass* SPI, SPISettings settings, int cs);
        void setup();
        float getAccelX();
        float getAccelY();
        float getAccelZ();
        int32_t getRawX();
        int32_t getRawY();
        int32_t getRawZ();
        float getVelX();
        float getVelY();
        float getVelZ();
        float getDeltaVelX();
        float getDeltaVelY();
        float getDeltaVelZ();
        float getGyroX();
        float getGyroY();
        float getGyroZ();
        float getDeltaGyroX();
        float getDeltaGyroY();
        float getDeltaGyroZ();
        void update(State rocketState);
        float getVerticalAccelMinusGravity();
        
    private:
        SPIClass* _SPI;
        SPISettings _settings;
        int _cs;
        float _accelX, _accelY, _accelZ;
        int32_t _accelXraw, _accelYraw, _accelZraw;
        float _velX, _velY, _velZ;
        float _deltaVelX, _deltaVelY, _deltaVelZ;
        float _gyroX, _gyroY, _gyroZ;
        float _deltaGyroX, _deltaGyroY, _deltaGyroZ;
        void _writeReg(byte reg, byte val);
        bool _dataReady();
        uint16_t _readReg(byte reg);
        uint16_t _frame(uint16_t tx);
        uint32_t _readS32(byte reg);
        uint32_t _lastUpdate;
        uint16_t _lastDataCntr;
        float _verticalAccelMinusGravity;
};

#endif
