#ifndef MBC5_H
#define MBC5_H

#include <iostream>
#include "../Cartridge.h"
#include "../../gb_global.h"

class MBC5 : public Cartridge {
    public:
        MBC5(std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romData, int romSize, int extRamCode);
        ~MBC5();

        uint8_t read(uint16_t address) override;
        void write(uint16_t address, uint8_t data) override;
        void setBattery(std::string title) override;       
    private:

    // Registers
    bool ramEnable; // Determines whether external RAM can be read or written
    uint16_t romBankNumber;
    uint8_t ramBankNumber;
    bool bankingMode; // false -> default, true -> advanced
};


#endif