#pragma once

#include <cstdint>


class cartridge {
    public:
        uint8_t romBank0[16384];
        uint8_t romBank1[16384];
        uint8_t ERAM[8192];
};

class mmu {
    public:

        cartridge cart;
        //WRAM 1 & 2
        uint8_t WRAM_1[4096];
        uint8_t WRAM_2[4096];   

        //methods
        void ld(uint8_t data, uint16_t address);
        void rd(uint8_t data, uint16_t address);
        
};
