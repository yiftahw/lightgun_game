#include <cerrno>
#include <cstring>
#include <thread>
#include "DataAcqPlayback.h"

DataAcqPlayback::DataAcqPlayback(std::string file_name, uint8_t fps) :
    input(file_name),
    fps(fps)
{
    if (!input.is_open())
    {
        printf("Failed to open file %s\n", file_name.c_str());
        printf("%s\n", std::strerror(errno));
    }
}

DataAcqPlayback::~DataAcqPlayback()
{
    if (input.is_open())
    {
        input.close();
    }
}

Snapshot DataAcqPlayback::get(bool no_sleep)
{
    // to simulate the requested fps, we sleep here
    // after profiling, the mapping takes around 70 microseconds
    // so we can sleep for the remaining time

    if (!no_sleep)
    {
        auto sleep_time = std::chrono::milliseconds(1000 / fps) - std::chrono::microseconds(70);
        std::this_thread::sleep_for(sleep_time);
    }

    if (!input.is_open())
    {
        return Snapshot::invalid();
    }

    if (input.eof())
    {
        input.clear();
        input.seekg(0, std::ios::beg);
    }

    if (!std::getline(input, line))
    {
        return Snapshot::invalid();
    }

    return snapshot_from_string(line);
}

Snapshot DataAcqPlayback::get()
{
    return get(false);
}

bool DataAcqPlayback::is_open()
{
    return input.is_open();
}
