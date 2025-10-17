#include "mmu.hpp"

#include <iostream>

extern uint8_t g_polled_actions;
extern uint8_t g_polled_directions;

void mmu::ld(uint8_t data, uint16_t address) {
    if (address <= 0xFF) {
        return; //bootrom is read only
    }
    else if (address >= 0 && address <= 0x1FFF) { //romBank 0
        ERAM_ENABLE = data;
    }
    else if (address >= 0x2000 && address <= 0x3FFF) { //switch rom bank number
        rom_bank_number = data;
    }
    else if (address >= 0x4000 && address <= 0x5FFF) {
        if (data >= 0 && data <= 3) {

        ram_bank_number = data;

        }
    }
    else if (address >= 0xA000 && address <= 0xBFFF) { //External Cartridge Ram

        if (ERAM_ENABLE == 0xA) {
            cart.ERAM[address - 0xA000] = data;
        }
        
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

        uint8_t temp = IO[0] & 0x0F;
        data |= temp;
        IO[0] = data | 0xC0;

    }
    else if (address == 0xFF02) {
        IO[2] = data;

        if (data & 0x80) {

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
        dataRet = cart.romBank[address]; 
    }
    else if (address >= 0x4000 && address <= 0x7FFF) { //ROMBANK 1 (CHANGES DEPENDING ON MAPPER)
        
        uint8_t mapper = cart.romBank[0x147];

        if (mapper != 0) {

            uint8_t rom_bank_number_final = rom_bank_number & 0b00011111;

            dataRet = cart.romBank[(address - 0x4000) + (rom_bank_number_final * 0x4000)];

        } else {

            dataRet = cart.romBank[address];
        }

    }
    else if (address >= 0xA000 && address <= 0xBFFF) { //External Cartridge Ram
        
        uint8_t ram_bank_number_final = ram_bank_number & 0b00000011;

        dataRet = cart.ERAM[(address - 0xA000) + (ram_bank_number_final * 0x2000)]; //0x2000 because each RAM bank is 8kib

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
        uint8_t output = state | 0x0F;

        if (!(state & 0x10)) { 
            output &= g_polled_directions;
        }
        if (!(state & 0x20)) { 
            output &= g_polled_actions; 
        }

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
    }
    else {
        std::cout << "BAD PEEK . ADDRESS: " << std::hex << +address << "\n";
    }
    return dataRet;
}


