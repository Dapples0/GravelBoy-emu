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
    }
    // Sanity check - Title
	char title[17];
	title[16] = 0x00;
	for (int i = 0; i < 16; i++) {
		title[i] = (char)romData[0][0x134 + i];
	}
	std::cout << "Title: " << title << "\n";

    // Sanity Check - CGB Flag
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
    bool cgbMode = cgb == 0xC0 || cgb == 0x80;
    // bool cgbMode = false;
    this->cgb = cgbMode;
    this->gpu->setCGBMode(cgbMode);
    this->wram = this->cgb 
    ? std::vector<std::vector<uint8_t>>(8, std::vector<uint8_t>(WRAM_BANK_SIZE)) 
    : std::vector<std::vector<uint8_t>>(2, std::vector<uint8_t>(WRAM_BANK_SIZE));
    return cgbMode;

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

        case 0x03: // MBC1 + RAM + Battery
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

    uint8_t res = 0x00;
    // std::cout << "Reading from " << address << "\n";
    // ROM Bank
    if (address >= 0x0000 && address <= 0x7FFF) {
        // skip boot rom
        res = this->rom->read(address);
    }
    
    // VRAM
    else if (address >= 0x8000 && address <= 0x9FFF) {
        res = gpu->readVRAM(address);
    }

    // External RAM
    else if (address >= 0xA000 && address <= 0xBFFF) {
        res = this->rom->read(address);
    }

    // WRAM + Echo RAM
    else if (address >= 0xC000 && address <= 0xFDFF) {

        res = this->readWRAM(address);
    }

    // OAM RAM
    else if (address >= 0xFE00 && address <= 0xFE9F) {
        res = this->gpu->readOAM(address);
    }
    // High RAM
    else if (address >= 0xFF80 && address <= 0xFFFE) {
        res = hram[address & 0x7F];
    }

    // Joypad
    else if (address == 0xFF00) {
        res = joypad->read();
    }

    // Timer and Divider
    else if (address >= 0xFF04 && address <= 0xFF07) {
        res = timer->read(address);
    }

    // IF
    else if (address == 0xFF0F) {
        res = this->interrupt->getIF();
    }

    // Audio
    else if (address >= 0xFF10 && address <= 0xFF3F) {
        res = 0x0;

    }

    // LCD
    else if (address >= 0xFF40 && address <= 0xFF4B) {
        res = this->gpu->readLCD(address);

    }

    // KEY1
    else if (cgb && address == 0xFF4D) {
        res = key1;
    }
    // VRAM Bank Select
    else if (cgb && address == 0xFF4F) {
        res = this->gpu->getVRAMBank();
    }
    // Boot ROM Map -> no bootrom so ignore
    else if (address == 0xFF50) {}
    // VRAM DMA
    else if (cgb && address >= 0xFF51 && address <= 0xFF55) {
        res = this->gpu->readHDMA(address);
    }
    // BG / OBJ Palettes
    else if (cgb && address >= 0xFF68 && address <= 0xFF6B) {
        res = this->gpu->readLCDColour(address);
    }
    // WRAM Bank Select
    else if (cgb && address == 0xFF70) {
        res = wramBank | 0xF8;
    }
    // IE
    else if (address == 0xFFFF) {
        res = this->interrupt->getIE();
    }
    return res;
}



void MMU::write8(uint16_t address, uint8_t data) {
    // std::cout << "Writing to " << address << "\n";
    if (address >= 0x0000 && address <= 0x7FFF) {
        this->rom->write(address, data);
    }
    // VRAM 
    else if (address >= 0x8000 && address <= 0x9FFF) {
        this->gpu->writeVRAM(address, data);
    }
    // External RAM
    else if (address >= 0xA000 && address <= 0xBFFF) {
        this->rom->write(address, data);
    }
    // WRAM -> for my implementation, wram bank setting shouldn't matter as wramBank = 0 is default on read/write
    else if (address >= 0xC000 && address <= 0xFDFF) {
        this->writeWRAM(address, data);
    }
    // OAM RAM
    else if (address >= 0xFE00 && address <= 0xFE9F) {
        this->gpu->writeOAM(address, data);
    }
    // High Ram
    else if (address >= 0xFF80 && address <= 0xFFFE) {
        hram[address & 0x7F] = data;
    }
    // Joypad
    else if (address == 0xFF00) {
        joypad->write(data);
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
    // Timer and Divider
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
    // LCD
    else if (address >= 0xFF40 && address <= 0xFF4B) {
        this->gpu->writeLCD(address, data);
    }
    // KEY1
    else if (cgb && address == 0xFF4D) {
        key1 = data;
    }
    // VRAM Bank Select
    else if (cgb && address == 0xFF4F) {
        this->gpu->setVRAMBank(data & 0x01);
    }
    // Boot rom map -> no bootrom so skipped
    else if (address == 0xFF50) {}
    // VRAM DMA
    else if (cgb && address >= 0xFF51 && address <= 0xFF55) {
        this->gpu->writeHDMA(address, data);
    }
    // BG / OBJ Palettes
    else if (cgb && address >= 0xFF68 && address <= 0xFF6B) {
        this->gpu->writeLCDColour(address, data);
    }
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

uint8_t MMU::getIF()
{
    return this->interrupt->getIF();
}

uint8_t MMU::getIE()
{
    return this->interrupt->getIE();
}

void MMU::setIF(uint8_t data)
{
    this->interrupt->setIF(data);
}

uint8_t MMU::readPeek(uint16_t address)
{

    uint8_t res = 0x00;
 
    // ROM Bank
    if (address >= 0x0000 && address <= 0x7FFF) {
        // skip boot rom
        res = this->rom->read(address);
    }
    
    // VRAM
    else if (address >= 0x8000 && address <= 0x9FFF) {
        res = 0x0;
    }

    // External RAM
    else if (address >= 0xA000 && address <= 0xBFFF) {
        res = this->rom->read(address);
    }

    // WRAM + Echo RAM
    else if (address >= 0xC000 && address <= 0xFDFF) {

        res = this->readWRAM(address);
    }

    // OAM RAM
    else if (address >= 0xFE00 && address <= 0xFE9F) {
        res = 0x0;
    }
    // High RAM
    else if (address >= 0xFF80 && address <= 0xFFFE) {
        res = hram[address & 0x7F];
    }


    // Joypad
    else if (address == 0xFF00) {
        res = joypad->read();
    }

    // Timer and Divider
    else if (address >= 0xFF04 && address <= 0xFF07) {
        res = timer->read(address);
    }

    // IF
    else if (address == 0xFF0F) {
        res = this->interrupt->getIF();
    }

    // Audio
    else if (address >= 0xFF10 && address <= 0xFF3F) {
        res = 0x0;

    }

    // GPU
    else if (address >= 0xFF40 && address <= 0xFF4B) {
        res = 0x0;

    }

    // OAM DMA transfer
    else if (address == 0xFF46) {
        res = 0x0;
    }

    // KEY1
    else if (cgb && address == 0xFF4D) {
        res = key1;
    }
    // VRAM Bank Select
    else if (cgb && address == 0xFF4F) {
        res = 0x0;
    }
    // Boot ROM Map
    else if (address == 0xFF50) {}
    // VRAM DMA
    else if (cgb && address >= 0xFF51 && address <= 0xFF55) {
        res = 0x0;
    }
    // BG / OBJ Palettes
    else if (cgb && address >= 0xFF68 && address <= 0xFF6B) {
        res = 0x0;
    }
    // WRAM Bank Select
    else if (cgb && address == 0xFF70) {
        res = wramBank | 0xF8;
    }
    // IE
    else if (address == 0xFFFF) {
        res = this->interrupt->getIE();
    }
    return res;
}

void MMU::OAMDMATransfer() {
    if (!gpu->checkOAMTransfer()) {
        return;
    }
    uint8_t oamDelay = gpu->checkOAMDelay();

    // After DMA is written to, wait 1 m-cycles (delay is initially set to 2 to account for writing to register during the same tick)
    if (oamDelay > 0) {
        gpu->setOAMDelay(--oamDelay);
        return;
    }
    // std::cout << "transfer\n";
    uint8_t bytes = gpu->getOAMDMABytes();
    uint8_t index = OAM_BANK_SIZE - bytes;
    uint16_t src = (gpu->getOAMDMA() << 8) | index;
    uint16_t dest = 0xFE00 | index;

    uint8_t data = this->read8(src);
    gpu->writeOAMTransfer(dest, data);
    bytes--;
    gpu->setOAMDMABytes(bytes);
    if (bytes == 0) gpu->setOAMTransfer(false);
}

void MMU::HDMATransfer(bool halt, uint8_t numBytes) {
    if (!cgb || !gpu->checkHDMATransfer()) {
        return;
    }

    if (gpu->getHDMAMode() == GENERAL_DMA) {
        uint16_t curTransfer = gpu->getCurTransfer();
        for (int i = 0; i < numBytes; ++i) {
            uint8_t data = this->read8(gpu->getHDMASrc() + curTransfer);
            this->write8(gpu->getHDMADes() + curTransfer, data);
            curTransfer++;
        }

        gpu->setCurTransfer(curTransfer);
        if (curTransfer > gpu->getHDMALength()) {
            gpu->endHDMATransfer();
        }

    } else {

        uint8_t mode = gpu->getPPUMode();

        // Should return if cpu is in halt but this causes the magen test to fail so idk probably black magic and duck tape

        if (mode != H_BLANK) {
            return;
        }

        if (!gpu->checkHBlankBurst()) {
            return;
        }
        uint16_t curTransfer = gpu->getCurTransfer();
        for (int i = 0; i < numBytes; ++i) {
            uint8_t data = this->read8(gpu->getHDMASrc() + curTransfer);
            this->write8(gpu->getHDMADes() + curTransfer, data);
            curTransfer++;
            if (curTransfer % 16 == 0 && curTransfer != 0) {
                gpu->setHBlankBurst(false);
            }
        }
        gpu->setCurTransfer(curTransfer);
        gpu->reduceHDMA(curTransfer / 16); // HDMA Length tracks how many 0x10 bytes have been transferred during HBlank DMA

        if (curTransfer >= gpu->getHDMALength()) {
            gpu->endHDMATransfer();
        }

    }
}


