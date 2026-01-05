#ifndef CPU_H
#define CPU_H

#include "../Memory/MMU.h"

class CPU {
    public:
        CPU();
        ~CPU();

        void connect(MMU *mmu);
        void setState(int mode);
        void execute();

        std::string debug();

        bool getDoubleSpeed();

        uint8_t op;
        bool cb;
        uint32_t cyclesPassed;
        uint32_t interruptCycles;
        std::array<uint8_t, 256> opcodeCycles = {
            4, 12, 8, 8, 4, 4, 8, 4, 20, 8, 8, 8, 4, 4, 8, 4, 
            4, 12, 8, 8, 4, 4, 8, 4, 12, 8, 8, 8, 4, 4, 8, 4, 
            8, 12, 8, 8, 4, 4, 8, 4, 8, 8, 8, 8, 4, 4, 8, 4, 
            8, 12, 8, 8, 12, 12, 12, 4, 8, 8, 8, 8, 4, 4, 8, 4, 
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, 
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, 
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, 
            8, 8, 8, 8, 8, 8, 4, 8, 4, 4, 4, 4, 4, 4, 8, 4, 
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, 
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, 
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, 
            4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, 
            8, 12, 12, 16, 12, 16, 8, 16, 8, 16, 12, 4, 12, 24, 8, 16, 
            8, 12, 12, 0, 12, 16, 8, 16, 8, 16, 12, 0, 12, 0, 8, 16, 
            12, 12, 8, 0, 0, 16, 8, 16, 16, 4, 16, 0, 0, 0, 8, 16, 
            12, 12, 8, 4, 0, 16, 8, 16, 12, 8, 16, 4, 0, 0, 8, 16, 
        };

        std::array<uint8_t, 256> opcodeCBCycles = {
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, 
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, 
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, 
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, 
            8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, 
            8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, 
            8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, 
            8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, 
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, 
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, 
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, 
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, 
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, 
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, 
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, 
            8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
        };

    private:
        MMU *mmu;
        Timer *timer;
        

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


        // Cycle handling
        uint32_t cycles = 0;

        int CGBMode;
        bool doubleSpeed;

        void resetGB();
        void resetCGB();
        void executeInstruction(uint8_t opcode);
        void executeCBInstruction(uint8_t opcode);
    
        void handleInterrupts();

        void tick();
    
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