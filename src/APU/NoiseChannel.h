#ifndef NOISECHANNEL_H
#define NOISECHANNEL_H

#include <cstdint>


class NoiseChannel {
    public:
        NoiseChannel();
        ~NoiseChannel();

        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t data);

        void clear();
        void tick();
        void tickLength();
        void tickEnv();
        
        uint8_t getOutputVolume();

        bool isActive();
    private:
        uint8_t NR41 = 0xFF; // Channel 4 Length Timer
        uint8_t NR42 = 0x00; // Channel 4 Volume & Envelope
        uint8_t NR43 = 0x00; // Channel 4 frequency & randomness
        uint8_t NR44 = 0xFB; // Channel 4 control

        bool active = false;
        uint16_t timer = 0;
        uint16_t lengthTimer = 0;
        uint16_t envelopeTimer = 0;
        uint16_t periodDivider = 0;
        uint8_t volume = 0;
        uint16_t lfsr = 0x0000;
};






#endif