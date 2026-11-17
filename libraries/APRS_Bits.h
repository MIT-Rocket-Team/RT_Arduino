#ifndef APRS_BITS_H
#define APRS_BITS_H

#include <stdint.h>
#include <stddef.h>
#include <Arduino.h>

#define APRS_MAX_FRAME 330
#define APRS_MAX_BITS 5000

typedef void (*bit_callback_t)(uint8_t bit);

class APRS_Bits {
public:
    APRS_Bits();
    void sendPacket(
        const char* source,
        const char* dest,
        const char* path,
        const char* info,
        bit_callback_t cb
    );

private:
    void buildAX25Frame(
        uint8_t* frame,
        size_t& frame_len,
        const char* source,
        const char* dest,
        const char* path,
        const char* info
    );

    void encodeCallsign(uint8_t* out, const char* callsign, bool last);
    uint16_t crc_ccitt(const uint8_t* data, size_t len);

    size_t bitStuff(uint8_t* out_bits, const uint8_t* in_bytes, size_t len);
    void nrziEncode(uint8_t* bits, size_t len);
};

#endif