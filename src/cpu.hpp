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
        bool stopped = false;
        bool halted  = false;

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
                    return;
                } else {
                    graphics.VRAM[address - 0x8000] = data;
                    return; 
                }
            } else if (address >= 0xFE00 && address <= 0xFE9F) {
                if (graphics.oamRestrict) {
                    return; 
                } else {
                    graphics.OAM[address - 0xFE00] = data;
                    return;
                }
            } else if (address >= 0xFEA0 && address <= 0xFEFF) {
                // std::cout << "NOT USABLE. PC = " << std::hex << +PC << " OPCODE = " << +opcode << " HL = " << +HL << "\n"; 
            } 
            else {
                mem.ld(data, address);
            }
        }
        uint8_t rd(uint16_t address) {
            
            if (address >= 0x8000 && address <= 0x9FFF) {
                if (graphics.vramRestrict) {
                    return 0xFF;
                } else {
                    return graphics.VRAM[address - 0x8000];
                }
            } else if (address >= 0xFE00 && address <= 0xFE9F) {
                if (graphics.oamRestrict) {
                    return 0xFF;
                } else {
                    return graphics.OAM[address - 0xFE00];
                }
            } else if (address >= 0xFEA0 && address <= 0xFEFF) {
                // std::cout << "NOT USABLE. PC = " << std::hex << +PC << " OPCODE = " << +opcode << " HL = " << +HL << "\n";
                return 0xFF; 
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
        void ADC(uint8_t byte);
        void SBC(uint8_t byte);
        uint16_t SPADD(uint8_t byte);
        uint8_t SWAP(uint8_t reg);
        uint8_t RES(uint8_t bit, uint8_t reg);
        uint8_t SET(uint8_t bit, uint8_t reg);

        //CB opcodes
        void PREFIXED();
        void BIT(int bit, uint8_t reg);
        void DAA();

        void PUSH_AF();
        void POP_AF();
        void PUSH(uint16_t addr);
        void POP(uint16_t& reg);

        uint8_t RL(uint8_t byte);
        uint8_t RLC(uint8_t byte);

        uint8_t RR(uint8_t byte);
        uint8_t RRC(uint8_t byte);

        void CP(uint8_t a, uint8_t b);
};
