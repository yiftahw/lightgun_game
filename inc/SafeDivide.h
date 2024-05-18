#pragma once

#include <exception>

template <typename T>
T SafeDivide(T numerator, T denominator)
{
    if (denominator == 0)
    {
        throw std::runtime_error("Division by zero");
    }
    return numerator / denominator;
}
