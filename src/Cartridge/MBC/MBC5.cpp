#include "MBC5.h"

MBC5::MBC5(std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romData, int romSize, int extRamCode) {
    this->romBank = romData;
    this->romSize = romSize;
    this->ramBank = getRamBank(extRamCode);
    this->bankingMode = false;
    this->ramEnable = false;
    this->romBankNumber = 0;
    this->ramBankNumber = 0;
}

MBC5::~MBC5() {
}

uint8_t MBC5::read(uint16_t address) {
    // ROM Bank 00
    if (address >= 0x0000 && address <= 0x3FFF) {
        return romBank[0][address];
    }
    // ROM Bank 00-1FF
    else if (address >= 0x4000 && address <= 0x7FFF) {
        return romBank[romBankNumber][address % ROM_BANK_SIZE];
    }
    // RAM Bank 00-0F
    else if (address >= 0xA000 && address <= 0xBFFF) {
        if (ramSize == 0 || !ramEnable || ramBank.size() == 0) {
            return 0xFF;
        }

        uint16_t relative_address = address & 0x1FFF;
        return ramBank[ramBankNumber % ramBank.size()][relative_address];
    }
    return 0xFF;
}

void MBC5::write(uint16_t address, uint8_t data) {
    // RAM Enable
    if (address >= 0x0000 && address <= 0x1FFF) {
        if ((data & 0x0F) == 0x0A) {
            ramEnable = true;
        } else {
            ramEnable = false;
            if (battery) save();
        }  
    }
    // 8 least significant bits of ROM Bank Number
    else if (address >= 0x2000 && address <= 0x2FFF) {
        romBankNumber = (romBankNumber & 0x100) | data;
    }
    // 9th bit of ROM Bank Number
    else if (address >= 0x3000 && address <= 0x3FFF) {
        romBankNumber = (romBankNumber & 0xFF) | ((data && 0x1) << 9);
    }
    // RAM Bank Number
    else if (address >= 0x4000 && address <= 0x5FFF) {
        ramBankNumber = (data & 0x0F);
    }
    // RAM Bank 00-0F
    else if (address >= 0xA000 && address <= 0xBFFF) {
        if (ramSize == 0 || !ramEnable || ramBank.size() == 0) {
            return;
        }

        uint16_t relative_address = address & 0x1FFF;
        ramBank[ramBankNumber % ramBank.size()][relative_address] = data;
    }
    
}

void MBC5::setBattery(std::string title, bool cgb) {
    path = title;

    // handles cases where cgb enhancement version of a game shares the same title of the dmg version
    if (cgb) path.append("cgb.sav");
    else path.append(".sav");
    path = "saves/" + path;
    std::cout << "Save filepath: " << path << "\n";
    std::string folder = "saves/";
    if (!std::filesystem::exists(folder)) std::filesystem::create_directories(folder);

    battery = loadSave();
}
