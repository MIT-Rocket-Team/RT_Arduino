#include "Arduino.h"
#include "cam.h"

cam::cam(Uart* ser) {
    _ser = ser;
}

void cam::begin() {
    _ser->begin(115200);
}


uint8_t cam::_crc8_dvb_s2(uint8_t crc, unsigned char a)
{
    crc ^= a;
    for (int ii = 0; ii < 8; ++ii) {
        if (crc & 0x80) {
            crc = (crc << 1) ^ 0xD5;
        } else {
            crc = crc << 1;
        }
    }

    return crc;
}

uint8_t cam::_crc(uint8_t *data, uint8_t len) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        crc = _crc8_dvb_s2(crc, data[i]);
    }
    return crc;
}

void cam::_sendFrame(uint8_t cmd, uint8_t *data, uint8_t len) {
    uint8_t frame[128];
    frame[0] = 0xCC;
    frame[1] = cmd;
    memcpy(frame + 2, data, len);
    frame[len + 2] = _crc(frame, len + 2);

    _ser->write(frame, len + 3);
}


void cam::getInfo() {
    _sendFrame(COMMAND_GET_DEVICE_INFO, nullptr, 0);
}

void cam::writeString(char* data, uint8_t len, uint8_t x, uint8_t y) {
    uint8_t buf[63];

    buf[0] = len;
    buf[1] = x;
    buf[2] = y;
    memcpy(buf + 3, data, len);
    buf[len + 3] = 0;

    _sendFrame(COMMAND_DISP_WRITE_HORT_STRING, buf, len + 4);
}


void cam::writeChar(char data, uint8_t x, uint8_t y) {
    uint8_t buf[63];

    buf[0] = x;
    buf[1] = y;
    buf[2] = data;

    _sendFrame(COMMAND_DISP_WRITE_CHAR, buf, 3);
}
