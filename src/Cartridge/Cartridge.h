#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <filesystem>
#include <ios>

#include "../constants.h"

class Cartridge {
    public:
        Cartridge();
        virtual ~Cartridge() = default;

        virtual uint8_t read(uint16_t address);
        virtual void write(uint16_t address, uint8_t data);        
        virtual void setBattery(std::string title);
    protected:
        // ROM Banks
        std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romBank;

        // Catridge External Ram Banks
        std::vector<std::vector<uint8_t>> ramBank;

        std::vector<std::vector<uint8_t>> getRamBank(int extRamSize);

        int romSize;
        int ramSize;

        bool battery = false;

        std::string path = "";

        bool loadSave();
        void save();
};






#endif