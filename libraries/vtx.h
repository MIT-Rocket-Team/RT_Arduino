//https://www.team-blacksheep.com/media/files/tbs_smartaudio_rev08.pdf
#include "Arduino.h"

class vtx {
    public:
        vtx(Uart* ser);
        void begin();
        void getSettings();
        void setPower(uint8_t power);
        void setFreq(uint16_t freq);

    private:
        Uart* _ser;
        uint8_t _crc(uint8_t *data, uint8_t len);
        void _sendFrame(uint8_t cmd, uint8_t *data, uint8_t len);
};
