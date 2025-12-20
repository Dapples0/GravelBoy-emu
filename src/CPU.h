#ifndef CPU_H
#define CPU_H

#include "MMU.h"

class CPU {
    public:
        CPU();
        ~CPU();

        void connect(MMU *mmu);
        void setState(int mode);
        void execute();
        void executeInstruction(uint8_t opcode);
    private:
        MMU *mmu;
        

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
        bool ime;

        int CGBMode;

        void resetGB();
        void resetCGB();

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

        // Generic Instructions
        void RET(bool condition);
        void CALL(bool condition);
        void RST(uint8_t vec);
        void JP(bool condition);

        // Arithmic Operations
        uint8_t INC8(uint8_t val);
        uint8_t DEC8(uint8_t val);
        void ADD8(uint8_t val);
        void ADD16(uint16_t val);
        void ADC(uint8_t val);
        void SUB(uint8_t val);
        void SBC(uint8_t val);

        // Inequality Operations
        void XOR(uint8_t val);
        void OR(uint8_t val);
        void CP(uint8_t val);

        // 0xCB Generic Instructions
        void RES();
        void SET();
        void SLA();
        void RLC();
        void RR();
        void SRA();
        void SRL();
        void BIT();
        void RES();


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