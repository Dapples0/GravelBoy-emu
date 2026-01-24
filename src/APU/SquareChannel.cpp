#include "SquareChannel.h"

SquareChannel::SquareChannel()
{
}

SquareChannel::~SquareChannel()
{
}

uint8_t SquareChannel::read(uint16_t address) {
    uint8_t res = 0xFF;
    if (address == 0xFF16) {
        res = NR21 | 0x3F;
    } else if (address == 0xFF17) {
        res = NR22;
    } else if (address == 0xFF18) {
        res = 0xFF;
    } else if (address == 0xFF19) {
        // Only Length Enable can be read
        res = NR24 | 0xBF;
    }
    return res;
}

void SquareChannel::write(uint16_t address, uint8_t data) {
    if (address == 0xFF16) {
        NR21 = data;
        lengthTimer = 64 - (NR21 & 0x3F);
    } else if (address == 0xFF17) {
        NR22 = data;
        if ((data & 0xF8) == 0) active = false;
    } else if (address == 0xFF18) {
        NR23 = data;
    } else if (address == 0xFF19) {
        NR24 = data;
        if ((data & 0x80) == 0x80) {
            active = true;

            if (lengthTimer == 0) lengthTimer = 64;

            envelopeTimer = NR22 & 0x07;

            volume = (NR22 >> 4) & 0x0F;

            periodDivider = NR23 | ((NR24 & 0x07) << 8);

            timer = (2048 - periodDivider) * 4;

            dutyPosition = 0;
        }

    }
}

void SquareChannel::clear() {
    NR21 = 0x3F;
    NR22 = 0x00;
    NR23 = 0xFF;
    NR24 = 0xBF;

    active = false;
    lengthTimer = 0;
    envelopeTimer = 0;
    volume = 0;
    timer = 0;
    dutyPosition = 0;
    periodDivider = 0;
}
