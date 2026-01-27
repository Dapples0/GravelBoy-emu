#include "MBC3.h"

extern struct gb_global gb_global;

MBC3::MBC3(std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romData, int romSize, int extRamCode, bool timer) {
    this->romBank = romData;
    this->romSize = romSize;
    this->ramBank = getRamBank(extRamCode);
    this->ramEnable = false;
    this->romBankNumber = 1;
    this->ramBankRTCSelect = 0;
    this->RTC = timer;
}

MBC3::~MBC3() {

}

uint8_t MBC3::read(uint16_t address) {
    // Read from ROM Bank 0
    if (address >= 0x0000 && address <= 0x3FFF) {
        return romBank[0][address];
    }
    // Read from ROM Bank 1-NN
    else if (address >= 0x4000 && address <= 0x7FFF) {
        return romBank[romBankNumber][address % ROM_BANK_SIZE];
    }
    // (External RAM) RAM Bank 00-07 or RTC Registers
    else if (address >= 0xA000 && address <= 0xBFFF) {
        if (!ramEnable || ramSize == 0 || ramBank.size() == 0) {
            return 0xFF;
        }
        uint16_t relative_address = address & 0x1FFF;
        if (ramBankRTCSelect <= 0x07) {
            return ramBank[ramBankRTCSelect % ramBank.size()][relative_address];
        }
        else if (ramBankRTCSelect >= 0x08 && ramBankRTCSelect <= 0x0C) {
            return RTCRegisters[ramBankRTCSelect - 0x08];
        }

    }
    return 0xFF;
}


void MBC3::write(uint16_t address, uint8_t data) {
    // RAM Enable (Write Only)
    if (address <= 0x1FFF) {
        if ((data & 0x0F) == 0x0A) {
            ramEnable = true;
        } else {
            ramEnable = false;
            if (battery) MBC3Save();
        }        
    }
    // ROM Bank Number (Write Only) - Selects which ROM Bank to use
    else if (address >= 0x2000 && address <= 0x3FFF) {
        romBankNumber = (data & 0x7F) == 0 ? 1 : (data & 0x7F);
    }
    // RAM Bank Number / RTC Register Select
    else if (address >= 0x4000 && address <= 0x5FFF) {
        ramBankRTCSelect = data;
    }
    // Latch Clock Data
    else if (address >= 0x6000 && address <= 0x7FFF) {
        // If latch is 0 and 1 is written to it we latch RTC
        if (latch == 0x00 && data == 0x01) {
            latchTimer();
        }
        latch = data;
    }
    // (External RAM) RAM Bank 00-07 or RTC Registers
    else if (address >= 0xA000 && address <= 0xBFFF) {
        if (ramSize == 0 || !ramEnable || ramBank.size() == 0) {
            return;
        }
        uint16_t relative_address = address & 0x1FFF;
        if (ramBankRTCSelect <= 0x07) {
            ramBank[ramBankRTCSelect % ramBank.size()][relative_address] = data;
        }
        else if (ramBankRTCSelect >= 0x08 && ramBankRTCSelect <= 0x0C) {
            if (ramBankRTCSelect == 0x08) {
                rtSec = data;
            }
            else if (ramBankRTCSelect == 0x09) {
                rtMin = data;
            } 
            else if (ramBankRTCSelect == 0x0A) {
                rtHours = data;
            }
            else if (ramBankRTCSelect == 0x0B) {
                rtDaysLower = data;
            }
            else if (ramBankRTCSelect == 0x0C) {
                rtDaysUpper = data;
            }
            updateTimer();
        }
    }

}

void MBC3::setBattery(std::string title) {
    path = title;

    // handles cases where cgb enhancement version of a game shares the same title of the dmg version
    if (gb_global.cgb) path.append("cgb.sav");
    else path.append(".sav");
    path = "saves/" + path;
    std::cout << "Save filepath: " << path << "\n";
    std::string folder = "saves/";
    if (!std::filesystem::exists(folder)) std::filesystem::create_directories(folder);

    if (!RTC) {
        battery = loadSave();
    } else {
        std::ifstream stream(path, std::ios::binary | std::ios::ate);
        if (stream.is_open()) {
            // Subtract 5 to consider RTC store in file and 8 to consider time
            if (ramSize == (int)stream.tellg() - sizeof(time_t) - RTCRegisters.size()) {
                stream.seekg(0, std::ios::beg);
                uint8_t byte;
                for (int i = 0; i < ramSize; ++i) {
                    // Doesn't handle cartridges with 2 KiB of external ram, but in theory no retail cartridges use it
                    int curBank = i / SRAM_BANK_SIZE;
                    byte= stream.get();
                    ramBank[curBank][i % SRAM_BANK_SIZE] = byte;
                }
                
                // RTC Registers
                for (int i = 0; i < 5; ++i) {
                    byte = stream.get();
                    RTCRegisters[i] = byte;
                }

                // Get cur time
                stream.read(reinterpret_cast<char*>(&curTime), sizeof(time_t));
                
                battery = true;
                updateTimer();
            } else {
                battery = false;
            }
        } else {
            // No save found
            battery = true;
            updateTimer();
        }
        stream.close();

        
    }
}

void MBC3::updateTimer() {
    if (!RTC) return;
    time_t now = time(nullptr);

    // Implementaion of timer update is taken from Ghost Boy implementation by GhostSonic21
    // May need to update/overhaul as RTC doesn't always function correctly in games
    if ((rtDaysUpper & 0x40) == 0x40) {
        curTime = now;
        return;
    }

    time_t diff = (unsigned int)(now - curTime);

    curTime = now;

    unsigned int seconds = rtSec + diff;
    rtSec = seconds % 60;

    unsigned int minutes = rtMin + (seconds / 60);
    rtMin = minutes % 60;

    unsigned int hours = rtHours + (minutes / 60);
    rtHours = hours % 24;

    unsigned int days = rtDaysLower | ((rtDaysUpper & 0x01) << 8);
    days += (hours / 24);

    rtDaysLower = days & 0xFF;
    rtDaysUpper = (rtDaysUpper & 0xFE) | ((days >> 8) & 0x01);

    if (days > 511) {
        rtDaysUpper |= 0x80;
    }


}

void MBC3::latchTimer() {
    updateTimer();
    RTCRegisters[0] = rtSec;
    RTCRegisters[1] = rtMin;
    RTCRegisters[2] = rtHours;
    RTCRegisters[3] = rtDaysLower;
    RTCRegisters[4] = rtDaysUpper;
}

void MBC3::MBC3Save() {
    if (!RTC) {
        save();
    }
    else {
        updateTimer();
        std::ofstream stream(path, std::ios::out | std::ios::binary);
        if (stream.is_open()) {
            char byte;
            for (int i = 0; i < ramSize; ++i) {
                // Doesn't handle cartridges with 2 KiB of external ram, but in theory no retail cartridges use it
                int curBank = i / SRAM_BANK_SIZE;
                byte = (char)ramBank[curBank][i % SRAM_BANK_SIZE];
                stream.write(&byte, 1);
            }

            // RTC Save
            for (int i = 0; i < 5; ++i) {
                byte = (char)RTCRegisters[i];
                stream.write(&byte, 1);
            }

            // Cur Time Save, 8 bytes
            stream.write(reinterpret_cast<const char*>(&curTime), sizeof(time_t));
        }
        stream.close();

    }
}
