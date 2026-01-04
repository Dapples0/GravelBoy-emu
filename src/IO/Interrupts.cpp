#include "Interrupts.h"

uint8_t Interrupts::getIF()
{
    return IF;
}

uint8_t Interrupts::getIE()
{
    return IE;
}

void Interrupts::setIF(uint8_t val) {
    IF = val;
}

void Interrupts::setIE(uint8_t val) {
    IE = val;
}
