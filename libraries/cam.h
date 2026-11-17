//https://support.runcam.com/hc/en-us/articles/360014537794-RunCam-Device-Protocol
#include "Arduino.h"

#define COMMAND_GET_DEVICE_INFO 0x00
#define COMMAND_DISP_WRITE_CHAR 0x21
#define COMMAND_DISP_WRITE_HORT_STRING 0x22

class cam {
    public:
        cam(Uart* ser);
        void begin();
        void getInfo();
        void writeString(char* data, uint8_t len, uint8_t x, uint8_t y);
        void writeChar(char data, uint8_t x, uint8_t y);

    private:
        Uart* _ser;
        uint8_t _crc(uint8_t *data, uint8_t len);
        uint8_t _crc8_dvb_s2(uint8_t crc, unsigned char a);
        void _sendFrame(uint8_t cmd, uint8_t *data, uint8_t len);
};
