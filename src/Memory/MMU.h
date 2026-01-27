#ifndef MMU_H
#define MMU_H

#include <cstdint>
#include <array>
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <ios>

#include "../constants.h"
#include "../APU/APU.h"
#include "../GPU/GPU.h"
#include "../IO/Joypad.h"
#include "../Cartridge/Cartridge.h"
#include "../IO/Timer.h"
#include "../Cartridge/MBC/NOMBC.h"
#include "../Cartridge/MBC/MBC1.h"
#include "../Cartridge/MBC/MBC3.h"
#include "../Cartridge/MBC/MBC5.h"
#include "../IO/Interrupts.h"
#include "../gb_global.h"


class MMU {
    public:
        MMU();
        ~MMU();

        void connect(GPU *gpu, Joypad *joypad, Timer *timer, APU *apu, Interrupts *interrupt);
        void loadRom(const char *filename);
        uint8_t read8(uint16_t address);
        void write8(uint16_t address, uint8_t data);

        uint8_t getIF();
        uint8_t getIE();
        void setIF(uint8_t val);

        void OAMDMATransfer();
        void HDMATransfer(bool halt, uint8_t numBytes);
    private:
        std::unique_ptr<Cartridge> rom; // ROM Banks + External RAM
        GPU *gpu; // VRAM + OAM
        Joypad *joypad; // Input
        Timer *timer; // Timer
        APU *apu;
        Interrupts *interrupt;

        // Work Ram
        std::vector<std::vector<uint8_t>> wram;
        uint8_t wramBank = 1;

        uint8_t serialByte;

        // High Ram
        std::array<uint8_t, 0x7F> hram = {};

        uint8_t key1 = 0;


        void setMBC(int type, std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romData, int romSize, int extRamSize, std::string title);
        uint8_t readWRAM(uint16_t address);
        void writeWRAM(uint16_t address, uint8_t data);

        uint8_t lastOAMByte;
};






#endif