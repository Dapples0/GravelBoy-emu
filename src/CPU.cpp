#include "CPU.h"

#include <iostream>

CPU::CPU() {


}

CPU::~CPU() {
}

void CPU::connect(MMU *mmu) {
    this->mmu = mmu;
}

void CPU::execute() {
    uint8_t opcode = mmu->read8(pc);

    std::cout << "opcode: " << opcode << "\n";

    executeInstruction(opcode);

    
}

void CPU::executeInstruction(uint8_t opcode) {
    uint8_t oldPC = pc; // For passing into 0xCB prefix instruction set
    pc++;
    switch (opcode) {
        case 0xCB: // 0xCB Prefixed
            break;
        case 0x00: // NOP
            break;
        case 0x01:

            break;

        // ADD

        // ADD 16 Bit


        // ADC


        // SUB

        // SBC

        // XOR

        // OR

        // CP

        // LD (nn), n


        // LD n, u8

        // LD n, (nn)

        // LD n, n


        default:
            std::cerr << "Bad instruction | Opcode: " << opcode << " PC: " << pc << "\n";
            break;
    }
}


void CPU::setState(int mode)
{
    CGBMode = mode;

    if (!mode) {
        resetGB();
        
    } else {
        resetCGB();
    }
}

void CPU::resetGB()
{
    registers[0] = 0x01;
    registers[1] = 0xB0;
    registers[2] = 0x00;
    registers[3] = 0x13;
    registers[4] = 0x00;
    registers[5] = 0xD8;
    registers[6] = 0x01;
    registers[7] = 0x4D;
    pc = 0x100;
    sp = 0xFFFE;
}

void CPU::resetCGB()
{
    registers[0] = 0x11;
    registers[1] = 0x80;
    registers[2] = 0x00;
    registers[3] = 0x00;
    registers[4] = 0xFF;
    registers[5] = 0x56;
    registers[6] = 0x00;
    registers[7] = 0x0D;
    pc = 0x100;
    sp = 0xFFFE;
}

void CPU::setZ(bool set) {
    if (set) {
        registers[1] |= 0x80;
    } else {
        registers[1] &= 0x7F;
    }
}

void CPU::setN(bool set) {
    if (set) {
        registers[1] |= 0x40;
    } else {
        registers[1] &= 0xBF;
    }
}

void CPU::setH(bool set) {
    if (set) {
        registers[1] |= 0x20;
    } else {
        registers[1] &= 0xDF;
    }
}

void CPU::setC(bool set) {
    if (set) {
        registers[1] |= 0x10;
    } else {
        registers[1] &= 0xEF;
    }
}

bool CPU::getZ() {
    return (registers[1] & 0x80) != 0;
}

bool CPU::getN()
{
    return (registers[1] & 0x40) != 0;
}

bool CPU::getH()
{
    return (registers[1] & 0x20) != 0;
}

bool CPU::getC()
{
    return (registers[1] & 0x10) != 0;
}


uint16_t CPU::getBC() {
    return registers[2] << 8 | registers[3];
}

uint16_t CPU::getDE() {
    return registers[4] << 8 | registers[5];
}

uint16_t CPU::getHL() {
    return registers[6] << 8 | registers[7];
}


void CPU::ADD8(uint8_t val) {
    setN(false);
    setH((registers[0] & 0xF) + (val & 0xF) > 0xF);
    setC((registers[0] + val) > 0xFF);
    
    registers[0] += val;

    setZ(registers[0] == 0);
}

void CPU::ADD16(uint16_t val) {

}

void CPU::ADC(uint8_t val) {
    setN(false);
    uint8_t carry = getC();
    setH((registers[0] & 0xF) + (val & 0xF) + carry > 0xF);
    setC((registers[0] + val + carry) > 0xFF);

    registers[0] += carry;

    setZ(registers[0] == 0);
}

void CPU::SUB(uint8_t val) {
    setN(true);

    setH((registers[0] & 0xF) < (val & 0xF));
    setC(registers[0] < val);

    registers[0] -= val;


    setZ(registers[0] == 0);
}

void CPU::SBC(uint8_t val) {
    setN(true);

    uint8_t carry = getC();

    setH((registers[0] & 0xF) < ((val & 0xF) + carry));
    setC(registers[0] < (val + carry));

    registers[0] -= carry;

    setZ(registers[0] == 0);
}


void CPU::XOR(uint8_t val) {
    setN(false);
    setC(false);
    setH(false);

    uint8_t res = registers[0] ^ val;

    setZ(res == 0);
    registers[0] = res;
}

void CPU::OR(uint8_t val) {
    setN(false);
    setC(false);
    setH(false);

    uint8_t res = registers[0] | val;

    setZ(res == 0);

    registers[0] = res;
}

void CPU::CP(uint8_t val) {
    setN(true);
    setC(registers[0] < val);
    setH((registers[0] & 0xF) < (val & 0xF));
    setZ(registers[0] == val);
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
