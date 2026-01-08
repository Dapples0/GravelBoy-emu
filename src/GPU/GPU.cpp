#include "GPU.h"

GPU::GPU()
{
}

GPU::~GPU() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void GPU::attatchSDL() {
    window = SDL_CreateWindow("gravel boy",
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, 
                                SCREEN_WIDTH * SCALE, 
                                SCREEN_HEIGHT * SCALE, 
                                SDL_WINDOW_SHOWN);

    if (window == nullptr) {
        std::cerr << "Could not create SDL window\n";
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (renderer == nullptr) {
        std::cerr << "Could not create SDL renderer\n";
        exit(1);
    }
    SDL_RenderSetScale(renderer, SCALE, SCALE);
}


void GPU::setCGBMode(bool mode) {
    cgb = mode;
}

uint8_t GPU::readVRAM(uint16_t address) {
    if (PPUmode == DRAWING_PIXELS) {
        return 0xFF;
    }
    uint16_t relative_address = address & 0x0FFF;
    if (cgb) {
        return vram[vramBank][relative_address];

    } else {
        return vram[0][relative_address];
    }
}

void GPU::writeVRAM(uint16_t address, uint8_t data) {
    if (PPUmode == DRAWING_PIXELS) {
        return;
    }
    uint16_t relative_address = address & 0x0FFF;
    if (cgb) {
        vram[vramBank][relative_address] = data;

    } else {
        vram[0][relative_address] = data;
    }
}

uint8_t GPU::readOAM(uint16_t address) {
    // OAM inaccessible in mode 2 and mode 3
    if (PPUmode == OAM_SCAN || PPUmode == DRAWING_PIXELS) {
        return 0xFF;
    }
    uint16_t relative_address = address & 0xFF;
    return oam[relative_address];
}

void GPU::writeOAM(uint16_t address, uint8_t data) {
    // OAM inaccessible in mode 2 and mode 3
    if (PPUmode == OAM_SCAN || PPUmode == DRAWING_PIXELS) {
        return;
    }
    uint16_t relative_address = address & 0xFF;
    oam[relative_address] = data;
}

uint8_t GPU::readLCD(uint16_t address) {
    // Some registers are write/read only so might change later TODO
    switch (address) {
        case LCDC_ADDRESS:
            return LCDC;
        case STAT_ADDRESS:
            return STAT;
        case SCY_ADDRESS:
            return SCY;
        case SCX_ADDRESS:
            return SCX;

        case LY_ADDRESS:
            return LY;

        case LYC_ADDRESS:
            return LYC ;

        case OAM_DMA_TRANSFER_ADDRESS:
            // Can't be read
            return 0xFF;
        case BGP_ADDRESS:
            return BGP;
        case OBP0_ADDRESS:
            return OBP0;
        case OBP1_ADDRESS:
            return OBP1;
        case WY_ADDRESS:
            return WY;
        case WX_ADDRESS:
            return WX;

        default:
            std::cerr << "Invalid LCD read address\n";
    }
}

void GPU::writeLCD(uint16_t address, uint8_t data) {
    // Some registers are write/read only so might change later TODO
    switch (address) {
        case LCDC_ADDRESS:
            LCDC = data;
            break;
        case STAT_ADDRESS:
            STAT = data;
            break;
        case SCY_ADDRESS:
            SCY = data;
            break;
        case SCX_ADDRESS:
            SCX = data;
            break;
        case LY_ADDRESS: // Read only
            break;
        case LYC_ADDRESS:
            LYC  = data;
            break;
        case OAM_DMA_TRANSFER_ADDRESS:
            // TODO HANDLE DMA OAM TRANSFER
            break;
        case BGP_ADDRESS:
            BGP = data;
            break;
        case OBP0_ADDRESS:
            OBP0 = data;
            break;
        case OBP1_ADDRESS:
            OBP1 = data;
            break;
        case WY_ADDRESS:
            WY = data;
            break;
        case WX_ADDRESS:
            WX = data;
            break;

        default:
            std::cerr << "Invalid LCD write address\n";
    }
}


uint8_t GPU::readHDMA(uint16_t address) {
    switch (address) {
        case HDMA1_ADDRESS: // Write only
            return 0xFF;
        case HDMA2_ADDRESS: // Write only
            return 0xFF;
        case HDMA3_ADDRESS: // Write only
            return 0xFF;
        case HDMA4_ADDRESS: // Write only
            return 0xFF;
        case HDMA5_ADDRESS:
            return (VRAMDMAlen & 0x7F) | (vramTransfer ? 0 : 1) << 7;;
        default:
            std::cerr << "Invalid HDMA read address\n";
            return 0xFF;
    }
}

void GPU::writeHDMA(uint16_t address, uint8_t data) {
   switch (address) {
        case HDMA1_ADDRESS:
            VRAMDMAsrc = (VRAMDMAsrc & 0x00FF) | (data << 8);
        case HDMA2_ADDRESS:
            VRAMDMAsrc = (VRAMDMAsrc & 0xFF00) | (data & 0xF0);
        case HDMA3_ADDRESS:
            VRAMDMAdes = (VRAMDMAdes & 0x00FF) | ((data & 0x1F) << 8);
        case HDMA4_ADDRESS:
            VRAMDMAdes = (VRAMDMAdes & 0xFF00) | (data & 0xF0);
        case HDMA5_ADDRESS:
            // TODO VRAM DMA Transfer
        default:

            std::cerr << "Invalid HDMA read address\n";

    }
}

uint8_t GPU::readLCDColour(uint16_t address) {
    switch (address) {
        case BCPS_ADDRESS:
            return BCPS;

        case BCPD_ADDRESS:
            if (PPUmode == DRAWING_PIXELS) {
                return 0xFF;
            }
            return bgPalette[BCPS & 0x3F];

        case OCPS_ADDRESS:

            return OCPS;
        
        case OCPD_ADDRESS:
            return objPalette[OCPS & 0x3F];
    }

    // Bad read
    return 0xFF;
}

void GPU::writeLCDColour(uint16_t address, uint8_t data) {
    switch (address) {
        case BCPS_ADDRESS:
            BCPS = data;
            break;
        case BCPD_ADDRESS:
            if (PPUmode == DRAWING_PIXELS) {
                return;
            }
            bgPalette[BCPS & 0x3F]  = data;

            // Check auto-increment
            if ((BCPS & 0x80) == 0x80) {
                BCPS = 0x80 | ((BCPS + 1) & 0x3F);
            }
            break;
        case OCPS_ADDRESS:
            OCPS = data;
            break;
        case OCPD_ADDRESS:
            if (PPUmode == DRAWING_PIXELS) {
                return;
            }
            objPalette[OCPS & 0x3F] = data;

            // Check auto-increment
            if ((OCPS & 0x80) == 0x80) {
                OCPS =  0x80 | ((OCPS + 1) & 0x3F);
            }
            break;
    }
}

uint8_t GPU::getVRAMBank()
{
    return this->vramBank;
}

void GPU::setVRAMBank(uint8_t data) {
    // MMU already handles the masking but mask here again just in case D: 
    vramBank = data & 0x01;
}

int GPU::getPPUMode()
{
    return PPUmode;
}
