#include "gb.h"
#include <iostream>
#include <fstream>

gb::gb() : cpu(), apu(), gpu(), joypad(), mmu(), timer() {
    // cpu = CPU();
    // apu = APU();
    // gpu = GPU();
    // joypad = Joypad();
    // mmu = MMU();
    // timer = Timer();

    cpu.connect(&mmu);
    mmu.connect(&gpu, &joypad, &timer, &apu);
    // timer.connect(&mmu);
    
}

gb::~gb()
{
}

void gb::run(const char *filename) {
    bool mode = mmu.loadRom(filename);

    std::cout << "--------------------------------\n";
    cpu.setState(mode);
    std::ofstream file("logs/cpu_debug.txt");
    uint32_t i = 0;
    while (1) {
        // if (i <= 325820) {
            file << cpu.debug();
        // }
        
        cpu.execute();
        i++;
    }

    file.close();

}

