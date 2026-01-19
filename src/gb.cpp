#include <iostream>
#include <fstream>
#include <iomanip>
#include <SDL2/SDL.h>

#include "gb.h"


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
    gpu.connect(&interrupt);
    joypad.connect(&interrupt);
    
}

gb::~gb()
{
}

void gb::run(const char *filename) {
    bool mode = mmu.loadRom(filename);

    std::cout << "--------------------------------\n";
    cpu.setMode(mode);

    // Initialise SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER)) {
        std::cerr << "Could not initialise SDL\n";
        exit(1);
    }
    gpu.attatchSDL();
    SDL_Event event;

    // std::ofstream file("logs/cpu_debug.txt");
    uint32_t i = 0;

    uint32_t cycles_until_poll = 0;
    bool running = true;
    while (running) {
        // if (i <= 325820) {
            // file << cpu.debug();
        // }

        // TODO remove later -> here to make tests run faster
        if (++cycles_until_poll > 10000) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }
        cycles_until_poll = 0;
    }
        
        // if (gpu.getPPUMode() != V_BLANK) 
        cpu.execute();
        
  

        // Debugging -> checks if cycles match expected cycles TODO: Remove
        if (cpu.cb) {
            if ((int)cpu.cyclesPassed + cpu.interruptCycles != cpu.cycles && !cpu.none) {
                std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << "CB" << (int)cpu.op << " | " << std::dec << cpu.cycles << "-" <<  (int)cpu.cyclesPassed << " \n";
                
            }
        } else {
            if ((int)cpu.cyclesPassed + cpu.interruptCycles != cpu.cycles && !cpu.none) {
                std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(2) <<  (int)cpu.op << " | " << std::dec << cpu.cycles << "-" <<  (int)cpu.cyclesPassed << " \n";
            }
        }
        cpu.cycles = 0;
        i++;
    }

    // file.close();

}

