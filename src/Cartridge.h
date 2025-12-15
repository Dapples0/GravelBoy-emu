#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <vector>

#include "MBC.h"
#include "constants.h"

class Cartridge {
    public:
        Cartridge();
        ~Cartridge();

    private:
        // ROM Banks
        std::vector<std::vector<uint8_t>> romBank;

        // Catridge Ram Banks
        std::vector<std::vector<uint8_t>> ramBank;

        uint8_t mbcType;
};






#endif