#include "ppu.hpp"
#include "mmu.hpp"

void ppu::tick() {

    clocks++;

    if (clocks >= 456) {
        clocks = 0;
        LY++;

        if (LY == 144) {
             //v-blank interrupt
        }
        if (LY > 153) {
            LY = 0;
        }
        mem->ld(LY, 0xFF44);
    }
    switch (ppuState) {
        case OAMSearch:
            //collect sprite data

            break;

        case PixelTransfer:
            //push pixel data to display

            break;
        
        case HBlank:

            //wait, the either oamsearch or vblank
            
            break;

        case VBlank:

            //wait then go back to sprite search for top of line
            break;
    }
}