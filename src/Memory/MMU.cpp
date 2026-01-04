#include "MMU.h"

#include <fstream>
#include <iostream>
#include <ios>

MMU::MMU()
{
}

MMU::~MMU()
{
}

void MMU::connect(GPU *gpu, Joypad *joypad, Timer *timer, APU *apu, Interrupts *interrupt) {
    this->gpu = gpu;
    this->joypad = joypad;
    this->timer = timer;
    this->apu = apu;
    this->interrupt = interrupt;
}

bool MMU::loadRom(const char *filename) {

    std::ifstream file;
    std::ifstream stream(filename, std::ios::binary | std::ios::ate);
    int romSize;
    int currentRomBank;
    std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romData; // Initial ROM Bank

    if (stream.is_open()) {
        romSize = (int)stream.tellg();
        std::cout << "Stream file size: " << romSize << "\n"; // File size
        romData = std::vector<std::array<uint8_t, ROM_BANK_SIZE>>(romSize / (ROM_BANK_SIZE));
        
        stream.seekg(0, std::ios::beg);

        for (int i = 0; i < romSize; i++) {
            currentRomBank = i / ROM_BANK_SIZE;
            uint8_t bbyte = stream.get();
            romData[currentRomBank][i % ROM_BANK_SIZE]= bbyte;
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
	std::cout << "ROM Size: " << 32 * (1 << headerRomSize) << "KB\n";

    // Determines MBC Type
    setMBC(type, romData, romSize, sRamSize);
    // this->cgb = cgb == 0xC0 || cgb == 0x80;
    this->cgb = false;
    this->wram = this->cgb 
    ? std::vector<std::vector<uint8_t>>(8, std::vector<uint8_t>(WRAM_BANK_SIZE)) 
    : std::vector<std::vector<uint8_t>>(2, std::vector<uint8_t>(WRAM_BANK_SIZE));
    return cgb == 0xC0 || cgb == 0x80;

}

void MMU::setMBC(int type, std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romData, int romSize, int sRamSize) {
    switch (type) {
        case 0x00: // ROM ONLY
            std::cout << "MBC Type: NOMBC\n";
            this->rom = std::make_unique<NOMBC>(romData, romSize, sRamSize);
            break;

        case 0x01: // MBC1
            std::cout << "MBC Type: MBC1\n";
            this->rom = std::make_unique<MBC1>(romData, romSize, 0);
            break;

        case 0x02: // MBC1 + RAM
            std::cout << "MBC Type: MBC1 + RAM\n";
            this->rom = std::make_unique<MBC1>(romData, romSize, sRamSize);
            break;     


        default:
            std::cout << "No MBC type found, defaulting to MBC1\n";
            break;
    }
}


uint8_t MMU::read8(uint16_t address)
{
    // std::cout << "Reading from " << address << "\n";
    // TEMP TODO: REMOVE
    if (address == 0xFF44) {
        return 0x90; // Fake a VBlank (144) so the test can proceed
    }
 
    // REMEMBER TO CONSIDER OFFSETS
    // ROM Bank
    if (address >= 0x0000 && address <= 0x7FFF) {
        // skip boot rom
        return this->rom->read(address);
    }
    
    // VRAM TODO
    if (address >= 0x8000 && address <= 0x9FFF) {
        return 0x0;
    }

    // External RAM TODO
    if (address >= 0xA000 && address <= 0xBFFF) {
        return this->rom->read(address);
    }

    // WRAM + Echo RAM
    if (address >= 0xC000 && address <= 0xFDFF) {

        return this->readWRAM(address);
    }

    // OAM RAM TODO
    if (address >= 0xFE00 && address <= 0xFE9F) {
        return 0x0;
    }
    // High RAM
    if (address >= 0xFF80 && address <= 0xFFFE) {
        return hram[address & 0x7F];
    }


    // Joypad TODO
    if (address == 0xFF00) {
        return 0xFF;
    }

    // Timer and Divider TODO
    if (address >= 0xFF04 && address <= 0xFF07) {
        return timer->read(address);
    }

    // IF
    if (address == 0xFF0F) {
        return this->interrupt->getIF();
    }

    // Audio TODO
    if (address >= 0xFF10 && address <= 0xFF3F) {
        return 0x0;

    }

    // GPU TODO
    if (address >= 0xFF40 && address <= 0xFF4B) {
        return 0x0;

    }

    // OAM DMA transfer TODO
    if (address == 0xFF46) {
        return 0x0;
    }

    // KEY1 TODO
    if (cgb && address == 0xFF4D) {
        return 0x0;
    }
    // VRAM Bank Select TODO
    if (cgb && address == 0xFF4F) {
        return 0x0;
    }
    // Boot ROM Map TODO
    if (address == 0xFF50) {}
    // VRAM DMA TODO
    if (cgb && address >= 0xFF51 && address <= 0xFF55) {
        return 0x0;
    }
    // BG / OBJ Palettes TOOD
    if (cgb && address >= 0xFF68 && address <= 0xFF6B) {
        return 0x0;
    }
    // WRAM Bank Select
    if (cgb && address == 0xFF70) {
        return wramBank | 0xF8;
    }
    // IE
    if (address == 0xFFFF) {
        return this->interrupt->getIE();
    }

    // Invalid Read
    return 0x0;
}

uint16_t MMU::read16(uint16_t address)
{
    return (read8(address + 1) << 8) | read8(address);
}



void MMU::write8(uint16_t address, uint8_t data) {
    // std::cout << "Writing to " << address << "\n";

    if (address >= 0x0000 && address <= 0x7FFF) {
        this->rom->write(address, data);
    }
    // VRAM TODO 
    else if (address >= 0x8000 && address <= 0x9FFF) {
    }
    // External RAM TODO
    else if (address >= 0xA000 && address <= 0xBFFF) {
        this->rom->write(address, data);
    }
    // WRAM -> for my implementation, wram bank setting shouldn't matter as wramBank = 0 is default on read/write
    else if (address >= 0xC000 && address <= 0xFDFF) {
        this->writeWRAM(address, data);
    }
    // OAM RAM TODO
    else if (address >= 0xFE00 && address <= 0xFE9F) {
    }
    // High Ram
    else if (address >= 0xFF80 && address <= 0xFFFE) {
        hram[address & 0x7F] = data;
    }
    // Joypad TODO
    else if (address == 0xFF00) {

    }
    // Serial Transfer Byte
    else if (address == 0xFF01) {
        serialByte = data;
    }
    // Serial Transfer Byte Debug Output
    else if (address == 0xFF02) {
        if (data & 0x80 || data & 0x81) {
            // Print the byte exactly as Blargg expects
            std::cout << (char)serialByte << std::flush;
        }
    }
    // Timer and Divider TODO
    else if (address >= 0xFF04 && address <= 0xFF07) {
        timer->write(address, data);
    }

    // IF
    else if (address == 0xFF0F) {
        this->interrupt->setIF(data);
    }
    // Audio TODO
    else if (address >= 0xFF10 && address <= 0xFF3F) {

    }
    // GPU TODO
    else if (address >= 0xFF40 && address <= 0xFF4B) {

    }
    // OAM DMA transfer TODO
    else if (address == 0xFF46) {}
    // KEY1 TODO
    else if (cgb && address == 0xFF4D) {}
    // VRAM Bank Select TODO
    else if (cgb && address == 0xFF4F) {}
    // Boot rom map TODO
    else if (address == 0xFF50) {}
    // VRAM DMA TODO
    else if (cgb && address >= 0xFF51 && address <= 0xFF55) {}
    // BG / OBJ Palettes TOOD
    else if (cgb && address >= 0xFF68 && address <= 0xFF6B) {}
    // WRAM Bank Select
    else if (cgb && address == 0xFF70) {
        wramBank = data & 0x07;
        if (cgb) {
            if (cgb && wramBank == 0) {
                wramBank = 1;
            }
        }
    }
    // IE
    else if (address == 0xFFFF) {
        this->interrupt->setIE(data);
    }
}

void MMU::write16(uint16_t address, uint16_t data)
{
    write8(address, data & 0xFF); // Low bit
    write8(address + 1, (data >> 8) & 0xFF); // High bit
}

uint8_t MMU::readWRAM(uint16_t address) {
    // Echo RAM handling
    if (address >= 0xE000 && address <= 0xFDFF) {
        address -= 0x2000;
    }
    uint16_t relative_address = address & 0x0FFF;

    // Read from Bank 0
    if (address >= 0xC000 && address <= 0xCFFF) {
        return wram[0][relative_address];
    }
    if (address >= 0xD000 && address <= 0xDFFF) {
        int bank;
        if (cgb) {
            bank = wramBank == 0 ? 1 : wramBank;
        } else {
            bank = 1;
        }
        return wram[bank][relative_address];
    }
    return 0xFF;
}

void MMU::writeWRAM(uint16_t address, uint8_t data) {
    // Echo RAM handling
    if (address >= 0xE000 && address <= 0xFDFF) {
        address -= 0x2000;
    }
    uint16_t relative_address = address & 0x0FFF;

    // Write to Bank 0
    if (address >= 0xC000 && address <= 0xCFFF) {
        wram[0][relative_address] = data;
    } 
    // Write to Bank 1-NN
    else if (address >= 0xD000 && address <= 0xDFFF) {
        int bank;
        if (cgb) {
            bank = wramBank == 0 ? 1 : wramBank;
        } else {
            bank = 1;
        }
        wram[bank][relative_address] = data;
    }
}