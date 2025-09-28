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

    std::cout << "\n\n\n\n";
        for (int i = 0; i < 0xFFFF; i++) {
            std::cout << std::hex << +gb.mem.rd(i) << " ";
        }
    std::cout << "\n\n";    
 
    //main runtime

    while (!WindowShouldClose()) {
        gb.execute();
        
        ClearBackground(BLACK);
    
        EndDrawing();
    }



    CloseWindow();

    return 1;
}
