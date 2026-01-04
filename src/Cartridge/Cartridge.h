#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <vector>
#include <array>

#include "../constants.h"

class Cartridge {
    public:
        Cartridge();
        virtual ~Cartridge() = default;

        virtual uint8_t read(uint16_t address);
        virtual void write(uint16_t address, uint8_t data);        

    protected:
        // ROM Banks
        std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romBank;

        // Catridge External Ram Banks
        std::vector<std::vector<uint8_t>> ramBank;

        std::vector<std::vector<uint8_t>> getRamBank(int sRamSize);

        int romSize;
        int ramSize;
};






#endif