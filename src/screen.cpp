#include "screen.h"

std::string point2d::to_string() const
{
    return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
}

Screen::Screen(SDL_Window *window, SDL_Renderer *renderer) : window(window), renderer(renderer)
{
}

Screen::~Screen()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

Screen *Screen::create(const char *title, int width, int height, float scale)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
        return nullptr;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        SDL_DestroyWindow(window);
        return nullptr;
    }

    SDL_RenderSetScale(renderer, scale, scale);

    return new Screen(window, renderer);
}

void Screen::add_pixel(point2d point)
{
    points.push_back(point);
}

void Screen::clear_pixels()
{
    points.clear();
}

void Screen::clear_screen()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void Screen::render_screen()
{
    clear_screen();

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (const point2d &point : points)
    {
        SDL_RenderDrawPointF(renderer, point.x, point.y);
    }
    SDL_RenderPresent(renderer);
}

void Screen::input()
{
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            SDL_Quit();
            exit(0);
        }
    }
}
