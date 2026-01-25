#ifndef SQUARESWEEPCHANNEL_H
#define SQUARESWEEPCHANNEL_H

#include <cstdint>

class SquareSweepChannel {
    public:
        SquareSweepChannel();
        ~SquareSweepChannel();

        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t data);

        void clear();
        void tick();
        void tickLength();
        void tickEnv();
        void tickSweep();
    private:
        uint8_t NR10 = 0x80; // Channel 1 Sweep
        uint8_t NR11 = 0xBF; // Channel 1 Length Timer & Duty Cycle
        uint8_t NR12 = 0xF3; // Channel 1 Volume & Envelope
        uint8_t NR13 = 0xFF; // Channel 1 period low
        uint8_t NR14 = 0xBF; // Channel 1 period high & control

        bool active = false;
        uint16_t timer = 0;
        uint16_t lengthTimer = 0;
        uint16_t envelopeTimer = 0;
        uint8_t volume = 0;
        uint16_t periodDivider = 0;

        uint8_t sweepTimer = 0;
        uint16_t shadowPeriodDivider = 0;
        bool sweepEnable = false;

        uint16_t dutyPosition = 0;
};






#endif