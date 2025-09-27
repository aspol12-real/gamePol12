#pragma once

#include <string>
#include <cstdint>
#include "mmu.hpp"
#include "ppu.hpp"

class cpu {
    public: 
        //memory
        mmu mem;
        ppu graphics;

        //registers
        uint16_t registers[4]; //AF, BC, DE, HL
        uint16_t PC;
        uint16_t SP;

        //methods
        void initialize(std::string rom);
        void ld(uint8_t data, uint16_t address) {
            if (address >= 0x8000 && address <= 0x9FFF) {
                graphics.VRAM[address] = data;
            } else {
                mem.ld(data, address);
            }
        }

        //flags


};