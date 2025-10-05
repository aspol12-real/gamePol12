#include "cpu.hpp"
#include "mmu.hpp"

#include <iostream>
#include <fstream>


void cpu::initialize(std::string rom) {
    
    PC = 0x0;
    SP = 0xFFFE;
    std::ifstream file;
    file.open(rom, std::ios::in | std::ios::binary);

    if (!file.is_open()) {
        std::cout << "Could not locate source file: " << rom <<  "\n";
        exit( 1 );
    } else {
        std::cout << "\n\nLoading rom file: " << rom << "\n";
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
        if (address >= 0 && address <= 0x3FFF) { //romBank 0
            mem.cart.romBank0[address] = byte;
        }
        else if (address >= 0x4000 && address <= 0x7FFF) { //romBank 1-NN (depending on mapper)
            mem.cart.romBank1[address - 0x4000] = byte;
        }
        address++;
    }

    file.close();

    ld(0x80, 0xFF40); //LCDC
    ld(0xFF, 0xFF00);

    ld(0b10000000, 0xFF02); //SERIAL PORT DISABLED

}

int cpu::execute() {


    //get current opcode
    opcode = rd(PC);

    uint16_t n16 = (rd(PC + 2) << 8) | rd(PC + 1);
    uint8_t  n8  =  rd(PC + 1);
    uint16_t a8 = 0xFF00 + n8;
    int8_t  e8 = static_cast<int8_t>(n8);

    if (disable_pending) {
        IME = false;
        disable_pending = false;
    }
    if (enable_pending) {
        IME = true;
        enable_pending = false;
    }
    //decode

    switch(opcode) {

        case 0x00: PC++; cycles = 4; break; //NOP
        case 0x01: BC = n16; PC += 3; cycles = 12; break;
        case 0x02: ld(get_A(), BC); PC++; cycles = 8; break;
        case 0x03: inc_BC(); PC++; cycles = 8; break;
        case 0x04: set_B(INC(get_B())); PC++; cycles = 4; break; //INC B
        case 0x05: set_B(DEC(get_B())); PC++; cycles = 4; break; //DEC B
        case 0x06: set_B(n8); PC += 2; cycles = 8; break;
        case 0x07: set_A(RLC(get_A())); set_ZF(false); PC++; cycles = 4; break;
        case 0x08: ld(SP, n16); PC += 3; cycles = 20; break;
        case 0x09: HL = ADD16(HL , BC); PC++; cycles = 8; break;
        case 0x0A: set_A(rd(BC)); PC++; cycles = 8; break;
        case 0x0B: dec_BC(); PC++; cycles = 8; break;
        case 0x0C: set_C(INC(get_C())); PC++; cycles = 4; break;
        case 0x0D: set_C(DEC(get_C())); PC++; cycles = 4; break;
        case 0x0E: set_C(n8); PC += 2; cycles = 8; break;
        case 0x0F: set_A(RRC(get_A())); set_ZF(false); PC++; cycles = 4; break;


        case 0x10: if(n8 == 0x00) {stopped = true;} PC += 2; cycles = 4; break;
        case 0x11: DE = n16; PC += 3; cycles = 12; break;
        case 0x12: ld(get_A(), DE); PC++; cycles = 8; break;
        case 0x13: inc_DE(); PC++; cycles = 8; break;
        case 0x14: set_D(INC(get_D())); PC++; cycles = 4; break;
        case 0x15: set_D(DEC(get_D())); PC++; cycles = 4; break;
        case 0x16: set_D(n8); PC += 2; cycles = 8; break;
        case 0x17: set_A(RL(get_A())); set_ZF(false); PC++; cycles = 4; break;
        case 0x18: PC += 2 + e8; cycles = 12; break;
        case 0x19: HL = ADD16(HL, DE); PC++; cycles = 8; break;
        case 0x1A: set_A(rd(DE)); PC++; cycles = 8; break;
        case 0x1B: dec_DE(); PC++; cycles = 8; break;
        case 0x1C: set_E(INC(get_E())); PC++; cycles = 4; break;
        case 0x1D: set_E(DEC(get_E())); PC++; cycles = 4; break;
        case 0x1E: set_E(n8); PC += 2; cycles = 8; break;
        case 0x1F: set_A(RR(get_A())); set_ZF(false); PC++; cycles = 4; break;


        case 0x20: if((get_ZF()) == 0){PC += 2 + e8; cycles = 12;} else { PC += 2; cycles = 8;}; break;
        case 0x21: HL = n16; PC += 3; cycles = 12; break;
        case 0x22: ld(get_A(), HL); inc_HL(); PC++; cycles = 8; break;
        case 0x23: inc_HL(); PC++; cycles = 8; break;
        case 0x24: set_H(INC(get_H())); PC++; cycles = 4; break;
        case 0x25: set_H(DEC(get_H())); PC++; cycles = 4; break;
        case 0x26: set_H(n8); PC += 2; cycles = 8; break;
        //missing 0x27 DAA
        case 0x28: if((get_ZF()) == 1){PC += 2 + e8; cycles = 12;} else { PC += 2; cycles = 8;}; break;
        case 0x29: HL = ADD16(HL, HL); PC++; cycles = 8; break;
        case 0x2A: set_A(rd(HL)); inc_HL(); PC++; cycles = 8; break;
        case 0x2B: dec_HL(); PC++; cycles = 8; break;
        case 0x2C: set_L(INC(get_L())); PC++; cycles = 4; break;
        case 0x2D: set_L(DEC(get_L())); PC++; cycles = 4; break;
        case 0x2E: set_L(n8); PC += 2; cycles = 8; break;
        case 0x2F: set_A(~get_A()); set_NF(true); set_HF(true);  PC++; cycles = 4; break; //CPL


        case 0x30: if((get_CF() == 0)) {PC += 2 + e8; cycles = 12;} else { PC += 2; cycles = 8;}; break;
        case 0x31: SP = n16; PC += 3; cycles = 12; break;
        case 0x32: ld(get_A(), HL); dec_HL(); PC++; cycles = 8; break;
        case 0x33: inc_SP(); PC++; cycles = 8; break;
        case 0x34: ld(INC(rd(HL)), HL); PC++; cycles = 12; break;
        case 0x35: ld(DEC(rd(HL)), HL); PC++; cycles = 12; break;
        case 0x36: ld(n8, HL); PC += 2; cycles = 12; break;
        case 0x37: set_NF(false); set_HF(false); set_CF(true); PC++; cycles = 4; break; //SCF 
        case 0x38: if((get_CF() == 1)) {PC += 2 + e8; cycles = 12;} else { PC += 2; cycles = 8;}; break;
        case 0x39: HL = ADD16(HL, SP); PC++; cycles = 8; break;
        case 0x3A: set_A(rd(HL)); dec_HL(); PC++; cycles = 8; break;
        case 0x3B: dec_SP(); PC++; cycles = 8; break;
        case 0x3C: set_A(INC(get_A())); PC++; cycles = 4; break;
        case 0x3D: set_A(DEC(get_A())); PC++; cycles = 4; break;
        case 0x3E: set_A(n8); PC += 2; cycles = 8; break;
        case 0x3F: set_CF(~get_CF()); set_NF(false); set_HF(false); PC++; cycles = 4; break; //CCF
        

        case 0x40: set_B(get_B()); PC++; cycles = 4; break;
        case 0x41: set_B(get_C()); PC++; cycles = 4; break;
        case 0x42: set_B(get_D()); PC++; cycles = 4; break;
        case 0x43: set_B(get_E()); PC++; cycles = 4; break;
        case 0x44: set_B(get_H()); PC++; cycles = 4; break;
        case 0x45: set_B(get_L()); PC++; cycles = 4; break;
        case 0x46: set_B(rd(HL)); PC++; cycles = 8; break;
        case 0x47: set_B(get_A()); PC++; cycles = 4; break;
        case 0x48: set_C(get_B()); PC++; cycles = 4; break;
        case 0x49: set_C(get_C()); PC++; cycles = 4; break;
        case 0x4A: set_C(get_D()); PC++; cycles = 4; break;
        case 0x4B: set_C(get_E()); PC++; cycles = 4; break;
        case 0x4C: set_C(get_H()); PC++; cycles = 4; break;
        case 0x4D: set_C(get_L()); PC++; cycles = 4; break;
        case 0x4E: set_C(rd(HL)); PC++; cycles = 8; break;
        case 0x4F: set_C(get_A()); PC++; cycles = 4; break;


        case 0x50: set_D(get_B()); PC++; cycles = 4; break;
        case 0x51: set_D(get_C()); PC++; cycles = 4; break;
        case 0x52: set_D(get_D()); PC++; cycles = 4; break;
        case 0x53: set_D(get_E()); PC++; cycles = 4; break;
        case 0x54: set_D(get_H()); PC++; cycles = 4; break;
        case 0x55: set_D(get_L()); PC++; cycles = 4; break;
        case 0x56: set_D(rd(HL)); PC++; cycles = 8; break;
        case 0x57: set_D(get_A()); PC++; cycles = 4; break;
        case 0x58: set_E(get_B()); PC++; cycles = 4; break;
        case 0x59: set_E(get_C()); PC++; cycles = 4; break;
        case 0x5A: set_E(get_D()); PC++; cycles = 4; break;
        case 0x5B: set_E(get_E()); PC++; cycles = 4; break;
        case 0x5C: set_E(get_H()); PC++; cycles = 4; break;
        case 0x5D: set_E(get_L()); PC++; cycles = 4; break;
        case 0x5E: set_E(rd(HL)); PC++; cycles = 8; break;
        case 0x5F: set_E(get_A()); PC++; cycles = 4; break;


        case 0x60: set_H(get_B()); PC++; cycles = 4; break;
        case 0x61: set_H(get_C()); PC++; cycles = 4; break;
        case 0x62: set_H(get_D()); PC++; cycles = 4; break;
        case 0x63: set_H(get_E()); PC++; cycles = 4; break;
        case 0x64: set_H(get_H()); PC++; cycles = 4; break;
        case 0x65: set_H(get_L()); PC++; cycles = 4; break;
        case 0x66: set_H(rd(HL)); PC++; cycles = 8; break;
        case 0x67: set_H(get_A()); PC++; cycles = 4; break;
        case 0x68: set_L(get_B()); PC++; cycles = 4; break;
        case 0x69: set_L(get_C()); PC++; cycles = 4; break;
        case 0x6A: set_L(get_D()); PC++; cycles = 4; break;
        case 0x6B: set_L(get_E()); PC++; cycles = 4; break;
        case 0x6C: set_L(get_H()); PC++; cycles = 4; break;
        case 0x6D: set_L(get_L()); PC++; cycles = 4; break;
        case 0x6E: set_L(rd(HL)); PC++; cycles = 8; break;
        case 0x6F: set_L(get_A()); PC++; cycles = 4; break;


        case 0x70: ld(get_B(), HL); PC++; cycles = 8; break;
        case 0x71: ld(get_C(), HL); PC++; cycles = 8; break;
        case 0x72: ld(get_D(), HL); PC++; cycles = 8; break;
        case 0x73: ld(get_E(), HL); PC++; cycles = 8; break;
        case 0x74: ld(get_H(), HL); PC++; cycles = 8; break;
        case 0x75: ld(get_L(), HL); PC++; cycles = 8; break;
        case 0x76: halted = true; PC++; cycles = 4; break;
        case 0x77: ld(get_A(), HL); PC++; cycles = 8; break;
        case 0x78: set_A(get_B()); PC++; cycles = 4; break;
        case 0x79: set_A(get_C()); PC++; cycles = 4; break;
        case 0x7A: set_A(get_D()); PC++; cycles = 4; break;
        case 0x7B: set_A(get_E()); PC++; cycles = 4; break;
        case 0x7C: set_A(get_H()); PC++; cycles = 4; break;
        case 0x7D: set_A(get_L()); PC++; cycles = 4; break;
        case 0x7E: set_A(rd(HL)); PC++; cycles = 8; break;
        case 0x7F: set_A(get_A()); PC++; cycles = 4; break;


        case 0x80: ADD8(get_B()); PC++; cycles = 4; break;
        case 0x81: ADD8(get_C()); PC++; cycles = 4; break;
        case 0x82: ADD8(get_D()); PC++; cycles = 4; break;
        case 0x83: ADD8(get_E()); PC++; cycles = 4; break;
        case 0x84: ADD8(get_H()); PC++; cycles = 4; break;
        case 0x85: ADD8(get_L()); PC++; cycles = 4; break;
        case 0x86: ADD8(rd(HL)); PC++; cycles = 8; break;
        case 0x87: ADD8(get_A()); PC++; cycles = 4; break;
        case 0x88: ADC(get_B()); PC++; cycles = 4; break;
        case 0x89: ADC(get_C()); PC++; cycles = 4; break;
        case 0x8A: ADC(get_D()); PC++; cycles = 4; break;
        case 0x8B: ADC(get_E()); PC++; cycles = 4; break;
        case 0x8C: ADC(get_H()); PC++; cycles = 4; break;
        case 0x8D: ADC(get_L()); PC++; cycles = 4; break;
        case 0x8E: ADC(rd(HL)); PC++; cycles = 8; break;
        case 0x8F: ADC(get_A()); PC++; cycles = 4; break;


        case 0x90: SUB(get_B()); PC++; cycles = 4; break;
        case 0x91: SUB(get_C()); PC++; cycles = 4; break;
        case 0x92: SUB(get_D()); PC++; cycles = 4; break;
        case 0x93: SUB(get_E()); PC++; cycles = 4; break;
        case 0x94: SUB(get_H()); PC++; cycles = 4; break;
        case 0x95: SUB(get_L()); PC++; cycles = 4; break;
        case 0x96: SUB(rd(HL)); PC++; cycles = 8; break;
        case 0x97: set_A(0); set_ZF(true); set_NF(true); set_HF(false); set_CF(false); PC++; cycles = 4; break;
        case 0x98: SBC(get_B()); PC++; cycles = 4; break;
        case 0x99: SBC(get_C()); PC++; cycles = 4; break;
        case 0x9A: SBC(get_D()); PC++; cycles = 4; break;
        case 0x9B: SBC(get_E()); PC++; cycles = 4; break;
        case 0x9C: SBC(get_H()); PC++; cycles = 4; break;
        case 0x9D: SBC(get_L()); PC++; cycles = 4; break;
        case 0x9E: SBC(rd(HL)); PC++; cycles = 8; break;
        case 0x9F: SBC(get_A()); PC++; cycles = 4; break;


        case 0xA0: set_A(AND(get_A(), get_B())); PC++; cycles = 4; break;
        case 0xA1: set_A(AND(get_A(), get_C())); PC++; cycles = 4; break;
        case 0xA2: set_A(AND(get_A(), get_D())); PC++; cycles = 4; break;
        case 0xA3: set_A(AND(get_A(), get_E())); PC++; cycles = 4; break;
        case 0xA4: set_A(AND(get_A(), get_H())); PC++; cycles = 4; break;
        case 0xA5: set_A(AND(get_A(), get_L())); PC++; cycles = 4; break;
        case 0xA6: set_A(AND(get_A(), rd(HL))); PC++; cycles = 8; break;
        case 0xA7: set_A(AND(get_A(), get_L())); PC++; cycles = 4; break;
        case 0xA8: set_A(XOR(get_A(), get_B())); PC++; cycles = 4; break;
        case 0xA9: set_A(XOR(get_A(), get_C())); PC++; cycles = 4; break;
        case 0xAA: set_A(XOR(get_A(), get_D())); PC++; cycles = 4; break;
        case 0xAB: set_A(XOR(get_A(), get_E())); PC++; cycles = 4; break;
        case 0xAC: set_A(XOR(get_A(), get_H())); PC++; cycles = 4; break;
        case 0xAD: set_A(XOR(get_A(), get_L())); PC++; cycles = 4; break;
        case 0xAE: set_A(XOR(get_A(), rd(HL))); PC++; cycles = 8; break;
        case 0xAF: set_A(0); set_ZF(true); set_NF(false); set_HF(false); set_CF(false); PC++; cycles = 4; break;


        case 0xB0: set_A(OR(get_A(), get_B())); PC++; cycles = 4; break;
        case 0xB1: set_A(OR(get_A(), get_C())); PC++; cycles = 4; break;
        case 0xB2: set_A(OR(get_A(), get_D())); PC++; cycles = 4; break;
        case 0xB3: set_A(OR(get_A(), get_E())); PC++; cycles = 4; break;
        case 0xB4: set_A(OR(get_A(), get_H())); PC++; cycles = 4; break;
        case 0xB5: set_A(OR(get_A(), get_L())); PC++; cycles = 4; break;
        case 0xB6: set_A(OR(get_A(), rd(HL))); PC++; cycles = 8; break;
        case 0xB7: set_A(OR(get_A(), get_L())); PC++; cycles = 4; break;
        case 0xB8: CP(get_A(), get_B()); PC++; cycles = 4; break;
        case 0xB9: CP(get_A(), get_C()); PC++; cycles = 4; break;
        case 0xBA: CP(get_A(), get_D()); PC++; cycles = 4; break;
        case 0xBB: CP(get_A(), get_E()); PC++; cycles = 4; break;
        case 0xBC: CP(get_A(), get_H()); PC++; cycles = 4; break;
        case 0xBD: CP(get_A(), get_L()); PC++; cycles = 4; break;
        case 0xBE: CP(get_A(), rd(HL)); PC++; cycles = 8; break;
        case 0xBF: set_ZF(true); set_NF(true); set_HF(false); set_CF(false); PC++; cycles = 4; break;


        case 0xC0: if(get_ZF() == 0) {POP(PC); cycles = 20;} else {PC++; cycles = 8;}; break;
        case 0xC1: POP(BC); PC++; cycles = 12; break;
        case 0xC2: if(get_ZF() == 0) {PC = n16; cycles = 16;} else {PC += 3; cycles = 12;}; break;
        case 0xC3: PC = n16; cycles = 16; break;
        case 0xC4: if(get_ZF() == 0) {PUSH(PC + 3); PC = n16; cycles = 24;} else {PC += 3; cycles = 12;}; break;
        case 0xC5: PUSH(BC); PC++; cycles = 16; break;
        case 0xC6: ADD8(n8); PC += 2; cycles = 8; break;
        case 0xC7: PUSH(PC + 1); PC = 0x00; cycles = 16; break; // RST $00
        case 0xC8: if(get_ZF() == 1) {POP(PC); cycles = 20;} else {PC++; cycles = 8;}; break;
        case 0xC9: POP(PC); cycles = 16; break; //return from subroutine
        case 0xCA: if(get_ZF() == 1) {PC = n16; cycles = 16;} else {PC += 3; cycles = 12;}; break;
        case 0xCB: PREFIXED(opcode); PC += 2; break;
        case 0xCC: if(get_ZF() == 1) {PUSH(PC + 3); PC = n16; cycles = 24;} else {PC += 3; cycles = 12;}; break;
        case 0xCD: PUSH(PC + 3); PC = n16; cycles = 24; break;  
        case 0xCE: ADC(n8); PC += 2; cycles = 8; break;
        case 0xCF: PUSH(PC + 1); PC = 0x08; cycles = 16; break; // RST $08


        case 0xD0: if(get_ZF() == 0) {POP(PC); cycles = 20;} else {PC++; cycles = 8;}; break;
        case 0xD1: POP(DE); PC++; cycles = 12; break;
        case 0xD2: if(get_CF() == 0) {PC = n16; cycles = 16;} else {PC += 3; cycles = 12;}; break;
        //ILLEGAL OPCODE 0xD3
        case 0xD4: if(get_CF() == 0) {PUSH(PC + 3); PC = n16; cycles = 24;} else {PC += 3; cycles = 12;}; break;
        case 0xD5: PUSH(DE); PC++; cycles = 16; break;
        case 0xD6: SUB(n8); PC += 2; cycles = 8; break;
        case 0xD7: PUSH(PC + 1); PC = 0x10; cycles = 16; break; // RST $10
        case 0xD8: if(get_CF() == 1) {POP(PC); cycles = 20;} else {PC++; cycles = 8;}; break;
        case 0xD9: POP(PC); cycles = 16; IME = true; break;
        case 0xDA: if(get_CF() == 1) {PC = n16; cycles = 16;} else {PC += 3; cycles = 12;}; break;
        //ILLEGAL OPCODE 0xDB
        case 0xDC: if(get_CF() == 1) {PUSH(PC + 3); PC = n16; cycles = 24;} else {PC += 3; cycles = 12;}; break;
        //ILLEGAL OPCODE 0xDE
        case 0xDE: SBC(n8); PC += 2; cycles = 8; break;
        case 0xDF: PUSH(PC + 1); PC = 0x18; cycles = 16; break; // RST $18


        case 0xE0: ld(get_A(), a8); PC += 2; cycles = 12; break;
        case 0xE1: POP(HL); PC++; cycles = 12; break;
        case 0xE2: ld(get_A(), 0xFF00 + get_C()); PC++; cycles = 8; break;
        //ILLEGAL OPCODE 0xE3
        //ILLEGAL OPCODE 0xE4
        case 0xE5: PUSH(HL); PC++; cycles = 16; break;
        case 0xE6: set_A(AND(get_A(), n8)); PC += 2; cycles = 8; break;
        case 0xE7: PUSH(PC + 1); PC = 0x20; cycles = 16; break; // RST $20
        case 0xE8: SP = SPADD(n8); PC += 2; cycles = 12; break;
        case 0xE9: PC = HL; cycles = 4; break;
        case 0xEA: ld(get_A(), n16); PC += 3; cycles = 16; break;
        //ILLEGAL OPCODE 0xEB
        //ILLEGAL OPCODE 0xEC
        //ILLEGAL OPCODE 0xED
        case 0xEE: set_A(XOR(get_A(), n8)); PC += 2; cycles = 8; break;
        case 0xEF: PUSH(PC + 1); PC = 0x28; cycles = 16; break; // RST $28


        case 0xF0: set_A(rd(a8)); PC += 2; cycles = 12; break;
        case 0xF1: POP_AF(); PC++; cycles = 16; break;
        case 0xF2: set_A(rd(get_C())); PC++; cycles = 8; break;
        case 0xF3: disable_pending = true; PC++; cycles = 4; break;
        //ILLEGAL OPCODE 0xF4
        case 0xF5: PUSH_AF(); PC++; cycles = 16; break;
        case 0xF6: set_A(OR(get_A(), n8)); PC += 2; cycles = 8; break;
        case 0xF7: PUSH(PC + 1); PC = 0x30; cycles = 16; break; // RST $30
        case 0xF8: HL = SPADD(n8); PC += 2; cycles = 12; break;
        case 0xF9: SP = HL; PC++; cycles = 8; break;
        case 0xFA: set_A(rd(n16)); PC += 3; cycles = 16; break;
        case 0xFB: enable_pending = true; PC++; cycles = 4; break;
        //ILLEGAL OPCODE 0xFC
        //ILLEGAL OPCODE 0xFD
        case 0xFE: CP(get_A(), n8); PC += 2; cycles = 8; break;
        case 0xFF:
            PUSH(PC + 1);
            PC = 0x38;
            cycles = 16;

            std::cout << "RST $38 hit! PC=" << std::hex << PC 
              << " SP=" << SP << " A=" << +get_A() 
              << " F=" << +get_F() << " BC=" << BC 
              << " DE=" << DE << " HL=" << HL << " OPCODE = " << +opcode << " LCDC = " << +rd(0xFF40) << "\n";
              exit( 1 );
            break; // RST $38

        default:
            std::cout << "UNKNOWN OPCODE: " << std::hex << +opcode << "\n";
            PC++;
    }

    if (IME) {

        uint8_t IE = rd(0xFFFF);
        uint8_t IF = rd(0xFF0F);

        uint8_t pending = IE & IF;

        if (pending > 0) {
            for (int i = 0; i <= 4; i++) {
                uint8_t interrupt_bit = (1 << i);

                if(pending & interrupt_bit) {
                    IME = false;

                    ld((IF & ~interrupt_bit), 0xFF0F); 
                    PUSH(PC); 
                    PC = 0x0040 | (i * 0x8);
                    cycles += 20;
                }    
            }
        }
    }


    //execute ppu for N cycles per cpu cycle only if LCD is on!
    if (rd(0xFF40) & 0x80) {
        for (int i = 0; i < cycles; i++) {
            graphics.tick();
        }
    }


    return cycles;
}

void cpu::PREFIXED(uint8_t opcode) {
    switch (rd(PC + 1)) {

        case 0x10: set_B(RL(get_B())); cycles = 12; break;
        case 0x11: set_C(RL(get_C())); cycles = 12; break;
        case 0x12: set_D(RL(get_D())); cycles = 12; break;
        case 0x13: set_E(RL(get_E())); cycles = 12; break;

        case 0x30: set_B(SWAP(get_B())); cycles = 12; break;
        case 0x31: set_C(SWAP(get_C())); cycles = 12; break;
        case 0x32: set_D(SWAP(get_D())); cycles = 12; break;
        case 0x33: set_E(SWAP(get_E())); cycles = 12; break;
        case 0x34: set_H(SWAP(get_H())); cycles = 12; break;
        case 0x35: set_L(SWAP(get_L())); cycles = 12; break;
        case 0x36: ld(SWAP(rd(HL)), HL); cycles = 16; break;
        case 0x37: set_A(SWAP(get_A())); cycles = 12; break;


        case 0x40: BIT(0, get_B()); cycles = 12; break;
        case 0x41: BIT(0, get_C()); cycles = 12; break;
        case 0x42: BIT(0, get_D()); cycles = 12; break;
        case 0x43: BIT(0, get_E()); cycles = 12; break;
        case 0x44: BIT(0, get_H()); cycles = 12; break;
        case 0x45: BIT(0, get_L()); cycles = 12; break;
        case 0x46: BIT(0, rd(HL)); cycles = 16; break;
        case 0x47: BIT(0, get_A()); cycles = 12; break;
        case 0x48: BIT(1, get_B()); cycles = 12; break;
        case 0x49: BIT(1, get_C()); cycles = 12; break;
        case 0x4A: BIT(1, get_D()); cycles = 12; break;
        case 0x4B: BIT(1, get_E()); cycles = 12; break;
        case 0x4C: BIT(1, get_H()); cycles = 12; break;
        case 0x4D: BIT(1, get_L()); cycles = 12; break;
        case 0x4E: BIT(1, rd(HL)); cycles = 16; break;
        case 0x4F: BIT(1, get_A()); cycles = 12; break;


        case 0x50: BIT(2, get_B()); cycles = 12; break;
        case 0x51: BIT(2, get_C()); cycles = 12; break;
        case 0x52: BIT(2, get_D()); cycles = 12; break;
        case 0x53: BIT(2, get_E()); cycles = 12; break;
        case 0x54: BIT(2, get_H()); cycles = 12; break;
        case 0x55: BIT(2, get_L()); cycles = 12; break;
        case 0x56: BIT(2, rd(HL)); cycles = 16; break;
        case 0x57: BIT(2, get_A()); cycles = 12; break;
        case 0x58: BIT(3, get_B()); cycles = 12; break;
        case 0x59: BIT(3, get_C()); cycles = 12; break;
        case 0x5A: BIT(3, get_D()); cycles = 12; break;
        case 0x5B: BIT(3, get_E()); cycles = 12; break;
        case 0x5C: BIT(3, get_H()); cycles = 12; break;
        case 0x5D: BIT(3, get_L()); cycles = 12; break;
        case 0x5E: BIT(3, rd(HL)); cycles = 16; break;
        case 0x5F: BIT(3, get_A()); cycles = 12; break;


        case 0x60: BIT(4, get_B()); cycles = 12; break;
        case 0x61: BIT(4, get_C()); cycles = 12; break;
        case 0x62: BIT(4, get_D()); cycles = 12; break;
        case 0x63: BIT(4, get_E()); cycles = 12; break;
        case 0x64: BIT(4, get_H()); cycles = 12; break;
        case 0x65: BIT(4, get_L()); cycles = 12; break;
        case 0x66: BIT(4, rd(HL)); cycles = 16; break;
        case 0x67: BIT(4, get_A()); cycles = 12; break;
        case 0x68: BIT(5, get_B()); cycles = 12; break;
        case 0x69: BIT(5, get_C()); cycles = 12; break;
        case 0x6A: BIT(5, get_D()); cycles = 12; break;
        case 0x6B: BIT(5, get_E()); cycles = 12; break;
        case 0x6C: BIT(5, get_H()); cycles = 12; break;
        case 0x6D: BIT(5, get_L()); cycles = 12; break;
        case 0x6E: BIT(5, rd(HL)); cycles = 16; break;
        case 0x6F: BIT(5, get_A()); cycles = 12; break;


        case 0x70: BIT(6, get_B()); cycles = 12; break;
        case 0x71: BIT(6, get_C()); cycles = 12; break;
        case 0x72: BIT(6, get_D()); cycles = 12; break;
        case 0x73: BIT(6, get_E()); cycles = 12; break;
        case 0x74: BIT(6, get_H()); cycles = 12; break;
        case 0x75: BIT(6, get_L()); cycles = 12; break;
        case 0x76: BIT(6, rd(HL)); cycles = 16; break;
        case 0x77: BIT(6, get_A()); cycles = 12; break;
        case 0x78: BIT(7, get_B()); cycles = 12; break;
        case 0x79: BIT(7, get_C()); cycles = 12; break;
        case 0x7A: BIT(7, get_D()); cycles = 12; break;
        case 0x7B: BIT(7, get_E()); cycles = 12; break;
        case 0x7C: BIT(7, get_H()); cycles = 12; break;
        case 0x7D: BIT(7, get_L()); cycles = 12; break;
        case 0x7E: BIT(7, rd(HL)); cycles = 16; break;
        case 0x7F: BIT(7, get_A()); cycles = 12; break;


        case 0x80: set_B(RES(0, get_B())); cycles = 12; break;
        case 0x81: set_C(RES(0, get_C())); cycles = 12; break;
        case 0x82: set_D(RES(0, get_D())); cycles = 12; break;
        case 0x83: set_E(RES(0, get_E())); cycles = 12; break;
        case 0x84: set_H(RES(0, get_H())); cycles = 12; break;
        case 0x85: set_L(RES(0, get_L())); cycles = 12; break;
        case 0x86: ld(RES(0, rd(HL)), HL); cycles = 16; break; 
        case 0x87: set_A(RES(0, get_A())); cycles = 12; break;
        case 0x88: set_B(RES(1, get_B())); cycles = 12; break;
        case 0x89: set_C(RES(1, get_C())); cycles = 12; break;
        case 0x8A: set_D(RES(1, get_D())); cycles = 12; break;
        case 0x8B: set_E(RES(1, get_E())); cycles = 12; break;
        case 0x8C: set_H(RES(1, get_H())); cycles = 12; break;
        case 0x8D: set_L(RES(1, get_L())); cycles = 12; break;
        case 0x8E: ld(RES(1, rd(HL)), HL); cycles = 16; break; 
        case 0x8F: set_A(RES(1, get_A())); cycles = 12; break;


        case 0x90: set_B(RES(2, get_B())); cycles = 12; break;
        case 0x91: set_C(RES(2, get_C())); cycles = 12; break;
        case 0x92: set_D(RES(2, get_D())); cycles = 12; break;
        case 0x93: set_E(RES(2, get_E())); cycles = 12; break;
        case 0x94: set_H(RES(2, get_H())); cycles = 12; break;
        case 0x95: set_L(RES(2, get_L())); cycles = 12; break;
        case 0x96: ld(RES(2, rd(HL)), HL); cycles = 16; break; 
        case 0x97: set_A(RES(2, get_A())); cycles = 12; break;
        case 0x98: set_B(RES(3, get_B())); cycles = 12; break;
        case 0x99: set_C(RES(3, get_C())); cycles = 12; break;
        case 0x9A: set_D(RES(3, get_D())); cycles = 12; break;
        case 0x9B: set_E(RES(3, get_E())); cycles = 12; break;
        case 0x9C: set_H(RES(3, get_H())); cycles = 12; break;
        case 0x9D: set_L(RES(3, get_L())); cycles = 12; break;
        case 0x9E: ld(RES(3, rd(HL)), HL); cycles = 16; break; 
        case 0x9F: set_A(RES(3, get_A())); cycles = 12; break;


        case 0xA0: set_B(RES(4, get_B())); cycles = 12; break;
        case 0xA1: set_C(RES(4, get_C())); cycles = 12; break;
        case 0xA2: set_D(RES(4, get_D())); cycles = 12; break;
        case 0xA3: set_E(RES(4, get_E())); cycles = 12; break;
        case 0xA4: set_H(RES(4, get_H())); cycles = 12; break;
        case 0xA5: set_L(RES(4, get_L())); cycles = 12; break;
        case 0xA6: ld(RES(4, rd(HL)), HL); cycles = 16; break; 
        case 0xA7: set_A(RES(4, get_A())); cycles = 12; break;
        case 0xA8: set_B(RES(5, get_B())); cycles = 12; break;
        case 0xA9: set_C(RES(5, get_C())); cycles = 12; break;
        case 0xAA: set_D(RES(5, get_D())); cycles = 12; break;
        case 0xAB: set_E(RES(5, get_E())); cycles = 12; break;
        case 0xAC: set_H(RES(5, get_H())); cycles = 12; break;
        case 0xAD: set_L(RES(5, get_L())); cycles = 12; break;
        case 0xAE: ld(RES(5, rd(HL)), HL); cycles = 16; break; 
        case 0xAF: set_A(RES(5, get_A())); cycles = 12; break;


        case 0xB0: set_B(RES(6, get_B())); cycles = 12; break;
        case 0xB1: set_C(RES(6, get_C())); cycles = 12; break;
        case 0xB2: set_D(RES(6, get_D())); cycles = 12; break;
        case 0xB3: set_E(RES(6, get_E())); cycles = 12; break;
        case 0xB4: set_H(RES(6, get_H())); cycles = 12; break;
        case 0xB5: set_L(RES(6, get_L())); cycles = 12; break;
        case 0xB6: ld(RES(6, rd(HL)), HL); cycles = 16; break; 
        case 0xB7: set_A(RES(6, get_A())); cycles = 12; break;
        case 0xB8: set_B(RES(7, get_B())); cycles = 12; break;
        case 0xB9: set_C(RES(7, get_C())); cycles = 12; break;
        case 0xBA: set_D(RES(7, get_D())); cycles = 12; break;
        case 0xBB: set_E(RES(7, get_E())); cycles = 12; break;
        case 0xBC: set_H(RES(7, get_H())); cycles = 12; break;
        case 0xBD: set_L(RES(7, get_L())); cycles = 12; break;
        case 0xBE: ld(RES(7, rd(HL)), HL); cycles = 16; break; 
        case 0xBF: set_A(RES(7, get_A())); cycles = 12; break;


        case 0xC0: set_B(SET(0, get_B())); cycles = 12; break;
        case 0xC1: set_C(SET(0, get_C())); cycles = 12; break;
        case 0xC2: set_D(SET(0, get_D())); cycles = 12; break;
        case 0xC3: set_E(SET(0, get_E())); cycles = 12; break;
        case 0xC4: set_H(SET(0, get_H())); cycles = 12; break;
        case 0xC5: set_L(SET(0, get_L())); cycles = 12; break;
        case 0xC6: ld(SET(0, rd(HL)), HL); cycles = 16; break; 
        case 0xC7: set_A(SET(0, get_A())); cycles = 12; break;
        case 0xC8: set_B(SET(1, get_B())); cycles = 12; break;
        case 0xC9: set_C(SET(1, get_C())); cycles = 12; break;
        case 0xCA: set_D(SET(1, get_D())); cycles = 12; break;
        case 0xCB: set_E(SET(1, get_E())); cycles = 12; break;
        case 0xCC: set_H(SET(1, get_H())); cycles = 12; break;
        case 0xCD: set_L(SET(1, get_L())); cycles = 12; break;
        case 0xCE: ld(SET(1, rd(HL)), HL); cycles = 16; break; 
        case 0xCF: set_A(SET(1, get_A())); cycles = 12; break;


        case 0xD0: set_B(SET(2, get_B())); cycles = 12; break;
        case 0xD1: set_C(SET(2, get_C())); cycles = 12; break;
        case 0xD2: set_D(SET(2, get_D())); cycles = 12; break;
        case 0xD3: set_E(SET(2, get_E())); cycles = 12; break;
        case 0xD4: set_H(SET(2, get_H())); cycles = 12; break;
        case 0xD5: set_L(SET(2, get_L())); cycles = 12; break;
        case 0xD6: ld(SET(2, rd(HL)), HL); cycles = 16; break; 
        case 0xD7: set_A(SET(2, get_A())); cycles = 12; break;
        case 0xD8: set_B(SET(3, get_B())); cycles = 12; break;
        case 0xD9: set_C(SET(3, get_C())); cycles = 12; break;
        case 0xDA: set_D(SET(3, get_D())); cycles = 12; break;
        case 0xDB: set_E(SET(3, get_E())); cycles = 12; break;
        case 0xDC: set_H(SET(3, get_H())); cycles = 12; break;
        case 0xDD: set_L(SET(3, get_L())); cycles = 12; break;
        case 0xDE: ld(SET(3, rd(HL)), HL); cycles = 16; break; 
        case 0xDF: set_A(SET(3, get_A())); cycles = 12; break;


        case 0xE0: set_B(SET(4, get_B())); cycles = 12; break;
        case 0xE1: set_C(SET(4, get_C())); cycles = 12; break;
        case 0xE2: set_D(SET(4, get_D())); cycles = 12; break;
        case 0xE3: set_E(SET(4, get_E())); cycles = 12; break;
        case 0xE4: set_H(SET(4, get_H())); cycles = 12; break;
        case 0xE5: set_L(SET(4, get_L())); cycles = 12; break;
        case 0xE6: ld(SET(4, rd(HL)), HL); cycles = 16; break; 
        case 0xE7: set_A(SET(4, get_A())); cycles = 12; break;
        case 0xE8: set_B(SET(5, get_B())); cycles = 12; break;
        case 0xE9: set_C(SET(5, get_C())); cycles = 12; break;
        case 0xEA: set_D(SET(5, get_D())); cycles = 12; break;
        case 0xEB: set_E(SET(5, get_E())); cycles = 12; break;
        case 0xEC: set_H(SET(5, get_H())); cycles = 12; break;
        case 0xED: set_L(SET(5, get_L())); cycles = 12; break;
        case 0xEE: ld(SET(5, rd(HL)), HL); cycles = 16; break; 
        case 0xEF: set_A(SET(5, get_A())); cycles = 12; break;


        case 0xF0: set_B(SET(6, get_B())); cycles = 12; break;
        case 0xF1: set_C(SET(6, get_C())); cycles = 12; break;
        case 0xF2: set_D(SET(6, get_D())); cycles = 12; break;
        case 0xF3: set_E(SET(6, get_E())); cycles = 12; break;
        case 0xF4: set_H(SET(6, get_H())); cycles = 12; break;
        case 0xF5: set_L(SET(6, get_L())); cycles = 12; break;
        case 0xF6: ld(SET(6, rd(HL)), HL); cycles = 16; break; 
        case 0xF7: set_A(SET(6, get_A())); cycles = 12; break;
        case 0xF8: set_B(SET(7, get_B())); cycles = 12; break;
        case 0xF9: set_C(SET(7, get_C())); cycles = 12; break;
        case 0xFA: set_D(SET(7, get_D())); cycles = 12; break;
        case 0xFB: set_E(SET(7, get_E())); cycles = 12; break;
        case 0xFC: set_H(SET(7, get_H())); cycles = 12; break;
        case 0xFD: set_L(SET(7, get_L())); cycles = 12; break;
        case 0xFE: ld(SET(7, rd(HL)), HL); cycles = 16; break; 
        case 0xFF: set_A(SET(7, get_A())); cycles = 12; break;


        default:
            std::cout << "UNKNOWN PREFIXED OPCODE!! " << +rd(PC + 1) << "\n";
            break;
    }
}

uint8_t cpu::AND(uint8_t a, uint8_t b) {
    uint8_t result = a & b;
    set_ZF(result == 0);
    set_CF(false);
    set_NF(true);
    set_HF(false);

    return result;
}

uint8_t cpu::OR(uint8_t a, uint8_t b) {
    uint8_t result = a | b;
    set_ZF(result == 0);
    set_CF(false);
    set_NF(false);
    set_HF(false);

    return result;
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

void cpu::PUSH_AF() {
    SP--;
    ld(get_A(), SP);
    SP--;
    ld((get_ZF() << 7) | (get_NF() << 6) | (get_HF() << 5) | (get_CF() << 4), SP);
}

void cpu::POP_AF() {
    uint8_t lowByte = rd(SP);

    set_ZF((lowByte >> 7) & 0x1);
    set_NF((lowByte >> 6) & 0x1);
    set_HF((lowByte >> 5) & 0x1);
    set_CF((lowByte >> 4) & 0x1);
    SP++;
    set_A(rd(SP));
    SP++;

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
    uint8_t carryFlag = (byte >> 7) & 0x1;
    uint8_t resultByte = (byte << 1) | carryBit;
    
    set_ZF(resultByte == 0);
    set_NF(false);
    set_HF(false);
    set_CF(carryFlag != 0);
    
    return resultByte;
}

uint8_t cpu::RR(uint8_t byte) {
    uint8_t carryBit = (get_F() & cf) << 3;
    uint8_t carryFlag = (byte << 4) & cf;
    uint8_t resultByte = (byte >> 1) | carryBit;
    
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

uint8_t cpu::RRC(uint8_t byte) {
    uint8_t carryBit = byte << 7;
    uint8_t carryFlag = (byte << 4) & cf;
    uint8_t resultByte = (byte >> 1) | carryBit;
    
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
    set_HF((byte & 0x0F) == 0x00); 
    set_NF(true);

    return byte - 1;
}

void cpu::CP(uint8_t a, uint8_t b) {
    uint8_t result = a - b;
    
    set_ZF(result == 0);
    set_NF(true);
    set_HF((a & 0x0F) < (b & 0x0F));
    set_CF(b > a);
}

void cpu::ADD8(uint8_t byte) {
    uint8_t result = get_A() + byte;
    set_A(result);

    set_ZF(result == 0);
    set_NF(false);
    set_HF(((get_A() & 0x0F) + (byte & 0x0F)) > 0x0F);  // Half-carry
    set_CF(result > 0xFF);    
}

void cpu::ADC(uint8_t byte) {

    uint16_t result16 = get_A() + byte + get_CF();
    uint8_t result = static_cast<uint8_t>(result16);
    set_A(result);

    set_ZF(result == 0);
    set_NF(false);
    set_HF(((get_A() & 0x0F) + (byte & 0x0F) + get_CF()) > 0x0F);
    set_CF(result16 > 0xFF);

}

void cpu::SBC(uint8_t byte) {

    uint8_t subtrahend = byte + get_CF();
    uint8_t result = get_A() - subtrahend;
    set_A(result);

    set_ZF(result == 0);
    set_NF(true);
    set_HF((get_A() & 0x0F) < (subtrahend & 0x0F));
    set_CF((byte + get_CF()) > get_A());
}

uint16_t cpu::SPADD(uint8_t byte) {

    uint8_t sp_low = SP & 0xFF;
    int32_t result = SP + static_cast<int8_t>(byte);
    uint16_t low_result = sp_low + byte;

    

    set_ZF(false);
    set_NF(false);
    set_HF((sp_low & 0x0F) + (byte & 0x0F) > 0x0F);
    set_CF(low_result > 0xFF);

    return static_cast<uint16_t>(result);
}
uint32_t cpu::ADD16(uint16_t a, uint16_t b) {
    uint32_t result = a + b;

    //ZERO FLAG IGNORED
    set_NF(false);
    set_HF(((a & 0x0FFF) + (b & 0x0FFF)) > 0x0FFF);
    set_CF(result > 0xFFFF); 
    
    return result & 0xFFFF;
}

void cpu::SUB(uint8_t byte) {
    uint8_t result = get_A() - byte;
    set_A(result);

    set_ZF(result == 0);
    set_NF(true);
    set_HF((get_A() & 0x0F) < (byte & 0x0F));
    set_CF(byte > get_A());
}

uint8_t cpu::SWAP(uint8_t reg) {

    uint8_t temp;
    uint8_t result;

    temp = (reg & 0x0F) << 4;
    result = ((reg & 0xF0) >> 4) | temp;


    set_ZF(result == 0);
    set_NF(false);
    set_HF(false);
    set_CF(false);

    return result;
}

uint8_t cpu::RES(uint8_t bit, uint8_t reg) {

    switch (bit) {
        case 0:

            return reg & 0b11111110;

            break;
        case 1:

            return reg & 0b11111101;

            break;
        case 2:

            return reg & 0b11111011;

            break;
        case 3:

            return reg & 0b11110111;

            break;
        case 4:

            return reg & 0b11101111;

            break;
        case 5:

            return reg & 0b11011111;

            break;
        case 6:

            return reg & 0b10111111;

            break;
        case 7:

            return reg & 0b01111111;

            break;
        default:
            std::cout << "INVALID BIT \n";
            return 0;
            break;  

    }
}

uint8_t cpu::SET(uint8_t bit, uint8_t reg) {

    switch (bit) {
        case 0:

            return reg | 0b00000001;

            break;
        case 1:

            return reg | 0b00000010;

            break;
        case 2:

            return reg | 0b00000100;

            break;
        case 3:

            return reg | 0b00001000;

            break;
        case 4:

            return reg | 0b00010000;

            break;
        case 5:

            return reg | 0b00100000;

            break;
        case 6:

            return reg | 0b01000000;

            break;
        case 7:

            return reg | 0b100000000;

            break;
        default:
            std::cout << "INVALID BIT \n";
            return 0;
            break;  

    }
}