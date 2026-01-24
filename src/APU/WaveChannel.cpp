#include "WaveChannel.h"

WaveChannel::WaveChannel()
{
}

WaveChannel::~WaveChannel()
{
}

uint8_t WaveChannel::read(uint16_t address) {
    uint8_t res = 0xFF;
    if (address == 0xFF1A) {
        res = NR30 | 0x7F;
    } else if (address == 0xFF1B) {
        res = 0xFF;
    } else if (address == 0xFF1C) {
        res = NR32 | 0x9F;
    } else if (address == 0xFF1D) {
        res = 0xFF;
    } else if (address == 0xFF1E) {
        res = NR34 | 0xBF;
    } else if (address >= 0xFF30 && address <= 0xFF3F) {
        if (active) return res;
        uint16_t relative_address = address & 0x000F;
        res = wavePatternRAM[relative_address];
    }
    return res;
}

void WaveChannel::write(uint16_t address, uint8_t data) {
    if (address == 0xFF1A) {
        NR30 = data;
        active = (NR30 & 0x80) == 0x80;
    } else if (address == 0xFF1B) {
        NR31 = data;
    } else if (address == 0xFF1C) {
        NR32 = data;
    } else if (address == 0xFF1D) {
        NR33 = data;
    } else if (address == 0xFF1E) {
        NR34 = data;

        if ((data & 0x80) == 0x80) {
            active = true;

            if (lengthTimer == 0) lengthTimer = 256;

            uint16_t periodDivider = NR33 | ((NR34 & 0x07) << 8);

            volume = (NR32 >> 5) & 0x03;
            
            timer = (2048 - periodDivider) * 2;
            
            wavePosition = 0;


        }
    } else if (address >= 0xFF30 && address <= 0xFF3F) {
        uint16_t relative_address = address & 0x000F;
        wavePatternRAM[relative_address] = data;
    }
}

void WaveChannel::clear() {
    NR30 = 0x00;
    NR31 = 0x00;
    NR32 = 0x00;
    NR33 = 0x00;
    NR34 = 0x00;

    active = false;
    periodDivider = 0;
    wavePosition = 0;
    volume = 0;
    lengthTimer = 0;
}
