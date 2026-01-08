#include "gb.h"
#include <iostream>
#include <fstream>
#include <iomanip>

gb::gb() : cpu(), apu(), gpu(), joypad(), mmu(), timer(), interrupt() {
    // cpu = CPU();
    // apu = APU();
    // gpu = GPU();
    // joypad = Joypad();
    // mmu = MMU();
    // timer = Timer();

    cpu.connect(&mmu, &timer, &gpu);
    mmu.connect(&gpu, &joypad, &timer, &apu, &interrupt);
    timer.connect(&interrupt);
    
}

gb::~gb()
{
}

void gb::run(const char *filename) {
    bool mode = mmu.loadRom(filename);

    std::cout << "--------------------------------\n";
    cpu.setMode(mode);
    std::ofstream file("logs/cpu_debug.txt");
    uint32_t i = 0;
    while (1) {
        // if (i <= 325820) {
            // file << cpu.debug();
        // }
        
        cpu.execute();
        if (cpu.getDoubleSpeed()) {
        }

        // Debugging -> checks if cycles match expected cycles TODO: Remove
        if (cpu.cb) {
            if ((int)cpu.cyclesPassed + cpu.interruptCycles != cpu.cycles) {
                std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << "CB" << (int)cpu.op << " | " << std::dec << cpu.cycles << "-" <<  (int)cpu.cyclesPassed << " \n";
                
            }
        } else {
            if ((int)cpu.cyclesPassed + cpu.interruptCycles != cpu.cycles) {
                std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(2) <<  (int)cpu.op << " | " << std::dec << cpu.cycles << "-" <<  (int)cpu.cyclesPassed << " \n";
            }
        }
        // std::cout << std::dec << "Cycles Used: " << (int)mmu.cycles << "\n";
        cpu.cycles = 0;
        i++;
    }

    file.close();

}

