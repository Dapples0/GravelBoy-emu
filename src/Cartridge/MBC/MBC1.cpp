#include "MBC1.h"
#include <iostream>
MBC1::MBC1(std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romData, int romSize, int sRamSize) {
    this->romBank = romData;
    this->romSize = romSize;
    this->ramSize = sRamSize;
    this->ramBank = getRamBank(sRamSize);
    this->bankingMode = false;
    this->ramWrite = false;
    this->romBankNumber = 1;
    this->ramBankNumber = 0;
}

MBC1::~MBC1()
{
}

uint8_t MBC1::read(uint16_t address) {
    // Read from ROM Bank 0
    if (address >= 0x0000 && address <= 0x3FFF) {
        if (bankingMode) {
            return romBank[(ramBankNumber << 5) % romBank.size()][address % ROM_BANK_SIZE];
        } else {
            return romBank[0][address];
        }
    }
    // Read from ROM Bank 1-NN
    else if (address >= 0x4000 && address <= 0x7FFF) {
        return romBank[((ramBankNumber << 5) + romBankNumber) % romBank.size()][(address - 0x4000) % ROM_BANK_SIZE];

    }
    // Read from External RAM
    else if (address >= 0xA000 && address <= 0xBFFF) {
        
        if (ramSize == 0 || !ramWrite || ramBank.size() == 0) {
            return 0xFF;
        }
        uint16_t relative_address = address & 0x1FFF;
        if (bankingMode) {
            // return ramBank[ramBankNumber][address % SRAM_BANK_SIZE];
            return ramBank[ramBankNumber][(address - 0xA000) % SRAM_BANK_SIZE];

        } else {
            // return ramBank[0][address % SRAM_BANK_SIZE];
            return ramBank[0][address - 0xA000];

        }
    }

    std::cout << "Bad Read\n";
    return 0xFF;
    
}

void MBC1::write(uint16_t address, uint8_t data) {
    // RAM Enable (Write Only)
    if (address <= 0x1FFF) {
        if ((data & 0x0F) == 0x0A) {
            ramWrite = true;
        } else {
            ramWrite = false;
            // battery save here TODO
        }        
    } 
    // ROM Bank Number (Write Only) - Selects which ROM Bank to use
    if (address >= 0x2000 && address <= 0x3FFF) {
        romBankNumber = (data & 0x1F) == 0 ? 1 : (data & 0x1F);
    }
    // RAM Bank Number (Write Only) 
    if (address >= 0x4000 && address <= 0x5FFF) {
        ramBankNumber = data & 0x03;
    }
    // Banking Mode Select (Write Only)
    if (address >= 0x6000 && address <= 0x7FFF) {
		if (data == 0x00) {
			bankingMode = false;
		}
		else if (data == 0x01) {
			bankingMode = true;
		}
    }
    // Write to External RAM
    if (address >= 0xA000 && address <= 0xBFFF) {
        
        if (ramSize == 0 || !ramWrite || ramBank.size() == 0) {
            return;
        }
        uint16_t relative_address = address & 0x1FFF;
        if (bankingMode) {
            ramBank[ramBankNumber % ramBank.size()][(address - 0xA000) % SRAM_BANK_SIZE] = data;

        } else {
            ramBank[0][address - 0xA000] = data;
        }
    }
}
