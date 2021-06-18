#pragma once

#include <stdint.h>

/*
Memory Map:
+---------------+= 0xFFF (4095) End of Chip-8 RAM
|               |
|               |
|               |
|               |
|               |
| 0x200 to 0xFFF|
|     Chip-8    |
| Program / Data|
|     Space     |
|               |
|               |
|               |
+- - - - - - - -+= 0x600 (1536) Start of ETI 660 Chip-8 programs
|               |
|               |
|               |
+---------------+= 0x200 (512) Start of most Chip-8 programs
| 0x000 to 0x1FF|
| Reserved for  |
|  interpreter  |
+---------------+= 0x000 (0) Start of Chip-8 RAM
*/

class CPU{
private:
    uint16_t stack[16]; //stack
    uint16_t sp; //stack pointer

    uint8_t memory[4096]; //chip 8 has 4 kilobytes of memory
    uint8_t V[16]; //on Chip 8, registers are represented with V[0 t F], there are 16 of them

    uint8_t st; //sound timer
    uint8_t dt; //delay timer

    
    uint16_t opcode; //on chip 8, all instructions are 2 bytes long, so, we declared it with uint16_t
    uint16_t pc; //16 bits program counter
    uint16_t I; //index register 

    void init();

public:
    //constructor and destructor functions
    CPU();
    ~CPU();

    uint8_t frame[64*32]; //for graphics, chip 8 supports 64x32 pixels, 64 pixels wide and 32 bit pixels in height
    uint8_t keypad[16]; //there 16 keypad buttons supported by chip 8
    bool drawFlag; //draw flag of chip 8 to update screen

    void execute();
    int loadROM(const char *rom_path);
};