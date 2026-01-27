#include "MMU.h"

extern struct gb_global gb_global;

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

void MMU::loadRom(const char *filename) {

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
            uint8_t byte = stream.get();
            romData[currentRomBank][i % ROM_BANK_SIZE]= byte;
        }
        stream.close();
    } else {
        std::cout << "Bad ROM" << "\n";
    }

    // Sanity check - Title -> title is between 0x0134 - 0x013E as it is the smallest possible title that won't extend to any possible manufacturer code
    char titleBuffer[17] = {0};
    std::copy(romData[0].begin() + 0x0134, romData[0].begin() + 0x013F, titleBuffer);

    std::string title(titleBuffer);
    std::cout << "Title: " << title << "\n";
    

    // Sanity Check - CGB Flag
    int cgb = (int)romData[0][0x143];
    std::cout << "CGB Flag: " << cgb << "\n";

    // Sanity Check - MBC type
    int type = (int)romData[0][0x147];
    std::cout << "Catridge Type: " << type << "\n";

    // Remember to save battery here as it is for save states

    // Sanity Check - External RAM Code
    int extRamCode = (int)romData[0][0x149];
    std::cout << "External RAM Code: " << extRamCode << "\n";

    

    // Sanity Check - ROM Size
	int headerRomSize = (int)romData[0][0x148];
	std::cout << "ROM Size: " << 32 * (1 << headerRomSize) << "KB\n";


    gb_global.cgb = cgb == 0xC0 || cgb == 0x80;
    this->gpu->setCGBMode();
    
    this->wram = gb_global.cgb 
    ? std::vector<std::vector<uint8_t>>(8, std::vector<uint8_t>(WRAM_BANK_SIZE)) 
    : std::vector<std::vector<uint8_t>>(2, std::vector<uint8_t>(WRAM_BANK_SIZE));

    // Determines MBC Type
    setMBC(type, romData, romSize, extRamCode, title);

}

void MMU::setMBC(int type, std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romData, int romSize, int extRamCode, std::string title) {
    switch (type) {
        case 0x00: // ROM ONLY
            std::cout << "MBC Type: NOMBC\n";
            this->rom = std::make_unique<NOMBC>(romData, romSize, extRamCode);
            break;

        case 0x01: // MBC1
            std::cout << "MBC Type: MBC1\n";
            this->rom = std::make_unique<MBC1>(romData, romSize, extRamCode);
            break;

        case 0x02: // MBC1 + RAM
            std::cout << "MBC Type: MBC1 + RAM\n";
            this->rom = std::make_unique<MBC1>(romData, romSize, extRamCode);
            break;     

        case 0x03: // MBC1 + RAM + Battery
            std::cout << "MBC Type: MBC1 + RAM + Battery\n";
            this->rom = std::make_unique<MBC1>(romData, romSize, extRamCode);
            this->rom->setBattery(title);
            break;

        case 0x0F: // MBC3 + Timer + Battery
            std::cout << "MBC Type: MBC3 + Timer + Battery\n";
            this->rom = std::make_unique<MBC3>(romData, romSize, extRamCode, true);
            this->rom->setBattery(title);
            break;

        case 0x10: // MBC3 + Timer + RAM + Battery
            std::cout << "MBC Type: MBC3 + Timer + RAM + Battery\n";
            this->rom = std::make_unique<MBC3>(romData, romSize, extRamCode, true);
            this->rom->setBattery(title);
            break;

        case 0x11: // MBC3
            std::cout << "MBC Type: MBC3\n";
            this->rom = std::make_unique<MBC3>(romData, romSize, extRamCode, false);
            break;

        case 0x12: // MBC3 + RAM
            std::cout << "MBC Type: MBC3 + RAM\n";
            this->rom = std::make_unique<MBC3>(romData, romSize, extRamCode, false);
            break;
            
        case 0x13: // MBC3 + RAM + Battery
            std::cout << "MBC Type: MBC3 + RAM + Battery\n";
            this->rom = std::make_unique<MBC3>(romData, romSize, extRamCode, false);
            this->rom->setBattery(title);
            break;

        case 0x19: // MBC5
            std::cout << "MBC Type: MBC5\n";
            this->rom = std::make_unique<MBC5>(romData, romSize, extRamCode);
            break;

        case 0x1A: // MBC5 + RAM
            std::cout << "MBC Type: MBC5 + RAM\n";
            this->rom = std::make_unique<MBC5>(romData, romSize, extRamCode);
            break;
            
        case 0x1B: // MBC5 + RAM + Battery
            std::cout << "MBC Type: MBC5 + RAM + Battery\n";
            this->rom = std::make_unique<MBC5>(romData, romSize, extRamCode);
            this->rom->setBattery(title);
            break;
            
        case 0x1C: // MBC5 + Rumble
            std::cout << "MBC Type: MBC5 + Rumble\n";
            this->rom = std::make_unique<MBC5>(romData, romSize, extRamCode);
            break;
            
        case 0x1D: // MBC5 + Rumble + RAM
            std::cout << "MBC Type: MBC5 + Rumble + RAM\n";
            this->rom = std::make_unique<MBC5>(romData, romSize, extRamCode);
            this->rom->setBattery(title);

            break;
            
        case 0x1E: // MBC + Rumble + RAM + Battery
            std::cout << "MBC Type: MBC + Rumble + RAM + Battery\n";
            this->rom = std::make_unique<MBC5>(romData, romSize, extRamCode);
            this->rom->setBattery(title);
            break;

        default:
            std::cout << "No MBC type found, defaulting to MBC1\n";
            this->rom = std::make_unique<MBC1>(romData, romSize, extRamCode);
            break;
    }
}


uint8_t MMU::read8(uint16_t address)
{
    uint8_t res = 0x00;
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
        res = apu->read(address);

    }

    // LCD
    else if (address >= 0xFF40 && address <= 0xFF4B) {
        res = this->gpu->readLCD(address);

    }

    // KEY1
    else if (gb_global.cgb && address == 0xFF4D) {
        res = key1;
    }
    // VRAM Bank Select
    else if (gb_global.cgb && address == 0xFF4F) {
        res = this->gpu->getVRAMBank();
    }
    // Boot ROM Map -> no bootrom so ignore
    else if (address == 0xFF50) {}
    // VRAM DMA
    else if (gb_global.cgb && address >= 0xFF51 && address <= 0xFF55) {
        res = this->gpu->readHDMA(address);
    }
    // BG / OBJ Palettes
    else if (gb_global.cgb && address >= 0xFF68 && address <= 0xFF6B) {
        res = this->gpu->readLCDColour(address);
    }
    // WRAM Bank Select
    else if (gb_global.cgb && address == 0xFF70) {
        res = wramBank | 0xF8;
    }
    // IE
    else if (address == 0xFFFF) {
        res = this->interrupt->getIE();
    }
    return res;
}



void MMU::write8(uint16_t address, uint8_t data) {
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
    // WRAM
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
    // Audio
    else if (address >= 0xFF10 && address <= 0xFF3F) {
        apu->write(address, data);
    }
    // LCD
    else if (address >= 0xFF40 && address <= 0xFF4B) {
        this->gpu->writeLCD(address, data);
    }
    // KEY1
    else if (gb_global.cgb && address == 0xFF4D) {
        key1 = data;
    }
    // VRAM Bank Select
    else if (gb_global.cgb && address == 0xFF4F) {
        this->gpu->setVRAMBank(data & 0x01);
    }
    // Boot rom map -> no bootrom so skipped
    else if (address == 0xFF50) {}
    // VRAM DMA
    else if (gb_global.cgb && address >= 0xFF51 && address <= 0xFF55) {
        this->gpu->writeHDMA(address, data);
    }
    // BG / OBJ Palettes
    else if (gb_global.cgb && address >= 0xFF68 && address <= 0xFF6B) {
        this->gpu->writeLCDColour(address, data);
    }
    // WRAM Bank Select
    else if (gb_global.cgb && address == 0xFF70) {
        wramBank = data & 0x07;
        if (gb_global.cgb) {
            if (gb_global.cgb && wramBank == 0) {
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
        if (gb_global.cgb) {
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
        if (gb_global.cgb) {
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
    if (!gb_global.cgb || !gpu->checkHDMATransfer()) {
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
        gpu->reduceHDMA(curTransfer / 16); // HDMA Length tracks how many 0x10 (16) bytes have been transferred during HBlank DMA

        if (curTransfer >= gpu->getHDMALength()) {
            gpu->endHDMATransfer();
        }

    }
}


