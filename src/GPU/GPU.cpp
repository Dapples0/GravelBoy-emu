#include "GPU.h"
#include <bitset>
#include <iomanip>

GPU::GPU() {
}

GPU::~GPU() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void GPU::connect(Interrupts *interrupt) {
    this->interrupt = interrupt;
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

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    if (texture == nullptr) {
        std::cerr << "Could not create SDL texture\n";
        exit(1); 
    }
}

void GPU::tick(uint8_t dots) {
    // Checks if LCDC LCD & PPU enable bit is off
    if ((LCDC & 0x80) != 0x80) {
        LY = 0x00;
        PPUmode = H_BLANK;
        STAT &= 0xFC;

        // TODO clear obj, background and scanline
        clear();

        return;
    }
    // Do 4 dots in normal speed and 2 dots in double speed
    for (int i = 0; i < dots; ++i) {
        PPUmode = STAT & 0x3;
        dotCount++;

        if (PPUmode == H_BLANK) {
            HBlank();
        }
        else if (PPUmode == V_BLANK) {
            VBlank();
        }
        else if (PPUmode == OAM_SCAN) {
            OAMScan();
        }
        else if (PPUmode == DRAWING_PIXELS) {
            // Hybrid FIFO + scanline rendering approach -> essentially renders whole scanline and waits until an appropriate number of dots have been passed
            DrawingPixels();
        }
        else {
            // Debugging
            std::cerr << "Bad PPU mode: " << PPUmode << "\n";
        }

        // STAT interrupt handler
        if (LY == LYC) {
            STAT |= 0x04;
        } else {
            STAT &= ~0x04;
        }

        bool interrupt_flag = false;
        if ((STAT & 0x40) == 0x40 && LY == LYC) interrupt_flag = true;
        

        if ((STAT & 0x20) == 0x20 && PPUmode == OAM_SCAN) interrupt_flag = true;
        
        if ((STAT & 0x10) == 0x10 && PPUmode == V_BLANK) interrupt_flag = true;
        

        if ((STAT & 0x08) == 0x08 && PPUmode == H_BLANK) interrupt_flag = true;
        
        if (interrupt_flag && interrupt_gate) {
            interrupt_gate = false;
            interrupt->setIF(interrupt->getIF() | 0x02);
        }

        if (!interrupt_flag) interrupt_gate = true;

        // Handles state if any state changes occur
        if (PPUmode == H_BLANK) {
            STAT = (STAT & 0xFC) | 0x00;
        }
        else if (PPUmode == V_BLANK) {
            STAT = (STAT & 0xFC) | 0x01;
        }
        else if (PPUmode == OAM_SCAN) {
            STAT = (STAT & 0xFC) | 0x02;
        }
        else if (PPUmode == DRAWING_PIXELS) {
            STAT = (STAT & 0xFC) | 0x03;
        }
        


    }
}


void GPU::setCGBMode(bool mode) {
    cgb = mode;
    vram.resize(cgb ? 2 : 1);
}

uint8_t GPU::readVRAM(uint16_t address) {
    if (PPUmode == DRAWING_PIXELS) {
        return 0xFF;
    }
    uint16_t relative_address = address & 0x1FFF;
    if (cgb) {
        return vram[vramBank][relative_address];

    } else {
        return vram[0][relative_address];
    }
}

uint8_t GPU::readVRAMBank(uint16_t address, uint8_t bank) {
    if (!cgb && bank == 1) {
        std::cerr << "Reading from second VRAM bank on DMG mode\n";
    }
    uint16_t relative_address = address & 0x1FFF;
    return vram[bank][relative_address];
}


void GPU::writeVRAM(uint16_t address, uint8_t data) {
    if (PPUmode == DRAWING_PIXELS) {
        return;
    }
    uint16_t relative_address = address & 0x1FFF;
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

void GPU::writeOAMTransfer(uint16_t address, uint8_t data) {
    uint16_t relative_address = address & 0xFF;
    oam[relative_address] = data;
}

uint8_t GPU::readLCD(uint16_t address) {
    // Some registers are write/read only so might change later
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
            return 0x00;
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
            return 0xFF;
    }
}

void GPU::writeLCD(uint16_t address, uint8_t data) {
    // Some registers are write/read only so might change later
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
            oamTransfer = true;
            oamDMABytes = 0xA0;
            // write data is the address source divided by $100, so during actual transfer we should shift this left by 8 bits
            OAMDMA = data;

            OAMDMADelay = 2;
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
            return HDMA5;
        default:
            std::cerr << "Invalid HDMA read address\n";
            return 0xFF;
    }
}

void GPU::writeHDMA(uint16_t address, uint8_t data) {
   switch (address) {
        case HDMA1_ADDRESS:
            HDMA1 = data;
            break;
        case HDMA2_ADDRESS:
            HDMA2 = data & 0xF0;
            break;
        case HDMA3_ADDRESS:
            // Ignore higher 3 bits and handle vram location
            HDMA3 = (data & 0x1F) | 0x80;
            break;
        case HDMA4_ADDRESS:
            HDMA4 = data & 0xF0;
            break;
        case HDMA5_ADDRESS:
            {
                if (!transfer) {
                    transfer = true;

                    HDMAmode = (data & 0x80) == 0x80 ? HBLANK_DMA : GENERAL_DMA;
                    
                    HDMALen = ((data & 0x7F) + 1) * 16; // Convert to byte format 

                    curTransfer = 0;

                    HDMASrc = (HDMA1 << 8) | HDMA2;
                    HDMADes = (HDMA3 << 8) | HDMA4;
                    std::cout << std::hex << std::uppercase << std::setfill('0') << "Source: 0x" << std::setw(4) << (int)HDMASrc  << std::dec << "\n";
                    std::cout << std::hex << std::uppercase << std::setfill('0') << "Source: 0x" << std::setw(4) << (int)HDMADes << std::dec << "\n";
                    std::cout << "Length: " << (int)HDMALen << "\n";
                    hblankBurst = true;
                    HDMA5 = (HDMAmode << 7) | (data & 0x7F);
                } else if ((data & 0x80) != 0x80 && transfer && HDMAmode == HBLANK_DMA) {
                    transfer = false;
                }

            }
            break;
        default:

            std::cerr << "Invalid HDMA write address\n";

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
            if (PPUmode == DRAWING_PIXELS) {
                return 0xFF;
            }
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

bool GPU::checkOAMTransfer()
{
    return oamTransfer;
}

void GPU::setOAMTransfer(bool val) {
    oamTransfer = val;
}

uint8_t GPU::checkOAMDelay()
{
    return OAMDMADelay;
}

void GPU::setOAMDelay(uint8_t val) {
    OAMDMADelay = val;
}

uint8_t GPU::getOAMDMABytes() {
    return oamDMABytes;
}

void GPU::setOAMDMABytes(uint8_t val) {
    oamDMABytes = val;
}

uint8_t GPU::getOAMDMA() {
    return OAMDMA;
}

void GPU::OAMScan() {
    // dot 2 (0-3) dot 4 (4-7) dot 6 (8-11) dot 8 (12-15) dot 10 (16-19) ... dot 80 (156-159)
    if (dotCount % 2 == 0) {
        uint8_t index = (dotCount * 2) - 4;

        // Look through one entry
        uint8_t posY = oam[index];
        uint8_t posX = oam[index + 1];
        uint8_t tileIndex = oam[index + 2];
        uint8_t attribute = oam[index + 3];

        uint8_t objSize = ((LCDC & 0x04) == 0x04) ? 16 : 8;
        uint16_t LYHeight = LY + 16;
        if (bufferNum != 10 && posX > 0 && LYHeight >= posY && LYHeight < posY + objSize) {
            oamBuffer[bufferNum][0] = posY;
            oamBuffer[bufferNum][1] = posX;
            oamBuffer[bufferNum][2] = tileIndex;
            oamBuffer[bufferNum][3] = attribute;


            bufferNum++;
        }
    }

    if (dotCount == 80) {
        PPUmode = DRAWING_PIXELS;
        if (LY >= WY) drawWindow = true;
    }
}

void GPU::DrawingPixels() {
    // Render scanline at the end of mode
    if (dotCount == 252) {
        renderScanline();
        PPUmode = H_BLANK;
        if(transfer && HDMAmode == HBLANK_DMA) hblankBurst = true;
    }


}

void GPU::HBlank() {
    if (dotCount == 456) {

        PPUmode = OAM_SCAN;
        LY++;
        dotCount = 0;

        if (LY == SCREEN_HEIGHT) {
            interrupt->setIF(interrupt->getIF() | 0x01);
            PPUmode = V_BLANK;
            windowLineCounter = 0;
            drawWindow = false;
        }
    }
}

void GPU::VBlank() {
    if (dotCount == 456) {
        LY++;
        dotCount = 0;
        if (LY == 154) {
            update();
            PPUmode = OAM_SCAN;
            LY = 0;
        }

    }


}

void GPU::renderScanline() {
    // LCDC enable registers
    bool windowEnabled = (LCDC & 0x20) == 0x20;
    bool objEnabled = (LCDC & 0x02) == 0x02;
    bool enableOrPrio = (LCDC & 0x01) == 0x01;

    // LCDC address registers
    uint16_t bgMapAddress = (LCDC & 0x08) == 0x08 ? 0x9C00 : 0x9800;
    uint16_t winMapAddress = (LCDC & 0x40) == 0x40 ? 0x9C00 : 0x9800;

    bool windowDrawn = false;
    uint16_t tileX, tileY, tileNum, address, colour, tileAddress;
    uint8_t tileAttr, bank, lower, upper, flipMask, colourId;
    std::array<uint8_t, SCREEN_HEIGHT * SCREEN_WIDTH> bgAttr = {};
    std::array<uint8_t, SCREEN_HEIGHT * SCREEN_WIDTH> bgColourId = {};
    std::array<uint8_t, SCREEN_HEIGHT * SCREEN_WIDTH> objPrio = {};
    // Draw window and background
    for (int i = 0; i < SCREEN_WIDTH; ++i) {
        if (windowEnabled && drawWindow && i >= (WX - 7)) {
            address = winMapAddress;
            tileX = (i - (WX - 7)) / 8;
            tileY = windowLineCounter / 8;
            windowDrawn = true;
        } else {
            address = bgMapAddress;
            tileX = ((SCX + i) / 8) & 0x1F;
            tileY = (((LY + SCY) & 0xFF) / 8) & 0x1F;          
        }

        tileNum = readVRAMBank((tileY * 32 + tileX + address), 0);
        tileAddress = (LCDC & 0x10) == 0x10 ? 0x8000  + (16 * tileNum): 0x9000 + (16 * (int8_t)tileNum);
        tileAttr = cgb ? readVRAMBank(tileY * 32 + tileX + address, 1) : 0;
        // Y flip
        if ((cgb && (tileAttr & 0x40) == 0x40)) {
            tileAddress += windowDrawn ? (14 - 2 * (windowLineCounter % 8)) :  (14 - 2 * ((LY + SCY) % 8));
        } 
        else {
            tileAddress += windowDrawn ? (2 * (windowLineCounter % 8)) : (2 * ((LY + SCY) % 8));
        }
            

        bank = (cgb) ? (tileAttr >> 3) & 0x01 : 0;        
        lower = readVRAMBank(tileAddress, bank);
        upper = readVRAMBank(tileAddress + 1, bank);

        // X flip
        flipMask = windowDrawn ? (i - (WX - 7)) % 8 : (SCX + i) % 8;
        
        // Tile attribute is always a unset in dmg mode
        if ((tileAttr & 0x20) != 0x20) flipMask = 7 - flipMask;

        flipMask = 1 << flipMask;

        colourId = ((upper & flipMask) ? 2 : 0 ) | ((lower & flipMask) ? 1 : 0);


        // Get colour
        if (cgb) {
            colour = readBGPalette(tileAttr & 0x07, colourId);
        } else {
            // if background enable not set, use colour 0 of BGP
            colour = enableOrPrio ? getDMGColour(colourId, BGP) : 0x7FFF;
        }
        SDL_Display[i + LY * SCREEN_WIDTH] = colour;

        // For BG priority in CGB mode
        if (cgb) {
            bgAttr[i + LY * SCREEN_WIDTH] = tileAttr;
            bgColourId[i + LY * SCREEN_WIDTH] = colourId;
        } else {
            // Store colour id for object to background priority as only colour ids 1-3 can be drawn over an object
            bgColourId[i + LY * SCREEN_WIDTH] = colourId;
        }


    }

    // Draw objects
    if (objEnabled) {
        for (int i = 0; i < SCREEN_WIDTH; ++i) {
            uint8_t objSize = ((LCDC & 0x04) == 0x04) ? 16 : 8;

            // Drawing priority, DMG: smaller x has a higher priority
            uint8_t dmgXPriority = 0xFF;

            // Loop through all oam buffer objects
            for (int j = 0; j < bufferNum; ++j) {
                uint8_t posY = oamBuffer[j][0];
                uint8_t posX = oamBuffer[j][1];
                uint8_t objTileNum = oamBuffer[j][2];
                uint8_t objAttr = oamBuffer[j][3];

                // Skip if current pixel does not contain the object
                if (posX > (i + 8) || (posX + 8) <= i + 8) continue;

                tileAddress = 0x8000;

                // Y flip
                if ((objAttr & 0x40) == 0x40) {
                    tileAddress +=  (objSize == 8) ? 
                                    (objTileNum * 16 + 2 * (7 - (LY - posY + 16))) : 
                                    (objTileNum & 0xFFFE) * 16 + 2 * (15 - (LY - posY + 16));

                } else {
                    tileAddress +=  (objSize == 8) ? 
                                    (objTileNum * 16 + 2 * (LY - posY + 16)) : 
                                    (objTileNum & 0xFFFE) * 16 + 2 * ((LY - posY + 16));
                }

                bank = (cgb) ? (objAttr >> 3) & 0x01 : 0;

                lower = readVRAMBank(tileAddress, bank);
                upper = readVRAMBank(tileAddress + 1, bank);

                // X Flip
                flipMask = (objAttr & 0x20) == 0x20 ? (i - posX + 8) : (7 - (i - posX + 8));
                flipMask = 1 << flipMask;

                colourId = ((upper & flipMask) ? 2 : 0 ) | ((lower & flipMask) ? 1 : 0);

                // Get colour
                if (cgb) {
                    colour = readObjPalette(objAttr & 0x07, colourId);
                } else {
                    colour = getDMGColour(colourId, (objAttr & 0x10) == 0x10 ? OBP1 : OBP0);
                }
                if (colourId == 0) continue;

                // object priority mode check
                if (cgb) {
                    // for cgb we prioritise the object that comes first in the oam buffer
                    if (objPrio[i + LY * SCREEN_WIDTH] != 0) continue;
                } else {
                    // Prioritise smaller x position else if equal take the first object in the oam buffer
                    if (posX > dmgXPriority || (posX == dmgXPriority && objPrio[i + LY * SCREEN_WIDTH] != 0)) continue;
                    dmgXPriority = posX;                    
                }

                bool showObj = true;
                if (cgb) {
                    if (bgColourId[i + LY * SCREEN_WIDTH] == 0) {
                        showObj = true;
                    } else if (!enableOrPrio) {
                        showObj = true;
                    } else if ((bgAttr[i + LY * SCREEN_WIDTH] & 0x80) == 0 && (objAttr & 0x80) == 0) {
                        showObj = true;
                    } else {
                        showObj = false;
                    }
                } else if ((objAttr & 0x80) == 0x80 && bgColourId[i + LY * SCREEN_WIDTH] != 0) {
                    showObj = false;
                }

                if (showObj) SDL_Display[i + LY * SCREEN_WIDTH] = colour;
                objPrio[i + LY * SCREEN_WIDTH] = colourId;
            }
        }

    }


    if (windowDrawn) windowLineCounter++;
    bufferNum = 0;
    oamBuffer = std::array<std::array<uint8_t, 4>, 10>{};
}

uint16_t GPU::getDMGColour(uint8_t id, uint8_t palette) {
    uint8_t shade;
    if (id == 0) {
        shade = palette & 0x03;
    } else if (id == 1) {
        shade = (palette & 0x0C) >> 2;
    } else if (id == 2) {
        shade = (palette & 0x30) >> 4;
    } else {
        shade = (palette & 0xC0) >> 6;
    }

    // Uses the Left + B colour palette, may add a custom one later D:
    if (shade == 0) {
        return 0x7FFF;
    } else if (shade == 1) {
        return 0x56B5;
    } else if (shade == 2) {
        return 0x294A;
    }
    return 0x0000;
}

uint16_t GPU::readBGPalette(uint8_t num, uint8_t id) {
    uint8_t lower = bgPalette[num * 8 + id * 2];
    uint8_t upper = bgPalette[num * 8 + id * 2 + 1]; 

    // RGB555 little-endian -> 0x0BBBBBGGGGGRRRRR
    uint16_t colour = upper << 8 | lower;

    uint8_t red = colour & 0x1F;
    uint8_t green = (colour >> 5) & 0x1F;
    uint8_t blue = (colour >> 10) & 0x1F;

    // SDL RGB555 -> 0x0RRRRRGGGGGBBBBB
    return (red << 10) | (green << 5) | blue;
}

uint16_t GPU::readObjPalette(uint8_t num, uint8_t id) {
    uint8_t lower = objPalette[num * 8 + id * 2];
    uint8_t upper = objPalette[num * 8 + id * 2 + 1];

    // RGB555 little-endian -> 0x0BBBBBGGGGGRRRRR
    uint16_t colour = upper << 8 | lower;
    
    uint8_t red = colour & 0x1F;
    uint8_t green = (colour >> 5) & 0x1F;
    uint8_t blue = (colour >> 10) & 0x1F;

    // SDL RGB555 -> 0x0RRRRRGGGGGBBBBB
    return (red << 10) | (green << 5) | blue;
}

void GPU::clear() {
    if (lcdOff) return;
    std::fill(SDL_Display.begin(), SDL_Display.end(), 0x7FFF);
    oamBuffer = std::array<std::array<uint8_t, 4>, 10>{};
    bufferNum = 0;
    windowLineCounter = 0;

    SDL_UpdateTexture(texture, NULL, SDL_Display.data(), SCREEN_WIDTH * sizeof(uint16_t));

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    lcdOff = true;
}

void GPU::update() {
    void* pixels;
    int pitch;

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    SDL_LockTexture(texture, nullptr, &pixels, &pitch);

    std::memcpy(pixels, SDL_Display.data(), SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint16_t));

    SDL_UnlockTexture(texture);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

bool GPU::checkHDMATransfer() {
    if (!transfer) {
        return false;
    }
    if (HDMAmode == GENERAL_DMA) {
        return true;
    }
    if (HDMAmode == HBLANK_DMA && hblankBurst && PPUmode == H_BLANK) {
        return true;
    }
    return false;
}

int GPU::getHDMAMode() {
    return HDMAmode;
}

uint16_t GPU::getCurTransfer() {
    return curTransfer;
}

uint16_t GPU::getHDMASrc() {
    return HDMASrc;
}

uint16_t GPU::getHDMADes() {
    return HDMADes;
}

void GPU::setCurTransfer(uint16_t val) {
    curTransfer = val;
}

uint16_t GPU::getHDMALength() {
    return HDMALen;
}

void GPU::setHBlankBurst(bool val) {
    hblankBurst = val;
}

bool GPU::checkHBlankBurst() {
    return hblankBurst;
}

void GPU::endHDMATransfer() {
    transfer = false;
    HDMA5 = 0xFF;
}

void GPU::reduceHDMA(uint16_t val) {
    HDMA5 = (((HDMALen - val) / 16) - 1) | (HDMAmode << 7);
}
