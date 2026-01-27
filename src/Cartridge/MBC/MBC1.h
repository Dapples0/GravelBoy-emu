#ifndef MBC1_H
#define MB1C_H

#include "../Cartridge.h"
#include "../../constants.h"
#include "../../gb_global.h"


class MBC1 : public Cartridge {
    public:
        MBC1(std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romData, int romSize, int extRamCode);
        ~MBC1();

        uint8_t read(uint16_t address) override;
        void write(uint16_t address, uint8_t data) override;
        void setBattery(std::string title) override;   
    private:

        // Registers
        bool ramEnable; // Determines whether external RAM can be read or written
        uint8_t romBankNumber;
        uint8_t ramBankNumber;
        bool bankingMode; // false -> default, true -> advanced
};


#endif