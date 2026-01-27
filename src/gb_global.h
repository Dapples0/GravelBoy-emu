#ifndef GB_GLOBAL_H
#define GB_GLOBAL_H

#include <array>

struct gb_global {
    std::array<uint8_t, 4> framerate = {60, 120, 180, 240};
    uint8_t frameRateIndex = 0;
    uint32_t ticksPerFrame;

    bool cgb = false;

};






#endif