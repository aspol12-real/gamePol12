#pragma once

#include <iostream>
#include <cstdint>
#include <deque>
#include <array>

class mmu;
const int GB_WIDTH = 160;
const int GB_HEIGHT = 144;

class ppu {
    private:

        mmu& mem;


        std::deque<uint8_t> background_fifo;
        std::array<uint8_t, GB_WIDTH> bg_raw_colors; 
        std::array<uint8_t, GB_WIDTH> final_colors;

    public:

        ppu(mmu& shared_memory) : mem(shared_memory){};

        const uint8_t h_blank       = 0b00000000;
        const uint8_t v_blank       = 0b00000001;
        const uint8_t oamsearch     = 0b00000010;
        const uint8_t pixeltransfer = 0b00000011;


        const int palette_address = 0xFF47;
        
        uint8_t temp = 0;

        uint8_t x = 0;

        bool oamRestrict = false;
        bool vramRestrict = false;
        bool vblank = false;
        bool ly_equals_wy = false;
        bool window_fetch = false;

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
        uint8_t fetcher_tile_x = 0;

        void tick();
        void set_ppu_mode(uint8_t mode);
        void addSprite(int i, uint8_t a, uint8_t b, uint8_t c, uint8_t d);
        uint8_t get_ppu_mode();
        void render_scanline(int LY);
        void fetch_tile_row(int current_pixel_x, int scanline_y);
        uint8_t get_color(uint8_t color_index, uint16_t palette_address);
};