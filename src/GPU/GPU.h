#ifndef GPU_H
#define GPU_H

#include <cstdint>
#include <array>
#include <memory>
#include <vector>
#include <iostream>
#include <cstring>
#include <SDL2/SDL.h>

#include "../constants.h"
#include "../IO/Interrupts.h"



class GPU {
    public:
        GPU();
        ~GPU();

        void connect(Interrupts *interrupt);
        void setCGBMode(bool mode);

        uint8_t readVRAM(uint16_t address);
        void writeVRAM(uint16_t address, uint8_t data);

        uint8_t readOAM(uint16_t address);
        void writeOAM(uint16_t address, uint8_t data);
        void writeOAMTransfer(uint16_t address, uint8_t data);

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

        bool checkOAMTransfer();

        void setOAMTransfer(bool val);

        uint8_t checkOAMDelay();

        void setOAMDelay(uint8_t val);

        uint8_t getOAMDMABytes();

        void setOAMDMABytes(uint8_t val);

        uint8_t getOAMDMA();

        bool checkHDMATransfer();
        int getHDMAMode();
        uint16_t getCurTransfer();
        uint16_t getHDMASrc();
        uint16_t getHDMADes();
        void setCurTransfer(uint16_t val);
        uint16_t getHDMALength();
        void setHBlankBurst(bool val);
        bool checkHBlankBurst();
        void endHDMATransfer();
        void reduceHDMA(uint16_t val);

        void tick(uint8_t dots);

        bool isFrameReady();
        void setFrameReady(bool val);
    private:
        Interrupts *interrupt;

        std::vector<std::array<uint8_t, VRAM_BANK_SIZE>> vram = {};
        uint8_t vramBank = 0;

        std::array<uint8_t, OAM_BANK_SIZE> oam = {}; // Displays up to 40 movable objects, so each object takes 4 indices in the array
        std::array<std::array<uint8_t, 4>, 10> oamBuffer = {}; // Stores up to 10 objects in buffer, cleared once drawn
        uint8_t bufferNum = 0;

        std::array<uint16_t, SCREEN_HEIGHT * SCREEN_WIDTH> SDL_Display = {};

        // HDMA
        bool transfer = false;
        int HDMAmode = 0;
        uint16_t HDMALen = 0;
        uint16_t curTransfer = 0;
        uint16_t HDMADes;
        uint16_t HDMASrc;
        bool hblankBurst = false;
        /**
         * 0 - Horizontal Blank
         * 1 - Vertical Blank
         * 2 - OAM Scan
         * 3 - Drawing Pixel
         */
        int PPUmode;
        uint32_t dotCount = 0;
        bool cgb;
        bool lcdOff = false;

        bool oamTransfer = false;
        uint8_t oamDMABytes;
        uint8_t oamTransferDelay;
        uint8_t OAMDMADelay;
        uint8_t oamLastByte = 0x00;


        uint16_t windowLineCounter = 0;
        bool drawWindow = false;

        bool interrupt_gate = true; // Initially set to True, basically here to ensure logical OR gate is implemented properly
        /**
         * Registers
         */
        uint8_t OAMDMA = 0x00;

        // LCD Control
        uint8_t LCDC = 0x91;

        // LCD Status
        uint8_t LY = 0x00; // LCD Y coodinate
        uint8_t LYC = 0x00; // LY compare
        uint8_t STAT = 0x85; // LCD Status

        // LCD Pos + Scrolling
        uint8_t SCY = 0x00; // BG viewport Y position
        uint8_t SCX = 0x00; // BG viewport X position
        uint8_t WX = 0x00; // Window X position
        uint8_t WY = 0x00; // Window Y position

        // LCD Monochrome Palettes
        uint8_t BGP = 0xFC; // BG palette data
        uint8_t OBP0 = 0x00; // OBJ palette 0
        uint8_t OBP1 = 0x00; // OBJ palette 1

        // LCD Colour Palettes
        uint8_t BCPS = 0x00; // Background Colour Palette Specification (index)
        std::array<uint8_t, PALETTE_SIZE> bgPalette = {}; // (BCPD) Background Colour Palette Data
        uint8_t OCPS = 0x00; // OBJ Colour Palette Specification
        std::array<uint8_t, PALETTE_SIZE> objPalette = {}; // (OCPD) OBJ Colour Palette data

        // VRAM DMA Transfer Registers
        uint8_t HDMA1 = 0x00;
        uint8_t HDMA2 = 0x00;
        uint8_t HDMA3 = 0x00;
        uint8_t HDMA4 = 0x00;
        uint8_t HDMA5 = 0x80;


        // SDL
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        SDL_Texture* texture = nullptr;

        // PPU Mode functions
        void OAMScan();
        void DrawingPixels();
        void HBlank();
        void VBlank();

        void renderScanline();

        uint8_t readVRAMBank(uint16_t address, uint8_t bank);

        uint16_t getDMGColour(uint8_t id, uint8_t palette);
        uint16_t readBGPalette(uint8_t num, uint8_t id);
        uint16_t readObjPalette(uint8_t num, uint8_t id);

        void clear();
        void update();

        bool frameReady = true;


};






#endif