#include <fstream>
#include <cstdint>
#include <iostream>
#include <raylib.h>
#include <cstdlib>
#include <iomanip>

#include "cpu.hpp"
#include "mmu.hpp"
#include "ppu.hpp"


const int screenWidth  = 160 * 4;
const int screenHeight = 144 * 4;

bool run = false;

const int instructionsPerFrame = 100;

int main(int argc, char *argv[]){

    cpu gb;
    
    std::string playerRom = argv[1];
    gb.initialize(playerRom);

    if (argc < 2) {
        std::cout << "USAGE: ./gb [filename].gb \n";
        exit( 1 );
    }
    
    
    InitWindow(screenWidth, screenHeight, "GB");
    SetTargetFPS(60);

 
    //main runtime

    while (!WindowShouldClose()) {


        if(run) {
            if (IsKeyPressed(KEY_W)) {
                run = false;
            }
            for (int i = 0; i < instructionsPerFrame; i++) {
                gb.execute();
            }
        } else {
            if (IsKeyPressed(KEY_S)) {
                gb.execute();  
            }
            if (IsKeyPressed(KEY_Q)) {
                run = true;
            }
        }
        ClearBackground(BLACK);
        

        DrawText(TextFormat("AF: %04x", gb.AF), 0, 0, 20, RED);
        DrawText(TextFormat("BC: %04x", gb.BC), 0, 30, 20, RED);
        DrawText(TextFormat("DE: %04x", gb.DE), 0, 60, 20, RED);
        DrawText(TextFormat("HL: %04x", gb.HL), 0, 90, 20, RED);
        DrawText(TextFormat("SP: %04x", gb.SP), 0, 120, 20, RED);
        DrawText(TextFormat("OPCODE: %02x", gb.mem.rd(gb.PC)), 0, 150, 20, RED);
        DrawText(TextFormat("PC: %04x", gb.PC), 0, 180, 20, RED);

        EndDrawing();
    }



    CloseWindow();

    return 1;
}
