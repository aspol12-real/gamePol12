#include <fstream>
#include <cstdint>
#include <iostream>
#include "external/raylib.h"
#include <cstdlib>
#include <iomanip>
#include <array>

#include "cpu.hpp"
#include "mmu.hpp"
#include "ppu.hpp"

const int GB_WIDTH = 160;
const int GB_HEIGHT = 144;
const int CELLSIZE = 4;

const int screenMarginSides = 400;

const int screenWidth  = GB_WIDTH * CELLSIZE + screenMarginSides;
const int screenHeight = GB_HEIGHT * CELLSIZE;
const int TARGET_CYCLES_PER_FRAME = 76000; 

const int debugX = GB_WIDTH * CELLSIZE + 2;

uint8_t buttons_pressed = 0;

//(active high)
uint8_t g_polled_actions = 0x0F;
uint8_t g_polled_directions = 0x0F;

bool dpad_enable = false;
bool buttons_enable = false;

bool run = false;
bool debug = true;


int peek_address = 0;


std::array<Color, 4> palette1 = {{
    {244, 233, 205, 255},
    {157, 190, 187, 255},
    {70, 129, 137, 255},
    {3, 25, 38, 255}
}};

std::array<Color, 4> palette2 = {{
    {191, 234, 195, 255},
    {112, 151, 117, 255},
    {65, 93, 67, 255},
    {17, 29, 19, 255}
}};

std::array<Color, 4> palette3 = {{
    {255, 240, 243, 255}, 
    {255, 77, 109, 255},  
    {164, 19, 60, 255},   
    {89, 13, 34, 255}     
}};

std::array<Color, 4> palette4 = {{
    {255, 255, 255, 255},
    {175, 175, 175, 255},
    {65, 65, 65, 255},
    {0, 0, 0, 255}
}};

std::array<Color, 4> current_Pallete = palette2;

void render_screen(ppu& graphics);
void draw_debug_overlay(cpu& gb, Font customfont);
void draw_tilemap_viewer(cpu& gb, int startX, int startY);

int main(int argc, char *argv[]){

    if (argc < 2) {
        std::cout << "USAGE: ./gb [filename].gb \n";
        exit( 1 );
    }

    InitWindow(screenWidth, screenHeight, "GB");

    Font customfont = LoadFont("res/fonts/JetBrainsMono-Bold.ttf");
    Image icon = LoadImage("res/icon.png"); 
    SetWindowIcon(icon);
    SetTargetFPS(60);

    cpu gb;

    std::string playerRom = argv[1];

    gb.initialize(playerRom);

    //main runtime

    while (!WindowShouldClose()) {

        g_polled_actions = 0x0F;
        g_polled_directions = 0x0F;

        if (IsKeyDown(KEY_Z)) g_polled_actions &= ~0x01; 
        if (IsKeyDown(KEY_X)) g_polled_actions &= ~0x02; 
        if (IsKeyDown(KEY_RIGHT_SHIFT)) g_polled_actions &= ~0x04; 
        if (IsKeyDown(KEY_ENTER)) g_polled_actions &= ~0x08; 

        if (IsKeyDown(KEY_RIGHT)) g_polled_directions &= ~0x01; 
        if (IsKeyDown(KEY_LEFT)) g_polled_directions &= ~0x02;
        if (IsKeyDown(KEY_UP)) g_polled_directions &= ~0x04; 
        if (IsKeyDown(KEY_DOWN)) g_polled_directions &= ~0x08;


        if(run) {
            if (IsKeyPressed(KEY_W)) {
                run = false;
            }

            int cycles_this_frame = 0;
            while (cycles_this_frame < TARGET_CYCLES_PER_FRAME) {
                int cycles_executed = gb.execute();
                cycles_this_frame += cycles_executed;
            }

        } else {
            if (IsKeyPressed(KEY_Q)) {
                run = true;
            }
            if (IsKeyPressed(KEY_S)) {
                gb.execute();  
                if (gb.halted){
                    std::cout << "HALTED!\n";
                }
            }
            if (IsKeyDown(KEY_D)) {
                for (int i = 0; i < 100; i++) {
                    gb.execute();  
                }
            }
            if (IsKeyPressed(KEY_P)) {
                std::cout << "\n\n\n\n";
                for (int i = 0; i < 0xFFFF; i++) {
                    std::cout << std::hex << +gb.rd(i) << " ";
                }
                std::cout << "\n\n"; 
            }
        }
        if (IsKeyPressed(KEY_TAB)) {
                debug = true;
            }
            if (IsKeyPressed(KEY_LEFT_SHIFT)) {
                debug = false;
            }
        
        //pallette switching
        if (IsKeyPressed(KEY_ONE)) {
            current_Pallete = palette1;
        }
        if (IsKeyPressed(KEY_TWO)) {
            current_Pallete = palette2;
        }
        if (IsKeyPressed(KEY_THREE)) {
            current_Pallete = palette3;
        }
        if (IsKeyPressed(KEY_FOUR)) {
            current_Pallete = palette4;
        }



        //rendering!

        BeginDrawing();
        ClearBackground({13, 12, 36, 255});

        if (gb.rd(0xFF40) & 0x80) {
            render_screen(gb.graphics);
        }

        if (debug) {draw_debug_overlay(gb, customfont);
        } else {
            
            uint8_t stat_mode = gb.rd(0xFF41) & 0b11;
    
            draw_tilemap_viewer(gb, debugX, 0);
        }

        EndDrawing();
    }



    CloseWindow();

    return 1;
}

void render_screen(ppu& graphics) {

    for (int y = 0; y < GB_HEIGHT; y++) {
        for (int x = 0; x < GB_WIDTH; x++) {
            
            int buffer_index = (y * GB_WIDTH) + x;
            uint8_t color_code = graphics.screenBuffer[buffer_index];

            Color pixel_color = current_Pallete[color_code & 0b11]; 
            
            DrawRectangle(x * CELLSIZE, y * CELLSIZE, CELLSIZE, CELLSIZE, pixel_color);
        }
    }
}

void draw_debug_overlay(cpu& gb, Font customfont) {

    DrawTextEx(customfont, TextFormat("AF: %04x, BC: %04x", gb.AF, gb.BC), {debugX, 0}, 32.0, 2.0, GREEN);
    DrawTextEx(customfont, TextFormat("DE: %04x, HL: %04x", gb.DE, gb.HL), {debugX, 30}, 32.0, 2.0, GREEN);
    DrawTextEx(customfont, TextFormat("SP: %04x, PC: %04x", gb.SP, gb.PC), {debugX, 60}, 32.0, 2.0, GREEN);
    DrawTextEx(customfont, TextFormat("OPCODE: %02x", gb.opcode), {debugX,90}, 32.0, 2.0, GREEN);
    DrawTextEx(customfont, TextFormat("FLAGS: %d, %d, %d, %d", gb.get_ZF(), gb.get_NF(), gb.get_HF(), gb.get_CF()), {debugX, 120}, 32.0, 2.0, GREEN);
    DrawTextEx(customfont, TextFormat("CYCLES: %d", gb.cycles), {debugX,150}, 32.0, 2.0, GREEN);
    DrawTextEx(customfont, TextFormat("[HL]: %02x, [BC]: %02x", gb.rd(gb.HL), gb.rd(gb.BC)), {debugX,180}, 32.0, 2.0, GREEN);
    DrawTextEx(customfont, TextFormat("LCDC: %02x, LY: %02x", gb.rd(0xFF40), gb.rd(0xFF44)), {debugX,210}, 32.0, 2.0, GREEN);
    DrawTextEx(customfont, TextFormat("SCX: %02x, SCY: %02x", gb.rd(0xFF42), gb.rd(0xFF43)), {debugX,240}, 32.0, 2.0, GREEN);
    DrawTextEx(customfont, TextFormat("IF: %02x, KEYPAD: %02x", gb.mem.interrupts, gb.rd(0xFF00)), {debugX,270}, 32.0, 2.0, GREEN);

}

void draw_tilemap_viewer(cpu& gb, int startX, int startY) {
    const uint16_t TILE_DATA_START = 0x8000;
    const uint16_t TILE_DATA_END = 0x97FF;
    const int TILE_BYTES = 16;
    const int TILE_SIZE_PXL = 2; 
    const int GB_TILE_DIM = 8; 

    const int TILES_PER_ROW = 16;
    const int TILE_SPACING = 2;
    const int TILE_WIDTH_SCREEN = GB_TILE_DIM * TILE_SIZE_PXL;

    int tileCount = 0;
    int currentY = startY;

    for (uint16_t addr = TILE_DATA_START; addr <= TILE_DATA_END; addr += TILE_BYTES) {
        
        int tileX = tileCount % TILES_PER_ROW;
        int tileY = tileCount / TILES_PER_ROW;

        int screenX = startX + (tileX * (TILE_WIDTH_SCREEN + TILE_SPACING));
        int screenY = currentY + (tileY * (TILE_WIDTH_SCREEN + TILE_SPACING));
        

        for (int y = 0; y < GB_TILE_DIM; y++) {


            uint8_t byte1 = gb.graphics.VRAM[(addr + (y * 2)) - TILE_DATA_START];     // low byte
            uint8_t byte2 = gb.graphics.VRAM[(addr + (y * 2) + 1) - TILE_DATA_START]; // high byte

            for (int x = 0; x < GB_TILE_DIM; x++) {

                int bitIndex = 7 - x;


                uint8_t color_index = 
                    ((byte2 >> bitIndex) & 0b1) << 1 |
                    ((byte1 >> bitIndex) & 0b1);
                
                Color pixel_color = current_Pallete[color_index];

                DrawRectangle(screenX + x * TILE_SIZE_PXL, screenY + y * TILE_SIZE_PXL, TILE_SIZE_PXL, TILE_SIZE_PXL, pixel_color);
            }
        }

        tileCount++;
    }
}