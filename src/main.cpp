#include <fstream>
#include <cstdint>
#include <iostream>
#include "external/raylib.h"
#include <cstdlib>
#include <iomanip>

#include "colors.hpp"
#include "cpu.hpp"
#include "mmu.hpp"
#include "ppu.hpp"


//render values
const int CELLSIZE = 4;

const int screenMarginSides = 400;

const int screenWidth  = GB_WIDTH * CELLSIZE + screenMarginSides;
const int screenHeight = GB_HEIGHT * CELLSIZE;
const int TARGET_CYCLES_PER_FRAME = 76000; 

const int debugX = GB_WIDTH * CELLSIZE + 50;


//execution flags
bool run = true;
bool debug = true;
bool screenOnly = false;

//input values/flags
uint8_t buttons_pressed = 0;
uint8_t g_polled_actions = 0x0F;
uint8_t g_polled_directions = 0x0F;
bool dpad_enable = false;
bool buttons_enable = false;



//declarations
void render_screen(ppu& graphics);
void draw_debug_overlay(cpu& gb, mmu& mem, Font customfont);
void draw_tilemap_viewer(cpu& gb, ppu& graphics, int startX, int startY);
void handle_inputs(cpu& gb, mmu& mem, ppu& graphics);
void tick_peripherals(mmu& mem, ppu& graphics, int cycles);
void render_all(cpu& gb, mmu& mem, ppu& graphics, Font customfont);

mmu mem;
ppu graphics(mem);
mem.connect_ppu(&graphics);
cpu gb(mem);

//it's showtime, folks
int main(int argc, char *argv[]){

    if (argc < 2) {
        std::cout << "USAGE: ./gb [filename].gb \n";
        exit( 1 );
    }

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "GB");

    Font customfont = LoadFont("res/fonts/JetBrainsMono-Bold.ttf");
    Image icon = LoadImage("res/icon.png"); 
    SetWindowIcon(icon);
    SetTargetFPS(60);

    std::string playerRom = argv[1];
    gb.initialize(playerRom);




    while (!WindowShouldClose()) {

        handle_inputs(gb, mem, graphics);

        if (run) {
            int cycles_this_frame = 0;
            while (cycles_this_frame < TARGET_CYCLES_PER_FRAME) {
                int cycles_executed = gb.execute();
                cycles_this_frame += cycles_executed;
                tick_peripherals(mem, graphics, cycles_executed);
            }
        }

        render_all(gb, mem, graphics, customfont);
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

void draw_debug_overlay(cpu& gb, mmu& mem, Font customfont) {

    DrawTextEx(customfont, TextFormat("AF: %04x, BC: %04x", gb.AF, gb.BC), {debugX, 0}, 32.0, 2.0, GREEN);
    DrawTextEx(customfont, TextFormat("DE: %04x, HL: %04x", gb.DE, gb.HL), {debugX, 30}, 32.0, 2.0, GREEN);
    DrawTextEx(customfont, TextFormat("SP: %04x, PC: %04x", gb.SP, gb.PC), {debugX, 60}, 32.0, 2.0, GREEN);
    DrawTextEx(customfont, TextFormat("OPCODE: %02x, HALT: %d", gb.opcode, gb.halted), {debugX,90}, 32.0, 2.0, GREEN);
    DrawTextEx(customfont, TextFormat("FLAGS: %d, %d, %d, %d", gb.get_ZF(), gb.get_NF(), gb.get_HF(), gb.get_CF()), {debugX, 120}, 32.0, 2.0, GREEN);
    DrawTextEx(customfont, TextFormat("CYCLES: %02d, IME: %d", gb.cycles, gb.IME), {debugX,150}, 32.0, 2.0, GREEN);
    DrawTextEx(customfont, TextFormat("[HL]: %02x, [BC]: %02x", mem.rd(gb.HL), mem.rd(gb.BC)), {debugX,180}, 32.0, 2.0, GREEN);
    DrawTextEx(customfont, TextFormat("LCDC: %02x, LY: %02x", mem.rd(0xFF40), mem.rd(0xFF44)), {debugX,210}, 32.0, 2.0, GREEN);
    DrawTextEx(customfont, TextFormat("SCX: %02x, SCY: %02x", mem.rd(0xFF43), mem.rd(0xFF42)), {debugX,240}, 32.0, 2.0, GREEN);
    DrawTextEx(customfont, TextFormat("IF: %02x, KEYPAD: %02x", mem.interrupts, mem.rd(0xFF00)), {debugX,270}, 32.0, 2.0, GREEN);

}

void draw_tilemap_viewer(cpu& gb, ppu& graphics, int startX, int startY) {
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


            uint8_t byte1 = graphics.VRAM[(addr + (y * 2)) - TILE_DATA_START];     // low byte
            uint8_t byte2 = graphics.VRAM[(addr + (y * 2) + 1) - TILE_DATA_START]; // high byte

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

void handle_inputs(cpu& gb, mmu& mem, ppu& graphics) {
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

        } else {
            if (IsKeyPressed(KEY_Q)) {
                run = true;
            }
            if (IsKeyPressed(KEY_S)) {
                int cycles_executed = gb.execute();
                tick_peripherals(mem, graphics, cycles_executed);
                if (gb.halted){
                    std::cout << "HALTED!\n";
                }
            }
            if (IsKeyDown(KEY_D)) {
                for (int i = 0; i < 100; i++) {
                    int cycles_executed = gb.execute();
                    tick_peripherals(mem, graphics, cycles_executed);
                }
            }
            if (IsKeyPressed(KEY_P)) {
                std::cout << "\n\n\n\n";
                for (int i = 0; i < 0xFFFF; i++) {
                    std::cout << std::hex << +mem.rd(i) << " ";
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
        
        if (IsKeyPressed(KEY_SPACE)) {
            if (!screenOnly) {
                screenOnly = true;
            } else {
                screenOnly = false;
            }
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
        if (IsKeyPressed(KEY_FIVE)) {
            current_Pallete = palette5;
        }
}

void tick_peripherals(mmu& mem, ppu& graphics, int cycles) {

    int tma_reload_cycles;
    int tma_reload_value;
    bool tma_reload_scheduled = false;

    uint8_t TAC = mem.rd(0xFF07);


    if (tma_reload_scheduled) {
        tma_reload_cycles--;
        
        if (tma_reload_cycles <= 0) {
            mem.ld(tma_reload_value, 0xFF05);
            tma_reload_scheduled = false;
        }
    }

    if (TAC & 0x4) {

        uint8_t TIMA = mem.rd(0xFF05);

        if ((TIMA) == 0xFF) { //TIMA overflow

            mem.ld(0, 0xFF05); //wrap to 0

            uint8_t TMA = mem.rd(0xFF06);

            uint8_t interruptFlag = mem.rd(0xFF0F);
            interruptFlag |= 0x4;
            mem.ld(interruptFlag, 0xFF0F);

            tma_reload_scheduled = true;
            tma_reload_value = mem.rd(0xFF06); 
            tma_reload_cycles = 4; 

        } else{
            mem.ld(TIMA + 1, 0xFF05);
        }
    }

    //increment div
    uint8_t div = mem.rd(0xFF04);
    div++;
    mem.ld(div, 0xFF04);


    //execute ppu for N cycles per cpu cycle only if LCD is on!
    if (mem.rd(0xFF40) & 0x80) {
        for (int i = 0; i < cycles; i++) {
            graphics.tick();
        }
    } else {
        graphics.set_ppu_mode(graphics.h_blank);
        uint8_t temp = mem.rd(0xFF40);
        temp &= 0x11111100;
        mem.ld(temp, 0xFF40);
    }

}

void render_all(cpu& gb, mmu& mem, ppu& graphics, Font customfont) {
    BeginDrawing();
        ClearBackground({13, 12, 36, 255});

        if (mem.rd(0xFF40) & 0x80) {
            render_screen(graphics);
        }

        if (!screenOnly) {
            SetWindowSize(screenWidth, screenHeight);
            if (debug) {
                draw_debug_overlay(gb, mem, customfont);
            } else {  
                uint8_t stat_mode = mem.rd(0xFF41) & 0b11;
                draw_tilemap_viewer(gb, graphics, debugX, 0);
            }
        } else {
            SetWindowSize(screenWidth - screenMarginSides, screenHeight);
        }

        EndDrawing();
}