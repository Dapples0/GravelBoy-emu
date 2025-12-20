#ifndef MMU_H
#define MMU_H

#include <cstdint>
#include <array>
#include <memory>

#include "constants.h"
#include "APU.h"
#include "GPU.h"
#include "Joypad.h"
#include "Cartridge.h"
#include "Timer.h"


class MMU {
    public:
        // MMU();
        // ~MMU();

        void connect(GPU *gpu, Joypad *joypad, Timer *timer, APU *apu);
        int loadRom(const char *filename);
        uint8_t read8(uint16_t address);
        uint16_t read16(uint16_t address);
        void write8(uint16_t address, uint8_t data);
        void write16(uint16_t address, uint16_t data);
    private:
        std::unique_ptr<Cartridge> rom; // ROM Banks + External RAM
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
        uint8_t ie = 0x00;
        uint8_t ieFlag = 0x00;

        void setMBC(int type, std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romData, int sRamSize);
};






#endif