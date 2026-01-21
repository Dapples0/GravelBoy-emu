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

std::vector<std::vector<uint8_t>> Cartridge::getRamBank(int extRamSize) {
    int numBanks;
    uint16_t bankSize = SRAM_BANK_SIZE;
    switch (extRamSize) {
        case 0x00:
            numBanks = 0;
            ramSize = 0;
            break;

        case 0x01:
            numBanks = 1;
            bankSize = SRAM_UNUSED_BANK_SIZE;
            ramSize = SRAM_UNUSED_BANK_SIZE;
            break;

        case 0x02:
            numBanks = 1;
            ramSize = SRAM_BANK_SIZE;
            break;
        
        case 0x03:
            numBanks = 4;
            ramSize = SRAM_BANK_SIZE * numBanks;
            break;
        
        case 0x04:
            numBanks = 16;
            ramSize = SRAM_BANK_SIZE * numBanks;
            break;

        case 0x05:
            numBanks = 8;
            ramSize = SRAM_BANK_SIZE * numBanks;
            break;

        default: // Default to 0x02
            numBanks = 1;
            ramSize = SRAM_BANK_SIZE;
            break;
    }
    std::cout << "Number of RAM Banks: " << numBanks << "\n";
    std::cout << "Bank Size: " << bankSize / 1000 << " KB" << "\n";
    std::cout << "Total Ram Size: " << ramSize / 1000 << " KB" << "\n";
    return std::vector<std::vector<uint8_t>>(numBanks, std::vector<uint8_t>(bankSize));
}

// Does nothing -> let subclass override the function
void Cartridge::setBattery(std::string title, bool cgb) {
}

bool Cartridge::loadSave() {
    bool loaded = false;
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (stream.is_open()) {
        if (ramSize == (int)stream.tellg()) {
            stream.seekg(0, std::ios::beg);
            for (int i = 0; i < ramSize; ++i) {
                // Doesn't handle cartridges with 2 KiB of external ram, but in theory no retail cartridges use it
                int curBank = i / SRAM_BANK_SIZE;
                uint8_t byte = stream.get();
                ramBank[curBank][i % SRAM_BANK_SIZE] = byte;
            }

            loaded = true;
        } else {
            loaded = false;
        }
    } else {
        // No save found
        loaded = true;
    }

    stream.close();
    return loaded;
}

void Cartridge::save() {
    std::ofstream stream(path, std::ios::out | std::ios::binary);
    if (stream.is_open()) {
        for (int i = 0; i < ramSize; ++i) {
            // Doesn't handle cartridges with 2 KiB of external ram, but in theory no retail cartridges use it
            int curBank = i / SRAM_BANK_SIZE;
            char byte = (char)ramBank[curBank][i % SRAM_BANK_SIZE];
            stream.write(&byte, 1);
        }
    }
    stream.close();


    
}