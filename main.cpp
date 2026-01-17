#include <iostream>

#include "src/gb.h"


int main() {
    gb gb;
    // 02-interrupts instr_timing interrupt_time dmg-acid2 cgb-acid2.gbc cpu_instrs
    // 01-read_timing  02-write_timing 03-modify_timing
    // hblank_vram_dma.gbc
    gb.run("rom/hblank_vram_dma.gbc");






    return 0;
}