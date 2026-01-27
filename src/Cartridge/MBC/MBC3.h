#ifndef MBC3_H
#define MB3C_H

#include "../Cartridge.h"
#include "../../constants.h"
#include "../../gb_global.h"

#include <iostream>


class MBC3 : public Cartridge {
    public:
        MBC3(std::vector<std::array<uint8_t, ROM_BANK_SIZE>> romData, int romSize, int extRamCode, bool timer);
        ~MBC3();

        uint8_t read(uint16_t address) override;
        void write(uint16_t address, uint8_t data) override;
        void setBattery(std::string title) override;   
    private:
        // Registers
        bool ramEnable; // Determines whether external RAM can be read or written
        uint8_t romBankNumber;
        uint8_t ramBankRTCSelect; // RAM Bank Number - or - RTC Register Select (Write Only)

        // Registers for timers, P.S this implementation does not wait 4 M cycles between accesses of RTC registers
        bool RTC;
        /**
         * 0 - RTC S - Seconds
         * 1 - RTC M - Minutes
         * 2 - RTC H - Hours
         * 3 - RTC DL - Lower Day Register
         * 4 - RTC DH - Upper Day Register
         */
        std::array<uint8_t, 5> RTCRegisters;

        // Time passed in real life
        uint8_t rtSec = 0x00;
        uint8_t rtMin = 0x00;
        uint8_t rtHours = 0x00;
        uint8_t rtDaysLower = 0x00;
        uint8_t rtDaysUpper = 0x00;

        // Time since last timer update
        time_t curTime = 0;

        uint8_t latch = 0x00;

        void updateTimer();
        void latchTimer();

        void MBC3Save();


};









#endif