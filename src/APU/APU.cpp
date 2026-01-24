#include "APU.h"

APU::APU() : square1(), square2(), wave(), noise() {
}

APU::~APU() {
}

uint8_t APU::read(uint16_t address)
{
    uint8_t res = 0xFF;
    if (address == 0xFF26) {
        // Should update NR52 to check if lower nibbles are set
        res = NR52;
    }
    // Sound Panning
    else if (address == 0xFF25) {
        res = NR51;
    }
    // Master volume & VIN Panning
    else if (address == 0xFF24) {
        res = NR50;
    }
    // Sound Channel 1
    else if (address >= 0xFF10 && address <= 0xFF14) {
        res = square1.read(address);
    }
    // Sound Channel 2
    else if (address >= 0xFF16 && address <= 0xFF19) {
        res = square2.read(address);
    }
    // Wave Channel
    else if (address >= 0xFF1A && address <= 0xFF1E) {
        res = wave.read(address);
    }
    // Noise Channel
    else if (address >= 0xFF20 && address <= 0xFF23) {
        res = noise.read(address);
    }
    // Wave Pattern RAM
    else if (address >= 0xFF30 && address <= 0xFF3F) {
        res = wave.read(address);
    }
    return res;
}

void APU::write(uint16_t address, uint8_t data) {
    if (address == 0xFF26) {
        if ((data & 0x80) == 0x80) {
            NR52 |= 0x80;
        } else {
            // Clear all registers and timer (if any)
            clear();
        }
        return;
    }
    // Audio registers become read only if audio is off
    // Sound Panning
    if (address == 0xFF25 && (NR52 & 0x80) != 0x00) {
        NR51 = data;
    }
    // Master volume & VIN Panning
    else if (address == 0xFF24 && (NR52 & 0x80) != 0x00) {
        NR50 = data;
    }
    // Sound Channel 1
    else if (address >= 0xFF10 && address <= 0xFF14 && (NR52 & 0x80) != 0x00) {
        square1.write(address, data);
    }
    // Sound Channel 2
    else if (address >= 0xFF16 && address <= 0xFF19 && (NR52 & 0x80) != 0x00) {
        square2.write(address, data);
    }
    // Wave Channel
    else if (address >= 0xFF1A && address <= 0xFF1E && (NR52 & 0x80) != 0x00) {
        wave.write(address, data);
    }
    // Noise Channel
    else if (address >= 0xFF20 && address <= 0xFF23 && (NR52 & 0x80) != 0x00) {
        noise.write(address, data);
    }
    // Wave Pattern RAM
    else if (address >= 0xFF30 && address <= 0xFF3F) {
        wave.write(address, data);
    }
}

void APU::tick(uint8_t cycles) {

}

void APU::clear() {
    NR52 = 0x00;
    NR51 = 0x00;
    NR50 = 0x00;

    square1.clear();
    square2.clear();
    wave.clear();
    noise.clear();
}
