#pragma once

#include <cstdint>

class ppu {
public:

    enum state {
        OAMSearch = 0,
        PixelTransfer = 1,
        HBlank = 2,
        VBlank = 3,
    };

    enum state ppuState;

    uint8_t VRAM[8192];
    uint8_t OAM[159];

    void tick();

};