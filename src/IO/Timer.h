#ifndef TIMER_H
#define TIMER_H

#include "../IO/Interrupts.h"
#include "../constants.h"

class Timer {
    public:
        Timer();
        ~Timer();

        void connect(Interrupts *interrupt);

        void write(uint16_t address, uint8_t data);
        uint8_t read(uint16_t address);
        void tick(uint32_t cyclesPassed);
    private:
        Interrupts *interrupt;

        // Registers
        uint16_t DIV = 0x0000; // Divider Register -> acts as internal counter
        uint8_t TIMA = 0x00; // Timer Counter
        uint8_t TMA = 0x00; // Timer Modulo
        uint8_t TAC = 0x00; // Timer Control

        uint8_t TIMADelay = 0;


        bool timerControlCheck();
        void incrementTIMA();


};






#endif