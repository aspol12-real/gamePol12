#pragma once

#include <cstdint>
#include "mmu.hpp"

class cpu {
    public: 
        //memory
        mmu mem;
        uint8_t bootRom[256];

        //registers
        uint16_t registers[4]; //AF, BC, DE, HL
        uint16_t PC;
        uint16_t SP;

        //methods
        void initialize();

};