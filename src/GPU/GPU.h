#ifndef GPU_H
#define GPU_H

#include <cstdint>
#include <array>
#include <memory>
#include <vector>

#include "../constants.h"

class GPU {
    public:
        GPU();
        ~GPU();

        void setMode(bool mode);

        uint8_t readVRAM(uint16_t address);
        void writeVRAM(uint16_t address, uint8_t data);

        uint8_t readOAM(uint16_t address);
        void writeOAM(uint16_t address, uint8_t data);

        uint8_t readLCD(uint16_t address);
        void setLCD(uint16_t address, uint8_t data);
        
        uint8_t getOAMDMA();
        void setOAMDMA(uint8_t data);

        
        uint8_t readVRAMDMA(uint16_t address);
        void writeVRAMDMA(uint16_t address, uint8_t data);

        uint8_t readLCDColour(uint16_t address);
        void writeLCDColour(uint16_t address, uint8_t data);
        

       
        uint8_t getVRAMBank();
        void setVRAMBank(uint8_t data);
         
    private:
        std::vector<std::array<uint8_t, VRAM_BANK_SIZE>> vram;
        uint8_t vramBank = 0;

        std::array<uint8_t, OAM_BANK_SIZE> oam;

        bool cgb;
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
        uint8_t BCPS = 0x00; // Background Colour Palette Specification
        uint8_t BCPD = 0x00; // Background Colour Palette Data
        uint8_t OCPS; // OBJ Colour Palette Specification
        uint8_t OCPD; // OBJ Colour Palette data




};






#endif