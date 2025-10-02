#pragma once

#include <iostream>
#include <string>
#include <cstdint>
#include "mmu.hpp"
#include "ppu.hpp"

class cpu {
    public: 
        //memory
        mmu mem;
        ppu graphics;

        cpu() : mem(), graphics(&mem) {}
        
        bool startup = false;
        bool IME = true;
        bool enable_pending = false;
        bool disable_pending = false;

        const uint8_t zf   = 0b10000000; //Zero Flag
        const uint8_t nf   = 0b01000000; //Subtraction Flag
        const uint8_t hf   = 0b00100000; //Half-Carry Flag
        const uint8_t cf   = 0b00010000; //Carry Flag

        uint8_t opcode = 0;

        int cycles = 0;

        //registers
        uint16_t AF = 0x01B0;
        uint16_t BC, DE, HL = 0x0000;
        uint8_t dataRet = 0;
        uint16_t PC = 0;
        uint16_t SP = 0xFFFE;

        //methods
        void initialize(std::string rom);
        void ld(uint8_t data, uint16_t address) {
            if (address >= 0x8000 && address <= 0x9FFF) {
                if (graphics.vramRestrict) {
                    std::cout << "VRAM IS LOCKED UP! Attempted from PC: " << std::hex << PC 
              << " HL: " << HL << " (Accessed addr: " << address << ")\n";
                    return;
                } else {
                    graphics.VRAM[address - 0x8000] = data;
                    return; 
                }
            } else if (address >= 0xFE00 && address <= 0xFE9F) {
                if (graphics.oamRestrict) {
                    std::cout << "OAM IS LOCKED UP! \n";
                    return; 
                } else {
                    graphics.OAM[address - 0xFE00] = data;
                    return;
                }
            } else if (address >= 0xFEA0 && address <= 0xFEFF) {
                std::cout << "NOT USABLE. PC = " << std::hex << +PC << " OPCODE = " << +opcode << " HL = " << +HL << "\n"; 
            } 
            else {
                mem.ld(data, address);
            }
        }
        uint8_t rd(uint16_t address) {
            
            if (address >= 0x8000 && address <= 0x9FFF) {
                if (graphics.vramRestrict) {
                    std::cout << "VRAM IS LOCKED UP! Attempted from PC: " << std::hex << PC 
              << " HL: " << HL << " (Accessed addr: " << address << ")\n";
                    return 0;
                } else {
                    return graphics.VRAM[address - 0x8000];
                }
            } else if (address >= 0xFE00 && address <= 0xFE9F) {
                if (graphics.oamRestrict) {
                    std::cout << "OAM IS LOCKED UP! \n";
                    return 0;
                } else {
                    return graphics.OAM[address - 0xFE00];
                }
            } else {
                return mem.rd(address);
            }
        }
        int execute();

        uint8_t get_A() const { return (AF >> 8) & 0xFF; }
        uint8_t get_F() const { return AF & 0xFF; }
        uint8_t get_B() const { return (BC >> 8) & 0xFF; }
        uint8_t get_C() const { return BC & 0xFF; }
        uint8_t get_D() const { return (DE >> 8) & 0xFF; }
        uint8_t get_E() const { return DE & 0xFF; }
        uint8_t get_H() const { return (HL >> 8) & 0xFF; }
        uint8_t get_L() const { return HL & 0xFF; }

        bool get_ZF() { return (AF & zf) != 0; }
        bool get_NF() { return (AF & nf) != 0; }
        bool get_HF() { return (AF & hf) != 0; }
        bool get_CF() { return (AF & cf) != 0; }

        void set_A(uint8_t value) { AF = (value << 8) | (AF & 0xFF); }
        void set_F(uint8_t value) { AF = (AF & 0xFF00) | value; }
        void set_B(uint8_t value) { BC = (value << 8) | (BC & 0xFF); }
        void set_C(uint8_t value) { BC = (BC & 0xFF00) | value; }
        void set_D(uint8_t value) { DE = (value << 8) | (DE & 0xFF); }
        void set_E(uint8_t value) { DE = (DE & 0xFF00) | value; }
        void set_H(uint8_t value) { HL = (value << 8) | (HL & 0xFF); }
        void set_L(uint8_t value) { HL = (HL & 0xFF00) | value; }

        void set_ZF(bool set) { 
            if (set) {
                AF |= zf;
            } else {
                AF &= ~zf;
            }
        }

        void set_NF(bool set) { 
            if (set) {
                AF |= nf; 
            } else {
                AF &= ~nf;
            }
        }

        void set_HF(bool set) { 
            if (set) {
                AF |= hf; 
            } else {
                AF &= ~hf;
            }
        }

        void set_CF(bool set) { 
            if (set) {
                AF |= cf; 
            } else {
                AF &= ~cf; 
            }
        }

        void inc_SP() { SP++; }
        void dec_SP() { SP--; }
        void inc_BC() { BC++; }
        void dec_BC() { BC--; }
        void inc_DE() { DE++; }
        void dec_DE() { DE--; }
        void inc_HL() { HL++; }
        void dec_HL() { HL--; }

        //arithmetic
        uint8_t AND(uint8_t a, uint8_t b);
        uint8_t OR(uint8_t a, uint8_t b);
        uint8_t XOR(uint8_t a, uint8_t b);
        uint8_t INC(uint8_t byte);
        uint8_t DEC(uint8_t byte);
        void ADD8(uint8_t byte);
        uint32_t ADD16(uint16_t a, uint16_t b);
        void SUB(uint8_t byte);


        //CB opcodes
        void PREFIXED(uint8_t opcode);
        void BIT(int bit, uint8_t reg);

        void PUSH_AF(uint16_t addr);
        void PUSH(uint16_t addr);
        void POP(uint16_t& reg);

        uint8_t RL(uint8_t byte);
        uint8_t RLC(uint8_t byte);

        uint8_t RR(uint8_t byte);
        uint8_t RRC(uint8_t byte);

        void CP(uint8_t a, uint8_t b);
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