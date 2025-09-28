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


    uint16_t AF = (A << 8) | F;
    uint16_t BC = (B << 8) | C;
    uint16_t DE = (D << 8) | E;
    uint16_t HL = (H << 8) | L;

    //decode

    switch(opcode) {

        //misc instructions
        case 0x0:
            //NOP
            PC++;
            break;

        //8-bit ld instructions
        case 0x02: // a --> [BC]

            ld(A, BC); 
            PC++;

            break;
        case 0x06: // n8 --> B

            B = mem.rd(PC + 1);
            PC += 2;

            break;
        case 0x0a: // [BC] --> a

            A = mem.rd(BC);
            PC++;

            break;
        case 0x0E: // n8 --> C

            C = mem.rd(PC + 1);
            PC += 2;

            break;
        case 0x12: // a --> [DE]

            ld(A, DE); 
            PC++;

            break;
        case 0x16: // n8 --> D

            D = mem.rd(PC + 1);
            PC += 2;

            break;
        case 0x1a: // [DE] --> a

            A = mem.rd(DE);
            PC++;

            break;
        case 0x1E: // n8 --> E

            E = mem.rd(PC + 1);
            PC += 2;

            break;
        case 0x22: // a --> [HL+]

            ld(A, HL);
            L++;
            if (L == 0) {  
                H++;
            }
            PC++;

            break;
        case 0x26: // n8 --> H

            H = mem.rd(PC + 1);
            PC += 2;

            break;
        case 0x2a: // [HL+] --> a

            A = mem.rd(HL);
            L++;
            if (L == 0) {  
                H++;
            }
            PC++;

            break;
    }

    std::cout << mem.rd(PC) << "\n";
    //execute
}




