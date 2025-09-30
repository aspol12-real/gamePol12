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
const int TARGET_CYCLES_PER_FRAME = 69905; 

bool run = false;


int main(int argc, char *argv[]){

    if (argc < 2) {
        std::cout << "USAGE: ./gb [filename].gb \n";
        exit( 1 );
    }

    cpu gb;
    
    std::string playerRom = argv[1];
    gb.initialize(playerRom);


    
    
    
    InitWindow(screenWidth, screenHeight, "GB");
    SetTargetFPS(60);

    
    std::cout << "\n\n\n\n";
        for (int i = 0; i < 0xFFFF; i++) {
            std::cout << std::hex << +gb.rd(i) << " ";
        }
    std::cout << "\n\n"; 

    //main runtime

    while (!WindowShouldClose()) {


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
                std::cout << "step! \n";
            }
            if (IsKeyDown(KEY_D)) {
                gb.execute();  
                std::cout << "step! \n";
            }
            if (IsKeyPressed(KEY_P)) {
                std::cout << "\n\n\n\n";
                for (int i = 0; i < 0xFFFF; i++) {
                    std::cout << std::hex << +gb.rd(i) << " ";
                }
                std::cout << "\n\n"; 
            }
        }
        
        ClearBackground(BLACK);

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
        DrawText(TextFormat("CURRENT PUSH: %04x", (gb.rd(gb.SP)) << 8 | gb.rd(gb.SP + 1)), 0, 350, 20, RED);
        DrawText(TextFormat("LY: %04x", gb.rd(0xFF44)), 0, 390, 20, RED);

        EndDrawing();
    }



    CloseWindow();

    return 1;
}
