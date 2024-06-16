#include "screen.h"

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

void Screen::add_pixel(SDL_FPoint point)
{
    points.push_back(point);
}

void Screen::add_segment(std::pair<SDL_FPoint, SDL_FPoint> segment)
{
    segments.push_back(segment);
}

void Screen::clear_pixels()
{
    points.clear();
}

void Screen::clear_segments()
{
    segments.clear();
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
    //SDL_RenderDrawPointsF(renderer, points.data(), points.size());
    
    for (const auto &segment : segments)
    {
        SDL_RenderDrawLineF(renderer, segment.first.x, segment.first.y, segment.second.x, segment.second.y);
    }

    for (const auto &point : points)
    {
        // create a rectangle and fill it with white color
        constexpr float rect_size = 6.0F;
        SDL_FRect rect = {point.x - (rect_size / 2.0F), point.y - (rect_size / 2.0F), rect_size, rect_size};
        SDL_RenderFillRectF(renderer, &rect);
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
