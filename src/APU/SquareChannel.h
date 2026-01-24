#ifndef SQUARECHANNEL_H
#define SQUARECHANNEL_H

#include <cstdint>

class SquareChannel {
    public:
        SquareChannel();
        ~SquareChannel();

        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t data);

        void clear();
    private:
        uint8_t NR21 = 0x3F; // Channel 2 Length Timer & Duty Cycle
        uint8_t NR22 = 0x00; // Channel 2 Volume & Envelope
        uint8_t NR23 = 0xFF; // Channel 2 period low
        uint8_t NR24 = 0xBF; // Channel 2 period high & control

        bool active = false;
        uint16_t lengthTimer = 0;
        uint16_t envelopeTimer = 0;
        uint8_t volume = 0;
        uint16_t timer = 0;
        uint16_t periodDivider = 0;

        uint16_t dutyPosition = 0;
};






#endif