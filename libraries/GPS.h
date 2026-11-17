#include "Arduino.h"

#include <stdint.h>

typedef struct __attribute__((packed))
{
    uint32_t iTOW;
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  min;
    uint8_t  sec;
    uint8_t  valid;

    uint32_t tAcc;
    int32_t  nano;

    uint8_t  fixType;
    uint8_t  flags;
    uint8_t  flags2;
    uint8_t  numSV;

    int32_t  lon;
    int32_t  lat;
    int32_t  height;
    int32_t  hMSL;

    uint32_t hAcc;
    uint32_t vAcc;

    int32_t  velN;
    int32_t  velE;
    int32_t  velD;
    int32_t  gSpeed;

    int32_t  headMot;

    uint32_t sAcc;
    uint32_t headAcc;

    uint16_t pDOP;
    uint16_t flags3;

    uint8_t  reserved1[4];

    int32_t  headVeh;
    int16_t  magDec;
    uint16_t magAcc;

} ubx_nav_pvt_t;


class GPS {
  public:
    GPS(Uart* gpsSer);
    void begin();
    void update();
    float getMaxAlt();
    void zeroAlt();
    uint32_t getITOW() { return _pkt.iTOW; }
    uint16_t getYear() { return _pkt.year; }
    uint8_t getMonth() { return _pkt.month; }
    uint8_t getDay() { return _pkt.day; }
    uint8_t getHour() { return _pkt.hour; }
    uint8_t getMin() { return _pkt.min; }
    uint8_t getSec() { return _pkt.sec; }
    uint8_t getValid() { return _pkt.valid; }
    uint32_t getTAcc() { return _pkt.tAcc; }
    int32_t getNano() { return _pkt.nano; }
    uint8_t getFixType() { return _pkt.fixType; }
    uint8_t getFlags() { return _pkt.flags; }
    uint8_t getFlags2() { return _pkt.flags2; }
    uint8_t getNumSV() { return _pkt.numSV; }
    int32_t getLon() { return _pkt.lon; }
    int32_t getLat() { return _pkt.lat; }
    float getHeight() { return (_pkt.height - _heightOffset) / 1000.0; }
    int32_t getHMSL() { return _pkt.hMSL; }
    uint32_t getHAcc() { return _pkt.hAcc; }
    uint32_t getVAcc() { return _pkt.vAcc; }
    int32_t getVelN() { return _pkt.velN; }
    int32_t getVelE() { return _pkt.velE; }
    int32_t getVelD() { return _pkt.velD; }
    int32_t getGSpeed() { return _pkt.gSpeed; }
    int32_t getHeadMot() { return _pkt.headMot; }
    uint32_t getSAcc() { return _pkt.sAcc; }
    uint32_t getHeadAcc() { return _pkt.headAcc; }
    uint16_t getPDOP() { return _pkt.pDOP; }
    uint16_t getFlags3() { return _pkt.flags3; }
    int32_t getHeadVeh() { return _pkt.headVeh; }
    int16_t getMagDec() { return _pkt.magDec; }
    uint16_t getMagAcc() { return _pkt.magAcc; }


  private:
    Uart* _gpsSer;
    ubx_nav_pvt_t _pkt;
    uint8_t _buf[98];
    bool _headerValid;
    bool _validateHeader();
    void _readPacket();
    bool _validateChecksum();
    float _maxAlt;
    float _heightOffset;
};
