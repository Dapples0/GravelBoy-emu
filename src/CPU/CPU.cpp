#include "CPU.h"

#include <iostream>
#include <iomanip>


CPU::CPU() {


}

CPU::~CPU() {
}

void CPU::connect(MMU *mmu) {
    this->mmu = mmu;
}

uint32_t CPU::execute() {
    
    handleInterrupts();
    if (!halt) {
        uint8_t opcode = mmu->read8(pc);     
        executeInstruction(opcode);
        cycles += cyclesPassed;

    } else {
        cyclesPassed = 4;
        cycles += cyclesPassed;
    }
    
    return cyclesPassed;
}

void CPU::executeInstruction(uint8_t opcode) {

    pc++;
    
    cyclesPassed = opcodeCycles[opcode];
    switch (opcode) {
        case 0xCB: // 0xCB Prefixed
            executeCBInstruction(mmu->read8(pc));
            break;

        /**
         * Misc Instructions
         */
        case 0x00: // NOP
            // Do nothing
            break;
        case 0x27: // DAA
        {
            uint8_t tempA = registers[REG_A];
            uint8_t adjustment = 0x00;
            if (getN()) {
                if (getH()) adjustment += 0x06;

                if (getC()) adjustment += 0x60;
                

                registers[REG_A] -= adjustment;


            } else {
                if (getH() || (registers[REG_A] & 0x0F) > 0x09) adjustment += 0x06;

                if (getC() || (registers[REG_A] > 0x99))  {
                    adjustment += 0x60;
                    setC(true);
                }
                
                registers[REG_A] += adjustment;
            }

            setH(false);
            setZ(registers[REG_A] == 0);
            
        }
            break;
            

        case 0x10: // STOP - NOTES: As per pandocs, licensed roms do not use STOP outside of GCB speed switching. So implementing DMG slow mode might not be necessary/can be implemented later as a TODO
        {
            // Only used for CGB
            if (!CGBMode) {
                return;
            }
            uint8_t key1 = mmu->read8(0xFF4D);

            // If armed switch to other mode
            if ((key1 & 0x01) == 0x01) {
                doubleSpeed = !doubleSpeed;

                // Clear bit0 and set speed
                ((key1 & 0x80) == 0x80) ? mmu->write8(0xFF4D, 0x00) : mmu->write8(0xFF4D, 0x80);   
            }

            pc++;            
        }
            break;


        case 0x07: // RLCA
        {
            uint8_t carry = (registers[REG_A] & 0x80) >> 7;
            registers[REG_A] = (registers[REG_A] << 1) | carry;

            setC(carry != 0);
            setN(false);
            setH(false);
            setZ(false);
        }
            break;

        case 0x0F: // RRCA
        {
            uint8_t carry = (registers[REG_A] & 0x01);
            registers[REG_A] = (registers[REG_A] >> 1) | (carry << 7);

            setC(carry != 0);
            setN(false);
            setH(false);
            setZ(false);        
        }
            break;

        case 0x17: // RLA
        {
            uint8_t cFlag = (uint8_t)getC();
            uint8_t carry = (registers[REG_A] & 0x80);
            registers[REG_A] = (registers[REG_A] << 1) | cFlag;

            setC(carry != 0);
            setN(false);
            setH(false);
            setZ(false);
        }
            break;

        case 0x1F: // RRA
        {
            uint8_t cFlag = (uint8_t)getC();
            uint8_t carry = (registers[REG_A] & 0x01);
            registers[REG_A] = (registers[REG_A] >> 1) | (cFlag << 7);

            setC(carry != 0);
            setN(false);
            setH(false);
            setZ(false);            
        }
            break;

        /**
         * Load Instructions
         */
        case 0x01: // LD BC, u16
            setBC(mmu->read16(pc));
            pc += 2;
            break;
        case 0x02: // LD (BC), A
            mmu->write8(getBC(), registers[REG_A]);
            break;

        case 0x06: // LD B, u8
            registers[REG_B] = mmu->read8(pc);
            pc++;
            break;

        case 0x0A: // LD A, (BC)
            registers[REG_A] = mmu->read8(getBC());
            break;

        case 0x0E: // LD C, u8
            registers[REG_C] = mmu->read8(pc);
            pc++;
            break;

        case 0x11: // LD DE, u16
            setDE(mmu->read16(pc));
            pc += 2;
            break;

        case 0x12: // LD (DE), A
            mmu->write8(getDE(), registers[REG_A]);
            break;

        case 0x16: // LD D, u8
            registers[REG_D] = mmu->read8(pc);
            pc++;
            break;

        case 0x1A: // LD A, (DE)
            registers[REG_A] = mmu->read8(getDE());
            break;

        case 0x1E: // LD E, u8
            registers[REG_E] = mmu->read8(pc);
            pc++;
            break;

        case 0x21: // LD HL, u16
            setHL(mmu->read16(pc));
            pc += 2;
            break;

        case 0x22: // LD (HL+), A
            mmu->write8(getHL(), registers[REG_A]);
            setHL(getHL() + 1);
            break;

        case 0x26: // LD H, u8
            registers[REG_H] = mmu->read8(pc);
            pc++;
            break;

        case 0x2A: // LD A, (HL+)
            registers[REG_A] = mmu->read8(getHL());
            setHL(getHL() + 1);
            break;

        case 0x2E: // LD L, u8
            registers[REG_L] = mmu->read8(pc);
            pc++;
            break;


        case 0x32: // LD (HL-), A
            mmu->write8(getHL(), registers[REG_A]);
            setHL(getHL() - 1);
            break;

        case 0x36: // LD (HL), u8
            mmu->write8(getHL(), mmu->read8(pc));
            pc++;
            break;

        case 0x3A: // LD A, (HL-)
            registers[REG_A] = mmu->read8(getHL());
            setHL(getHL() - 1);
            break;

        case 0x3E: // LD A, u8
            registers[REG_A] = mmu->read8(pc);
            pc++;
            break;
    
        case 0x40: // LD B, B
            registers[REG_B] = registers[REG_B];
            break;

        case 0x41: // LD B, C
            registers[REG_B] = registers[REG_C];
            break;

        case 0x42: // LD B, D
            registers[REG_B] = registers[REG_D];
            break;

        case 0x43: // LD B, E
            registers[REG_B] = registers[REG_E];
            break;
        
        case 0x44: // LD B, H
            registers[REG_B] = registers[REG_H];
            break;

        case 0x45: // LD B, L
            registers[REG_B] = registers[REG_L];
            break;

        case 0x46: // LD B, (HL)
            registers[REG_B] = mmu->read8(getHL());
            break;

        case 0x47: // LD B, A
            registers[REG_B] = registers[REG_A];
            break;
        
        case 0x48: // LD C, B
            registers[REG_C] = registers[REG_B];
            break;

        case 0x49: // LD C, C
            registers[REG_C] = registers[REG_C];
            break;

        case 0x4A: // LD C, D
            registers[REG_C] = registers[REG_D];
            break;

        case 0x4B: // LD C, E
            registers[REG_C] = registers[REG_E];
            break;

        case 0x4C: // LD C, H
            registers[REG_C] = registers[REG_H];
            break;

        case 0x4D: // LD C, L
            registers[REG_C] = registers[REG_L];
            break;

        case 0x4E: // LD C, (HL)
            registers[REG_C] = mmu->read8(getHL());
            break;

        case 0x4F: // LD C, A
            registers[REG_C] = registers[REG_A];
            break;

        case 0x50: // LD D, B
            registers[REG_D] = registers[REG_B];
            break;

        case 0x51: // LD D, C
            registers[REG_D] = registers[REG_C];
            break;

        case 0x52: // LD D, D
            registers[REG_D] = registers[REG_D];
            break;

        case 0x53: // LD D, E
            registers[REG_D] = registers[REG_E];
            break;

        case 0x54: // LD D, H
            registers[REG_D] = registers[REG_H];
            break;

        case 0x55: // LD D, L
            registers[REG_D] = registers[REG_L];
            break;


        case 0x56: // LD D, (HL)
            registers[REG_D] = mmu->read8(getHL());
            break;

        case 0x57: // LD D, A
            registers[REG_D] = registers[REG_A];
            break;

        case 0x58: // LD E, B
            registers[REG_E] = registers[REG_B];
            break;

        case 0x59: // LD E, C
            registers[REG_E] = registers[REG_C];
            break;

        case 0x5A: // LD E, D
            registers[REG_E] = registers[REG_D];
            break;

        case 0x5B: // LD E, E
            registers[REG_E] = registers[REG_E];
            break;

        case 0x5C: // LD E, H
            registers[REG_E] = registers[REG_H];
            break;

        case 0x5D: // LD E, L
            registers[REG_E] = registers[REG_L];
            break;

        case 0x5E: // LD E, (HL)
            registers[REG_E] = mmu->read8(getHL());
            break;

        case 0x5F: // LD E, A
            registers[REG_E] = registers[REG_A];
            break;

        case 0x60: // LD H, B
            registers[REG_H] = registers[REG_B];
            break;

        case 0x61: // LD H, C
            registers[REG_H] = registers[REG_C];
            break;

        case 0x62: // LD H, D
            registers[REG_H] = registers[REG_D];
            break;

        case 0x63: // LD H, E
            registers[REG_H] = registers[REG_E];
            break;

        case 0x64: // LD H, H
            registers[REG_H] = registers[REG_H];
            break;

        case 0x65: // LD H, L
            registers[REG_H] = registers[REG_L];
            break;

        case 0x66: // LD H, (HL)
            registers[REG_H] = mmu->read8(getHL());
            break;

        case 0x67: // LD H, A
            registers[REG_H] = registers[REG_A];
            break;

        case 0x68: // LD L, B
            registers[REG_L] = registers[REG_B];
            break;


        case 0x69: // LD L, C
            registers[REG_L] = registers[REG_C];
            break;

        case 0x6A: // LD L, D
            registers[REG_L] = registers[REG_D];
            break;

        case 0x6B: // LD L, E
            registers[REG_L] = registers[REG_E];
            break;


        case 0x6C: // LD L, H
            registers[REG_L] = registers[REG_H];
            break;

        case 0x6D: // LD L, L
            registers[REG_L] = registers[REG_L];
            break;

        case 0x6E: // LD L, (HL)
            registers[REG_L] = mmu->read8(getHL());
            break;

        case 0x6F: // LD L, A
            registers[REG_L] = registers[REG_A];
            break;

        case 0x70: // LD (HL), B
            mmu->write8(getHL(), registers[REG_B]);
            break;

        case 0x71: // LD (HL), C
            mmu->write8(getHL(), registers[REG_C]);
            break;

        case 0x72: // LD (HL), D
            mmu->write8(getHL(), registers[REG_D]);
            break;
        case 0x73: // LD (HL), E
            mmu->write8(getHL(), registers[REG_E]);
            break;
        case 0x74: // LD (HL), H
            mmu->write8(getHL(), registers[REG_H]);
            break;

        case 0x75: // LD (HL), L
            mmu->write8(getHL(), registers[REG_L]);
            break;

        case 0x77: // LD (HL), A
            mmu->write8(getHL(), registers[REG_A]);
            break;

        case 0x78: // LD A, B
            registers[REG_A] = registers[REG_B];
            break;

        case 0x79: // LD A, C
            registers[REG_A] = registers[REG_C];
            break;

        case 0x7A: // LD A, D
            registers[REG_A] = registers[REG_D];
            break;

        case 0x7B: // LD A, E
            registers[REG_A] = registers[REG_E];
            break;

        case 0x7C: // LD A, H
            registers[REG_A] = registers[REG_H];
            break;

        case 0x7D: // LD A, L
            registers[REG_A] = registers[REG_L];
            break;

        case 0x7E: // LD A, (HL)
            registers[REG_A] = mmu->read8(getHL());
            break;

        case 0x7F: // LD A, A
            registers[REG_A] = registers[REG_A];
            break;

        case 0xE0: // LD (FF00+u8), A
            mmu->write8(0xFF00 + mmu->read8(pc), registers[REG_A]);
            pc++;
            break;

        case 0xE2: // LD (FF00+C), A
            mmu->write8(0xFF00 + registers[REG_C], registers[REG_A]);
            break;

        case 0xEA: // LD (u16), A
            mmu->write8(mmu->read16(pc), registers[REG_A]);
            pc += 2;
            break;

        case 0xF0: // LD A, (FF00+u8)
            registers[REG_A] = mmu->read8(0xFF00 + mmu->read8(pc));
            pc++;
            break;

        case 0xF2: // LD A, (FF00+C),
            registers[REG_A] = mmu->read8(0xFF00 + registers[REG_C]);
            break;

        case 0xFA: // LD A, (u16)
            registers[REG_A] = mmu->read8(mmu->read16(pc));
            pc += 2;
            break; 

        /**
         * 8-bit Arithmetic Instructions
         */
        case 0x04: // INC B
            registers[REG_B] = INC8(registers[REG_B]);
            break;
        
        case 0x05: // DEC B
            registers[REG_B] = DEC8(registers[REG_B]);
            break;

        case 0x0C: // INC C
            registers[REG_C] = INC8(registers[REG_C]);
            break;
    
        case 0x0D: // DEC C
            registers[REG_C] = DEC8(registers[REG_C]);
            break;

        case 0x14: // INC D
            registers[REG_D] = INC8(registers[REG_D]);
            break;

        case 0x15: // DEC D
            registers[REG_D] = DEC8(registers[REG_D]);
            break;

        case 0x1C: // INC E
            registers[REG_E] = INC8(registers[REG_E]);
            break;

        case 0x1D: // DEC E
            registers[REG_E] = DEC8(registers[REG_E]);
            break;

        case 0x24: // INC H
            registers[REG_H] = INC8(registers[REG_H]);
            break;

        case 0x25: // DEC H
            registers[REG_H] = DEC8(registers[REG_H]);
            break;
        
        case 0x2C: // INC L
            registers[REG_L] = INC8(registers[REG_L]);
            break;
        
        case 0x2D: // DEC L
            registers[REG_L] = DEC8(registers[REG_L]);
            break;

        case 0x34: // INC (HL)
            mmu->write8(getHL(), INC8(mmu->read8(getHL())));
            break;

        case 0x35: // DEC (HL)
            mmu->write8(getHL(), DEC8(mmu->read8(getHL())));
            break;
    
        case 0x3C: // INC A
            registers[REG_A] = INC8(registers[REG_A]);
            break;

        case 0x3D: // DEC A
            registers[REG_A] = DEC8(registers[REG_A]);
            break;

        case 0x80: // ADD A, B
            ADD8(registers[REG_B]);
            break;

        case 0x81: // ADD A, C
            ADD8(registers[REG_C]);
            break;

        case 0x82: // ADD A, D
            ADD8(registers[REG_D]);
            break;

        case 0x83: // ADD A, E
            ADD8(registers[REG_E]);
            break;
            
        case 0x84:// ADD A, H
            ADD8(registers[REG_H]);
            break;

        case 0x85: // ADD A, L
            ADD8(registers[REG_L]);
            break;

        case 0x86: // ADD A, (HL)
            ADD8(mmu->read8(getHL()));
            break;

        case 0x87: // ADD A, A
            ADD8(registers[REG_A]);
            break;

        case 0x88: // ADC A, B
            ADC(registers[REG_B]);
            break;

        case 0x89: // ADC A, C
            ADC(registers[REG_C]);
            break;

        case 0x8A: // ADC A, D
            ADC(registers[REG_D]);
            break;

        case 0x8B: // ADC A, E
            ADC(registers[REG_E]);
            break;

        case 0x8C: // ADC A, H
            ADC(registers[REG_H]);
            break;

        case 0x8D: // ADC A, L
            ADC(registers[REG_L]);
            break;

        case 0x8E: // ADC A, (HL)
            ADC(mmu->read8(getHL()));
            break;

        case 0x8F: // ADC A, A
            ADC(registers[REG_A]);
            break;

        case 0x90: // SUB A, B
            SUB(registers[REG_B]);
            break;

        case 0x91: // SUB A, C
            SUB(registers[REG_C]);
            break;

        case 0x92: // SUB A, D
            SUB(registers[REG_D]);
            break;

        case 0x93: // SUB A, E
            SUB(registers[REG_E]);
            break;

        case 0x94: // SUB A, H
            SUB(registers[REG_H]);
            break;

        case 0x95: // SUB A, L
            SUB(registers[REG_L]);
            break;

        case 0x96: // SUB A, (HL)
            SUB(mmu->read8(getHL()));
            break;

        case 0x97: // SUB A, A
            SUB(registers[REG_A]);
            break;

        case 0x98: // SBC A, B
            SBC(registers[REG_B]);
            break;

        case 0x99: // SBC A, C
            SBC(registers[REG_C]);
            break;

        case 0x9A: // SBC A, D
            SBC(registers[REG_D]);
            break;

        case 0x9B: // SBC A, E
            SBC(registers[REG_E]);
            break;

        case 0x9C: // SBC A, H
            SBC(registers[REG_H]);
            break;

        case 0x9D: // SBC A, L
            SBC(registers[REG_L]);
            break;

        case 0x9E: // SBC A, (HL)
            SBC(mmu->read8(getHL()));
            break;

        case 0x9F: // SBC A, A
            SBC(registers[REG_A]);
            break;

        case 0xC6: // ADD A, u8
            ADD8(mmu->read8(pc));
            pc++;
            break;

        case 0xCE: // ADC A, u8
            ADC(mmu->read8(pc));
            pc++;
            break;


        case 0xD6: // SUB A, u8
            SUB(mmu->read8(pc));
            pc++;
            break;

        case 0xDE: // SBC A, u8
            SBC(mmu->read8(pc));
            pc++;
            break;

        /**
         * 16-bit Arithmetic Instructions
         */ 
        case 0x03: // INC BC
            setBC(getBC() + 1);
            break;
            

        case 0x13: // INC DE
            setDE(getDE() + 1);
            break;
        
        case 0x23: // INC HL
            setHL(getHL() + 1);
            break;

        case 0x09: // ADD HL, BC
            ADD16(getBC());
            break;

        case 0x19: // ADD HL, DE
            ADD16(getDE());
            break;

        case 0x29: // ADD HL, HL
            ADD16(getHL());
            break;
    

        case 0x0B: // DEC BC
            setBC(getBC() - 1);
            break;

        case 0x1B: // DEC DE
            setDE(getDE() - 1);
            break;

        case 0x2B: // DEC HL
            setHL(getHL() - 1);
            break;

        /**
         * Bitwise Logic Instructions
         */
        case 0x2F: // CPL
            registers[REG_A] = ~registers[REG_A];
            setN(true);
            setH(true);
            break;

        case 0xA0: // AND A, B
            AND(registers[REG_B]);
            break;

        case 0xA1: // AND A, C
            AND(registers[REG_C]);
            break;

        case 0xA2: // AND A, D
            AND(registers[REG_D]);
            break;

        case 0xA3: // AND A, E
            AND(registers[REG_E]);
            break;

        case 0xA4: // AND A, H
            AND(registers[REG_H]);
            break;

        case 0xA5: // AND A, L
            AND(registers[REG_L]);
            break;

        case 0xA6: // AND A, (HL)
            AND(mmu->read8(getHL()));
            break;

        case 0xA7: // AND A, A
            AND(registers[REG_A]);
            break;

        case 0xA8: // XOR A, B
            XOR(registers[REG_B]);
            break;

        case 0xA9: // AND A, C
            XOR(registers[REG_C]);
            break;

        case 0xAA: // AND A, D
            XOR(registers[REG_D]);
            break;

        case 0xAB: // AND A, E
            XOR(registers[REG_E]);
            break;

        case 0xAC: // AND A, H
            XOR(registers[REG_H]);
            break;

        case 0xAD: // AND A, L
            XOR(registers[REG_L]);
            break;

        case 0xAE: // AND A, (HL)
            XOR(mmu->read8(getHL()));
            break;

        case 0xAF: // AND A, A
            XOR(registers[REG_A]);
            break;

        case 0xB0: // OR A, B
            OR(registers[REG_B]);
            break;

        case 0xB1: // OR A, C
            OR(registers[REG_C]);
            break;

        case 0xB2: // OR A, D
            OR(registers[REG_D]);
            break;

        case 0xB3: // OR A, E
            OR(registers[REG_E]);
            break;

        case 0xB4: // OR A, H
            OR(registers[REG_H]);
            break;

        case 0xB5: // OR A, L
            OR(registers[REG_L]);
            break;

        case 0xB6: // OR A, (HL)
            OR(mmu->read8(getHL()));
            break;

        case 0xB7: // OR A, A
            OR(registers[REG_A]);
            break;

        case 0xB8: // CP A, B
            CP(registers[REG_B]);
            break;

        case 0xB9: // CP A, C
            CP(registers[REG_C]);
            break;

        case 0xBA: // CP A, D
            CP(registers[REG_D]);
            break;

        case 0xBB: // CP A, E
            CP(registers[REG_E]);
            break;

        case 0xBC: // CP A, H
            CP(registers[REG_H]);
            break;

        case 0xBD: // CP A, L
            CP(registers[REG_L]);
            break;

        case 0xBE: // CP A, (HL)
            CP(mmu->read8(getHL()));
            break;

        case 0xBF: // CP A, A
            CP(registers[REG_A]);
            break;

        case 0xE6: // AND A, u8
            AND(mmu->read8(pc));
            pc++;
            break;

        case 0xEE: // XOR A, u8
            XOR(mmu->read8(pc));
            pc++;
            break;

        case 0xF6: // OR A, u8
            OR(mmu->read8(pc));
            pc++;
            break;

        case 0xFE: // CP A, u8
            CP(mmu->read8(pc));
            pc++;
            break;

        /**
         * Jumps and Subroutine Instructions
         */
        case 0x18: // JR i8
            JR(true);
            break;

        case 0x20: // JR NZ, i8
            JR(!getZ());
            break;

        case 0x28: // JR Z, i8
            JR(getZ());
            break;

        case 0x30: // JR NC, i8
            JR(!getC());
            break;

        case 0x38: // JR C, i8
            JR(getC());
            break;

        case 0xC4: // CALL NZ, u16
            CALL(!getZ());
            break;

        case 0xCC: // CALL Z
            CALL(getZ());
            break;

        case 0xCD: // CALL u16
            CALL(true);
            break;

        case 0xD4: // CALL NC
            CALL(!getC());
            break;

        case 0xDC: // CALL C
            CALL(getC());
            break;

        case 0xC2: // JP NZ, u16
            JP(!getZ());
            break;

        case 0xC3: // JP u16
            JP(true);
            break;

        case 0xCA: // JP Z
            JP(getZ());
            break;

        case 0xD2: // JP NC, u16
            JP(!getC());
            break;

        case 0xDA: // JP C, u16
            JP(getC());
            break;

        case 0xE9: // JP HL
            pc = getHL();
            break;

        case 0xC0: // RET NZ
            RET(!getZ());
            break;

        case 0xC8: // RET Z
            RET(getZ());
            break;

        case 0xC9: // RET
            RET(true);

            // Unconditonal RET has cycles be 16
            cyclesPassed = 16;
            break;

        case 0xD0: // RET NC
            RET(!getC());
            break;

        case 0xD8: // RET C
            RET(getC());
            break;

        case 0xD9: // RETI
            // Essentially an EI then RET instruction, so ei_hold can be ignored
            ime = true;
            ei_hold = false;

            RET(true);


            // Unconditonal RETI has cycles be 16
            cyclesPassed = 16;
            break;
        
        case 0xC7: // RST 00h
            RST(0x00);
            break;
        
        case 0xCF: // RST 08h
            RST(0x08);
            break;

        case 0xD7: // RST 10h
            RST(0x10);
            break;

        case 0xDF: // ST 18h
            RST(0x18);
            break;
            
        case 0xE7: // RST 20h
            RST(0x20);
            break;

        case 0xEF: // RST 28h
            RST(0x28);
            break;

        case 0xF7: // RST 30h
            RST(0x30);
            break;

        case 0xFF: // RST 38h
            RST(0x38);
            break;

        /**
         * Carry Flag Instructions
         */
        case 0x37: // SCF
            setC(true);
            setN(false);
            setH(false);
            break;

        case 0x3F: // CCF
            setC(!getC());
            setN(false);
            setH(false);
            break;
        
        /**
         * Stack Manipulation Instructions
         */
        case 0x08: // LD (u16), SP
            mmu->write16(mmu->read16(pc), sp);
            pc += 2;
            break;

        case 0x31: // LD SP, u16
            sp = mmu->read16(pc);
            pc += 2;
            break;        

        case 0x33: // INC SP
            sp++;
            break;
        
        case 0x39: // ADD HL, SP
            ADD16(sp);
            break;


        case 0x3B: // DEC SP
            sp--;
            break;

        case 0xC1: // POP BC
            setBC(mmu->read16(sp));
            sp += 2;
            break;

        case 0xC5: // PUSH BC
            sp -= 2;
            mmu->write16(sp, getBC());
            break;


        case 0xD1: // POP DE
            setDE(mmu->read16(sp));
            sp += 2;      
            break;

        case 0xD5: // PUSH DE
            sp -= 2;
            mmu->write16(sp, getDE());
            break;

        case 0xE1: // POP HL
            setHL(mmu->read16(sp));
            sp += 2;
            break;

        case 0xE5: // PUSH HL
            sp -= 2;
            mmu->write16(sp, getHL());
            break;
        

        case 0xE8: // ADD SP, i8
        {
            uint8_t val = mmu->read8(pc);
            
            setZ(false);
            setN(false);
            setH(((sp & 0x0F) + (val & 0x0F)) > 0x0F);
            setC(((sp & 0xFF) + val) > 0xFF);

            sp += (int8_t)val;
            pc++;
        }
            break;
        case 0xF1: // POP AF
            setAF(mmu->read16(sp));
            sp += 2;
            break;

        case 0xF5: // PUSH AF
            sp -= 2;
            mmu->write16(sp, getAF());
            break;

        case 0xF8: // LD HL, SP + i8
        {
            uint8_t val = mmu->read8(pc);

            setZ(false);
            setN(false);
            setH(((sp & 0x0F) + (val & 0x0F)) > 0x0F);
            setC(((sp & 0xFF) + val) > 0xFF);

            setHL(sp + (int8_t)val);
            pc++;
        }
            break;

        case 0xF9: // LD SP, HL
            sp = getHL();
            break;

        
        /**
         * IE Instructions
         */

        case 0x76: // HALT
            halt = true;


            // Halt Bug
            if (!ime && ((mmu->read8(IE_ADDRESS) & mmu->read8(IF_ADDRESS)) != 0)) {
            }
            break;

        case 0xF3: // DI
            ime = false;
            ei_hold = false;
            break;

        case 0xFB: // EI
            ei_hold = true;
            break;


        default:
            std::cerr << "Bad instruction | Opcode: " << opcode << " PC: " << pc - 1 << "\n";
            break;
    }
}

void CPU::executeCBInstruction(uint8_t opcode) {
    pc++;
    cyclesPassed = opcodeCBCycles[opcode];
    switch (opcode) {
        /**
         * RLC Instructions 
         */
        case 0x00: // RLC B
            registers[REG_B] = RLC(registers[REG_B]);
            break;        
        case 0x01: // RLC C
            registers[REG_C] = RLC(registers[REG_C]);
            break;
        case 0x02: // RLC D
            registers[REG_D] = RLC(registers[REG_D]);
            break;
        case 0x03: // RLC E
            registers[REG_E] = RLC(registers[REG_E]);
            break;
        case 0x04: // RLC H
            registers[REG_H] = RLC(registers[REG_H]);
            break;
        case 0x05: // RLC L
            registers[REG_L] = RLC(registers[REG_L]);
            break;
        case 0x06: // RLC (HL)
            mmu->write8(getHL(), RLC(mmu->read8(getHL())));
            break;
        case 0x07: // RLC A
            registers[REG_A] = RLC(registers[REG_A]);
            break;
        case 0x08: // RRC B
            registers[REG_B] = RRC(registers[REG_B]);
            break;
        case 0x09: // RRC C
            registers[REG_C] = RRC(registers[REG_C]);
            break;
        case 0x0A: // RRC D
            registers[REG_D] = RRC(registers[REG_D]);
            break;
        case 0x0B: // RRC E
            registers[REG_E] = RRC(registers[REG_E]);
            break;
        case 0x0C: // RRC H
            registers[REG_H] = RRC(registers[REG_H]);
            break;
        case 0x0D: // RRC L
            registers[REG_L] = RRC(registers[REG_L]);
            break;
        case 0x0E: // RRC (HL)
            mmu->write8(getHL(), RRC(mmu->read8(getHL())));
            break;
        case 0x0F: // RRC A
            registers[REG_A] = RRC(registers[REG_A]);
            break;
        case 0x10: // RL B
            registers[REG_B] = RL(registers[REG_B]);
            break;
        case 0x11: // RL C
            registers[REG_C] = RL(registers[REG_C]);
            break;
        case 0x12: // RL D
            registers[REG_D] = RL(registers[REG_D]);
            break;
        case 0x13: // RL E
            registers[REG_E] = RL(registers[REG_E]);
            break;
        case 0x14: // RL H
            registers[REG_H] = RL(registers[REG_H]);
            break;
        case 0x15: // RL L
            registers[REG_L] = RL(registers[REG_L]);
            break;
        case 0x16: // RL (HL)
            mmu->write8(getHL(), RL(mmu->read8(getHL())));
            break;
        case 0x17: // RL A
            registers[REG_A] = RL(registers[REG_A]);
            break;
        case 0x18: // RR B
            registers[REG_B] = RR(registers[REG_B]);
            break;
        case 0x19: // RR C
            registers[REG_C] = RR(registers[REG_C]);
            break;
        case 0x1A: // RR D
            registers[REG_D] = RR(registers[REG_D]);
            break;
        case 0x1B: // RR E
            registers[REG_E] = RR(registers[REG_E]);
            break;
        case 0x1C: // RR H
            registers[REG_H] = RR(registers[REG_H]);
            break;
        case 0x1D: // RR L
            registers[REG_L] = RR(registers[REG_L]);
            break;
        case 0x1E: // RR (HL)
            mmu->write8(getHL(), RR(mmu->read8(getHL())));
            break;
        case 0x1F: // RR A
            registers[REG_A] = RR(registers[REG_A]);
            break;
        case 0x20: // SLA B
            registers[REG_B] = SLA(registers[REG_B]);
            break;
        case 0x21: // SLA C
            registers[REG_C] = SLA(registers[REG_C]);
            break;
        case 0x22: // SLA D
            registers[REG_D] = SLA(registers[REG_D]);
            break;
        case 0x23: // SLA E
            registers[REG_E] = SLA(registers[REG_E]);
            break;
        case 0x24: // SLA H
            registers[REG_H] = SLA(registers[REG_H]);
            break;
        case 0x25: // SLA L
            registers[REG_L] = SLA(registers[REG_L]);
            break;
        case 0x26: // SLA (HL)
            mmu->write8(getHL(), SLA(mmu->read8(getHL())));
            break;
        case 0x27: // SLA A
            registers[REG_A] = SLA(registers[REG_A]);
            break;
        case 0x28: // SRA B
            registers[REG_B] = SRA(registers[REG_B]);
            break;
        case 0x29: // SRA C
            registers[REG_C] = SRA(registers[REG_C]);
            break;
        case 0x2A: // SRA D
            registers[REG_D] = SRA(registers[REG_D]);
            break;
        case 0x2B: // SRA E
            registers[REG_E] = SRA(registers[REG_E]);
            break;
        case 0x2C: // SRA H
            registers[REG_H] = SRA(registers[REG_H]);
            break;
        case 0x2D: // SRA L
            registers[REG_L] = SRA(registers[REG_L]);
            break;
        case 0x2E: // SRA (HL)
            mmu->write8(getHL(), SRA(mmu->read8(getHL())));
            break;
        case 0x2F: // SRA A
            registers[REG_A] = SRA(registers[REG_A]);
            break;
        case 0x30: // SWAP B
            registers[REG_B] = SWAP(registers[REG_B]);
            break;
        case 0x31: // SWAP C
            registers[REG_C] = SWAP(registers[REG_C]);
            break;
        case 0x32: // SWAP D
            registers[REG_D] = SWAP(registers[REG_D]);
            break;
        case 0x33: // SWAP E
            registers[REG_E] = SWAP(registers[REG_E]);
            break;
        case 0x34: // SWAP H
            registers[REG_H] = SWAP(registers[REG_H]);
            break;
        case 0x35: // SWAP L
            registers[REG_L] = SWAP(registers[REG_L]);
            break;
        case 0x36: // SWAP (HL)
            mmu->write8(getHL(), SWAP(mmu->read8(getHL())));
            break;
        case 0x37: // SWAP A
            registers[REG_A] = SWAP(registers[REG_A]);
            break;
        case 0x38: // SRL B
            registers[REG_B] = SRL(registers[REG_B]);
            break;
        case 0x39: // SRL C
            registers[REG_C] = SRL(registers[REG_C]);
            break;
        case 0x3A: // SRL D
            registers[REG_D] = SRL(registers[REG_D]);
            break;
        case 0x3B: // SRL E
            registers[REG_E] = SRL(registers[REG_E]);
            break;
        case 0x3C: // SRL H
            registers[REG_H] = SRL(registers[REG_H]);
            break;
        case 0x3D: // SRL L
            registers[REG_L] = SRL(registers[REG_L]);
            break;
        case 0x3E: // SRL (HL)
            mmu->write8(getHL(), SRL(mmu->read8(getHL())));
            break;
        case 0x3F: // SRL A
            registers[REG_A] = SRL(registers[REG_A]);
            break;

        case 0x40: // BIT 0, B
            BIT(0, registers[REG_B]);
            break;
        
        case 0x41: // BIT 0, C
            BIT(0, registers[REG_C]);
            break;
        
        case 0x42: // BIT 0, D
            BIT(0, registers[REG_D]);
            break;
        
        case 0x43: // BIT 0, E
            BIT(0, registers[REG_E]);
            break;
        
        case 0x44: // BIT 0, H
            BIT(0, registers[REG_H]);
            break;
        
        case 0x45: // BIT 0, L
            BIT(0, registers[REG_L]);
            break;
        
        case 0x46: // BIT 0, (HL)
            BIT(0, mmu->read8(getHL()));
            break;
        
        case 0x47: // BIT 0, A
            BIT(0, registers[REG_A]);
            break;
        
        case 0x48: // BIT 1, B
            BIT(1, registers[REG_B]);
            break;
        
        case 0x49: // BIT 1, C
            BIT(1, registers[REG_C]);
            break;
        
        case 0x4A: // BIT 1, D
            BIT(1, registers[REG_D]);
            break;
        
        case 0x4B: // BIT 1, E
            BIT(1, registers[REG_E]);
            break;
        
        case 0x4C: // BIT 1, H
            BIT(1, registers[REG_H]);
            break;
        
        case 0x4D: // BIT 1, L
            BIT(1, registers[REG_L]);
            break;
        
        case 0x4E: // BIT 1, (HL)
            BIT(1, mmu->read8(getHL()));
            break;
        
        case 0x4F: // BIT 1, A
            BIT(1, registers[REG_A]);
            break;
        
        case 0x50: // BIT 2, B
            BIT(2, registers[REG_B]);
            break;
        
        case 0x51: // BIT 2, C
            BIT(2, registers[REG_C]);
            break;
        
        case 0x52: // BIT 2, D
            BIT(2, registers[REG_D]);
            break;
        
        case 0x53: // BIT 2, E
            BIT(2, registers[REG_E]);
            break;
        
        case 0x54: // BIT 2, H
            BIT(2, registers[REG_H]);
            break;
        
        case 0x55: // BIT 2, L
            BIT(2, registers[REG_L]);
            break;
        
        case 0x56: // BIT 2, (HL)
            BIT(2, mmu->read8(getHL()));
            break;
        
        case 0x57: // BIT 2, A
            BIT(2, registers[REG_A]);
            break;
        
        case 0x58: // BIT 3, B
            BIT(3, registers[REG_B]);
            break;
        
        case 0x59: // BIT 3, C
            BIT(3, registers[REG_C]);
            break;
        
        case 0x5A: // BIT 3, D
            BIT(3, registers[REG_D]);
            break;
        
        case 0x5B: // BIT 3, E
            BIT(3, registers[REG_E]);
            break;
        
        case 0x5C: // BIT 3, H
            BIT(3, registers[REG_H]);
            break;
        
        case 0x5D: // BIT 3, L
            BIT(3, registers[REG_L]);
            break;
        
        case 0x5E: // BIT 3, (HL)
            BIT(3, mmu->read8(getHL()));
            break;
        
        case 0x5F: // BIT 3, A
            BIT(3, registers[REG_A]);
            break;
        
        case 0x60: // BIT 4, B
            BIT(4, registers[REG_B]);
            break;
        
        case 0x61: // BIT 4, C
            BIT(4, registers[REG_C]);
            break;
        
        case 0x62: // BIT 4, D
            BIT(4, registers[REG_D]);
            break;
        
        case 0x63: // BIT 4, E
            BIT(4, registers[REG_E]);
            break;
        
        case 0x64: // BIT 4, H
            BIT(4, registers[REG_H]);
            break;
        
        case 0x65: // BIT 4, L
            BIT(4, registers[REG_L]);
            break;
        
        case 0x66: // BIT 4, (HL)
            BIT(4, mmu->read8(getHL()));
            break;
        
        case 0x67: // BIT 4, A
            BIT(4, registers[REG_A]);
            break;
        
        case 0x68: // BIT 5, B
            BIT(5, registers[REG_B]);
            break;
        
        case 0x69: // BIT 5, C
            BIT(5, registers[REG_C]);
            break;
        
        case 0x6A: // BIT 5, D
            BIT(5, registers[REG_D]);
            break;
        
        case 0x6B: // BIT 5, E
            BIT(5, registers[REG_E]);
            break;
        
        case 0x6C: // BIT 5, H
            BIT(5, registers[REG_H]);
            break;
        
        case 0x6D: // BIT 5, L
            BIT(5, registers[REG_L]);
            break;
        
        case 0x6E: // BIT 5, (HL)
            BIT(5, mmu->read8(getHL()));
            break;
        
        case 0x6F: // BIT 5, A
            BIT(5, registers[REG_A]);
            break;
        
        case 0x70: // BIT 6, B
            BIT(6, registers[REG_B]);
            break;
        
        case 0x71: // BIT 6, C
            BIT(6, registers[REG_C]);
            break;
        
        case 0x72: // BIT 6, D
            BIT(6, registers[REG_D]);
            break;
        
        case 0x73: // BIT 6, E
            BIT(6, registers[REG_E]);
            break;
        
        case 0x74: // BIT 6, H
            BIT(6, registers[REG_H]);
            break;
        
        case 0x75: // BIT 6, L
            BIT(6, registers[REG_L]);
            break;
        
        case 0x76: // BIT 6, (HL)
            BIT(6, mmu->read8(getHL()));
            break;
    
        case 0x77: // BIT 6, A
            BIT(6, registers[REG_A]);
            break;
    
        case 0x78: // BIT 7, B
            BIT(7, registers[REG_B]);
            break;
        
        case 0x79: // BIT 7, C
            BIT(7, registers[REG_C]);
            break;
        
        case 0x7A: // BIT 7, D
            BIT(7, registers[REG_D]);
            break;
        
        case 0x7B: // BIT 7, E
            BIT(7, registers[REG_E]);
            break;
        
        case 0x7C: // BIT 7, H
            BIT(7, registers[REG_H]);
            break;
        
        case 0x7D: // BIT 7, L
            BIT(7, registers[REG_L]);
            break;
        
        case 0x7E: // BIT 7, (HL)
            BIT(7, mmu->read8(getHL()));
            break;
        
        case 0x7F: // BIT 7, A
            BIT(7, registers[REG_A]);
            break;
        
        case 0x80: // RES 0, B
            registers[REG_B] = RES(0, registers[REG_B]);
            break;

        case 0x81: // RES 0, C
            registers[REG_C] = RES(0, registers[REG_C]);
            break;

        case 0x82: // RES 0, D
            registers[REG_D] = RES(0, registers[REG_D]);
            break;

        case 0x83: // RES 0, E
            registers[REG_E] = RES(0, registers[REG_E]);
            break;

        case 0x84: // RES 0, H
            registers[REG_H] = RES(0, registers[REG_H]);
            break;

        case 0x85: // RES 0, L
            registers[REG_L] = RES(0, registers[REG_L]);
            break;

        case 0x86: // RES 0, (HL)
            mmu->write8(getHL(), RES(0, mmu->read8(getHL())));
            break;

        case 0x87: // RES 0, A
            registers[REG_A] = RES(0, registers[REG_A]);
            break;

        case 0x88: // RES 1, B
            registers[REG_B] = RES(1, registers[REG_B]);
            break;

        case 0x89: // RES 1, C
            registers[REG_C] = RES(1, registers[REG_C]);
            break;

        case 0x8A: // RES 1, D
            registers[REG_D] = RES(1, registers[REG_D]);
            break;

        case 0x8B: // RES 1, E
            registers[REG_E] = RES(1, registers[REG_E]);
            break;

        case 0x8C: // RES 1, H
            registers[REG_H] = RES(1, registers[REG_H]);
            break;

        case 0x8D: // RES 1, L
            registers[REG_L] = RES(1, registers[REG_L]);
            break;

        case 0x8E: // RES 1, (HL)
            mmu->write8(getHL(), RES(1, mmu->read8(getHL())));
            break;

        case 0x8F: // RES 1, A
            registers[REG_A] = RES(1, registers[REG_A]);
            break;

        case 0x90: // RES 2, B
            registers[REG_B] = RES(2, registers[REG_B]);
            break;

        case 0x91: // RES 2, C
            registers[REG_C] = RES(2, registers[REG_C]);
            break;

        case 0x92: // RES 2, D
            registers[REG_D] = RES(2, registers[REG_D]);
            break;

        case 0x93: // RES 2, E
            registers[REG_E] = RES(2, registers[REG_E]);
            break;

        case 0x94: // RES 2, H
            registers[REG_H] = RES(2, registers[REG_H]);
            break;

        case 0x95: // RES 2, L
            registers[REG_L] = RES(2, registers[REG_L]);
            break;

        case 0x96: // RES 2, (HL)
            mmu->write8(getHL(), RES(2, mmu->read8(getHL())));
            break;

        case 0x97: // RES 2, A
            registers[REG_A] = RES(2, registers[REG_A]);
            break;

        case 0x98: // RES 3, B
            registers[REG_B] = RES(3, registers[REG_B]);
            break;

        case 0x99: // RES 3, C
            registers[REG_C] = RES(3, registers[REG_C]);
            break;

        case 0x9A: // RES 3, D
            registers[REG_D] = RES(3, registers[REG_D]);
            break;

        case 0x9B: // RES 3, E
            registers[REG_E] = RES(3, registers[REG_E]);
            break;

        case 0x9C: // RES 3, H
            registers[REG_H] = RES(3, registers[REG_H]);
            break;

        case 0x9D: // RES 3, L
            registers[REG_L] = RES(3, registers[REG_L]);
            break;

        case 0x9E: // RES 3, (HL)
            mmu->write8(getHL(), RES(3, mmu->read8(getHL())));
            break;

        case 0x9F: // RES 3, A
            registers[REG_A] = RES(3, registers[REG_A]);
            break;

        case 0xA0: // RES 4, B
            registers[REG_B] = RES(4, registers[REG_B]);
            break;

        case 0xA1: // RES 4, C
            registers[REG_C] = RES(4, registers[REG_C]);
            break;

        case 0xA2: // RES 4, D
            registers[REG_D] = RES(4, registers[REG_D]);
            break;

        case 0xA3: // RES 4, E
            registers[REG_E] = RES(4, registers[REG_E]);
            break;

        case 0xA4: // RES 4, H
            registers[REG_H] = RES(4, registers[REG_H]);
            break;

        case 0xA5: // RES 4, L
            registers[REG_L] = RES(4, registers[REG_L]);
            break;

        case 0xA6: // RES 4, (HL)
            mmu->write8(getHL(), RES(4, mmu->read8(getHL())));
            break;

        case 0xA7: // RES 4, A
            registers[REG_A] = RES(4, registers[REG_A]);
            break;

        case 0xA8: // RES 5, B
            registers[REG_B] = RES(5, registers[REG_B]);
            break;

        case 0xA9: // RES 5, C
            registers[REG_C] = RES(5, registers[REG_C]);
            break;

        case 0xAA: // RES 5, D
            registers[REG_D] = RES(5, registers[REG_D]);
            break;

        case 0xAB: // RES 5, E
            registers[REG_E] = RES(5, registers[REG_E]);
            break;

        case 0xAC: // RES 5, H
            registers[REG_H] = RES(5, registers[REG_H]);
            break;

        case 0xAD: // RES 5, L
            registers[REG_L] = RES(5, registers[REG_L]);
            break;

        case 0xAE: // RES 5, (HL)
            mmu->write8(getHL(), RES(5, mmu->read8(getHL())));
            break;

        case 0xAF: // RES 5, A
            registers[REG_A] = RES(5, registers[REG_A]);
            break;

        case 0xB0: // RES 6, B
            registers[REG_B] = RES(6, registers[REG_B]);
            break;

        case 0xB1: // RES 6, C
            registers[REG_C] = RES(6, registers[REG_C]);
            break;

        case 0xB2: // RES 6, D
            registers[REG_D] = RES(6, registers[REG_D]);
            break;

        case 0xB3: // RES 6, E
            registers[REG_E] = RES(6, registers[REG_E]);
            break;

        case 0xB4: // RES 6, H
            registers[REG_H] = RES(6, registers[REG_H]);
            break;

        case 0xB5: // RES 6, L
            registers[REG_L] = RES(6, registers[REG_L]);
            break;

        case 0xB6: // RES 6, (HL)
            mmu->write8(getHL(), RES(6, mmu->read8(getHL())));
            break;

        case 0xB7: // RES 6, A
            registers[REG_A] = RES(6, registers[REG_A]);
            break;

        case 0xB8: // RES 7, B
            registers[REG_B] = RES(7, registers[REG_B]);
            break;

        case 0xB9: // RES 7, C
            registers[REG_C] = RES(7, registers[REG_C]);
            break;

        case 0xBA: // RES 7, D
            registers[REG_D] = RES(7, registers[REG_D]);
            break;

        case 0xBB: // RES 7, E
            registers[REG_E] = RES(7, registers[REG_E]);
            break;

        case 0xBC: // RES 7, H
            registers[REG_H] = RES(7, registers[REG_H]);
            break;

        case 0xBD: // RES 7, L
            registers[REG_L] = RES(7, registers[REG_L]);
            break;

        case 0xBE: // RES 7, (HL)
            mmu->write8(getHL(), RES(7, mmu->read8(getHL())));
            break;

        case 0xBF: // RES 7, A
            registers[REG_A] = RES(7, registers[REG_A]);
            break;

        case 0xC0: // SET 0, B
            registers[REG_B] = SET(0, registers[REG_B]);
            break;

        case 0xC1: // SET 0, C
            registers[REG_C] = SET(0, registers[REG_C]);
            break;

        case 0xC2: // SET 0, D
            registers[REG_D] = SET(0, registers[REG_D]);
            break;

        case 0xC3: // SET 0, E
            registers[REG_E] = SET(0, registers[REG_E]);
            break;

        case 0xC4: // SET 0, H
            registers[REG_H] = SET(0, registers[REG_H]);
            break;

        case 0xC5: // SET 0, L
            registers[REG_L] = SET(0, registers[REG_L]);
            break;

        case 0xC6: // SET 0, (HL)
            mmu->write8(getHL(), SET(0, mmu->read8(getHL())));
            break;

        case 0xC7: // SET 0, A
            registers[REG_A] = SET(0, registers[REG_A]);
            break;

        case 0xC8: // SET 1, B
            registers[REG_B] = SET(1, registers[REG_B]);
            break;

        case 0xC9: // SET 1, C
            registers[REG_C] = SET(1, registers[REG_C]);
            break;

        case 0xCA: // SET 1, D
            registers[REG_D] = SET(1, registers[REG_D]);
            break;

        case 0xCB: // SET 1, E
            registers[REG_E] = SET(1, registers[REG_E]);
            break;

        case 0xCC: // SET 1, H
            registers[REG_H] = SET(1, registers[REG_H]);
            break;

        case 0xCD: // SET 1, L
            registers[REG_L] = SET(1, registers[REG_L]);
            break;

        case 0xCE: // SET 1, (HL)
            mmu->write8(getHL(), SET(1, mmu->read8(getHL())));
            break;

        case 0xCF: // SET 1, A
            registers[REG_A] = SET(1, registers[REG_A]);
            break;

        case 0xD0: // SET 2, B
            registers[REG_B] = SET(2, registers[REG_B]);
            break;

        case 0xD1: // SET 2, C
            registers[REG_C] = SET(2, registers[REG_C]);
            break;

        case 0xD2: // SET 2, D
            registers[REG_D] = SET(2, registers[REG_D]);
            break;

        case 0xD3: // SET 2, E
            registers[REG_E] = SET(2, registers[REG_E]);
            break;

        case 0xD4: // SET 2, H
            registers[REG_H] = SET(2, registers[REG_H]);
            break;

        case 0xD5: // SET 2, L
            registers[REG_L] = SET(2, registers[REG_L]);
            break;

        case 0xD6: // SET 2, (HL)
            mmu->write8(getHL(), SET(2, mmu->read8(getHL())));
            break;

        case 0xD7: // SET 2, A
            registers[REG_A] = SET(2, registers[REG_A]);
            break;

        case 0xD8: // SET 3, B
            registers[REG_B] = SET(3, registers[REG_B]);
            break;

        case 0xD9: // SET 3, C
            registers[REG_C] = SET(3, registers[REG_C]);
            break;

        case 0xDA: // SET 3, D
            registers[REG_D] = SET(3, registers[REG_D]);
            break;

        case 0xDB: // SET 3, E
            registers[REG_E] = SET(3, registers[REG_E]);
            break;

        case 0xDC: // SET 3, H
            registers[REG_H] = SET(3, registers[REG_H]);
            break;

        case 0xDD: // SET 3, L
            registers[REG_L] = SET(3, registers[REG_L]);
            break;

        case 0xDE: // SET 3, (HL)
            mmu->write8(getHL(), SET(3, mmu->read8(getHL())));
            break;

        case 0xDF: // SET 3, A
            registers[REG_A] = SET(3, registers[REG_A]);
            break;

        case 0xE0: // SET 4, B
            registers[REG_B] = SET(4, registers[REG_B]);
            break;

        case 0xE1: // SET 4, C
            registers[REG_C] = SET(4, registers[REG_C]);
            break;

        case 0xE2: // SET 4, D
            registers[REG_D] = SET(4, registers[REG_D]);
            break;

        case 0xE3: // SET 4, E
            registers[REG_E] = SET(4, registers[REG_E]);
            break;

        case 0xE4: // SET 4, H
            registers[REG_H] = SET(4, registers[REG_H]);
            break;

        case 0xE5: // SET 4, L
            registers[REG_L] = SET(4, registers[REG_L]);
            break;

        case 0xE6: // SET 4, (HL)
            mmu->write8(getHL(), SET(4, mmu->read8(getHL())));
            break;

        case 0xE7: // SET 4, A
            registers[REG_A] = SET(4, registers[REG_A]);
            break;

        case 0xE8: // SET 5, B
            registers[REG_B] = SET(5, registers[REG_B]);
            break;

        case 0xE9: // SET 5, C
            registers[REG_C] = SET(5, registers[REG_C]);
            break;

        case 0xEA: // SET 5, D
            registers[REG_D] = SET(5, registers[REG_D]);
            break;

        case 0xEB: // SET 5, E
            registers[REG_E] = SET(5, registers[REG_E]);
            break;

        case 0xEC: // SET 5, H
            registers[REG_H] = SET(5, registers[REG_H]);
            break;

        case 0xED: // SET 5, L
            registers[REG_L] = SET(5, registers[REG_L]);
            break;

        case 0xEE: // SET 5, (HL)
            mmu->write8(getHL(), SET(5, mmu->read8(getHL())));
            break;

        case 0xEF: // SET 5, A
            registers[REG_A] = SET(5, registers[REG_A]);
            break;

        case 0xF0: // SET 6, B
            registers[REG_B] = SET(6, registers[REG_B]);
            break;

        case 0xF1: // SET 6, C
            registers[REG_C] = SET(6, registers[REG_C]);
            break;

        case 0xF2: // SET 6, D
            registers[REG_D] = SET(6, registers[REG_D]);
            break;

        case 0xF3: // SET 6, E
            registers[REG_E] = SET(6, registers[REG_E]);
            break;

        case 0xF4: // SET 6, H
            registers[REG_H] = SET(6, registers[REG_H]);
            break;

        case 0xF5: // SET 6, L
            registers[REG_L] = SET(6, registers[REG_L]);
            break;

        case 0xF6: // SET 6, (HL)
            mmu->write8(getHL(), SET(6, mmu->read8(getHL())));
            break;

        case 0xF7: // SET 6, A
            registers[REG_A] = SET(6, registers[REG_A]);
            break;

        case 0xF8: // SET 7, B
            registers[REG_B] = SET(7, registers[REG_B]);
            break;

        case 0xF9: // SET 7, C
            registers[REG_C] = SET(7, registers[REG_C]);
            break;

        case 0xFA: // SET 7, D
            registers[REG_D] = SET(7, registers[REG_D]);
            break;

        case 0xFB: // SET 7, E
            registers[REG_E] = SET(7, registers[REG_E]);
            break;

        case 0xFC: // SET 7, H
            registers[REG_H] = SET(7, registers[REG_H]);
            break;

        case 0xFD: // SET 7, L
            registers[REG_L] = SET(7, registers[REG_L]);
            break;

        case 0xFE: // SET 7, (HL)
            mmu->write8(getHL(), SET(7, mmu->read8(getHL())));
            break;

        case 0xFF: // SET 7, A
            registers[REG_A] = SET(7, registers[REG_A]);
            break;

        
        default:
            std::cerr << "Bad instruction | 0xCB Opcode: " << opcode << " PC: " << pc - 2 << "\n";
            break;
    }
}



void CPU::setState(int mode)
{
    resetGB();
    CGBMode = false;
    // CGBMode = mode;
    
    // if (!mode) {
    //     resetGB();
        
    // } else {
    //     resetCGB();
    // }
}

void CPU::resetGB()
{
    registers[REG_A] = 0x01;
    registers[REG_F] = 0xB0;
    registers[REG_B] = 0x00;
    registers[REG_C] = 0x13;
    registers[REG_D] = 0x00;
    registers[REG_E] = 0xD8;
    registers[REG_H] = 0x01;
    registers[REG_L] = 0x4D;
    pc = 0x100;
    sp = 0xFFFE;
    doubleSpeed = false;
    halt = false;
}

void CPU::resetCGB()
{
    registers[REG_A] = 0x11;
    registers[REG_F] = 0x80;
    registers[REG_B] = 0x00;
    registers[REG_C] = 0x00;
    registers[REG_D] = 0xFF;
    registers[REG_E] = 0x56;
    registers[REG_H] = 0x00;
    registers[REG_L] = 0x0D;
    pc = 0x100;
    sp = 0xFFFE;
    doubleSpeed = false;
    halt = false;
}

void CPU::setZ(bool set) {
    if (set) {
        registers[REG_F] |= 0x80;
    } else {
        registers[REG_F] &= 0x7F;
    }
}

void CPU::setN(bool set) {
    if (set) {
        registers[REG_F] |= 0x40;
    } else {
        registers[REG_F] &= 0xBF;
    }
}

void CPU::setH(bool set) {
    if (set) {
        registers[REG_F] |= 0x20;
    } else {
        registers[REG_F] &= 0xDF;
    }
}

void CPU::setC(bool set) {
    if (set) {
        registers[REG_F] |= 0x10;
    } else {
        registers[REG_F] &= 0xEF;
    }
}

bool CPU::getZ() {
    return (registers[REG_F] & 0x80) != 0;
}

bool CPU::getN()
{
    return (registers[REG_F] & 0x40) != 0;
}

bool CPU::getH()
{
    return (registers[REG_F] & 0x20) != 0;
}

bool CPU::getC()
{
    return (registers[REG_F] & 0x10) != 0;
}


uint16_t CPU::getBC() {
    return (registers[REG_B] << 8) | registers[REG_C];
}

uint16_t CPU::getDE() {
    return (registers[REG_D] << 8) | registers[REG_E];
}

uint16_t CPU::getHL() {
    return (registers[REG_H] << 8) | registers[REG_L];
}

uint16_t CPU::getAF()
{
    return (registers[REG_A] << 8) | registers[REG_F];
}


void CPU::setBC(uint16_t val) {
    registers[REG_B] = (val >> 8) & 0xFF;
    registers[REG_C] = val & 0xFF;
}

void CPU::setDE(uint16_t val) {
    registers[REG_D] = (val >> 8) & 0xFF;
    registers[REG_E] = val & 0xFF;
}

void CPU::setHL(uint16_t val) {
    registers[REG_H] = (val >> 8) & 0xFF;
    registers[REG_L] = val & 0xFF;
}

void CPU::setAF(uint16_t val) {
    registers[REG_A] = (val >> 8) & 0xFF;
    registers[REG_F] = val & 0xF0;
}



void CPU::RET(bool condition) {
    if (condition) {
        uint8_t low = mmu->read8(sp++);
        uint8_t high = mmu->read8(sp++);
        pc = (high << 8) | low;
        
        // With condition cycles passed is 20
        cyclesPassed = 20;
        // pc = mmu->read8(sp++);
        // sp += 2;
    }
}

void CPU::CALL(bool condition) {
    uint16_t address = mmu->read16(pc);
    pc += 2;
    if (condition) {
        sp -= 2;
        mmu->write16(sp, pc);

        // Implicit jump
        pc = address;

        // With condition cycles passed is 12
        cyclesPassed = 24;
    }
}

void CPU::RST(uint8_t vec) {
    sp -= 2;
    mmu->write16(sp, pc);
    pc = vec;
}

void CPU::JP(bool condition) {
    if (condition) {
        pc = mmu->read16(pc);

        // With condition cycles passed is 16
        cyclesPassed = 16;
    } else {
        pc += 2;
    }
}

void CPU::JR(bool condition) {
    if (condition) {
        pc += (int8_t)mmu->read8(pc);

        // With condition cycles passed is 12
        cyclesPassed = 12;
    }    
    pc++;

}


uint8_t CPU::INC8(uint8_t val) {
    uint8_t res = val + 1;
    
    setN(false);
    setH((val & 0xF) + (1 & 0xF) > 0xF);
    setZ(res == 0);

    return res;
}


uint8_t CPU::DEC8(uint8_t val) {
    setN(true);
    setH((val & 0xF) < (1 & 0xF));
    
    uint8_t res = val - 1;
    setZ(res == 0);

    return res;
}

void CPU::ADD8(uint8_t val) {
    setN(false);
    setH((registers[REG_A] & 0xF) + (val & 0xF) > 0xF);
    setC((registers[REG_A] + val) > 0xFF);
    
    registers[REG_A] += val;

    setZ(registers[REG_A] == 0);
}

void CPU::ADD16(uint16_t val) {
    uint32_t res = getHL() + val;
    setN(false);
    setC(res > 0xFFFF);
    setH(((getHL() & 0x0FFF) + (val & 0x0FFF)) > 0x0FFF);

    // Set H and L registers
    setHL(res);
}

void CPU::ADC(uint8_t val) {
    setN(false);
    uint8_t carry = getC();
    setH((registers[REG_A] & 0xF) + (val & 0xF) + carry > 0xF);
    setC((registers[REG_A] + val + carry) > 0xFF);

    registers[REG_A] += carry + val;

    setZ(registers[REG_A] == 0);
}

void CPU::SUB(uint8_t val) {
    setN(true);

    setH((registers[REG_A] & 0xF) < (val & 0xF));
    setC(registers[REG_A] < val);

    registers[REG_A] -= val;


    setZ(registers[REG_A] == 0);
}

void CPU::SBC(uint8_t val) {
    setN(true);

    uint8_t carry = getC();

    setH((registers[REG_A] & 0xF) < ((val & 0xF) + carry));
    setC(registers[REG_A] < (val + carry));

    registers[REG_A] -= (val + carry);

    setZ(registers[REG_A] == 0);
}

void CPU::AND(uint8_t val) {
    setN(false);
    setH(true);
    setC(false);

    uint8_t res = registers[REG_A] & val;

    setZ(res == 0);

    registers[REG_A] = res;
}


void CPU::XOR(uint8_t val) {
    setN(false);
    setC(false);
    setH(false);

    uint8_t res = registers[REG_A] ^ val;

    setZ(res == 0);
    registers[REG_A] = res;
}

void CPU::OR(uint8_t val) {
    setN(false);
    setC(false);
    setH(false);

    uint8_t res = registers[REG_A] | val;

    setZ(res == 0);

    registers[REG_A] = res;
}

void CPU::CP(uint8_t val) {
    setN(true);
    setC(registers[REG_A] < val);
    setH((registers[REG_A] & 0xF) < (val & 0xF));
    setZ(registers[REG_A] == val);
}

// void CPU::setHAdd(uint8_t left, uint8_t right) {
//     setH((left & 0xF) + (right & 0xF) > 0xF);
// }

// void CPU::setHAdc(uint8_t left, uint8_t right, uint8_t carry) {
//     setH((left & 0xF) + (right & 0xF) + carry > 0xF);
// }

// void CPU::setHSub(uint8_t left, uint8_t right) {
//     setH((left & 0xF) < (right & 0xF));
// }

// void CPU::setHSbc(uint8_t left, uint8_t right, uint8_t carry) {
//     setH((left & 0xF) < ((right & 0xF) + carry));
// }

// void CPU::setCAdd(uint8_t left, uint8_t right) {
//     setC((left + right) > 0xFF);
// }

// void CPU::setCAdc(uint8_t left, uint8_t right, uint8_t carry) {
//     setC((left + right + carry) > 0xFF);
// }

// void CPU::setCSub(uint8_t left, uint8_t right) {
//     setC(left < right);
// }

// void CPU::setCSbc(uint8_t left, uint8_t right, uint8_t carry) {
//     setC(left < (right + carry));
// }

uint8_t CPU::RES(uint8_t pos, uint8_t reg) {
    uint8_t mask = ~(1 << pos);
    
    return reg &= mask;
}

uint8_t CPU::RLC(uint8_t val) {
    uint8_t carry = (val & 0x80) >> 7;
    uint8_t res = (val << 1) | carry;

    setC(carry != 0);
    setN(false);
    setH(false);
    setZ(res == 0);
    return res;
}

uint8_t CPU::RRC(uint8_t val) {
    uint8_t carry = (val & 0x01);
    uint8_t res = (val >> 1) | (carry << 7);

    setC(carry != 0);
    setN(false);
    setH(false);
    setZ(res == 0);
    return res;
}

uint8_t CPU::RL(uint8_t val) {
    uint8_t cFlag = (uint8_t)getC();
    uint8_t carry = (val & 0x80);
    uint8_t res = (val << 1) | cFlag;

    setC(carry != 0);
    setN(false);
    setH(false);
    setZ(res == 0);
    return res;
}

uint8_t CPU::RR(uint8_t val) {
    uint8_t cFlag = (uint8_t)getC();
    uint8_t carry = (val & 0x01);
    uint8_t res = (val >> 1) | (cFlag << 7);

    setC(carry != 0);
    setN(false);
    setH(false);
    setZ(res == 0);
    return res;
}

uint8_t CPU::SRA(uint8_t val) {
    uint8_t sign = (val & 0x80);
    uint8_t carry = (val & 0x01);
    uint8_t res = (val >> 1) | sign;
    setC(carry != 0);
    setN(false);
    setH(false);
    setZ(res == 0);
    return res;
}

uint8_t CPU::SLA(uint8_t val) {
    uint8_t res = val << 1;

    setC((val & 0x80) != 0);
    setN(false);
    setH(false);
    setZ(res == 0);
    return res;
}

uint8_t CPU::SRL(uint8_t val) {
    uint8_t carry = (val & 0x01);
    uint8_t res = (val >> 1);

    setC(carry != 0);
    setN(false);
    setH(false);
    setZ(res == 0);
    return res;
}

uint8_t CPU::SWAP(uint8_t val) {
    uint8_t lower = (val & 0xF) << 4;
    uint8_t upper = (val >> 4);
    uint8_t res = lower | upper;
    setC(false);
    setN(false);
    setH(false);
    setZ(res == 0);
    return res;
}

uint8_t CPU::SET(uint8_t pos, uint8_t reg) {
    return reg | (0x01 << pos);
}

void CPU::BIT(uint8_t pos, uint8_t reg) {
    setN(false);
    setH(true);
    setZ((reg & (0x01 << pos)) == 0);
}

std::string CPU::debug() {
    uint8_t opcode = mmu->read8(pc);
    // std::cout << i << " | ";
    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0')
        << "A:"  << std::setw(2) <<  (int)registers[REG_A] << " "
        << "F:"  << std::setw(2) <<  (int)registers[REG_F] << " "
        << "B:"  << std::setw(2) <<  (int)registers[REG_B] << " "
        << "C:"  << std::setw(2) <<  (int)registers[REG_C] << " "
        << "D:"  << std::setw(2) <<  (int)registers[REG_D] << " "
        << "E:"  << std::setw(2) <<  (int)registers[REG_E] << " "
        << "H:"  << std::setw(2) <<  (int)registers[REG_H] << " "
        << "L:"  << std::setw(2) <<  (int)registers[REG_L] << " "
        << "SP:" << std::setw(4) << sp << " "
        << "PC:" << std::setw(4) << pc << " "
        << "PCMEM:"
        << std::setw(2) << (int)opcode << ","
        << std::setw(2) << (int)mmu->read8(pc + 1) << ","
        << std::setw(2) << (int)mmu->read8(pc + 2) << ","
        << std::setw(2) << (int)mmu->read8(pc + 3)
        << std::dec << "\n";

    return ss.str();
}

bool CPU::getDoubleSpeed()
{
    return doubleSpeed;
}


void CPU::handleInterrupts() {
    uint8_t iFlag = mmu->read8(IF_ADDRESS);
    uint8_t ie = mmu->read8(IE_ADDRESS);
    
    if (ei_hold) {
        ime = true;
        ei_hold = false;
        return;
    }

    // vBlank
    if ((iFlag & VBLANK_BIT) != 0 && (ie & VBLANK_BIT) != 0) {
        halt = false;
        if (ime) {
            ime = false;
            ei_hold = false;
            mmu->write8(IF_ADDRESS, iFlag & ~VBLANK_BIT);

            cycles += 20;
            RST(VBLANK_INT);
        }
    } 
    // LCD
    else if ((iFlag & LCD_BIT) != 0 && (ie & LCD_BIT) != 0) {
        halt = false;
        if (ime) {
            ime = false;
            ei_hold = false;
            mmu->write8(IF_ADDRESS, iFlag & ~LCD_BIT);

            cycles += 20;
            RST(STAT_INT);
        }
    } 
    // Timer
    else if ((iFlag & TIMER_BIT) != 0 && (ie & TIMER_BIT) != 0) {
        halt = false;
        if (ime) {
            ime = false;
            ei_hold = false;
            mmu->write8(IF_ADDRESS, iFlag & ~TIMER_BIT);

            cycles += 20;
            RST(TIMER_INT);
        }
    }
    // Serial
    else if ((iFlag & SERIAL_BIT) != 0 && (ie & SERIAL_BIT) != 0) {
        halt = false;
        if (ime) {
            ime = false;
            ei_hold = false;
            mmu->write8(IF_ADDRESS, iFlag & ~SERIAL_BIT);

            cycles += 20;
            RST(SERIAL_INT);
        }
    }
    // Joypad
    else if ((iFlag & JOYPAD_BIT) != 0 && (ie & JOYPAD_BIT) != 0) {
        halt = false;
        if (ime) {
            ime = false;
            ei_hold = false;
            mmu->write8(IF_ADDRESS, iFlag & ~JOYPAD_BIT);

            cycles += 20;
            RST(JOYPAD_INT);
        }
    } 
}
