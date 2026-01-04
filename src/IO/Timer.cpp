#include "Timer.h"
#include <iostream>


Timer::Timer()
{
        
}

Timer::~Timer()
{
}

void Timer::connect(Interrupts *interrupt) {
    this->interrupt = interrupt;
}

void Timer::write(uint16_t address, uint8_t data) {
    if (address == DIV_ADDRESS) {
        bool preTimerCheck = timerControlCheck();

        DIV = 0x0000;


        bool postTimerCheck = timerControlCheck();

        if (preTimerCheck && !postTimerCheck) {
            incrementTIMA();
        }
    }
    else if (address == TIMA_ADDRESS) {
        TIMA = data;
        TIMADelay = 0;
    }
    else if (address == TMA_ADDRESS) {
        TMA = data;
    }
    else if (address == TAC_ADDRESS) {
        bool preTimerCheck = timerControlCheck();
        
        TAC = data & 0x07;

        bool postTimerCheck = timerControlCheck();

        if (preTimerCheck && !postTimerCheck) {
            incrementTIMA();
        }

    } 
    else {
        // Should not ever occur if function is used properly
        std::cout << "Bad write at Timer\n";
    }

}

uint8_t Timer::read(uint16_t address) {
    if (address == DIV_ADDRESS) {
        return (DIV >> 8);
    }
    if (address == TIMA_ADDRESS) {
        return TIMA;
    }
    if (address == TMA_ADDRESS) {
        return TMA;
    }
    if (address == TAC_ADDRESS) {
        return TAC;
    } 


    // Should not ever occur if function is used properly
    std::cout << "Bad read at Timer\n";
    return 0x00;
}

void Timer::tick(uint32_t cyclesPassed) {
    // Loop through each individual cycle passed per cpu operation
    for (uint32_t i = 0; i < cyclesPassed; ++i) {
        if (TIMADelay > 0) {
            TIMADelay--;
            if (TIMADelay == 0) {
                TIMA = TMA;
                interrupt->setIF(interrupt->getIF() | 0x04);
            }
        }
        bool preTimerCheck = timerControlCheck();
        

        DIV++;

        bool postTimerCheck = timerControlCheck();

        if (preTimerCheck && !postTimerCheck) {
            if (TIMA == 0xFF) {
                TIMA = 0;
                TIMADelay = 4;
            } else {
                TIMA++;
            }
        }
    }
}

bool Timer::timerControlCheck()
{
    if ((TAC & 0x4) == 0) return false;

    switch(TAC & 0x3) {
        case 0: // 1024 T-cylces
            return (DIV & (1 << 9)) != 0;
        case 1: // 16 T-cylces
            return (DIV & (1 << 3)) != 0;
        case 2: // 64 T-cylces
            return (DIV & (1 << 5)) != 0;
        case 3: // 256 T-cylces
            return (DIV & (1 << 7)) != 0;

        default:
            // Should never occur, if it somehow does then return false idk
            return false;
    }
}

void Timer::incrementTIMA() {
    if (TIMA == 0xFF) {
        TIMA = 0;
        TIMADelay = 4;
    } else {
        TIMA++;
    }
}