#include "mmu.hpp"

#include <iostream>

extern uint8_t g_polled_actions;
extern uint8_t g_polled_directions;

void mmu::ld(uint8_t data, uint16_t address) {
    if (address <= 0xFF) {
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
    else if (address >= 0xE000 && address <= 0xFDFF) { //echo ram
        uint16_t mirrored_address = address - 0x2000; 

        if (mirrored_address >= 0xC000 && mirrored_address <= 0xCFFF) { 
            WRAM_1[mirrored_address - 0xC000] = data;
        } else if (mirrored_address >= 0xD000 && mirrored_address <= 0xDFFF) { 
            WRAM_2[mirrored_address - 0xD000] = data;
        } 
    }
    else if (address == 0xFF00) {
        //only write to high nibble

        //std::cout << "I WANT TO WRITE " << std::hex << +data << "\n";
        uint8_t temp = IO[0] & 0x0F;
        data |= temp;
        IO[0] = data | 0xC0;

    }
    else if (address == 0xFF01) {
        IO[1] = data;
    }
    else if (address == 0xFF02) {
        IO[2] = data;

        if (data & 0x80) {

            std::cout << (char)IO[1]; 

            IO[2] &= ~0x80; 
            IO[0x0F] |= 0x08;
        }
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
        interrupts = data;
        // std::cout << "INTERRUPT REGISTER POKED WITH: " << std::hex << +data << "\n";
    }
    else {
        std::cout << "BAD POKE . ADDRESS: " << std::hex << +address << "\n";
        return;
    }
}

uint8_t  mmu::rd(uint16_t address) {
    if (address <= 0xFF && bootRomEnabled) { //BOOTROM
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

        uint16_t mirrored_address = address - 0x2000; 

        if (mirrored_address >= 0xC000 && mirrored_address <= 0xCFFF) { 
            dataRet = WRAM_1[mirrored_address - 0xC000];
        } else if (mirrored_address >= 0xD000 && mirrored_address <= 0xDFFF) { 
            dataRet = WRAM_2[mirrored_address - 0xD000];
        } 
    }
    else if (address == 0xFF00) { //INPUT READ

        uint8_t state = IO[0];
        uint8_t output = state | 0xF;

        // std::cout << "Polling Request (IO[0]): " << std::hex << +state << "\n";

        if (!(state & 0x10)) { 
            output &= (g_polled_actions | 0xF0); 
            //std::cout << +g_polled_actions << "\n";
        }
        if (!(state & 0x20)) { 
            output &= (g_polled_directions | 0xF0); 
            //std::cout << +g_polled_directions << "\n";
        }

        // std::cout << "JOYP Result: " << std::hex << +dataRet << "\n";

        dataRet = output | 0xC0;
    } 
    else if (address >= 0xFF00 && address <= 0xFF7F) { //I/O registers
        dataRet = IO[address - 0xFF00];
    }
    else if (address >= 0xFF80 && address <= 0xFFFE) { //HRAM
        dataRet = HRAM[address - 0xFF80];
    }
    else if (address == 0xFFFF) {
        dataRet = interrupts;
        //std::cout << "INTERRUPT REGISTER PEEKED! \n";
    }
    else {
        std::cout << "BAD PEEK . ADDRESS: " << std::hex << +address << "\n";
    }
    return dataRet;
}


