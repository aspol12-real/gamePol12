#pragma once

#include <cstdint>
#include <deque>

class mmu;

class ppu {
    public:
        mmu* mem;
        ppu(mmu* mmu_ptr) : mem(mmu_ptr) {}

        const uint8_t h_blank       = 0b00000000;
        const uint8_t v_blank       = 0b00000001;
        const uint8_t oamsearch     = 0b00000010;
        const uint8_t pixeltransfer = 0b00000011;
        
        const int GB_WIDTH = 160;
        const int GB_HEIGHT = 144;


        uint8_t temp = 0;

        uint8_t x = 0;                 
        std::deque<uint8_t> pixel_fifo;

        bool oamRestrict = false;
        bool vramRestrict = false;
        bool vblank = false;

        uint8_t VRAM[8192];
        uint8_t OAM[159];

        uint8_t spritebuffer[40] = {0};
        uint8_t screenBuffer[144 * 160] = {0}; //144*160 pixels

        //pixel transfer related variables
        uint8_t fetched_low_byte;
        uint8_t fetched_high_byte;
        uint16_t tileNumAddr;
        uint8_t tile_number;
        uint8_t map_col_x;
        uint8_t map_row_y;
        uint16_t tileAddress;

        int clocks = 0;

        uint8_t LY = 0;

        const int FINDABLE_SPRITES = 10;
        int spritesFound = 0;

        void tick();
        void set_ppu_mode(uint8_t mode);
        void addSprite(int i, uint8_t a, uint8_t b, uint8_t c, uint8_t d);
        uint8_t get_ppu_mode();
        void renderPixel();
        uint8_t get_color(uint8_t color_index, uint16_t palette_address);
};