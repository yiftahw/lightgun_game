#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <string>

class Screen
{
public:
    static Screen *create(const char *title, int width, int height, float scale = 1.0f);
    ~Screen();

    void add_pixel(SDL_FPoint point);
    void clear_pixels();
    void render_screen();
    void input();

private:
    Screen(SDL_Window *window, SDL_Renderer *renderer);
    void clear_screen();
    SDL_Event event;
    SDL_Window *window;
    SDL_Renderer *renderer;
    std::vector<SDL_FPoint> points;

};
