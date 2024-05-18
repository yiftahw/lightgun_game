#pragma once

#include <cstdint>
#include "IDataAcq.h"

class DataAcqSim : public IDataAcq
{
public:
    DataAcqSim();
    ~DataAcqSim();

    Snapshot get() override;

private:
    uint16_t counter;
};
