#include "MMU.h"
#include "MBC/NOMBC.h"

#include <fstream>
#include <iostream>
#include <ios>

// MMU::MMU()
// {
// }

// MMU::~MMU()
// {
// }

void MMU::connect(GPU *gpu, Joypad *joypad, Timer *timer, APU *apu) {
    this->gpu = gpu;
    this->joypad = joypad;
    this->timer = timer;
    this->apu = apu;

}

int MMU::loadRom(const char *filename) {

    std::ifstream file;
    std::ifstream stream(filename, std::ios::binary | std::ios::ate);
    int romSize;
    int currentRomBank;
    std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romData; // Initial ROM Bank

    if (stream.is_open()) {
        char byte;
        romSize = (int)stream.tellg();
        std::cout << "Stream file size: " << romSize << "\n"; // File size
        romData = std::vector<std::array<uint8_t, ROM_BANK_SIZE>>(romSize / (ROM_BANK_SIZE));
        
        stream.seekg(0, std::ios::beg);

        for (int i = 0; i < romSize; i++) {
            currentRomBank = i / ROM_BANK_SIZE;
            char byte;
            stream.get(byte);
            romData[currentRomBank][i % ROM_BANK_SIZE]= byte;
        }
        stream.close();
    } else {
        std::cout << "Bad ROM" << "\n";
        exit(1);
    }

    // Sanity check - Title
	char title[17];
	title[16] = 0x00;
	for (int i = 0; i < 16; i++) {
		title[i] = (char)romData[0][0x134 + i];
	}
	std::cout << "Title: " << title << "\n";

    int cgb = (int)romData[0][0x143];

    std::cout << "CGB Flag: " << cgb << "\n";

    // Sanity Check - MBC type
    int type = (int)romData[0][0x147];
    std::cout << "Catridge Type: " << type << "\n";

    // Remember to save battery here as it is for save states

    // Sanity Check - SRAM Size
    int sRamSize = (int)romData[0][0x149];
    std::cout << "SRAM Size: " << sRamSize << "\n";

    

    // Sanity Check - ROM Size
	int headerRomSize = (int)romData[0][0x148];
	// Print
	std::cout << "ROM Size: " << 32 * (1 << headerRomSize) << "KB\n";


    // Determines MBC Type
    setMBC(type, romData, sRamSize);

    return cgb;

}

void MMU::setMBC(int type, std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romData, int sRamSize) {
    switch (type) {
        case 0x00: // ROM ONLY
            std::cout << "MBC Type: NOMBC\n";
            this->rom = std::make_unique<NOMBC>(romData, sRamSize);
            break;





        default:
            std::cout << "No MBC type found, defaulting to MBC1\n";
            break;
    }
}


uint8_t MMU::read8(uint16_t address)
{
    // REMEMBER TO CONSIDER OFFSETS
    // ROM Bank
    if (address <= 0x7FFF) {
        return this->rom->read(address);
    }
    
    // VRAM
    if (address <= 0x9FFF) {
        return 0;
    }

    // External RAM
    if (address <= 0xBFFF) {
        return 0;
    }

    // WRAM
    if (address <= 0xDFFF) {
        return 0;
    }

    // Echo RAM
    if (address <= 0xFDFF) {
        return 0;
    }

    // OAM RAM
    if (address <= 0xFE9F) {
        return 0;
    }

    // I/O Registers
    if (address <= 0xFF7F) {
        return 0;
    }

    // High RAM
    if (address <= 0xFFFE) {
        return 0;
    }

    // IE
    if (address == 0xFFFF) {
        return 0;
    }

    // Invalid Read
    return 0;
}

uint16_t MMU::read16(uint16_t address)
{
    return (read8(address + 1) << 8) | read8(address);
}



void MMU::write8(uint16_t address, uint8_t data)
{
}

void MMU::write16(uint16_t address, uint16_t data)
{
    write8(address, data & 0xFF); // Low bit
    write8(address + 1, (data >> 8) & 0xFF); // High bit
}
