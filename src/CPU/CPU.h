#ifndef CPU_H
#define CPU_H

#include "../Memory/MMU.h"
#include "../IO/Timer.h"
#include "../GPU/GPU.h"
#include "../APU/APU.h"

class CPU {
    public:
        CPU();
        ~CPU();

        void connect(MMU *mmu, Timer *timer, GPU *gpu, APU *apu);
        void setMode(bool mode);
        void execute();

        bool getDoubleSpeed();

    private:
        MMU *mmu;
        Timer *timer;
        GPU *gpu;
        APU *apu;

        /** GP Registers
         * 0 - A
         * 1 - F -> lower 4 bits contains flag registers z, n, h, c (do not confuse the flag registers and gp registers)
         * 2 - B
         * 3 - C
         * 4 - D
         * 5 - E
         * 6 - H
         * 7 - L
         */
        std::array<uint8_t, 8> registers;

        // SP Registers
        uint16_t sp;
        uint16_t pc;

        // Interupts
        bool ime = false; // Interupt master enable flag
        bool halt = false;
        bool halt_bug = false;
        bool ei_hold = false;


        bool CGBMode;
        bool doubleSpeed;
        int32_t stop_delay = -1;

        void resetGB();
        void resetCGB();
        void executeInstruction(uint8_t opcode);
        void executeCBInstruction(uint8_t opcode);
    
        void handleInterrupts();

        void tick();
        uint8_t read8(uint16_t address);
        uint16_t read16(uint16_t address);
        void write8(uint16_t address, uint8_t data);
        void write16(uint16_t address, uint16_t data);
 
        // Set Flag Registers
        void setZ(bool set);
        void setN(bool set);
        void setH(bool set);
        void setC(bool set);


        // Get Flag registers;
        bool getZ();
        bool getN();
        bool getH();
        bool getC();

        // 16-bit flag registers
        uint16_t getBC();
        uint16_t getDE();
        uint16_t getHL();
        uint16_t getAF();

        // 16-bit flag registers setters
        void setBC(uint16_t val);
        void setDE(uint16_t val);
        void setHL(uint16_t val);
        void setAF(uint16_t val);

        // Jump and Subroutine Instructions
        void RET(bool condition);
        void RETUNC(); // Unconditional RET -> has different timing then conditional RET
        void CALL(bool condition);
        void RST(uint8_t vec);
        void JP(bool condition);
        void JR(bool condition);

        // Arithmic Operations
        uint8_t INC8(uint8_t val);
        uint16_t INC16(uint16_t val);
        uint8_t DEC8(uint8_t val);
        uint16_t DEC16(uint16_t val);
        void ADD8(uint8_t val);
        void ADD16(uint16_t val);
        void ADC(uint8_t val);
        void SUB(uint8_t val);
        void SBC(uint8_t val);
        

        // Inequality Operations
        void AND(uint8_t val);
        void XOR(uint8_t val);
        void OR(uint8_t val);
        void CP(uint8_t val);

        // 0xCB Generic Instructions
        uint8_t RES(uint8_t pos, uint8_t reg);
        uint8_t RLC(uint8_t val);
        uint8_t RRC(uint8_t val);
        uint8_t RL(uint8_t val);
        uint8_t RR(uint8_t val);
        uint8_t SRA(uint8_t val);
        uint8_t SLA(uint8_t val);
        uint8_t SRL(uint8_t val);
        uint8_t SWAP(uint8_t val);
        uint8_t SET(uint8_t pos, uint8_t reg);
        void BIT(uint8_t pos, uint8_t reg);

        // Flag helpers
        // void setHAdd(uint8_t left, uint8_t right);
        // void setHAdc(uint8_t left, uint8_t right, uint8_t carry);
        // void setHSub(uint8_t left, uint8_t right);
        // void setHSbc(uint8_t left, uint8_t right, uint8_t carry);
        // void setCAdd(uint8_t left, uint8_t right);
        // void setCAdc(uint8_t left, uint8_t right, uint8_t carry);
        // void setCSub(uint8_t left, uint8_t right);
        // void setCSbc(uint8_t left, uint8_t right, uint8_t carry);


};




#endif