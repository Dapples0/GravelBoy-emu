#include "APU.h"

APU::APU() : square1(), square2(), wave(), noise() {
    
    sampleBuffer.reserve(4096);
}

APU::~APU() {
    SDL_PauseAudioDevice(audioDeviceId, 1);
    SDL_ClearQueuedAudio(audioDeviceId);
    SDL_CloseAudioDevice(audioDeviceId);
}

void APU::initialiseSDL() {
    SDL_AudioSpec audioSpec;
    SDL_AudioSpec obtainedSpec;
    SDL_zero(audioSpec);
    SDL_zero(obtainedSpec);

    audioSpec.freq = 44100;
    audioSpec.format = AUDIO_F32SYS;
    audioSpec.channels = 2;
    audioSpec.samples = 512;
    audioSpec.callback = nullptr;

    audioDeviceId = SDL_OpenAudioDevice(NULL, 0, &audioSpec, &obtainedSpec, 0);

    SDL_PauseAudioDevice(audioDeviceId, 0);
}

uint8_t APU::read(uint16_t address)
{
    uint8_t res = 0xFF;
    if (address == 0xFF26) {
        res = (NR52 & 0x80) | 0x70;
        if (square1.isActive()) res |= 0x01;
        else res &= ~0x01;
        if (square2.isActive()) res |= 0x02;
        else res &= ~0x02;
        if (wave.isActive()) res |= 0x04;
        else res &= ~0x04;
        if (noise.isActive()) res |= 0x08;
        else res &= ~0x08;
    }
    // Sound Panning
    else if (address == 0xFF25) {
        res = NR51;
    }
    // Master volume & VIN Panning
    else if (address == 0xFF24) {
        res = NR50;
    }
    // Sound Channel 1
    else if (address >= 0xFF10 && address <= 0xFF14) {
        res = square1.read(address);
    }
    // Sound Channel 2
    else if (address >= 0xFF16 && address <= 0xFF19) {
        res = square2.read(address);
    }
    // Wave Channel
    else if (address >= 0xFF1A && address <= 0xFF1E) {
        res = wave.read(address);
    }
    // Noise Channel
    else if (address >= 0xFF20 && address <= 0xFF23) {
        res = noise.read(address);
    }
    // Wave Pattern RAM
    else if (address >= 0xFF30 && address <= 0xFF3F) {
        res = wave.read(address);
    }
    return res;
}

void APU::write(uint16_t address, uint8_t data) {
    if (address == 0xFF26) {
        bool turningOn = ((data & 0x80) == 0x80) && ((NR52 & 0x80) != 0x80);
        if (turningOn) {
            frameSequenceCount = 2048; // May not be needed
            frameSequencer = 0;
        }
        if ((data & 0x80) == 0x80) {
            NR52 |= 0x80;
        } else {
            // Clear all registers and timer (if any)
            clear();
            
        }
        return;
    }
    // Audio registers become read only if audio is off
    // Sound Panning
    if (address == 0xFF25 && (NR52 & 0x80) != 0x00) {
        NR51 = data;
    }
    // Master volume & VIN Panning
    else if (address == 0xFF24 && (NR52 & 0x80) != 0x00) {
        NR50 = data;
    }
    // Sound Channel 1
    else if (address >= 0xFF10 && address <= 0xFF14 && (NR52 & 0x80) != 0x00) {
        square1.write(address, data);
    }
    // Sound Channel 2
    else if (address >= 0xFF16 && address <= 0xFF19 && (NR52 & 0x80) != 0x00) {
        square2.write(address, data);
    }
    // Wave Channel
    else if (address >= 0xFF1A && address <= 0xFF1E && (NR52 & 0x80) != 0x00) {
        wave.write(address, data);
    }
    // Noise Channel -> writes to NR41 is never ignored
    else if ((address >= 0xFF20 && address <= 0xFF23 && (NR52 & 0x80) != 0x00) || address == 0xFF20) {
        noise.write(address, data);
    }
    // Wave Pattern RAM
    else if (address >= 0xFF30 && address <= 0xFF3F) {
        wave.write(address, data);
    }
}

void APU::tick(uint8_t cycles) {
    for (int i = 0; i < cycles; ++i) {
        if ((NR52 & 0x80) == 0x80) {
            if (--frameSequenceCount == 0) {
                frameSequenceCount = 8192;
                
                switch (frameSequencer) {
                    case 0:
                        square1.tickLength();
                        square2.tickLength();
                        wave.tickLength();
                        noise.tickLength();
                        break;

                    case 2:
                        square1.tickSweep();
                        square1.tickLength();
                        square2.tickLength();
                        wave.tickLength();
                        noise.tickLength();
                        break;

                    case 4:
                        square1.tickLength();
                        square2.tickLength();
                        wave.tickLength();
                        noise.tickLength();
                        break;

                    case 6:
                        square1.tickSweep();
                        square1.tickLength();
                        square2.tickLength();
                        wave.tickLength();
                        noise.tickLength();
                        break;


                    case 7:
                        square1.tickEnv();
                        square2.tickEnv();
                        noise.tickEnv();
                        break;

                    default:
                        break;
                }
                frameSequencer = (frameSequencer + 1) % 8;
            }

            square1.tick();
            square2.tick();
            wave.tick();
            noise.tick();            
        }

        if (--sampleCount == 0) {
            sampleCount = 95;

            float left = 0.0f;
            float right = 0.0f;
            uint8_t leftVol = ((NR50 >> 4) & 0x07) + 1;
            uint8_t rightVol = (NR50 & 0x07) + 1;

            // Left
            if ((NR51 & 0x10) == 0x10) left += (float)square1.getOutputVolume();
            if ((NR51 & 0x20) == 0x20) left += (float)square2.getOutputVolume();
            if ((NR51 & 0x40) == 0x40) left += (float)wave.getOutputVolume();
            if ((NR51 & 0x80) == 0x80) left += (float)noise.getOutputVolume();

            left = (left * leftVol) / 8.0f;

            // Right
            if ((NR51 & 0x01) == 0x01) right += (float)square1.getOutputVolume();
            if ((NR51 & 0x02) == 0x02) right += (float)square2.getOutputVolume();
            if ((NR51 & 0x04) == 0x04) right += (float)wave.getOutputVolume();
            if ((NR51 & 0x08) == 0x08) right += (float)noise.getOutputVolume();

            right = (right * rightVol) / 8.0f;

            sampleBuffer.push_back(left * 0.02f);
            sampleBuffer.push_back(right * 0.02f);
        }

        if (sampleBuffer.size() >= 512) {
            if (SDL_GetQueuedAudioSize(audioDeviceId) < 8192 * sizeof(float)) {
                SDL_QueueAudio(audioDeviceId, sampleBuffer.data(), sampleBuffer.size()*sizeof(float));
            }

            sampleBuffer.clear();
           
        }

    }
}

void APU::clear() {
    NR52 = 0x00;
    NR51 = 0x00;
    NR50 = 0x00;

    frameSequenceCount = 8192;
    frameSequencer = 0;
    sampleCount = 95;

    sampleBuffer.clear();

    square1.clear();
    square2.clear();
    wave.clear();
    noise.clear();
}

