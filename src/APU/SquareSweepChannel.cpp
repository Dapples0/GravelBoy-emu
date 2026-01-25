#include "SquareSweepChannel.h"

SquareSweepChannel::SquareSweepChannel()
{
}

SquareSweepChannel::~SquareSweepChannel()
{
}

uint8_t SquareSweepChannel::read(uint16_t address) {
    uint8_t res = 0xFF;
    if (address == 0xFF10) {
        res = NR10 | 0x80;
    } else if (address == 0xFF11) {
        res = NR11 | 0x3F;
    } else if (address == 0xFF12) {
        res = NR12;
    } else if (address == 0xFF13) {
        res = 0xFF;
    } else if (address == 0xFF14) {
        // Only Length Enable can be read
        res = NR14 | 0xBF;
    }
    return res;
}

void SquareSweepChannel::write(uint16_t address, uint8_t data) {
    if (address == 0xFF10) {
        NR10 = data;
    } else if (address == 0xFF11) {
        NR11 = data;
        lengthTimer = 64 - (NR11 & 0x3F);
    } else if (address == 0xFF12) {
        NR12 = data;
        if ((data & 0xF8) == 0) active = false;
    } else if (address == 0xFF13) {
        NR13 = data;
    } else if (address == 0xFF14) {
        NR14 = data;
        if ((data & 0x80) == 0x80) {
            if ((NR12 & 0xF8) != 0) active = true;
            
            if (lengthTimer == 0) lengthTimer = 64;

            envelopeTimer = NR12 & 0x07;

            volume = (NR12 >> 4) & 0x0F;

            periodDivider = NR13 | ((NR14 & 0x07) << 8);
            shadowPeriodDivider = periodDivider;

            uint8_t sweepPeriod = (NR10 >> 4) & 0x07;
            sweepTimer = (sweepPeriod == 0x00) ? 8 : sweepPeriod;
            sweepEnable = (sweepPeriod != 0x00 || ((NR10 & 0x07) != 0x00));

            timer = (2048 - periodDivider) * 4;
        }

    }
}

void SquareSweepChannel::clear() {
    NR10 = 0x00;
    NR11 = 0x00;
    NR12 = 0x00;
    NR13 = 0x00;
    NR14 = 0x00;

    active = false;
    timer = 0;
    lengthTimer = 0;
    envelopeTimer = 0;
    volume = 0;
    periodDivider = 0;

    sweepTimer = 0;
    shadowPeriodDivider = 0;
    sweepEnable = false;
    dutyPosition = 0;
}

void SquareSweepChannel::tick() {
    if (timer > 0) {
        timer--;
        if (timer == 0) {
            timer = (2048 - periodDivider) * 4;
            dutyPosition = (dutyPosition + 1) % 8;
        }
    }
}

void SquareSweepChannel::tickLength() {
    if ((NR14 & 0x40) == 0x40) {
        if (lengthTimer > 0) {
            lengthTimer --;

            if (lengthTimer == 0) {
                active = false;
            }
        }
    }
}

void SquareSweepChannel::tickEnv() {
    uint8_t period = (NR12 & 0x07);
    if (period == 0) return;
    if (envelopeTimer > 0) {
        envelopeTimer--;

        if (envelopeTimer == 0) {
            envelopeTimer = period;
            if ((NR12 & 0x08) == 0x08 && volume < 15) volume++;
            else if ((NR12 & 0x08) != 0x08 && volume > 0) volume--;
        }


    }
}

void SquareSweepChannel::tickSweep() {
    if (sweepTimer > 0) {
        sweepTimer--;

        if (sweepTimer == 0) {
            uint8_t sweepPeriod = (NR10 >> 4) & 0x07;
            sweepTimer = (sweepPeriod == 0x00) ? 8 : sweepPeriod;

            if (sweepEnable && sweepPeriod > 0) {
                uint16_t newPeriod = calculateSweep();

                if (newPeriod <= 2047 && (NR10 & 0x07) > 0) {
                    shadowPeriodDivider = newPeriod;
                    periodDivider = newPeriod;

                    calculateSweep();
                }
            }
        }
    }
}

uint8_t SquareSweepChannel::getOutputVolume() {
    uint8_t output = 0;
    if (active && (NR12 & 0xF8) != 0) {
        uint8_t duty = (NR11 >> 6) & 0x03;
        output = volume * dutyTable[duty][dutyPosition];
    }

    return output;
}

bool SquareSweepChannel::isActive()
{
return active;
}

uint16_t SquareSweepChannel::calculateSweep() {
    uint8_t sweepStep = (NR10 & 0x07);
    uint8_t sweepDir = (NR10 >> 3) & 0x01;

    uint16_t newPeriod = shadowPeriodDivider >> sweepStep;

    if (sweepDir == 0x01) newPeriod = shadowPeriodDivider - newPeriod;
    else newPeriod = shadowPeriodDivider + newPeriod;
    
    if (newPeriod > 2047) active = false;


    return newPeriod;
}
