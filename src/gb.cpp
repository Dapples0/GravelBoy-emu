#include <iostream>
#include <fstream>
#include <iomanip>
#include <SDL2/SDL.h>
#include <array>

#include "gb.h"

struct gb_global gb_global;


gb::gb() : cpu(), apu(), gpu(), joypad(), mmu(), timer(), interrupt() {
    cpu.connect(&mmu, &timer, &gpu, &apu);
    mmu.connect(&gpu, &joypad, &timer, &apu, &interrupt);
    timer.connect(&interrupt);
    gpu.connect(&interrupt);
    joypad.connect(&interrupt);
    
}

gb::~gb()
{
}

void gb::run(const char *filename) {
    mmu.loadRom(filename);

    std::cout << "--------------------------------\n";
    cpu.setMode();

    // Initialise SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER)) {
        std::cerr << "Could not initialise SDL\n";
        exit(1);
    }
    gpu.attatchSDL();
    apu.initialiseSDL();
    SDL_Event event;

    bool running = true;

    /** Toggles framerate
     * Increasing framerate does not speed up the "real-time" that is passed in game
     * so for games like pokemon GSC, save time and pokegear time will not sync
     */
    uint32_t lastFrameTime = SDL_GetTicks();
    gb_global.ticksPerFrame = 1000 / gb_global.framerate[0];

    while (running) {
        if (!gpu.isFrameReady()) {
            cpu.execute();
        } else {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) running = false;
                if (event.type == SDL_KEYDOWN) {
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_MINUS:
                            gb_global.frameRateIndex = gb_global.frameRateIndex == 0 ? 0 : --gb_global.frameRateIndex;
                            gb_global.ticksPerFrame = 1000 / gb_global.framerate[gb_global.frameRateIndex];
                            break;

                        case SDL_SCANCODE_EQUALS:
                            gb_global.frameRateIndex = gb_global.frameRateIndex == 3 ? 3 : ++gb_global.frameRateIndex;
                            gb_global.ticksPerFrame = 1000 / gb_global.framerate[gb_global.frameRateIndex];
                            break;

                        default:
                            break;

                            
                    }
                }
            }
            uint32_t frameTime = SDL_GetTicks() - lastFrameTime;
            if (frameTime < gb_global.ticksPerFrame) {
                SDL_Delay(gb_global.ticksPerFrame - frameTime);
            }
            lastFrameTime = SDL_GetTicks();
            gpu.setFrameReady(false);
        }
        

    }


}

