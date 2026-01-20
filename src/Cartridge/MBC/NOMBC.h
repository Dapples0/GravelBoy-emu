#ifndef NOMBC_H
#define NOMBC_H

#include "../Cartridge.h"

class NOMBC : public Cartridge {
    public:
        NOMBC(std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romData, int romSize, int extRamCode);
        ~NOMBC();

        uint8_t read(uint16_t address) override;
        void write(uint16_t address, uint8_t data) override;        
    private:
};


#endif