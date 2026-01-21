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

    uint32_t lastFrameTime = SDL_GetTicks();
    uint32_t ticksPerFrame = 1000 / FRAMERATE;

    while (running) {        
        if (!gpu.isFrameReady()) {
            cpu.execute();
        } else {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) running = false;
            }
            uint32_t frameTime = SDL_GetTicks() - lastFrameTime;
            if (frameTime < ticksPerFrame) {
                SDL_Delay(ticksPerFrame - frameTime);
            }
            lastFrameTime = SDL_GetTicks();
            gpu.setFrameReady(false);
        }
        
        
  

        // Debugging -> checks if cycles match expected cycles TODO: Remove
        // if (cpu.cb) {
        //     if ((int)cpu.cyclesPassed + cpu.interruptCycles != cpu.cycles && !cpu.none) {
        //         std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << "CB" << (int)cpu.op << " | " << std::dec << cpu.cycles << "-" <<  (int)cpu.cyclesPassed << " \n";
                
        //     }
        // } else {
        //     if ((int)cpu.cyclesPassed + cpu.interruptCycles != cpu.cycles && !cpu.none) {
        //         std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(2) <<  (int)cpu.op << " | " << std::dec << cpu.cycles << "-" <<  (int)cpu.cyclesPassed << " \n";
        //     }
        // }
        // cpu.cycles = 0;
        // i++;
    }

    // file.close();

}

