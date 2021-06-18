#include <iostream>
#include "cpu.hpp"


//chip 8 supports hexadecimal characters from 0 to F
//and each of them are represented with 5 bytes
//so, a total of 80 bytes are used for 16 hexadecimal numbers from 0 to F
unsigned char font[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

//define functions of CPU class
CPU::CPU(){

}

CPU::~CPU(){

}

//initialize the CPU.
void CPU::init(){
    //chip 8 has 4 KB of memory
    //and the first 512 bytes or from 0x00 to 0x200 memory are reserved for chip 8 interpreter
    //and from 0x200 to 0xFFF or 512 to 4096 bytes, the memory is reserved for loading and running programs

    //so, let's initialize registers, stack pointers and opcodes
    pc = 0x200; //program counter
    opcode = 0; //opcode
    I = 0; //index register
    sp = 0; //stack pointer

    //intialize the frame buffer or graphics
    for(int i=0; i<64*32; i++){
        frame[i] = 0;
    }

    //clear the stack, keypad and registers
    for(int i=0; i<16; i++){
        stack[i] = 0;
        V[i] = 0;
        keypad[i] = 0;
    }

    //clear the memory
    for(int i=0; i<4096; i++){
        memory[i] = 0;
    }

    //now we have a memory of 4096 bytes, and we need to load the CHIP 8 interpreter upto 0x200
    //first, load the font into the memory
    for(int i=0; i<80; i++){
        memory[i] = font[i];
    }

    //set sound and delay timers
    st = 0;
    dt = 0;
}

int CPU::loadROM(const char *rom_path){
    //initialise the CPU
    init(); //this sets all the required registers, memory and graphics buffer from 0x000 to 0x200

    std::cout << "Loading ROM into memory" << std::endl;

    //open the specified rom
    FILE *fp = fopen(rom_path, "rb");
    if(fp == nullptr){
        std::cerr << "Failed to open ROM" << std::endl;
        return -1;
    }

    //if fp != nullptr, then find the size of the rom
    fseek(fp, 0, SEEK_END); //seek to the end of the file
    int rom_size = ftell(fp); //store the size of the file
    fseek(fp, 0, SEEK_SET); //seek to the start of the file

    //allocate a buffer to store the rom
    char *buffer = (char*)malloc(sizeof(char)*rom_size);
    if(buffer == nullptr){
        std::cout << "Failed to allocate memory for ROM" << std::endl;
    }

    //if the allocation was successful, then copy the ROM file contents into the buffer
    fread(buffer, sizeof(char), (size_t)rom_size, fp);

    //once the contents of ROM are stored in the buffer, load it into the chip memory after 0x200 or after 512 bytes of memory
    //and this is only possible if rom_size is less than 4096 - 512
    if(rom_size < (4096-512)){
        for(int i=0; i<rom_size; i++){
            memory[512+i] = (uint8_t)buffer[i];
        }
    }
    else{
        std::cerr << "ROM file size is too large, cannot fit into memory." << std::endl;
    }

    //we have set up everything from 0x000 to 0x200 in the init function and
    //we have also loaded our ROM into memory from 0x200 to 0xFFF.
    //all, we have to do is execute this loaded memory with the help of program counter by moving it back and forth

    //close the file and free the buffer to prevent memory leaks
    fclose(fp);
    free(buffer);

    //as everything went fine, return true
    return 0;
}

//shamelessly copied this giant switch statement from https://github.com/JamesGriffin/CHIP-8-Emulator
//fetch and execute instructions from 0x200 to 0x4096 from memory using program counter
void CPU::execute(){
    //on CHIP 8, each instruction is 2 bytes long
    
    //the program counter is currently at 0x200, fetch instructions from 0x200 with the help of program counter
    //memory[pc] << 8, fetches the instruction from pc position and left shifts it and then adds the next instruction
    //for example, if pc = 0x304 and at that address if the instruction is 0010 1011 and at 0x305 if the instruction is 0111 1011
    //then these 2 bytes will be stored in 16 bit opcode variable as follows
    //if, opcode = 0000 0000 0000 0000
    //memory[pc] << 8 will make the opcode look like 0010 1011 0000 0000
    //and performing OR with memory[pc+1] will give  0010 1011 0111 1011
    //in this way, we will fetch two bytes of instructions at a time into the opcode variable
    //opcode = memory[pc] << 8 | memory[pc+1];

    //or we can do this directly as follows
    opcode = memory[pc + 0]; //fetch the opcode from pc
    opcode = opcode << 8; //left shift it
    opcode = opcode | memory[pc+1]; //fetch the next opcode and OR it with the next 8 bits of opcode

    //decode and execute the fetched instruction from memory using the following giant switch statement
    switch (opcode & 0xF000)
    {
        case 0x0000:
            switch(opcode & 0x000F){
                //0x00E0 - Clear Screen
                case 0x0000:
                    for(int i=0; i<64*32; i++){
                        frame[i] = 0;
                    }
                    drawFlag = true;
                    pc += 2;
                    break;
                
                //0x00EE - return from subroutine
                case 0x000E:
                    --sp;
                    pc = stack[sp];
                    pc += 2;
                    break;
                
                default:
                    printf("Invalid opcode : %.4x\n", opcode);
                    exit(1);
            }
            break;
        
        //0x1NNN - jumps to NNN address
        case 0x1000:
            pc = opcode & 0x0FFF; //decode the opcode using 0xFFF, so that we get the 12 bits
            break;

        //0x2NNN - call the subroutine at NNN
        case 0x2000:
            stack[sp] = pc;
            ++sp;
            pc = opcode & 0x0FFF; //decode the opcode using 0xFFF, so that we get the 12 bits
            break;
        
        //0x3NNN - if vx == nn, then skip the next instruction
        //here x in Vx is the first 8 bits of opcode. it can be obtained by right shifting 8 bits
        case 0x3000:
            if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                pc += 4; //skip the next two bytes or next instructions
            else
                pc += 2; //do not skip, if vx != nn
            break;
        
        //0x4NNN - if vx != nn, then skip the next instruction
        case 0x4000:
            if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                pc += 4;
            else
                pc += 2;
            break;
        
        //0x5XY0 - skip the next instruction if vx = vy
        case 0x5000:
            if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
                pc += 4;
            else
                pc += 2;
            break;

        // 0x6XNN - Sets VX to NN.
        case 0x6000:
            V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            pc += 2;
            break;

        // 0x7XNN - Adds NN to VX.
        case 0x7000:
            V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            pc += 2;
            break;

        // 0x8XY_
        case 0x8000:
            switch (opcode & 0x000F) {

                // 0x8XY0 - Set VX to the value of VY.
                case 0x0000:
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // 0x8XY1 - Set VX to (VX | VY).
                case 0x0001:
                    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // 0x8XY2 - Set VX to (VX & VY).
                case 0x0002:
                    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // 0x8XY3 - Sets VX to (VX ^ VY).
                case 0x0003:
                    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // 0x8XY4 - Add VY to VX. when there's a carry, set VF to 1
                // and to 0 when there isn't.
                case 0x0004:
                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                        V[0xF] = 1; //carry
                    else
                        V[0xF] = 0;
                    pc += 2;
                    break;

                // 0x8XY5 - subtract VY from VX. set VF 0 when
                // there's a borrow, and 1 when there isn't.
                case 0x0005:
                    if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
                        V[0xF] = 0; // there is a borrow
                    else
                        V[0xF] = 1;
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // 0x8XY6 - Shifts VX right by one. VF is set to the value of
                // the least significant bit of VX before the shift.
                case 0x0006:
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
                    V[(opcode & 0x0F00) >> 8] >>= 1;
                    pc += 2;
                    break;

                // 0x8XY7: Sets VX to VY minus VX. VF is set to 0 when there's
                // a borrow, and 1 when there isn't.
                case 0x0007:
                    if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])	// VY-VX
                        V[0xF] = 0; // there is a borrow
                    else
                        V[0xF] = 1;
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                // 0x8XYE: Shifts VX left by one. VF is set to the value of
                // the most significant bit of VX before the shift.
                case 0x000E:
                    V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    pc += 2;
                    break;

                default:
                    printf("\nUnknown op code: %.4X\n", opcode);
                    exit(3);
            }
            break;

        // 0x9XY0 - Skips the next instruction if VX != VY.
        case 0x9000:
            if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
                pc += 4;
            else
                pc += 2;
            break;

        // ANNN - Sets I to the address NNN.
        case 0xA000:
            I = opcode & 0x0FFF;
            pc += 2;
            break;

        // BNNN - Jumps to the address NNN plus V0.
        case 0xB000:
            pc = (opcode & 0x0FFF) + V[0];
            break;

        // CXNN - Sets VX to a random number, masked by NN.
        case 0xC000:
            V[(opcode & 0x0F00) >> 8] = (rand() % (0xFF + 1)) & (opcode & 0x00FF);
            pc += 2;
            break;

        
        // DXYN: Draw a sprite at coordinate (VX, VY) that has a width of 8 and height of N pixels
        // Each row of 8 pixels is read as bit-coded starting from memory
        // location I;
        // I value doesn't change after the execution of this instruction.
        // VF is set to 1 if any screen pixels are flipped from set to unset
        // when the sprite is drawn, and to 0 if that doesn't happen.

        case 0xD000:
        {
            unsigned short x = V[(opcode & 0x0F00) >> 8];
            unsigned short y = V[(opcode & 0x00F0) >> 4];
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            V[0xF] = 0;
            for (int yline = 0; yline < height; yline++)
            {
                pixel = memory[I + yline];
                for(int xline = 0; xline < 8; xline++)
                {
                    if((pixel & (0x80 >> xline)) != 0)
                    {
                        if(frame[(x + xline + ((y + yline) * 64))] == 1)
                        {
                            V[0xF] = 1;
                        }
                        frame[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }

            drawFlag = true;
            pc += 2;
        }
            break;

        // EX__
        case 0xE000:

            switch (opcode & 0x00FF) {
                // EX9E - Skips the next instruction if the key stored
                // in VX is pressed.
                case 0x009E:
                    if (keypad[V[(opcode & 0x0F00) >> 8]] != 0)
                        pc +=  4;
                    else
                        pc += 2;
                    break;

                // EXA1 - Skips the next instruction if the key stored
                // in VX isn't pressed.
                case 0x00A1:
                    if (keypad[V[(opcode & 0x0F00) >> 8]] == 0)
                        pc +=  4;
                    else
                        pc += 2;
                    break;

                default:
                    printf("\nUnknown op code: %.4X\n", opcode);
                    exit(3);
            }
            break;

        // FX__
        case 0xF000:
            switch(opcode & 0x00FF)
            {
                // FX07 - Sets VX to the value of the delay timer
                case 0x0007:
                    V[(opcode & 0x0F00) >> 8] = dt;
                    pc += 2;
                    break;

                // FX0A - A key press is awaited, and then stored in VX
                case 0x000A:
                {
                    bool key_pressed = false;

                    for(int i = 0; i < 16; ++i)
                    {
                        if(keypad[i] != 0)
                        {
                            V[(opcode & 0x0F00) >> 8] = i;
                            key_pressed = true;
                        }
                    }

                    // If no key is pressed, return and try again.
                    if(!key_pressed)
                        return;

                    pc += 2;
                }
                    break;

                // FX15 - Sets the delay timer to VX
                case 0x0015:
                    dt = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                // FX18 - Sets the sound timer to VX
                case 0x0018:
                    st = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                // FX1E - Adds VX to I
                case 0x001E:
                    // VF is set to 1 when range overflow (I+VX>0xFFF), and 0
                    // when there isn't.
                    if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    I += V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                // FX29 - Sets I to the location of the sprite for the
                // character in VX. Characters 0-F (in hexadecimal) are
                // represented by a 4x5 font
                case 0x0029:
                    I = V[(opcode & 0x0F00) >> 8] * 0x5;
                    pc += 2;
                    break;

                // FX33 - Stores the Binary-coded decimal representation of VX
                // at the addresses I, I plus 1, and I plus 2
                case 0x0033:
                    memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
                    memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
                    pc += 2;
                    break;

                // FX55 - Stores V0 to VX in memory starting at address I
                case 0x0055:
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                        memory[I + i] = V[i];

                    // On the original interpreter, when the
                    // operation is done, I = I + X + 1.
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;

                case 0x0065:
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                        V[i] = memory[I + i];

                    // On the original interpreter,
                    // when the operation is done, I = I + X + 1.
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;

                default:
                    printf ("Unknown opcode [0xF000]: 0x%X\n", opcode);
            }
            break;

        default:
            printf("\nUnimplemented opcode: %.4X\n", opcode);
            exit(3);
    }

    // Update timers
    if (dt > 0){
        --dt;
    }

    if (st > 0){
        if(st == 1){
            //Implement sound using SDL here
        }
        --st;
    }
}