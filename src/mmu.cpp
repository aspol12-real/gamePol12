#include "mmu.hpp"
#include "cpu.hpp"
#include "apu.hpp"

void mmu::ld(uint8_t data, uint16_t address) {
    if (0x0000 <= address <= 0x3FFF) { //romBank 0
        cart.romBank0[address] = data;
    }
    else if (0x4000 <= address <= 0x7FFF) { //romBank 1-NN (depending on mapper)
        cart.romBank1[address] = data;
    }
}

/*
    std::string playerRom = argv[1];
    std::ifstream file;
    file.open(playerRom, std::ios::in | std::ios::binary);

    char byte;
    int index = 0;
    while (file.get(byte) && index < 256) {
         = static_cast<uint8_t>(byte); 
        index++;
    }

    if (!file.is_open()) {
        std::cout << "Could not locate source file! \n";
        exit( 1 );
    } else {
        std::cout << "Loading rom file: " << rom << "\n";
    }

    file.close();

    */