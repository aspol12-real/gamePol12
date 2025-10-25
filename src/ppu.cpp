#include "ppu.hpp"
#include "mmu.hpp"

void ppu::tick() {

    clocks++; //increment clocks ONCE per tick

    if (LY < 144) {
        if (clocks == 1 && LY < 144) {  //OAM SEARCH (ONLY OAM CANNOT BE ACCESSED)
            set_ppu_mode(oamsearch);

            spritesFound = 0;
            for (int i = 0; i < 40; i++) {

                uint8_t sprite_Y = OAM[i * 4];
                uint8_t sprite_X = OAM[i * 4 + 1];
                uint8_t tile_number = OAM[i * 4 + 2];
                uint8_t sprite_flags = OAM[i * 4 + 3];

                int spriteHeight = (mem.rd(0xFF40) & 0x04) ? 16 : 8;
                if (((sprite_X + 8) > 0) && ((LY + 16) >= sprite_Y) && ((LY + 16) < (sprite_Y + spriteHeight))) {
                    spritesFound++;
                    if (spritesFound <= FINDABLE_SPRITES) {
                        addSprite(spritesFound - 1, sprite_Y, sprite_X, tile_number, sprite_flags);
                    } else {
                        
                    }
                }
            }

            vramRestrict = false;
            oamRestrict = true;
        }
        else if (clocks == 80) { //Pixel Transfer (VRAM & OAM CANNOT BE ACCESSED)
            set_ppu_mode(pixeltransfer);
            render_scanline(LY);
            oamRestrict = true;
            vramRestrict = true;

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
            uint8_t temp = mem.rd(0xFF0F);
            temp |= 0x1;
            mem.ld(temp, 0xFF0F);

            oamRestrict = false;
            vramRestrict = false;
            set_ppu_mode(v_blank);
        }
        if (LY > 153) { //ENTER OAM SEARCH AFTER LAST VBLANK LINE

            LY = 0;

        } 

        mem.ld(LY, 0xFF44); //updates LY register
    }
}


void ppu::render_scanline(int LY) {

    uint16_t tile_num_addr_base;
    uint16_t tile_num_addr;
    uint16_t tile_data_addr_base;
    uint16_t tile_data_addr;
    uint8_t tile_num;
    uint16_t offset;
    uint8_t low_byte;
    uint8_t high_byte;

    uint8_t LCDC = mem.rd(0xFF40);
    uint8_t SCY  = mem.rd(0xFF42);
    uint8_t SCX  = mem.rd(0xFF43);
    uint8_t WY   = mem.rd(0xFF4A);
    uint8_t WX   = mem.rd(0xFF4B);

    bool masterEnable = LCDC & 0x80;
    bool wnTileMap    = LCDC & 0x40;
    bool wnEnable     = LCDC & 0x20;
    bool bgWinTile    = LCDC & 0x10;
    bool bgTileMap    = LCDC & 0x08;
    bool objSize      = LCDC & 0x04;
    bool objEnable    = LCDC & 0x02;
    bool bgPriority   = LCDC & 0x01;

    background_fifo.clear();
    fetch_tile_row(0, LY);
    int discard_count = SCX % 8;
    int current_pixel_x = 0;

    for (int i = 0; i < discard_count; ++i) {
        if (!background_fifo.empty()) {
            background_fifo.pop_front();
        } else {
            break; 
        }
    }

    while (current_pixel_x < GB_WIDTH) {

        if (background_fifo.size() < 8) {
            int tile_x = (current_pixel_x + SCX) / 8;
            fetch_tile_row(current_pixel_x + 8, LY);
        }

        if (!background_fifo.empty()) {
            
            uint8_t bg_color_index = background_fifo.front();
            background_fifo.pop_front();

            bg_raw_colors[current_pixel_x] = bg_color_index;

 
            final_colors[current_pixel_x] = get_color(bg_color_index, palette_address); 
            
            current_pixel_x++;
        } else {
            break; 
        }
    }

    if (objEnable) {
        for (int j = 0; j < spritesFound; j++) {
            uint8_t obj_y = spritebuffer[j * 4];
            uint8_t obj_x = spritebuffer[j * 4 + 1];
            uint8_t obj_index = spritebuffer[j * 4 + 2];
            uint8_t obj_attributes = spritebuffer[j * 4 + 3];

            bool use_palette1  = obj_attributes & 0x10;
            bool x_flip        = obj_attributes & 0x20;
            bool y_flip        = obj_attributes & 0x40;
            bool bg_priority   = obj_attributes & 0x80;
            
            int spriteHeight = (LCDC & 0x04) ? 16 : 8;

            if (LY >= (obj_y - 16) && LY < (obj_y - 16 + spriteHeight)) {
                uint8_t row = LY - (obj_y - 16); 
                if (y_flip) row = (spriteHeight - 1) - row;
                
                uint8_t tile_index_to_use = obj_index;
                if (spriteHeight == 16) {
                    tile_index_to_use &= 0xFE; 
                    if (row >= 8) {
                        tile_index_to_use |= 0x01;
                        row -= 8;
                    }
                }
                
                uint16_t tile_addr = 0x8000 + tile_index_to_use * 16 + row * 2;
                uint8_t low_byte  = mem.rd(tile_addr);
                uint8_t high_byte = mem.rd(tile_addr + 1);

                for (int x = 0; x < 8; x++) {
                    int pixel_x = obj_x - 8 + x; 
                    if (pixel_x < 0 || pixel_x >= GB_WIDTH) continue;

                    int bit = x_flip ? x : 7 - x;
                    uint8_t color_index = ((high_byte >> bit) & 1) << 1 | ((low_byte >> bit) & 1);

                    if (color_index == 0) continue; 

                    if (bg_priority && bg_raw_colors[pixel_x] != 0) continue;

                    uint8_t palette_addr = use_palette1 ? 0xFF49 : 0xFF48;
                    uint8_t final_color = get_color(color_index, palette_addr);

                    final_colors[pixel_x] = final_color;
                }
            }
        }
    }

    for (int x = 0; x < GB_WIDTH; x++) {
        int line_start = LY * GB_WIDTH;
        screenBuffer[line_start + x] = final_colors[x];
    }
}


void ppu::set_ppu_mode(uint8_t mode) {

    temp = mem.rd(0xFF41);
    temp &= 0b11111100;
    temp |= mode;
    mem.ld(temp, 0xFF41); 

}


uint8_t ppu::get_color(uint8_t color_index, uint16_t palette_address) {

    uint8_t palette_data = mem.rd(palette_address);
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
    return mem.rd(0xFF41) & 0x03;
}

void ppu::fetch_tile_row(int current_pixel_x, int scanline_y) {
    
    uint8_t LCDC = mem.rd(0xFF40);
    uint8_t SCY  = mem.rd(0xFF42);
    uint8_t SCX  = mem.rd(0xFF43);
    uint8_t WY   = mem.rd(0xFF4A);
    uint8_t WX   = mem.rd(0xFF4B);

    bool wnTileMap    = LCDC & 0x40;
    bool wnEnable     = LCDC & 0x20;
    bool bgWinTile    = LCDC & 0x10;
    bool bgTileMap    = LCDC & 0x08;

    bool drawing_window = wnEnable && (WX <= 166) && (scanline_y >= WY) && (current_pixel_x >= (WX - 7));

    uint8_t tile_y_offset;
    uint16_t tile_num_addr_base;
    uint16_t tile_data_addr_base = (bgWinTile) ? 0x8000 : 0x9000;
    uint8_t tile_num;

    if (drawing_window) {

        uint8_t window_line = scanline_y - WY;
        uint8_t map_y = (window_line / 8) & 0x1F; 
        uint8_t map_x = ((current_pixel_x - (WX - 7)) / 8) & 0x1F; 
        tile_y_offset = (window_line % 8);
        tile_num_addr_base = (wnTileMap) ? 0x9C00 : 0x9800;
        
        uint16_t tile_num_addr = tile_num_addr_base + map_y * 32 + map_x;
        tile_num = mem.rd(tile_num_addr);
    } else {

        uint8_t map_y = ((scanline_y + SCY) / 8) & 0x1F; 
        uint8_t map_x = ((current_pixel_x + SCX) / 8) & 0x1F; 
        tile_y_offset = (scanline_y + SCY) % 8;
        tile_num_addr_base = (bgTileMap) ? 0x9C00 : 0x9800;

        uint16_t tile_num_addr = tile_num_addr_base + map_y * 32 + map_x;
        tile_num = mem.rd(tile_num_addr);
    }


    uint16_t tile_data_addr;
    if (bgWinTile) {
        tile_data_addr = tile_data_addr_base + (tile_num * 16) + tile_y_offset * 2;
    } else {
        int16_t signed_tile_num = (int8_t)tile_num;
        tile_data_addr = tile_data_addr_base + (signed_tile_num * 16) + tile_y_offset * 2;
    }

    uint8_t low_byte = mem.rd(tile_data_addr);
    uint8_t high_byte = mem.rd(tile_data_addr + 1);

    for (int bit = 7; bit >= 0; bit--) {
        uint8_t color_index = ((high_byte >> bit) & 1) << 1 | ((low_byte >> bit) & 1);
        background_fifo.push_back(color_index);
    }
}