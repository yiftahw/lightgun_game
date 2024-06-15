#include <thread>
#include <stdio.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <tuple>
#include <SDL2/SDL.h>
#include "DataAcqHTTP.h"
#include "screen.h"
#include "Snapshot.h"
#include "consts.h"
#include "PointMapping.h"
#include "DataAcqPlayback.h"

enum class playback_mode
{
    CURSOR,
    DEBUG
};

enum class data_acq_mode
{
    HTTP_RECORD,
    HTTP_LIVE,
    PLAYBACK
};

// record raw data from the HTTP server
void record(IDataAcq *data_acq, std::string file_name, uint32_t samples, uint8_t fps)
{
    std::ofstream output(file_name, std::ios::out);
    if (!output.is_open())
    {
        printf("Failed to open file %s\n", file_name.c_str());
        return;
    }

    while (samples-- > 0)
    {
        auto snapshot = data_acq->get();
        output << snapshot.to_string() << std::endl;
        std::cout << snapshot.to_string() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps));
    }

    output.close();
}

void play(IDataAcq *data_acq, Screen *screen, screen_constants constants, playback_mode mode)
{
    while (true)
    {
        auto snapshot = data_acq->get();

        if (mode == playback_mode::DEBUG)
        {
            printf("Snapshot: %s\n", snapshot.to_string().c_str());
            
            std::vector<PointF> mapped_points(dfrobot_snapshot_size);
            for (auto &point : snapshot.points)
            {
                auto mapped = map_dfrobot_to_screen(point, constants);
                mapped_points.push_back(mapped);
            }

            screen->clear_pixels();
            for (auto &point : mapped_points)
            {
                screen->add_pixel({point.x, point.y});
            }
        }
        else // playback_mode::CURSOR
        {
            auto pt = map_snapshot_to_screen(snapshot, constants);
            if (pt.x == 0 && pt.y == 0)
            {
                continue;
            }

            screen->clear_pixels();
            screen->add_pixel({pt.x, pt.y});
        }

        screen->render_screen();
        screen->input();
    }
}

std::tuple<Screen*, screen_constants> init_screen()
{
    if (0 != SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return {nullptr, screen_constants(0, 0, 1.0f)};
    }

    SDL_DisplayMode dm;
    if (0 != SDL_GetDesktopDisplayMode(0, &dm))
    {
        SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
        return {nullptr, screen_constants(0, 0, 1.0f)};
    }
    printf("Display mode: %dx%d\n", dm.w, dm.h);

    auto width = 0.95 * dm.w;
    auto height = dm.h - (dm.w - width);
    printf("Width: %.2f, Height: %.2f\n", width, height);

    auto screen = Screen::create("Hello, World!", width, height);
    if (screen == nullptr)
    {
        printf("Failed to create screen!\n");
        return {nullptr, screen_constants(0, 0, 1.0f)};
    }

    screen_constants constants(width, height, 1.0f);
    return {screen, constants};
}


int main(int argc, char** argv)
{
    // SDL library uses macros to bypass our main function and it calls our main function
    // this causes our main function to must have argc and argv parameters
    // to bypass the "unused parameter" warning from "Werror" flag, we can use this line
    if (argc == 0 && argv == nullptr) {}

    data_acq_mode acq_mode = data_acq_mode::HTTP_LIVE;
    playback_mode cursor_mode = playback_mode::CURSOR;

    IDataAcq *data_acq = nullptr;
    if (acq_mode == data_acq_mode::PLAYBACK)
    {
        auto playback_acq = new DataAcqPlayback("raw_data.txt", 30);
        if (!playback_acq->is_open())
        {
            return -1;
        }
        data_acq = playback_acq;
    }
    else
    {
        data_acq = new DataAcqHTTP("10.100.102.34:80");
    }

    if (acq_mode == data_acq_mode::HTTP_RECORD)
    {
        record(data_acq, "record.txt", 1200, 60);
    }
    else
    {
        auto [screen, constants] = init_screen();
        if (screen == nullptr)
        {
            return -1;
        }

        play(data_acq, screen, constants, cursor_mode);
    }

    return 0;
}
