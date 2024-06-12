#pragma once

#include <string>
#include <fstream>
#include "IDataAcq.h"

class DataAcqPlayback : public IDataAcq
{
public:
    DataAcqPlayback(std::string file_name, uint8_t fps);
    ~DataAcqPlayback();

    Snapshot get() override;
    bool is_open();

private:
    std::ifstream input;
    std::string line;
    uint8_t fps;
};
