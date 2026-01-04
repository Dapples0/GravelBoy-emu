#ifndef GB_H
#define GB_h

#include "APU/APU.h"
#include "CPU/CPU.h"
#include "GPU/GPU.h"
#include "IO/Joypad.h"
#include "Memory/MMU.h"
#include "Cartridge/Cartridge.h"
#include "IO/Timer.h"
#include "IO/Interrupts.h"



class gb {
    public:
        gb();
        ~gb();

        void run(const char *filename);
        const int WIDTH = 160;
        const int HEIGHT = 144;

    private:
        CPU cpu;
        APU apu;
        GPU gpu;
        Joypad joypad;
        MMU mmu;
        Timer timer;
        Interrupts interrupt;

};






#endif