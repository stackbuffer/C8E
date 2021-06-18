#pragma once

#include <SDL2/SDL.h>

class RenderWindow{
public:
    SDL_Window *window;
    SDL_Renderer *renderer;
    RenderWindow(const char *p_title, int p_w, int p_h);
    void render(SDL_Texture *p_texture);
    void updateTexture(SDL_Texture *p_texture, uint32_t pixels[]);
    void display();
    void clear();
    void cleanUp();
};