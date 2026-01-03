#include "Cartridge.h"

#include <iostream>
Cartridge::Cartridge()
{
}

// Does nothing -> let subclass override the function
uint8_t Cartridge::read(uint16_t address)
{
    return 0;
}

// Does nothing -> let subclass override the function
void Cartridge::write(uint16_t address, uint8_t data)
{
}

std::vector<std::vector<uint8_t>> Cartridge::getRamBank(int sRamSize) {
    int numBanks;
    uint16_t bankSize = SRAM_BANK_SIZE;
    switch (sRamSize) {
        case 0x00:
            numBanks = 0;
            break;

        case 0x01:
            numBanks = 1;
            bankSize = SRAM_UNUSED_BANK_SIZE;
            break;

        case 0x02:
            numBanks = 1;
            break;
        
        case 0x03:
            numBanks = 4;
            break;
        
        case 0x04:
            numBanks = 16;
            break;

        case 0x05:
            numBanks = 8;
            break;

        default: // Default to 0x01
            numBanks = 1;
            break;
    }

    return std::vector<std::vector<uint8_t>>(numBanks, std::vector<uint8_t>(bankSize));
}
