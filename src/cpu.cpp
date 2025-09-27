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


    PC = 0x0;
    SP = 0x0;


}






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