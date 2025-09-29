#pragma once

#include <string>
#include <cstdint>
#include "mmu.hpp"
#include "ppu.hpp"

class cpu {
    public: 
        //memory
        mmu mem;
        ppu graphics;

        const uint8_t zf   = 0b10000000; //Zero Flag
        const uint8_t subf = 0b01000000; //Subtraction Flag
        const uint8_t hcf  = 0b00100000; //Half-Carry Flag
        const uint8_t cf   = 0b00010000; //Carry Flag

        uint8_t opcode;

        //registers
        uint16_t AF, BC, DE, HL;

        uint16_t PC = 0;
        uint16_t SP = 0;

        //methods
        void initialize(std::string rom);
        void ld(uint8_t data, uint16_t address) {
            if (address >= 0x8000 && address <= 0x9FFF) {
                graphics.VRAM[address - 0x8000] = data;
            } else {
                mem.ld(data, address);
            }
        }
        void execute();

        uint8_t get_A() const { return (AF >> 8) & 0xFF; }
        uint8_t get_F() const { return AF & 0xFF; }
        uint8_t get_B() const { return (BC >> 8) & 0xFF; }
        uint8_t get_C() const { return BC & 0xFF; }
        uint8_t get_D() const { return (DE >> 8) & 0xFF; }
        uint8_t get_E() const { return DE & 0xFF; }
        uint8_t get_H() const { return (HL >> 8) & 0xFF; }
        uint8_t get_L() const { return HL & 0xFF; }

        void set_A(uint8_t value) { AF = (value << 8) | (AF & 0xFF); }
        void set_F(uint8_t value) { AF = (AF & 0xFF00) | value; }
        void set_B(uint8_t value) { BC = (value << 8) | (BC & 0xFF); }
        void set_C(uint8_t value) { BC = (BC & 0xFF00) | value; }
        void set_D(uint8_t value) { DE = (value << 8) | (DE & 0xFF); }
        void set_E(uint8_t value) { DE = (DE & 0xFF00) | value; }
        void set_H(uint8_t value) { HL = (value << 8) | (HL & 0xFF); }
        void set_L(uint8_t value) { HL = (HL & 0xFF00) | value; }

        void inc_SP() { SP++; }
        void dec_SP() { SP--; }
        void inc_BC() { BC++; }
        void dec_BC() { BC--; }
        void inc_DE() { DE++; }
        void dec_DE() { DE--; }
        void inc_HL() { HL++; }
        void dec_HL() { HL--; }

        //arithmetic
        uint8_t XOR(uint8_t a, uint8_t b);

        //CB opcodes
        void PREFIXED(uint8_t opcode);
        void BIT(int bit, uint8_t reg);

        void PUSH(uint16_t addr);
        void POP(uint16_t reg);

        void RL(uint8_t byte);
        void RLC(uint8_t byte);

        void RR(uint8_t byte);
        void RLA(uint8_t byte);

};

/*
31 fe ff af 21 ff 9f 32 cb 7c 20 fb 21 26 ff e 11 3e 80 32 e2 c 3e f3 e2 32 3e 77 77 3e fc e0
47 11 4 1 21 10 80 1a cd 95 0 cd 96 0 13 7b fe 34 20 f3 11 d8 0 6 8 1a 13 22 23 5 20 f9 3e 19
ea 10 99 21 2f 99 e c 3d 28 8 32 d 20 f9 2e f 18 f3 67 3e 64 57 e0 42 3e 91 e0 40 4 1e 2 e c f0
44 fe 90 20 fa d 20 f7 1d 20 f2 e 13 24 7c 1e 83 fe 62 28 6 1e c1 fe 64 20 6 7b e2 c 3e 87 e2 f0
42 90 e0 42 15 20 d2 5 20 4f 16 20 18 cb 4f 6 4 c5 cb 11 17 c1 cb 11 17 5 20 f5 22 23 22 23 c9 ce ed 
66 66 cc d 0 b 3 73 0 83 0 c 0 d 0 8 11 1f 88 89 0 e dc cc 6e e6 dd dd d9 99 bb bb 67 63 6e e ec cc dd
dc 99 9f bb b9 33 3e 3c 42 b9 a5 b9 a5 42 3c 21 4 1 11 a8 0 1a 13 be 20 fe 23 7d fe 34 20 f5 6 19 78 86
23 5 20 fb 86 20 fe 3e 1 e0 50 
*/