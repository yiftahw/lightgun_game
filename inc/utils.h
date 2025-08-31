#pragma once

#include <optional>
#include "util_concepts.h"

template <AssociativeContainer MapType>
std::optional<typename MapType::mapped_type> optional_map_get(const MapType &map, const typename MapType::key_type &key)
{
    auto it = map.find(key);
    if (it == map.end())
    {
        return std::nullopt;
    }
    return it->second;
}
