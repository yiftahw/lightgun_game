#include <stdio.h>
#include <SDL2/SDL.h>
#include "screen.h"

void example()
{
    auto screen = Screen::create("Hello, World!", 800, 600);
    if (screen == nullptr)
    {
        printf("Failed to create screen!\n");
        return;
    }

    screen->add_pixel({0, 0});
    screen->add_pixel({800, 600});

    while (true)
    {
        screen->render_screen();
        screen->input();
    }
}

int main(int argv, char** args)
{
    printf("Hello, World!\n");

   example();

    return 0;
}
