#ifndef GPU_H
#define GPU_H

#include <cstdint>
#include <array>
#include <memory>
#include <vector>
#include <iostream>

#include "../constants.h"
#include <SDL2/SDL.h>


class GPU {
    public:
        GPU();
        ~GPU();

        void setCGBMode(bool mode);

        uint8_t readVRAM(uint16_t address);
        void writeVRAM(uint16_t address, uint8_t data);

        uint8_t readOAM(uint16_t address);
        void writeOAM(uint16_t address, uint8_t data);

        uint8_t readLCD(uint16_t address);
        void writeLCD(uint16_t address, uint8_t data);

        
        uint8_t readHDMA(uint16_t address);
        void writeHDMA(uint16_t address, uint8_t data);

        uint8_t readLCDColour(uint16_t address);
        void writeLCDColour(uint16_t address, uint8_t data);
        

       
        uint8_t getVRAMBank();
        void setVRAMBank(uint8_t data);
         
        void attatchSDL();

        int getPPUMode();
    private:
        std::vector<std::array<uint8_t, VRAM_BANK_SIZE>> vram; // No matter what mode treat vram as 2 banks
        uint8_t vramBank = 0;

        std::array<uint8_t, OAM_BANK_SIZE> oam;

        /**
         * 0 - Horizontal Blank
         * 1 - Vertical Blank
         * 2 - OAM Scan
         * 3 - Drawing Pixel
         */
        int PPUmode;
        bool cgb;
        bool oamTransfer;
        uint8_t oamTransferDelay;
        bool vramTransfer;
        /**
         * Registers
         */
        uint8_t OAMDMA = 0x00;

        // LCD Control
        uint8_t LCDC = 0x00;

        // LCD Status
        uint8_t LY; // LCD Y coodinate
        uint8_t LYC; // LY compare
        uint8_t STAT = 0x00; // LCD Status

        // LCD Pos + Scrolling
        uint8_t SCY; // BG viewport Y position
        uint8_t SCX; // BG viewport X position
        uint8_t WX; // Window X position
        uint8_t WY; // Window Y position

        // LCD Monochrome Palettes
        uint8_t BGP = 0x00; // BG palette data
        uint8_t OBP0; // OBJ palette 0
        uint8_t OBP1; // OBJ palette 1

        // LCD Colour Palettes
        uint8_t BCPS = 0x00; // Background Colour Palette Specification (index)
        std::array<uint16_t, PALETTE_SIZE> bgPalette; // (BCPD) Background Colour Palette Data
        uint8_t OCPS; // OBJ Colour Palette Specification
        std::array<uint16_t, PALETTE_SIZE> objPalette; // (OCPD) OBJ Colour Palette data

        // VRAM DMA Transfer Registers
        uint16_t VRAMDMAsrc = 0x0000;
        uint16_t VRAMDMAdes = 0x0000;
        uint8_t VRAMDMAlen = 0x00;


        // SDL
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;

};






#endif