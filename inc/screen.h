#pragma once

#include <SDL2/SDL.h>
#include <vector>

struct point2d
{
    float x;
    float y;
};

class Screen
{
public:
    static Screen *create(const char *title, int width, int height);
    ~Screen();

    void add_pixel(point2d point);
    void clear_pixels();
    void render_screen();
    void input();

private:
    Screen(SDL_Window *window, SDL_Renderer *renderer);
    void clear_screen();
    SDL_Event event;
    SDL_Window *window;
    SDL_Renderer *renderer;
    std::vector<point2d> points;

};
