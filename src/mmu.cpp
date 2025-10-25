#include "mmu.hpp"
#include "ppu.hpp"

extern uint8_t g_polled_actions;
extern uint8_t g_polled_directions;

void mmu::connect_ppu(ppu* ppu_ptr) {
    this->graphics = ppu_ptr;
}


void mmu::ld(uint8_t data, uint16_t address) {
    if (address <= 0xFF) {
        return; //bootrom is read only
    }

    if (address >= 0 && address <= 0x1FFF) { //romBank 0
        ERAM_ENABLE = data % 0x0F;
    }
    else if (address >= 0x2000 && address <= 0x3FFF) { //switch rom bank number
        
        uint8_t bank_value = data & 0b00011111;


        if (bank_value == 0x00 || bank_value == 0x20 || 
            bank_value == 0x40 || bank_value == 0x60) {
            bank_value += 1;
        }

        rom_bank_number = (rom_bank_number & 0b11100000) | bank_value;
    }
    else if (address >= 0x4000 && address <= 0x5FFF) {


        data &= 0b00000011;
        ram_bank_number = data;


    }
    else if (address >= 0x6000 && address <= 0x7FFF) {

        // banking_mode = data & 0x1;

    }
    else if (address >= 0x8000 && address <= 0x9FFF) {
        if (!graphics) return;
        if (graphics->vramRestrict) {
            return;
        } else {
            graphics->VRAM[address - 0x8000] = data;
            return; 
        }
    }
    else if (address >= 0xA000 && address <= 0xBFFF) { //External Cartridge Ram
        if ((ERAM_ENABLE & 0x0F) != 0x0A) {
            return;
        }

        uint8_t bank = (banking_mode == 0)
            ? 0
            : (ram_bank_number & 0b00000011);

        size_t offset = (address - 0xA000) + (bank * 0x2000);
        if (offset < 0x8000) {
            cart.ERAM[offset] = data;
        }
        
    }
    else if (address >= 0xC000 && address <= 0xCFFF) { //WRAM 1
        WRAM_1[address - 0xC000] = data;    
    }
    else if (address >= 0xD000 && address <= 0xDFFF) { //WRAM 2
        WRAM_2[address - 0xD000] = data;
    } 

    else if (address >= 0xE000 && address <= 0xFDFF) { 

        uint16_t wram_index = address - 0xE000; 

        if (wram_index < 0x1000) { 
            WRAM_1[wram_index] = data;
        } else { 
            WRAM_2[wram_index - 0x1000] = data;
        }
    }
    else if (address >= 0xFE00 && address <= 0xFE9F) {
        if (!graphics) return;
        if (graphics->oamRestrict) {
            return; 
        } else {
            graphics->OAM[address - 0xFE00] = data;
            return;
        }
    }
    else if (address >= 0xFEA0 && address <= 0xFEFF) {
        return; 
    }
    else if (address == 0xFF00) {

        uint8_t temp = IO[0] & 0x0F;
        data |= temp;
        IO[0] = data | 0xC0;

    }
    else if (address == 0xFF01) {
        std::cout << std::hex << data;
        IO[1] = data;
    }
    else if (address == 0xFF02) {
        IO[2] = data;

        if (data & 0x80) {

            IO[2] &= ~0x80; 
            IO[0x0F] |= 0x08;
        }
    }
    else if (address == 0xFF46) { //OAM DMA TRANSFER

        uint16_t source_addr = data * 0x100;

        for (int i = 0; i < 0xA0; i++) {
            uint8_t byte = rd(source_addr + i);
            ld(byte, 0xFE00 + i);
        }

        IO[0x46] = data;
    }

    else if (address == 0xFF50) {
        bootRomEnabled = false; 
    }
    else if (address >= 0xFF00 && address <= 0xFF7F) { //I/O registers
        IO[address - 0xFF00] = data;
    }
    else if (address >= 0xFF80 && address <= 0xFFFE) { //HRAM
        HRAM[address - 0xFF80] = data;
    }
    else if (address == 0xFFFF) {
        interrupts = data;
    }
    else {
        std::cout << "BAD POKE . ADDRESS: " << std::hex << +address << "\n";
        return;
    }
}

uint8_t  mmu::rd(uint16_t address) {

    if (bootRomEnabled && address <= 0x00FF) {
        return bootRom[address];
    } 

    if (address >= 0 && address <= 0x3FFF) { //ROMBANK 0
        return cart.romBank[address]; 
    }
    else if (address >= 0x4000 && address <= 0x7FFF) { //ROMBANK 1 (CHANGES DEPENDING ON MAPPER)
        
        uint8_t mapper = cart.romBank[0x147];

        if (mapper != 0) {

            if (banking_mode == 0){
                rom_bank_number_final = rom_bank_number & 0b00011111;
            } else if (banking_mode == 1) {

                uint8_t upper_bits = ram_bank_number << 5; 
                rom_bank_number_final = (rom_bank_number & 0b00011111) | upper_bits;
            }

            if (rom_bank_number_final == 0 || rom_bank_number_final == 0x20 || 
                rom_bank_number_final == 0x40 || rom_bank_number_final == 0x60) {
                rom_bank_number_final += 1;
            }
            return cart.romBank[(address - 0x4000) + (rom_bank_number_final * 0x4000)];

        } else {

            return cart.romBank[address];
        }

    }
    else if (address >= 0x8000 && address <= 0x9FFF) {
        if (!graphics) return 0xFF;
        if (graphics->vramRestrict) {
            return 0xFF;
        } else {
            return graphics->VRAM[address - 0x8000];
        }
    }
    else if (address >= 0xA000 && address <= 0xBFFF) { //External Cartridge Ram
        
        if ((ERAM_ENABLE & 0x0F) != 0x0A) {
            return 0xFF;
        }

        uint8_t bank;
        
        if (banking_mode == 0) { 
            bank = 0;
        } else {
            bank = ram_bank_number & 0b00000011; 
        }
        size_t offset = (address - 0xA000) + (bank * 0x2000);

        if (offset < 0x8000) {
            return cart.ERAM[offset];
        }

    }
    else if (address >= 0xC000 && address <= 0xCFFF) { //WRAM 1
        return WRAM_1[address - 0xC000];  
    }
    else if (address >= 0xD000 && address <= 0xDFFF) { //WRAM 2
        return WRAM_2[address - 0xD000];
    } 
    else if (address >= 0xE000 && address <= 0xFDFF) { //echo ram

        uint16_t mirrored_address = address - 0x2000; 

        if (mirrored_address >= 0xC000 && mirrored_address <= 0xCFFF) { 
            return WRAM_1[mirrored_address - 0xC000];
        } else if (mirrored_address >= 0xD000 && mirrored_address <= 0xDFFF) { 
            return WRAM_2[mirrored_address - 0xD000];
        } 
    }
    else if (address >= 0xFE00 && address <= 0xFE9F) {
        if (!graphics) return 0xFF;
        if (graphics->oamRestrict) {
            return 0xFF;
        } else {
            return graphics->OAM[address - 0xFE00];
        }
    }
    else if (address >= 0xFEA0 && address <= 0xFEFF) {
        return 0xFF; 
    }
    else if (address == 0xFF00) { //INPUT READ

        uint8_t state = IO[0];
        uint8_t output = state | 0x0F;

        if (!(state & 0x10)) { 
            output &= g_polled_directions;
        }
        if (!(state & 0x20)) { 
            output &= g_polled_actions; 
        }

        return output | 0xC0;

    } 
    else if (address == 0xFF01) {
        return IO[1];
    }
    else if (address >= 0xFF00 && address <= 0xFF7F) { //I/O registers
        return IO[address - 0xFF00];
    }
    else if (address >= 0xFF80 && address <= 0xFFFE) { //HRAM
        return HRAM[address - 0xFF80];
    }
    else if (address == 0xFFFF) {
        return interrupts;
    }
    else {
        std::cout << "BAD PEEK . ADDRESS: " << std::hex << +address << "\n";
    }
    return 0xFF;
}


