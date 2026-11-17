#include "Arduino.h"
#include "SPI.h"

#define READ 0x80

#define GYRO_XOUT_L 0x44
#define GYRO_XOUT_H 0x43
#define GYRO_YOUT_L 0x46
#define GYRO_YOUT_H 0x45
#define GYRO_ZOUT_L 0x48
#define GYRO_ZOUT_H 0x47

#define GYRO_OFFSET_ROLL_DPS -0.2525f
#define GYRO_OFFSET_PITCH_DPS 0.2441f
#define GYRO_OFFSET_YAW_DPS 0.4376f

class gyro {
    public:
        gyro(SPIClass* SPI, SPISettings settings, int cs);
        void begin();
        void config();
        void update();
        int16_t getRawX();
        int16_t getRawY();
        int16_t getRawZ();
        float getDpsX();
        float getDpsY();
        float getDpsZ();
        float getRoll();
        float getPitch();
        float getYaw();
        float getRollRate();
        float getAngleFromVertical();
        void zeroRollPitchYaw();
        uint8_t readReg(uint8_t reg);
        
    private:
        SPIClass* _SPI;
        SPISettings _settings;
        int _cs;
        int16_t _rawX;
        int16_t _rawY;
        int16_t _rawZ;
        int16_t _dpsX;
        int16_t _dpsY;
        int16_t _dpsZ;
        void _writeReg(byte reg, byte val);
        uint32_t _lastUpdate;
        float _roll;
        float _pitch;
        float _yaw;
        float _angleFromVertical;
};