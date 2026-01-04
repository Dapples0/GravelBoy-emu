#include <cstdint>
#ifndef CONSTANTS_H
#define CONSTANTS_H

#define ROM_BANK_SIZE 0x4000
#define WRAM_BANK_SIZE 0x1000
#define SRAM_BANK_SIZE 0x2000
#define SRAM_UNUSED_BANK_SIZE 0x800
#define WRAM_OFFSET 0xC000

#define REG_A 0
#define REG_F 1
#define REG_B 2
#define REG_C 3
#define REG_D 4
#define REG_E 5
#define REG_H 6
#define REG_L 7

#define IE_ADDRESS 0xFFFF
#define IF_ADDRESS 0xFF0F
#define DIV_ADDRESS 0xFF04
#define TIMA_ADDRESS 0xFF05
#define TMA_ADDRESS 0xFF06
#define TAC_ADDRESS 0xFF07

#define VBLANK_BIT 0x01
#define LCD_BIT 0x02
#define TIMER_BIT 0x04
#define SERIAL_BIT 0x08
#define JOYPAD_BIT 0x10
#define VBLANK_INT 0x40
#define STAT_INT 0x48
#define TIMER_INT 0x50
#define SERIAL_INT 0x58
#define JOYPAD_INT 0x60

#define CLOCK_SPEED 0x400000
#define DOUBLE_CLOCK_SPEED 0x800000
#define DIV_RATE 0x4000
#endif