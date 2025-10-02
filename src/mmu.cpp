#include "mmu.hpp"

#include <algorithm>
#include <iostream>

void mmu::setBootRom(uint8_t* rom, size_t size) {
    std::copy(rom, rom + size, bootRom);
}

void mmu::ld(uint8_t data, uint16_t address) {
    if (startup && address <= 0xFF) {
        return; //bootrom is read only
    }
    else if (address >= 0 && address <= 0x3FFF) { //romBank 0
        return; //never write
    }
    else if (address >= 0x4000 && address <= 0x7FFF) { //romBank 1-NN (depending on mapper)
        return; //never write
    }
    else if (address >= 0xA000 && address <= 0xBFFF) { //External Cartridge Ram
        cart.ERAM[address - 0xA000] = data;
    }
    else if (address >= 0xC000 && address <= 0xCFFF) { //WRAM 1
        WRAM_1[address - 0xC000] = data;    
    }
    else if (address >= 0xD000 && address <= 0xDFFF) { //WRAM 2
        WRAM_2[address - 0xD000] = data;
    } 
    else if (address == 0xFF50) {
        if (data == 0x01) {
            bootRomEnabled = false; 
        }
    }
    else if (address >= 0xFF00 && address <= 0xFF7F) { //I/O registers
        IO[address - 0xFF00] = data;
    }
    else if (address >= 0xFF80 && address <= 0xFFFE) { //HRAM
        HRAM[address - 0xFF80] = data;
    }
    else if (address == 0xFFFF) {
        std::cout << "INTERRUPT REGISTER POKED! \n";
    }
    else {
        std::cout << "BAD POKE . ADDRESS: " << std::hex << +address << "\n";
        exit( 1 );
        return;
    }
}

uint8_t  mmu::rd(uint16_t address) {
    if (startup && address <= 0xFF && bootRomEnabled) { //BOOTROM
        dataRet = bootRom[address];
    }
    else if (address >= 0 && address <= 0x3FFF) { //ROMBANK 0
        dataRet = cart.romBank0[address];
    }
    else if (address >= 0x4000 && address <= 0x7FFF) { //ROMBANK 1
        dataRet = cart.romBank1[address - 0x4000];
    }
    else if (address >= 0xA000 && address <= 0xBFFF) { //External Cartridge Ram
        dataRet = cart.ERAM[address - 0xA000];
    }
    else if (address >= 0xC000 && address <= 0xCFFF) { //WRAM 1
        dataRet = WRAM_1[address - 0xC000];  
    }
    else if (address >= 0xD000 && address <= 0xDFFF) { //WRAM 2
        dataRet = WRAM_2[address - 0xD000];
    } 
    else if (address >= 0xE000 && address <= 0xFDFF) { //echo ram
        dataRet = WRAM_1[address - 0xE000];  
    }
    else if (address >= 0xFF00 && address <= 0xFF7F) { //I/O registers
        dataRet = IO[address - 0xFF00];
    }
    else if (address >= 0xFF80 && address <= 0xFFFE) { //HRAM
        dataRet = HRAM[address - 0xFF80];
    }
    else {
        std::cout << "BAD PEEK . ADDRESS: " << std::hex << +address << "\n";
        exit( 1 );
    }
    return dataRet;
}


