#include <fstream>
#include <cstdint>
#include <iostream>
#include <raylib.h>
#include <cstdlib>
#include <iomanip>
#include <array>

#include "cpu.hpp"
#include "mmu.hpp"
#include "ppu.hpp"

const int GB_WIDTH = 160;
const int GB_HEIGHT = 144;
const int CELLSIZE = 4;

const int screenWidth  = GB_WIDTH * CELLSIZE;
const int screenHeight = GB_HEIGHT * CELLSIZE;
const int TARGET_CYCLES_PER_FRAME = 76000; 

uint8_t buttons_pressed = 0;
uint8_t dpad_state;
uint8_t buttons_state;

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
    {161, 204, 165, 255},
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

std::array<Color, 4> current_Pallete = palette1;

void render_screen(ppu& graphics);
void draw_debug_overlay(cpu& gb);

int main(int argc, char *argv[]){

    if (argc < 2) {
        std::cout << "USAGE: ./gb [filename].gb \n";
        exit( 1 );
    }

    InitWindow(screenWidth, screenHeight, "GB");
    SetTargetFPS(60);

    cpu gb;

    std::string playerRom = argv[1];
    gb.initialize(playerRom);
    //main runtime

    while (!WindowShouldClose()) {

        buttons_pressed = 0;

        //get gameboy keypad input
        if (IsKeyDown(KEY_UP)) {
            buttons_enable = false;
            dpad_enable = true;
            buttons_pressed |= 0b0100;
        }
        if (IsKeyDown(KEY_DOWN)) {
            buttons_enable = false;
            dpad_enable = true;
            buttons_pressed |= 0b1000;
        }
        if (IsKeyDown(KEY_LEFT)) {
            buttons_enable = false;
            dpad_enable = true;
            buttons_pressed |= 0b0010;
        }
        if (IsKeyDown(KEY_RIGHT)) {
            buttons_enable = false;
            dpad_enable = true;
            buttons_pressed |= 0b0001;
        }

        if (IsKeyDown(KEY_RIGHT_SHIFT)) {
            buttons_enable = true;
            dpad_enable = false;
            buttons_pressed |= 0b0100;
        }
        if (IsKeyDown(KEY_ENTER)) {
            buttons_enable = true;
            dpad_enable = false;
            buttons_pressed |= 0b1000;
        }
        if (IsKeyDown(KEY_Z)) {
            buttons_enable = true;
            dpad_enable = false;
            buttons_pressed |= 0b0010;
        }
        if (IsKeyDown(KEY_X)) {
            buttons_enable = true;
            dpad_enable = false;
            buttons_pressed |= 0b0001;
        }

        if(buttons_enable) {
            buttons_pressed |= 0b00100000; 
        } else if (dpad_enable) {
            buttons_pressed |= 0b00010000;   
        } else {
            buttons_pressed = 0xFF;
        }

        gb.ld(buttons_pressed, 0xFF00);

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

        ClearBackground(BLACK);

        if (gb.rd(0xFF40) & 0x80) {
            render_screen(gb.graphics);
        }


        if(debug) { 
            draw_debug_overlay(gb);
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

void draw_debug_overlay(cpu& gb) {
    DrawText(TextFormat("AF: %04x", gb.AF), 0, 0, 20, RED);
    DrawText(TextFormat("BC: %04x", gb.BC), 0, 30, 20, RED);
    DrawText(TextFormat("DE: %04x", gb.DE), 0, 60, 20, RED);
    DrawText(TextFormat("HL: %04x", gb.HL), 0, 90, 20, RED);
    DrawText(TextFormat("SP: %04x", gb.SP), 0, 120, 20, RED);
    DrawText(TextFormat("OPCODE: %02x", gb.opcode), 0, 150, 20, RED);
    DrawText(TextFormat("PC: %04x", gb.PC), 0, 180, 20, RED);
    DrawText(TextFormat("ZF: %d", gb.get_ZF()), 0, 210, 20, RED);
    DrawText(TextFormat("NF: %d", gb.get_NF()), 0, 240, 20, RED);
    DrawText(TextFormat("HF: %d", gb.get_HF()), 0, 270, 20, RED);
    DrawText(TextFormat("CF: %d", gb.get_CF()), 0, 300, 20, RED);
    DrawText(TextFormat("cycles: %d", gb.cycles), 0, 330, 20, RED);
    DrawText(TextFormat("LCDC: %04x", gb.rd(0xFF40)), 0, 360, 20, RED);
    DrawText(TextFormat("SCY: %04x", gb.rd(0xFF42)), 0, 390, 20, RED);
    DrawText(TextFormat("SCX: %04x", gb.rd(0xFF43)), 0, 420, 20, RED);
    DrawText(TextFormat("WY: %04x", gb.rd(0xFF4A)), 0, 450, 20, RED);
    DrawText(TextFormat("WX: %04x", gb.rd(0xFF4B)), 0, 480, 20, RED);
    DrawText(TextFormat("PALLETTE: %04x", gb.rd(0xFF47)), 0, 510, 20, RED);
    DrawText(TextFormat("INTERRUPT: %04x", gb.mem.interrupts), 0, 540, 20, RED);
}

