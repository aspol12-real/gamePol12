#include "cpu.hpp"
#include "mmu.hpp"

#include <iostream>
#include <fstream>


void cpu::initialize(std::string rom) {
    
    mem.startup = true;

    std::ifstream file;
    file.open(rom, std::ios::in | std::ios::binary);

    if (!file.is_open()) {
        std::cout << "Could not locate source file: " << rom <<  "\n";
        exit( 1 );
    } else {
        std::cout << "Loading rom file: " << rom << "\n";
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0);

    if (fileSize < 0x4000 || fileSize > 0x800000) {
        std::cout << "Invalid ROM size: " << fileSize << " bytes\n";
        exit(1);
    }

    char byte;
    int address = 0;

    while (file.get(byte) && address < 0x8000) {
        ld(byte, address);
        address++;
    }

    file.close();

}

void cpu::execute() {
    //get current opcode
    uint8_t opcode = mem.rd(PC);

    uint16_t n16 = (mem.rd(PC + 2) << 8) | mem.rd(PC + 1);
    uint8_t  n8  =  mem.rd(PC + 1);
    uint16_t a8 = 0xFF00 + n8;

    //decode

    switch(opcode) {

        case 0x00: PC++; break; //NOP
        case 0x01: HL = n16; PC += 3; break;
        case 0x02: ld(get_A(), BC); PC++; break;
        case 0x03: inc_BC(); PC++; break;
        case 0x04: set_B(INC(get_B())); PC++; break; //INC B
        case 0x05: set_B(DEC(get_B())); PC++; break; //DEC B
        case 0x06: set_B(n8); PC += 2; break;
        case 0x07: set_A(RLC(get_A())); PC++; break;
        case 0x08: ld(SP, n16); PC += 3; break;

        case 0x0A: set_A(rd(BC)); PC++; break;
        case 0x0B: dec_BC(); PC++; break;
        case 0x0C: set_C(INC(get_C())); PC++; break;
        case 0x0D: set_C(DEC(get_C())); PC++; break;
        case 0x0E: set_C(n8); PC += 2; break;

        case 0x11: DE = n16; PC += 3; break;
        case 0x12: ld(get_A(), DE); PC++; break;
        case 0x13: inc_DE(); PC++; break;
        case 0x14: set_D(INC(get_D())); PC++; break;
        case 0x15: set_D(DEC(get_D())); PC++; break;
        case 0x16: set_D(n8); PC += 2; break;
        case 0x17: set_A(RL(get_A())); PC++; break;
        case 0x18: PC += 2 + static_cast<int8_t>(n8); break;

        case 0x1A: set_A(rd(DE)); PC++; break;
        case 0x1B: dec_DE(); PC++; break;
        case 0x1C: set_E(INC(get_E())); PC++; break;
        case 0x1D: set_E(DEC(get_E())); PC++; break;
        case 0x1E: set_E(n8); PC += 2; break;

        case 0x20: if((get_ZF()) == 0){PC += 2 + static_cast<int8_t>(n8);} else { PC += 2;}; break;
        case 0x21: HL = n16; PC += 3; break;
        case 0x22: ld(get_A(), HL); inc_HL(); PC++; break;
        case 0x23: inc_HL(); PC++; break;
        case 0x24: set_H(INC(get_H())); PC++; break;
        case 0x25: set_H(DEC(get_H())); PC++; break;
        case 0x26: set_H(n8); PC += 2; break;

        case 0x28: if((get_ZF()) == 1){PC += 2 + static_cast<int8_t>(n8);} else { PC += 2;}; break;

        case 0x2A: set_A(rd(HL)); inc_HL(); PC++; break;
        case 0x2B: dec_HL(); PC++; break;
        case 0x2C: set_L(INC(get_L())); PC++; break;
        case 0x2D: set_L(DEC(get_L())); PC++; break;
        case 0x2E: set_L(n8); PC += 2; break;
        case 0x2F: set_A(~get_A()); set_NF(true); set_HF(true);  PC++; break; //CPL

        case 0x31: SP = n16; PC += 3; break;
        case 0x32: ld(get_A(), HL); dec_HL(); PC++; break;
        case 0x33: inc_SP(); PC++; break;
        case 0x34: ld(INC(rd(HL)), HL); PC++; break;
        case 0x35: ld(DEC(rd(HL)), HL); PC++; break;
        case 0x36: ld(n8, HL); PC += 2; break;

        case 0x3A: set_A(rd(HL)); dec_HL(); PC++; break;
        case 0x3B: dec_SP(); PC++; break;
        case 0x3C: set_A(INC(get_A())); PC++; break;
        case 0x3D: set_A(DEC(get_A())); PC++; break;

        case 0x3E: set_A(n8); PC += 2; break;
        
        //reg to reg
        case 0x40: set_B(get_B()); PC++; break;
        case 0x41: set_B(get_C()); PC++; break;
        case 0x42: set_B(get_D()); PC++; break;
        case 0x43: set_B(get_E()); PC++; break;
        case 0x44: set_B(get_H()); PC++; break;
        case 0x45: set_B(get_L()); PC++; break;
        case 0x46: set_B(rd(HL)); PC++; break;
        case 0x47: set_B(get_A()); PC++; break;

        case 0x48: set_C(get_B()); PC++; break;
        case 0x49: set_C(get_C()); PC++; break;
        case 0x4A: set_C(get_D()); PC++; break;
        case 0x4B: set_C(get_E()); PC++; break;
        case 0x4C: set_C(get_H()); PC++; break;
        case 0x4D: set_C(get_L()); PC++; break;
        case 0x4E: set_C(rd(HL)); PC++; break;
        case 0x4F: set_C(get_A()); PC++; break;

        case 0x50: set_D(get_B()); PC++; break;
        case 0x51: set_D(get_C()); PC++; break;
        case 0x52: set_D(get_D()); PC++; break;
        case 0x53: set_D(get_E()); PC++; break;
        case 0x54: set_D(get_H()); PC++; break;
        case 0x55: set_D(get_L()); PC++; break;
        case 0x56: set_D(rd(HL)); PC++; break;
        case 0x57: set_D(get_A()); PC++; break;

        case 0x58: set_E(get_B()); PC++; break;
        case 0x59: set_E(get_C()); PC++; break;
        case 0x5A: set_E(get_D()); PC++; break;
        case 0x5B: set_E(get_E()); PC++; break;
        case 0x5C: set_E(get_H()); PC++; break;
        case 0x5D: set_E(get_L()); PC++; break;
        case 0x5E: set_E(rd(HL)); PC++; break;
        case 0x5F: set_E(get_A()); PC++; break;

        case 0x60: set_H(get_B()); PC++; break;
        case 0x61: set_H(get_C()); PC++; break;
        case 0x62: set_H(get_D()); PC++; break;
        case 0x63: set_H(get_E()); PC++; break;
        case 0x64: set_H(get_H()); PC++; break;
        case 0x65: set_H(get_L()); PC++; break;
        case 0x66: set_H(rd(HL)); PC++; break;
        case 0x67: set_H(get_A()); PC++; break;

        case 0x68: set_L(get_B()); PC++; break;
        case 0x69: set_L(get_C()); PC++; break;
        case 0x6A: set_L(get_D()); PC++; break;
        case 0x6B: set_L(get_E()); PC++; break;
        case 0x6C: set_L(get_H()); PC++; break;
        case 0x6D: set_L(get_L()); PC++; break;
        case 0x6E: set_L(rd(HL)); PC++; break;
        case 0x6F: set_L(get_A()); PC++; break;

        case 0x70: ld(get_B(), HL); PC++; break;
        case 0x71: ld(get_C(), HL); PC++; break;
        case 0x72: ld(get_D(), HL); PC++; break;
        case 0x73: ld(get_E(), HL); PC++; break;
        case 0x74: ld(get_H(), HL); PC++; break;
        case 0x75: ld(get_L(), HL); PC++; break;

        case 0x77: ld(get_A(), HL); PC++; break;

        case 0x78: set_A(get_B()); PC++; break;
        case 0x79: set_A(get_C()); PC++; break;
        case 0x7A: set_A(get_D()); PC++; break;
        case 0x7B: set_A(get_E()); PC++; break;
        case 0x7C: set_A(get_H()); PC++; break;
        case 0x7D: set_A(get_L()); PC++; break;
        case 0x7E: set_A(rd(HL)); PC++; break;
        case 0x7F: set_A(get_A()); PC++; break;

        case 0x86: 

        case 0x90: SUB(get_B()); PC++; break;

        case 0xAF: set_A(XOR(get_A(), get_A())); PC++; break;

        case 0xBE: CP(get_A(), rd(HL)); PC++; break;

        case 0xC1: POP(BC); PC++; break;

        case 0xC5: PUSH(BC); PC++; break;

        case 0xC9: POP(PC); break; //return from subroutine

        case 0xCB: PREFIXED(opcode); PC += 2; break;

        case 0xCD: PUSH(PC + 3); PC = n16; break;  

        case 0xD1: POP(DE); PC++; break;

        case 0xD5: PUSH(DE); PC++; break;

        case 0xE0: ld(get_A(), a8); PC += 2; break;
        case 0xE1: POP(HL); PC++; break;
        case 0xE2: ld(get_A(), 0xFF00 + get_C()); PC++; break;

        case 0xE5: PUSH(HL); PC++; break;

        case 0xEA: ld(get_A(), n16); PC += 3; break;

        case 0xF0: set_A(rd(a8)); PC += 2; break;
        case 0xF2: set_A(rd(get_C())); PC++; break;
        case 0xFA: set_A(rd(n16)); PC += 3; break;

        case 0xFE: CP(get_A(), n8); PC += 2; break;

        default:
            std::cout << "UNKNOWN OPCODE: " << std::hex << +opcode << "\n";
    }
    //execute
}

void cpu::PREFIXED(uint8_t opcode) {
    switch (rd(PC + 1)) {
        case 0x11: set_C(RL(get_C())); break;
        case 0x7C: BIT(7, get_H()); break;
    }
}




uint8_t cpu::XOR(uint8_t a, uint8_t b) {
    uint8_t result = a ^ b;
    set_ZF(result == 0);
    set_CF(false);
    set_NF(false);
    set_HF(false);

    return result;
}

void cpu::BIT(int bit, uint8_t reg) {
    uint8_t result = (reg >> bit) & 0x1;

    uint8_t flags = 0;
    flags |= hf;

    if (result == 0) {
        flags |= zf;
    }

    set_F(flags);
}

void cpu::PUSH(uint16_t addr) {
    SP -= 2;
    ld(addr & 0xFF, SP);     
    ld(addr >> 8, SP + 1);
}

void cpu::POP(uint16_t& reg) {

    uint8_t low = rd(SP);
    SP++;
    uint8_t high = rd(SP); 
    SP++;
    reg = (high << 8) | low;

}

uint8_t cpu::RL(uint8_t byte) {
    uint8_t carryBit = (get_F() & cf) >> 4;
    uint8_t carryFlag = (byte >> 3) & cf;
    uint8_t resultByte = (byte << 1) | carryBit;
    
    set_ZF(resultByte == 0);
    set_NF(false);
    set_HF(false);
    set_CF(carryFlag != 0);
    
    return resultByte;
}

uint8_t cpu::RLC(uint8_t byte) {
    uint8_t carryBit = byte >> 7;
    uint8_t carryFlag = (byte >> 3) & cf;
    uint8_t resultByte = (byte << 1) | carryBit;
    
    set_ZF(resultByte == 0);
    set_NF(false);
    set_HF(false);
    set_CF(carryFlag != 0);
    
    return resultByte;
}

uint8_t cpu::INC(uint8_t byte) {

    set_ZF((byte + 1) == 0);
    set_NF(false);

    return byte + 1;
}

uint8_t cpu::DEC(uint8_t byte) {

    set_ZF((byte - 1) == 0);
    set_NF(true);

    return byte - 1;
}

void cpu::CP(uint8_t a, uint8_t b) {
    uint8_t result = a - b;
    
    set_ZF(result == 0);
    set_NF(true);

    set_CF(b > a);
}

void cpu::ADD(uint8_t byte) {
    uint8_t result = get_A() + byte;
    set_A(get_A() + byte);

    set_ZF((get_A() + byte) == 0);
    set_NF(false);
    set_HF(((get_A() & 0x0F) + (byte & 0x0F)) > 0x0F);  // Half-carry
    set_CF(result > 0xFF);    
}

void cpu::SUB(uint8_t byte) {
    set_A(get_A() - byte);

    set_ZF((get_A() - byte) == 0);
    set_NF(true);

    set_CF(byte > get_A());
}