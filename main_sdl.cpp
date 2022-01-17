#ifndef ARDUINO

#include <SDL.h>
#include <stdlib.h>

#include "game.hpp"

static constexpr int ZOOM = 6;

SDL_Window* window;

uint8_t wait_btn()
{
    SDL_Event e;
    for(;;)
    {
        if(!SDL_WaitEvent(&e) || e.type == SDL_QUIT)
        {
            SDL_DestroyWindow(window);
            SDL_Quit();
            exit(0);
        }
        if(e.type == SDL_KEYDOWN)
        {
            switch(e.key.keysym.scancode)
            {
            case SDL_SCANCODE_UP   : return BTN_UP;
            case SDL_SCANCODE_DOWN : return BTN_DOWN;
            case SDL_SCANCODE_LEFT : return BTN_LEFT;
            case SDL_SCANCODE_RIGHT: return BTN_RIGHT;
            case SDL_SCANCODE_A    : return BTN_A;
            case SDL_SCANCODE_B    : return BTN_B;
            default: break;
            }
        }
    }
}

void paint_left()
{
    SDL_Surface* s = SDL_GetWindowSurface(window);
    Uint32 const COLOR_WHITE = SDL_MapRGB(s->format, 255, 255, 255);
    Uint32 const COLOR_BLACK = SDL_MapRGB(s->format, 0, 0, 0);
    for(int i = 0; i < 512; ++i)
    {
        int x = i % 64;
        int y = (i / 64) * 8;
        uint8_t b = buf[i];
        for(int j = 0; j < 8; ++j, b >>= 1)
        {
            Uint32 color = (b & 1) ? COLOR_WHITE : COLOR_BLACK;
            SDL_Rect r = { ZOOM * x, ZOOM * (y + j), ZOOM, ZOOM };
            SDL_FillRect(s, &r, color);
        }
    }
    SDL_UpdateWindowSurface(window);
}

void paint_right()
{
    SDL_Surface* s = SDL_GetWindowSurface(window);
    Uint32 const COLOR_WHITE = SDL_MapRGB(s->format, 255, 255, 255);
    Uint32 const COLOR_BLACK = SDL_MapRGB(s->format, 0, 0, 0);
    for(int i = 0; i < 512; ++i)
    {
        int x = i % 64 + 64;
        int y = (i / 64) * 8;
        uint8_t b = buf[i];
        for(int j = 0; j < 8; ++j, b >>= 1)
        {
            Uint32 color = (b & 1) ? COLOR_WHITE : COLOR_BLACK;
            SDL_Rect r = { ZOOM * x, ZOOM * (y + j), ZOOM, ZOOM };
            SDL_FillRect(s, &r, color);
        }
    }
    SDL_UpdateWindowSurface(window);
}

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
        "ardurogue",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        ZOOM * 128,
        ZOOM * 64,
        0
    );

    game_setup();
    for(;;)
        game_loop();

    return 0;
}

#endif
