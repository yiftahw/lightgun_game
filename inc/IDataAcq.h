#pragma once

#include "Snapshot.h"

class IDataAcq
{
public:
    virtual ~IDataAcq() = default;
    virtual Snapshot get() = 0;
};
