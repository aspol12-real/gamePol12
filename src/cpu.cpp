#include "cpu.hpp"
#include "mmu.hpp"

#include <iostream>
#include <fstream>


void cpu::initialize(std::string rom) {
    
    mem.startup = true;

    //LOAD BOOTROM

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

        //misc instructions
        case 0x00: PC++; break; //NOP
        case 0x01: HL = n16; PC += 3; break;
        //8-bit ld instructions
        case 0x02: ld(get_A(), BC); PC++; break;
        case 0x03: inc_BC(); PC++; break;

        case 0x06: set_B(n8); PC += 2; break;
        case 0x08: ld(SP, n16); PC += 3; break;

        case 0x0a: set_A(mem.rd(BC)); PC++; break;
        case 0x0b: dec_BC(); PC++; break;
        case 0x0c: set_C(get_C() + 1); PC++; break;

        case 0x0E: set_C(n8); PC += 2; break;

        case 0x11: DE = n16; PC += 3; break;
        case 0x12: ld(get_A(), DE); PC++; break;
        case 0x13: inc_DE(); PC++; break;

        case 0x16: set_D(n8); PC += 2; break;

        case 0x1A: set_A(mem.rd(DE)); PC++; break;
        case 0x1B: dec_DE(); PC++; break;

        case 0x1E: set_E(n8); PC += 2; break;

        case 0x20: if((get_F() & 0b10000000) == 0){PC += 2 + static_cast<int8_t>(n8);} else { PC += 2;}; break;
        case 0x21: HL = n16; PC += 3; break;
        case 0x22: ld(get_A(), HL); inc_HL(); PC++; break;
        case 0x23: inc_HL(); PC++; break;

        case 0x26: set_H(n8); PC += 2; break;

        case 0x2A: set_A(mem.rd(HL)); inc_HL(); PC++; break;
        case 0x2B: dec_HL(); PC++; break;

        case 0x2E: set_L(n8); PC += 2; break;

        case 0x31: SP = n16; PC += 3; break;
        case 0x32: ld(get_A(), HL); dec_HL(); PC++; break;
        case 0x33: inc_SP(); PC++; break;

        case 0x36: ld(n8, HL); PC += 2; break;

        case 0x3A: set_A(mem.rd(HL)); dec_HL(); PC++; break;
        case 0x3B: dec_SP(); PC++; break;

        case 0x3E: set_A(n8); PC += 2; break;
        
        //reg to reg
        case 0x40: set_B(get_B()); PC++; break;
        case 0x41: set_B(get_C()); PC++; break;
        case 0x42: set_B(get_D()); PC++; break;
        case 0x43: set_B(get_E()); PC++; break;
        case 0x44: set_B(get_H()); PC++; break;
        case 0x45: set_B(get_L()); PC++; break;
        case 0x46: set_B(mem.rd(HL)); PC++; break;
        case 0x47: set_B(get_A()); PC++; break;

        case 0x48: set_C(get_B()); PC++; break;
        case 0x49: set_C(get_C()); PC++; break;
        case 0x4A: set_C(get_D()); PC++; break;
        case 0x4B: set_C(get_E()); PC++; break;
        case 0x4C: set_C(get_H()); PC++; break;
        case 0x4D: set_C(get_L()); PC++; break;
        case 0x4E: set_C(mem.rd(HL)); PC++; break;
        case 0x4F: set_C(get_A()); PC++; break;

        case 0x50: set_D(get_B()); PC++; break;
        case 0x51: set_D(get_C()); PC++; break;
        case 0x52: set_D(get_D()); PC++; break;
        case 0x53: set_D(get_E()); PC++; break;
        case 0x54: set_D(get_H()); PC++; break;
        case 0x55: set_D(get_L()); PC++; break;
        case 0x56: set_D(mem.rd(HL)); PC++; break;
        case 0x57: set_D(get_A()); PC++; break;

        case 0x58: set_E(get_B()); PC++; break;
        case 0x59: set_E(get_C()); PC++; break;
        case 0x5A: set_E(get_D()); PC++; break;
        case 0x5B: set_E(get_E()); PC++; break;
        case 0x5C: set_E(get_H()); PC++; break;
        case 0x5D: set_E(get_L()); PC++; break;
        case 0x5E: set_E(mem.rd(HL)); PC++; break;
        case 0x5F: set_E(get_A()); PC++; break;

        case 0x60: set_H(get_B()); PC++; break;
        case 0x61: set_H(get_C()); PC++; break;
        case 0x62: set_H(get_D()); PC++; break;
        case 0x63: set_H(get_E()); PC++; break;
        case 0x64: set_H(get_H()); PC++; break;
        case 0x65: set_H(get_L()); PC++; break;
        case 0x66: set_H(mem.rd(HL)); PC++; break;
        case 0x67: set_H(get_A()); PC++; break;

        case 0x68: set_L(get_B()); PC++; break;
        case 0x69: set_L(get_C()); PC++; break;
        case 0x6A: set_L(get_D()); PC++; break;
        case 0x6B: set_L(get_E()); PC++; break;
        case 0x6C: set_L(get_H()); PC++; break;
        case 0x6D: set_L(get_L()); PC++; break;
        case 0x6E: set_L(mem.rd(HL)); PC++; break;
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
        case 0x7E: set_A(mem.rd(HL)); PC++; break;
        case 0x7F: set_A(get_A()); PC++; break;

        case 0xAF: set_A(XOR(get_A(), get_A())); PC++; break;

        case 0xC1: POP(BC); PC++l break;

        case 0xC5: PUSH(BC); PC++; break;

        case 0xCB: PREFIXED(opcode); PC += 2; break;

        case 0xCD: PUSH(PC + 3); PC = n16; break;  

        case 0xD1: POP(DE); PC++; break;

        case 0xD5: PUSH(DE); PC++; break;

        case 0xE0: ld(get_A(), a8); PC += 2; break;
        case 0xE1: POP(HL); PC++; break;
        case 0xE2: ld(get_A(), 0xFF00 + get_C()); PC++; break;

        case 0xE5: PUSH(HL); PC++; break;

        case 0xEA: ld(get_A(), n16); PC += 3; break;

        case 0xF0: set_A(mem.rd(a8)); PC += 2; break;
        case 0xF2: set_A(mem.rd(get_C())); PC++; break;
        case 0xFA: set_A(mem.rd(n16)); PC += 3; break;

    }
    //execute
}

uint8_t cpu::XOR(uint8_t a, uint8_t b) {
    uint8_t result = a ^ b;
    uint8_t flags = 0;

    if (result == 0) {
        flags |= zf;
    }

    set_F(flags);
    return result;
}

void cpu::PREFIXED(uint8_t opcode) {
    switch (mem.rd(PC + 1)) {

        case 0x7C:
            BIT(7, get_H());
            break;
    }
}

void cpu::BIT(int bit, uint8_t reg) {
    uint8_t result = (reg >> bit) & 0x1;

    uint8_t flags = 0;
    flags |= hcf;

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

void cpu::POP(uint16_t reg) {

    SP++;
    (reg >> 8) & 0xFF = mem.rd(SP); 
    SP++;
    reg & 0xFF = mem.rd(SP);

}

void cpu::RL(uint8_t byte) {
    
}