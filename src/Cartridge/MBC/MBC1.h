#ifndef MBC1_H
#define MB1C_H

#include "../../constants.h"

#include "../Cartridge.h"

class MBC1 : public Cartridge {
    public:
        MBC1(std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romData, int romSize, int sRamSize);
        ~MBC1();

        uint8_t read(uint16_t address) override;
        void write(uint16_t address, uint8_t data) override;        
    private:
        bool battery = false;

        // Registers
        bool ramWrite; // Determines whether externaml RAM can be read or written
        uint8_t romBankNumber;
        uint8_t ramBankNumber;
        bool bankingMode; // false -> default, true -> advanced

};


#endif