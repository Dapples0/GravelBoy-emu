#ifndef INTERRUPTS_H
#define INTERRUPTS_H
#include <cstdint>

class Interrupts {
    public:
        uint8_t getIF();
        uint8_t getIE();
        void setIF(uint8_t val);
        void setIE(uint8_t val);
    private:
       // Interrupt Enable Registers
        uint8_t IF = 0x00;
        uint8_t IE = 0x00;
        
};






#endif