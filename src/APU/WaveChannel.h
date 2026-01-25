#ifndef WAVECHANNEL_H
#define WAVECHANNEL_H

#include <cstdint>
#include <array>

class WaveChannel {
    public:
        WaveChannel();
        ~WaveChannel();

        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t data);

        void clear();
        void tick();
        void tickLength();

        uint8_t getOutputVolume();

        bool isActive();
    private:
        uint8_t NR30 = 0x7F; // Channel 3 DAC Enable
        uint8_t NR31 = 0xFF;// Channel 3 Length Timer
        uint8_t NR32 = 0x9F; // Channel 3 output level
        uint8_t NR33 = 0xFF; // Channel 3 period low
        uint8_t NR34 = 0xBF; // Channel 3 period high & control
        std::array<uint8_t, 16> wavePatternRAM; // Wave Pattern RAM

        bool active = false;
        uint16_t timer = 0;
        uint16_t lengthTimer = 0;
        uint16_t periodDivider = 0;
        uint8_t wavePosition = 0;

};






#endif