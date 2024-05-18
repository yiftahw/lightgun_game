#include <thread>
#include <stdio.h>
#include <iostream>
#include <SDL2/SDL.h>
#include "DataAcqHTTP.h"
#include "screen.h"
#include "Snapshot.h"
#include "DataAcqSim.h"
#include <cmath>
#include "consts.h"
#include "PointMapping.h"

int main(int argc, char** argv)
{
    // SDL library uses macros to bypass our main function and it calls our main function
    // this causes our main function to must have argc and argv parameters
    // to bypass the "unused parameter" warning from "Werror" flag, we can use this line
    if (argc == 0 && argv == nullptr) {}

    if (0 != SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return -1;
    }

    SDL_DisplayMode dm;
    if (0 != SDL_GetDesktopDisplayMode(0, &dm))
    {
        SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
        return -1;
    }
    printf("Display mode: %dx%d\n", dm.w, dm.h);

    auto width = 0.95 * dm.w;
    auto height = dm.h - (dm.w - width);
    printf("Width: %.2f, Height: %.2f\n", width, height);

    auto screen = Screen::create("Hello, World!", width, height);
    if (screen == nullptr)
    {
        printf("Failed to create screen!\n");
        return -1;
    }

    screen_constants constants(width, height, 1.0f);

    DataAcqHTTP client("10.100.102.34:80");
    while (true)
    {
        auto snapshot = client.get();
        auto pt = map_snapshot_to_screen(snapshot, constants);
        //printf("Snapshot: %s -> %s\n", snapshot.to_string().c_str(), pt.to_string().c_str());

        if (pt.x == 0 && pt.y == 0)
        {
            //printf("Invalid point\n");
            continue;
        }

        screen->clear_pixels();
        screen->add_pixel({pt.x, pt.y});

        screen->render_screen();
        screen->input();
    }

    return 0;
}
