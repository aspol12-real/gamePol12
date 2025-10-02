#include <fstream>
#include <cstdint>
#include <iostream>
#include <raylib.h>
#include <cstdlib>
#include <iomanip>

#include "cpu.hpp"
#include "mmu.hpp"
#include "ppu.hpp"

const int GB_WIDTH = 160;
const int GB_HEIGHT = 144;
const int CELLSIZE = 4;

const int screenWidth  = GB_WIDTH * CELLSIZE;
const int screenHeight = GB_HEIGHT * CELLSIZE;
const int TARGET_CYCLES_PER_FRAME = 69905; 

uint8_t buttons_pressed = 0;
uint8_t dpad_state;
uint8_t buttons_state;

bool dpad_enable = false;
bool buttons_enable = false;
bool run = false;
bool debug = true;

int peek_address = 0;

Color richblack = {3, 25, 38, 255};
Color teal      = {70, 129, 137, 255};
Color ash       = {157, 190, 187, 255};
Color parchment = {244, 233, 205, 255};

Color palette[4] = {
    parchment,
    ash,
    teal,
    richblack
};



void render_screen(ppu& graphics);

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
            buttons_pressed |= 0b100000; 
        } else if (dpad_enable) {
            buttons_pressed |= 0b010000;   
        } else {
            buttons_pressed = 0;
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
        
        ClearBackground(BLACK);

        if (gb.rd(0xFF40) & 0x80) {
            render_screen(gb.graphics);
        }


        if(debug) {
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
            DrawText(TextFormat("BUTTON: %04x", gb.rd(0xFF00)), 0, 540, 20, RED);
            
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

            Color pixel_color = palette[color_code & 0b11]; 
            
            DrawRectangle(x * CELLSIZE, y * CELLSIZE, CELLSIZE, CELLSIZE, pixel_color);
        }
    }
}

