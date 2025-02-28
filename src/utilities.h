#pragma once
#include <climits>
#include <type_traits>
#include <vector>
#include <string>
#include <limits>
#include "cgt_basics.h"

// split string on whitespace
std::vector<std::string> split_string(const std::string& str);

bool is_int(const std::string& str);

// test under/overflow for addition
template <class T>
bool addition_will_wrap(const T& x, const T& y)
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

template <class T>
inline bool safe_add(T& x, const T& y)
{
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);

    if (addition_will_wrap(x, y))
    {
        return false;
    }

    x += y;
    return true;
}

// TODO is there a faster way to do this? Also should remove signedness?
template <class T1, class T2>
bool left_shift_will_wrap(const T1& shiftee, const T2& shift_amount)
{
    const T2 n_bits = sizeof(T1) * CHAR_BIT;

    if (shift_amount >= n_bits)
    {
        return true;
    }

    if (shift_amount == 0)
    {
        return false;
    }

    T1 mask = T1(-1);
    mask <<= (n_bits - shift_amount);

    return (mask & shiftee) != T1(0);
}



template <class T>
void print_bits(std::ostream& os, const T& x)
{
    static_assert(std::is_integral_v<T>);

    const int n_bits = sizeof(T) * CHAR_BIT;

    // T could be signed, so we can't iteratively shift right from 
    // the most significant bit
    const T mask = T(1);

    for (int i = 0; i < n_bits; i++)
    {
        os << (((mask << (n_bits - 1 - i)) & x) != 0);
    }
}



relation relation_from_search_results(bool le_known, bool is_le, bool ge_known, bool is_ge);

bool string_starts_with(const std::string& str, const std::string& word);
bool string_ends_with(const std::string& str, const std::string& word);
