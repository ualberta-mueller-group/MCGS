#pragma once
#include <vector>
#include <string>
#include <limits>
#include "cgt_basics.h"

// split string on whitespace
std::vector<std::string> split_string(const std::string& str);

bool is_int(const std::string& str);

// test under/overflow for addition
template <class T>
bool addition_wraps(const T& x, const T& y)
{
    const T& min = std::numeric_limits<T>::min();
    const T& max = std::numeric_limits<T>::max();

    if (x > 0 && y > 0)
    {
        return y > (max - x);
    } else if (x < 0 && y < 0)
    {
        return y < (min - x);
    }

    return false;
}

relation relation_from_search_results(bool le_known, bool is_le, bool ge_known, bool is_ge);
