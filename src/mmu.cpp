#include "mmu.hpp"



#include <algorithm>

void mmu::setBootRom(uint8_t* rom, size_t size) {
    std::copy(rom, rom + size, bootRom);
}

void mmu::ld(uint8_t data, uint16_t address) {
    if (startup && address <= 0xFF) {
        return; //bootrom is read only
    }
    else if (address >= 0 && address <= 0x3FFF) { //romBank 0
        cart.romBank0[address] = data;
    }
    else if (address >= 0x4000 && address <= 0x7FFF) { //romBank 1-NN (depending on mapper)
        cart.romBank1[address % 0x3FFF] = data;
    }
    else if (address >= 0xA000 && address <= 0xBFFF) { //External Cartridge Ram
        cart.ERAM[address % 0x1FFF] = data;
    }
    else if (address >= 0xC000 && address <= 0xCFFF) {
        WRAM_1[address % 0xFFF] = data;    
    }
    else if (address >= 0xD000 && address <= 0xDFFF) {
        WRAM_2[address % 0xFFF] = data;
    }
}

uint8_t  mmu::rd(uint16_t address) {
    if (startup && address <= 0xFF) {
        dataRet = bootRom[address];
    }
    else if (address >= 0 && address <= 0x3FFF) {
        dataRet = cart.romBank0[address];
    }
    else if (address >= 0x4000 && address <= 0x7FFF) {
        dataRet = cart.romBank0[address % 0x3FFF];
    }

    return dataRet;
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