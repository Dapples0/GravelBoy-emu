#include "NOMBC.h"
#include <iostream>
NOMBC::NOMBC(std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romData, int romSize,  int sRamSize) {
    this->romBank = romData;
    this->romSize = romSize;
    this->ramBank = getRamBank(sRamSize);
    this->ramSize = 0;
}

NOMBC::~NOMBC()
{
}

uint8_t NOMBC::read(uint16_t address)
{
    std::cout << "Reading ROM bank : " <<  address / ROM_BANK_SIZE << "\n";
    std::cout << "Address Location: " << address % ROM_BANK_SIZE << "\n";
    return romBank[address / ROM_BANK_SIZE][address % ROM_BANK_SIZE];
}

void NOMBC::write(uint16_t address, uint8_t data)
{
}
