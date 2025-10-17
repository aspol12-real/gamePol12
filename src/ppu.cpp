#include "ppu.hpp"
#include "mmu.hpp"
#include <iostream>

void ppu::tick() {

    clocks++; //increment clocks ONCE per tick

    if (LY < 144) {
        if (clocks >= 0 && clocks <= 79) {  //OAM SEARCH (ONLY OAM CANNOT BE ACCESSED)
            set_ppu_mode(oamsearch);

            spritesFound = 0;
            for (int i = 0; i < 40; i++) {

                uint8_t sprite_Y = OAM[i * 4];
                uint8_t sprite_X = OAM[i * 4 + 1];
                uint8_t tile_number = OAM[i * 4 + 2];
                uint8_t sprite_flags = OAM[i * 4 + 3];

                const int SPRITE_HEIGHT = 8;
                if (((sprite_X + 8) > 0) && ((LY + 16) >= sprite_Y) && ((LY + 16) < (sprite_Y + SPRITE_HEIGHT))) {
                    spritesFound++;
                    if (spritesFound <= FINDABLE_SPRITES) {
                        addSprite(spritesFound - 1, sprite_Y, sprite_X, tile_number, sprite_flags);
                    } else {
                        return;
                    }
                }
            }

            vramRestrict = false;
            oamRestrict = true;
        }
        else if (clocks == 80) { //Pixel Transfer (VRAM & OAM CANNOT BE ACCESSED)
            set_ppu_mode(pixeltransfer);
            oamRestrict = true;
            vramRestrict = true;

        }
        else if (clocks >= 80 && clocks < 252) {
            if (get_ppu_mode() == pixeltransfer && x < 160) {
                renderPixel();
            }
        }
        else if (clocks == 252) { //H-Blank (VRAM AND OAM CAN BE ACCESSED)
            set_ppu_mode(h_blank);
            oamRestrict = false;
            vramRestrict = false;
        }
    }
    if (clocks >= 456) { // NEW SCAN LINE

        clocks = 0;
        x = 0;
        LY++;

        if (LY == 144) { //ENTER VBLANK SCANLINE PERIOD
            uint8_t temp = mem->rd(0xFF0F);
            temp |= 0x1;
            mem->ld(temp, 0xFF0F);

            oamRestrict = false;
            vramRestrict = false;
            set_ppu_mode(v_blank);
        }
        if (LY > 153) { //ENTER OAM SEARCH AFTER LAST VBLANK LINE

            LY = 0;

        } 

        mem->ld(LY, 0xFF44); //updates LY register
    }
}










void ppu::renderPixel() {

    uint8_t low_byte = 0;
    uint8_t high_byte = 0;
    uint8_t tile_num;
    
    uint8_t wnTileMap            = mem->rd(0xFF40) & 0x40; // 01000000
    uint8_t wnEnable             = mem->rd(0xFF40) & 0x20; // 00100000
    uint8_t TilenumOffsetEnable  = mem->rd(0xFF40) & 0x10; // 00010000
    uint8_t bgTileMap            = mem->rd(0xFF40) & 0x08; // 00001000
    uint8_t spriteSize           = mem->rd(0xFF40) & 0x04; // 00000100
    uint8_t objEnable            = mem->rd(0xFF40) & 0x02; // 00000010
    uint8_t bgWinPriority        = mem->rd(0xFF40) & 0x01; // 00000001

    if (x % 8 == 0) { //every 8 pixels

        uint8_t SCY = mem->rd(0xFF42);
        uint8_t SCX = mem->rd(0xFF43);

        uint8_t WX  = mem->rd(0xFF4A);
        uint8_t WY  = mem->rd(0xFF4B);

        uint16_t tile_y = 0;
        uint16_t tile_x = 0;

        uint16_t tile_data_addr = 0;
        
        uint16_t tile_num_offset = 0;

        bool in_window = ((x >= WX - 7) && LY >= WY);

        uint16_t first_map_addr = 0;

        if (wnEnable && in_window) {

            if (wnTileMap != 0) {
                first_map_addr = 0x9C00;
            } else {
                first_map_addr = 0x9800;
            }

            uint16_t wn_x_coord = x - (WX - 7); // x relative to window left edge (WX-7)
            uint16_t wn_y_coord = LY - WY;      // y relative to window top edge (WY)

            tile_x = wn_x_coord / 8; // Index of the tile column (0-31)
            tile_y = (wn_y_coord / 8) * 32;

        } else {

            if (bgTileMap != 0) {
                first_map_addr = 0x9C00;
            } else {
                first_map_addr = 0x9800;
            }

            
            tile_x = (x + SCX) / 8;
            tile_y = ((LY + SCY) / 8) * 32;
        }
        
    

        // calculate VRAM address where tile number is stored for this tile

        uint16_t tile_map_addr = first_map_addr + tile_y + tile_x; 
        uint16_t vram_index = tile_map_addr - 0x8000;

        // fetch tile number in tilemap

        tile_num = VRAM[vram_index];
        

        uint8_t tile_row_in_data = (wnEnable && in_window) ? ((LY - WY) % 8) : ((LY + SCY) % 8);


        // get tile data for the tile number


        if (TilenumOffsetEnable != 0 ) {

            tile_data_addr = 0x8000 + (tile_num * 16) + (tile_row_in_data * 2);

        } else {

            int16_t signed_tile_num = (int8_t)tile_num;
            tile_data_addr = 0x9000 + (signed_tile_num * 16) + (tile_row_in_data * 2);

        }
        
        uint16_t vram_data_index = tile_data_addr - 0x8000;
        
        low_byte = VRAM[vram_data_index];
        high_byte = VRAM[vram_data_index + 1];
        

        //push pixel data for tile to pixel FIFO

        pixel_fifo.clear();
        for (int i = 0; i < 8; i++) {
            int bit_idx = 7 - i;
            uint8_t color = ((high_byte >> bit_idx) & 1) << 1 | ((low_byte >> bit_idx) & 1);
            pixel_fifo.push_back(color);
        }
    

    }

    if (pixel_fifo.empty()) {
        x++;
        return;
    }
    
    uint8_t color_idx = pixel_fifo.front(); //get front pixel

    pixel_fifo.pop_front(); //pop pixel to move fifo
    
    uint8_t bg_color = get_color(color_idx, 0xFF47);

    uint8_t final_color = bg_color; //TODO: change
    

    //for every X value
    if (objEnable != 0) {

        for (int i = 0; i < spritesFound; i++) {
            uint8_t obj_x = spritebuffer[i * 4 + 1];
            uint8_t obj_y = spritebuffer[i * 4];
            
            if ((x >= obj_x - 8) && (x < obj_x)) {
                uint8_t flags = spritebuffer[i * 4 + 3];
                uint8_t tile_num_oam = spritebuffer[i * 4 + 2];

                uint8_t screen_start_x = obj_x - 8;
                uint8_t screen_start_y = obj_y - 16;

                uint8_t sprite_pixel_x = x - screen_start_x;  

                int spriteheight;

                if (spriteSize != 0) {
                    spriteheight = 16;
                } else {
                    spriteheight = 8;
                }


                if ((LY < screen_start_y) || (LY >= screen_start_y + spriteheight)) {
                    continue; 
                }

                uint8_t sprite_pixel_y_abs = LY - screen_start_y;
                uint8_t tile_y_index = sprite_pixel_y_abs;
                uint8_t current_tile_num = tile_num_oam;

                if (spriteheight == 16) {
                    if (flags & 0x40) {
                        if (sprite_pixel_y_abs < 8) {
                            current_tile_num = tile_num_oam | 0x01; 
                            tile_y_index = 7 - sprite_pixel_y_abs;
                        } else { 
                            current_tile_num = tile_num_oam & 0xFE;
                            tile_y_index = 15 - sprite_pixel_y_abs; 
                        }
                    } else { 

                        if (sprite_pixel_y_abs < 8) { 
                            current_tile_num = tile_num_oam & 0xFE;
                            tile_y_index = sprite_pixel_y_abs;
                        } else {
                            current_tile_num = tile_num_oam | 0x01;
                            tile_y_index = sprite_pixel_y_abs - 8;
                        }
                    }
                } else { 
                    if (flags & 0x40) {
                        tile_y_index = 7 - sprite_pixel_y_abs;
                    }
                }

                bool x_flip = flags & 0x20;


                uint16_t tile_addr = 0x8000 + (current_tile_num * 16) + (tile_y_index * 2);
                uint8_t low_byte = VRAM[tile_addr - 0x8000];
                uint8_t high_byte = VRAM[tile_addr - 0x8000 + 1];
                

                int bit_idx;
                if (flags & 0x20) {
                 
                    bit_idx = sprite_pixel_x;
                } else {
                   
                    bit_idx = 7 - sprite_pixel_x;
                }
                

                uint8_t sprite_color = ((high_byte >> bit_idx) & 1) << 1 | ((low_byte >> bit_idx) & 1);


                if (sprite_color != 0) {

                    uint16_t palette_addr = (flags & 0x10) ? 0xFF49 : 0xFF48;
                    final_color = get_color(sprite_color, palette_addr);
                    break;
                }
            }
        }
    }


    screenBuffer[LY * 160 + x] = final_color;
    x++;
}











void ppu::set_ppu_mode(uint8_t mode) {

    temp = mem->rd(0xFF41);
    temp &= 0b11111100;
    temp |= mode;
    mem->ld(temp, 0xFF41); 

}


uint8_t ppu::get_color(uint8_t color_index, uint16_t palette_address) {

    uint8_t palette_data = mem->rd(palette_address);
    uint8_t shift = color_index * 2;
    uint8_t final_palette_index = (palette_data >> shift) & 0b11; 
    
    return final_palette_index;
}



void ppu::addSprite(int i, uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    
    spritebuffer[i * 4] = a;
    spritebuffer[i * 4 + 1] = b;
    spritebuffer[i * 4 + 2] = c;
    spritebuffer[i * 4 + 3] = d;

}

uint8_t ppu::get_ppu_mode() {
    return mem->rd(0xFF41) & 0x03;
}