#include <thread>
#include "DataAcqPlayback.h"

DataAcqPlayback::DataAcqPlayback(std::string file_name, uint8_t fps) :
    input(file_name),
    fps(fps)
{
    if (!input.is_open())
    {
        printf("Failed to open file %s\n", file_name.c_str());
    }
}

DataAcqPlayback::~DataAcqPlayback()
{
    if (input.is_open())
    {
        input.close();
    }
}

Snapshot DataAcqPlayback::get()
{
    // to simulate the requested fps, we sleep here
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps));

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
