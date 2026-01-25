#ifndef APU_H
#define APU_H

#include "NoiseChannel.h"
#include "SquareSweepChannel.h"
#include "SquareChannel.h"
#include "WaveChannel.h"
#include <cstdint>
#include <vector>
#include <SDL2/SDL.h>
#include <iostream>


class APU {
    public:
        APU();
        ~APU();

        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t data);

        void tick(uint8_t cycles);

        void initialiseSDL();
    private:
        uint8_t NR52 = 0xF1; // Audio Master Control
        uint8_t NR51 = 0xF3; // Sound Panning
        uint8_t NR50 = 0x77; // Master Volume & VIN Panning
        
        SquareSweepChannel square1;
        SquareChannel square2;
        WaveChannel wave;
        NoiseChannel noise;
        
        void clear();

        uint32_t frameSequenceCount = 8192;
        uint8_t frameSequencer = 0;
        uint32_t sampleCount = 95;

        std::vector<float> sampleBuffer;
        
        SDL_AudioDeviceID audioDeviceId;
        
};






#endif