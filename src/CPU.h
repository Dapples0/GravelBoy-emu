#ifndef CPU_H
#define CPU_H

#include "MMU.h"

class CPU {
    public:
        CPU();
        ~CPU();

    private:
        MMU *mmu;
        

        // GP Registers
        std::array<uint8_t, 8> registers;

        // SP Registers
        uint16_t sp;
        uint16_t pc;

        // Interupts
        bool ime;
};






#endif