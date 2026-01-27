#include "Joypad.h"
#include <iostream>
Joypad::Joypad() {
    joyp = 0x00;
}

Joypad::~Joypad() {
}

void Joypad::connect(Interrupts *interrupt) {
    this->interrupt = interrupt;
}


void Joypad::write(uint8_t data) {

    joyp = (joyp & 0x0F) | (data & 0x30);
}

uint8_t Joypad::read() {

    uint8_t res = 0xCF;
    res |= (joyp & 0x30);

    const uint8_t *key = SDL_GetKeyboardState(NULL);
    uint8_t buttons = 0x0F;

    if (!(res & JOYPAD_SELECT_DIRECTION_PAD)) {
        if (key[SDL_SCANCODE_RIGHT]) buttons &= ~JOYPAD_DIRECTION_RIGHT;
        if (key[SDL_SCANCODE_LEFT]) buttons &= ~JOYPAD_DIRECTION_LEFT;
        if (key[SDL_SCANCODE_UP]) buttons &= ~JOYPAD_DIRECTION_UP;
        if (key[SDL_SCANCODE_DOWN]) buttons &= ~JOYPAD_DIRECTION_DOWN;
    }

    if (!(res & JOYPAD_SELECT_BUTTONS)) {
        if (key[SDL_SCANCODE_X]) buttons &= ~JOYPAD_BUTTON_A; // Button A
        if (key[SDL_SCANCODE_Z]) buttons &= ~JOYPAD_BUTTON_B; // Button B
        if (key[SDL_SCANCODE_C]) buttons &= ~JOYPAD_BUTTON_SELECT; // Select
        if (key[SDL_SCANCODE_SPACE]) buttons &= ~JOYPAD_BUTTON_START; // Start
    }

    if ((joyp & 0x0F) != buttons && buttons != 0x0F) {
        interrupt->setIF(interrupt->getIF() | 0x10); 
    }

    joyp = (res & 0x30) | buttons;
    return joyp;
}
