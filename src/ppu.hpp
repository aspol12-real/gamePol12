#pragma once

#include <cstdint>
class mmu;

class ppu {
public:
    mmu* mem;
    ppu(mmu* mmu_ptr) : mem(mmu_ptr) {}

    enum state {
        OAMSearch = 0,
        PixelTransfer = 1,
        HBlank = 2,
        VBlank = 3,
    };

    enum state ppuState;

    uint8_t VRAM[8192];
    uint8_t OAM[159];

    int clocks = 0;
    uint8_t LY = 0;
    uint8_t x  = 0;

    void tick();

};