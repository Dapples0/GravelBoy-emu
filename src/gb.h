#ifndef GB_H
#define GB_h

#include "APU.h"
#include "CPU.h"
#include "GPU.h"
#include "Joypad.h"
#include "MMU.h"
#include "Cartridge.h"


class gb {
    public:
        gb();
        ~gb();

        const int WIDTH = 160;
        const int HEIGHT = 144;

    private:
        CPU cpu;
        APU apu;
        GPU gpu;
        Joypad joypad;
        MMU mmu;
        Timer timer;

};






#endif