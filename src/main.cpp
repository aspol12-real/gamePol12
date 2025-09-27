#include <fstream>
#include <cstdint>
#include <iostream>
#include <vector>
#include <raylib.h>
#include <cstdlib>
#include <iomanip>

#include "cpu.hpp"
#include "mmu.hpp"
#include "ppu.hpp"


const int screenWidth  = 160 * 2;
const int screenHeight = 144 * 2;

int main(int argc, char *argv[]){

    cpu gb;
    
    gb.initialize();

    if (argc < 2) {
        std::cout << "USAGE: ./gb [filename].gb \n";
        exit( 1 );
    }
    
    
    InitWindow(screenWidth, screenHeight, "GB");
    SetTargetFPS(60);

    std::cout << "\n\n BOOTROM: \n\n";
        for (int i = 0; i < 256; i++) {
            std::cout << std::hex << +gb.bootRom[i] << " ";
        }
    std::cout << "\n\n";

    std::cout << "\n\n\n\n";
        for (int i = 0; i < 0x3FFF; i++) {
            std::cout << std::hex << +gb.mem.cart.romBank0[i] << " ";
        }
    std::cout << "\n\n";    
    //main runtime

    while (!WindowShouldClose()) {
        ClearBackground(BLACK);
        EndDrawing();
    }



    CloseWindow();

    return 1;
}
