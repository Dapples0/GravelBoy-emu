#ifndef MMU_H
#define MMU_H

#include <cstdint>
#include <array>
#include <memory>

#include "../constants.h"
#include "../APU/APU.h"
#include "../GPU/GPU.h"
#include "../IO/Joypad.h"
#include "../Cartridge/Cartridge.h"
#include "../IO/Timer.h"
#include "../Cartridge/MBC/NOMBC.h"
#include "../Cartridge/MBC/MBC1.h"
#include "../IO/Interrupts.h"


class MMU {
    public:
        MMU();
        ~MMU();

        void connect(GPU *gpu, Joypad *joypad, Timer *timer, APU *apu, Interrupts *interrupt);
        bool loadRom(const char *filename);
        uint8_t read8(uint16_t address);
        uint16_t read16(uint16_t address);
        void write8(uint16_t address, uint8_t data);
        void write16(uint16_t address, uint16_t data);

        uint8_t getIF();
        uint8_t getIE();
        void setIF(uint8_t val);
        void setIE(uint8_t val);

        void tick(uint8_t val);
        int cycles = 0;
        // Debugging read
        uint8_t readPeek(uint16_t address);
    private:
        std::unique_ptr<Cartridge> rom; // ROM Banks + External RAM
        GPU *gpu; // VRAM + Echo RAM + OAM
        Joypad *joypad; // Input
        Timer *timer; // Timer
        APU *apu;
        Interrupts *interrupt;

        // Work Ram
        std::vector<std::vector<uint8_t>> wram;
        uint8_t wramBank = 1;

        // I/O Registers
        std::array<uint8_t, 0x80> io;
        uint8_t serialByte;

        // High Ram
        std::array<uint8_t, 0x7F> hram;

        // CGB Flag -> also present in CPU
        bool cgb;

        uint8_t key1 = 0;


        void setMBC(int type, std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romData, int romSize, int sRamSize);
        uint8_t readWRAM(uint16_t address);
        void writeWRAM(uint16_t address, uint8_t data);
};






#endif