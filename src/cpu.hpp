#pragma once

#include "mmu.hpp"

#include <iostream>
#include <string>
#include <cstdint>

class cpu {
    private:

        mmu& mem;

    public: 

        cpu(mmu& shared_memory) : mem(shared_memory){};

        bool IME = true;
        bool ime_schedule = false;
        bool enable_pending = false;
        bool disable_pending = false;
        bool stopped = false;
        bool halted  = false;
        bool haltBug = false;

        const uint8_t zf   = 0b10000000; //Zero Flag
        const uint8_t nf   = 0b01000000; //Subtraction Flag
        const uint8_t hf   = 0b00100000; //Half-Carry Flag
        const uint8_t cf   = 0b00010000; //Carry Flag

        const int MAX_ROM_SIZE = 8388608; 

        uint8_t opcode = 0;

        int cycles = 0;

        //registers
        uint16_t AF = 0x01B0;
        uint16_t BC, DE, HL = 0x0000;
        uint8_t dataRet = 0;
        uint16_t PC;
        uint16_t SP;

        //methods
        void initialize(std::string rom);

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
            uint8_t f = AF & 0xFF; 
    
            if (set) {
                f |= cf; 
            } else {
                f &= ~cf; 
            }
            AF = (AF & 0xFF00) | f;
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
        void DAA();


        //CB opcodes
        void PREFIXED();
        void BIT(int bit, uint8_t reg);
        uint8_t SWAP(uint8_t reg);
        uint8_t RES(uint8_t bit, uint8_t reg);
        uint8_t SET(uint8_t bit, uint8_t reg);
        uint8_t SRA(uint8_t byte);
        uint8_t SRL(uint8_t byte);
        uint8_t SLA(uint8_t byte);
        uint8_t RL(uint8_t byte);
        uint8_t RLC(uint8_t byte);
        uint8_t RR(uint8_t byte);
        uint8_t RRC(uint8_t byte);


        //control flow
        void PUSH_AF();
        void POP_AF();
        void PUSH(uint16_t addr);
        void POP(uint16_t& reg);
        void CP(uint8_t a, uint8_t b);
        void stop(uint8_t n8);
        void halt();

};
