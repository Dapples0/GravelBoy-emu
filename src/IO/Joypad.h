#ifndef JOYPAD_H
#define JOYPAD_H

#include <SDL2/SDL.h>
#include <array>

#include "../constants.h"
#include "Interrupts.h"


class Joypad {
    public:
        Joypad();
        ~Joypad();

        void connect(Interrupts *interrupt);
        void write(uint8_t data);
        uint8_t read();
    private:
        Interrupts *interrupt;

        uint8_t joyp;

};






#endif