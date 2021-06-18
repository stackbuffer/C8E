#include <iostream>
#include <SDL2/SDL.h>
#include <windows.h>
#include <thread>
#include <chrono>
#include "cpu.hpp"
#include "renderwindow.hpp"

//set the keymap 
uint8_t keymap[16] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v,
};

int main(int argc, char *argv[]){
    if(argc != 2){
        std::cout << "Usage : main <ROM file>" << std::endl;
        return 1;
    }

    CPU cpu = CPU(); //create the CPU object

    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cout << "Could not initialize SDL. " << SDL_GetError();
        exit(1);
    }

    RenderWindow window = RenderWindow("CHIP-8 Emulator in C++", 1024, 512);

    SDL_Texture *texture = SDL_CreateTexture(window.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

    //temporary buffer for pixels
    uint32_t pixels[2048];

    if(cpu.loadROM(argv[1]) == -1)
        return 2;
    
    //execution loop
    while(true){
        cpu.execute();

        SDL_Event e;

        while(SDL_PollEvent(&e)){
            if(e.type == SDL_QUIT){
                exit(0);
            }

            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE)
                    exit(0);

                for (int i = 0; i < 16; ++i) {
                    if (e.key.keysym.sym == keymap[i]) {
                        cpu.keypad[i] = 1;
                    }
                }
            }
            
            if (e.type == SDL_KEYUP) {
                for (int i = 0; i < 16; ++i) {
                    if (e.key.keysym.sym == keymap[i]) {
                        cpu.keypad[i] = 0;
                    }
                }
            }
        }

        //if drawFlag is set to true, re-render the SDL screen
        if(cpu.drawFlag){
            cpu.drawFlag = false; //set back to false

            //store frame buffer in our temporary pixel buffer
            for(int i=0; i<64*32; i++){
                uint8_t pixel = cpu.frame[i];
                pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
            }

            //update the texture with the new pixels
            window.updateTexture(texture, pixels);

            //clear the screen
            window.clear();
            //render the texture
            window.render(texture);
            //display it
            window.display();

        }

        //to slow down the emulation, I had to use this function. 
        //For some reason sleep function from std::thread is not working on my windows 10.
        Sleep(1);

        //comment the above line and uncomment the below line, if you are on linux
        //std::this_thread::sleep_for(std::chrono::microseconds(1000));
    }

    window.cleanUp();
    SDL_Quit();

    return 0;
}
