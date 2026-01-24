#include "NoiseChannel.h"

NoiseChannel::NoiseChannel()
{
}

NoiseChannel::~NoiseChannel()
{
}

uint8_t NoiseChannel::read(uint16_t address)
{
    uint8_t res = 0xFF;
    if (address == 0xFF20) {
        res = 0xFF;
    } else if (address == 0xFF21) {
        res = NR42;
    } else if (address == 0xFF22) {
        res = NR43;
    } else if (address == 0xFF23) {
        res = NR44 | 0xBF;
    }
    return res;
}

void NoiseChannel::write(uint16_t address, uint8_t data) {
    if (address == 0xFF20) {
        NR41 = data;
        lengthTimer = 64 - (NR41 & 0x3F);
    } else if (address == 0xFF21) {
        NR42 = data;
        if ((data & 0xF8) == 0) active = false;
    } else if (address == 0xFF22) {
        NR43 = data;
    } else if (address == 0xFF23) {
        NR44 = data;
        if ((data & 0x80) == 0x80) {
            active = true;

            if (lengthTimer == 0) lengthTimer = 64;

            envelopeTimer = NR42 & 0x07;

            volume = (NR42 >> 4) & 0x0F;

            lfsr = 0x7FFF;

            uint8_t clockDivider = NR43 & 0x07;
            uint8_t clockShift = (NR43 >> 4) & 0x0F;

            uint16_t divisor = (clockDivider == 0) ? 8 : 16 * clockDivider;
            timer = divisor << clockShift;
        }
    }
}

void NoiseChannel::clear() {
    NR41 = 0xFF;
    NR42 = 0x00;
    NR43 = 0x00;
    NR44 = 0xFB;

    timer = 0;
    lengthTimer = 0;
    envelopeTimer = 0;
    periodDivider = 0;
    volume = 0;
    lfsr = 0x7FFF; // Not sure if all bits should be cleared or reset
}
