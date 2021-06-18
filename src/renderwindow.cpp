#include <iostream>
#include <SDL2/SDL.h>
#include "renderwindow.hpp"

//constructor
RenderWindow::RenderWindow(const char *p_title, int p_w, int p_h){
    window = nullptr;
    renderer = nullptr;

    //create the window
    window = SDL_CreateWindow(p_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, p_w, p_h, SDL_WINDOW_SHOWN);

    if(window == nullptr){
        std::cout << "cannot create window. " << SDL_GetError() << std::endl;
    }
    
    //create the renderer on the window
    renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetLogicalSize(renderer, p_w, p_h);
}


void RenderWindow::render(SDL_Texture *p_texture){
    SDL_RenderCopy(renderer, p_texture, nullptr, nullptr);
}

void RenderWindow::updateTexture(SDL_Texture *p_texture, uint32_t pixels[]){
    SDL_UpdateTexture(p_texture, nullptr, pixels, 64*sizeof(uint32_t));
}

void RenderWindow::display(){
    SDL_RenderPresent(renderer);
}

void RenderWindow::clear(){
    SDL_RenderClear(renderer);
}

void RenderWindow::cleanUp(){
    SDL_DestroyWindow(window);
}