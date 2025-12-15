#ifndef MMU_H
#define MMU_H

#include <cstdint>
#include <array>

#include "constants.h"
#include "APU.h"
#include "GPU.h"
#include "Joypad.h"
#include "Cartridge.h"
#include "Timer.h"


class MMU {
    public:
        MMU();
        ~MMU();

    private:
        Cartridge *rom; // ROM Banks + External RAM
        GPU *gpu; // VRAM + Echo RAM + OAM
        Joypad *joypad; // Input
        Timer *timer; // Timer
        APU *apu;

        // Work Ram
        std::array<std::array<uint8_t, 0x1000>, 8> wram;
        uint8_t wramBank;

        // I/O Registers
        std::array<uint8_t, 0x80> io;

        // High Ram
        std::array<uint8_t, 0x7F> hram;

        // Interrupt Enable Registers
        uint8_t ie;
        uint8_t ieFlag;

};






#endif