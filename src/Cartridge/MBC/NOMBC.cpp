#include "NOMBC.h"
#include <iostream>
NOMBC::NOMBC(std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romData, int romSize, int extRamCode) {
    this->romBank = romData;
    this->romSize = romSize;
    this->ramBank = getRamBank(extRamCode);
    this->ramSize = 0;
}

NOMBC::~NOMBC()
{
}

uint8_t NOMBC::read(uint16_t address)
{
    return romBank[address / ROM_BANK_SIZE][address % ROM_BANK_SIZE];
}

void NOMBC::write(uint16_t address, uint8_t data)
{
}
