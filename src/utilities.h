#pragma once
#include <climits>
#include <type_traits>
#include <vector>
#include <string>
#include "cgt_basics.h"

//////////////////////////////////////// general utility functions
template <class T>
inline constexpr size_t size_in_bits(const T& expr)
{
    return sizeof(expr) * CHAR_BIT;
}

template <class T>
inline constexpr size_t size_in_bits()
{
    return sizeof(T) * CHAR_BIT;
}

template <class T>
void print_bits(std::ostream& os, const T& x)
{
    static_assert(std::is_integral_v<T>);

    const size_t n_bits = size_in_bits<T>();

    // T could be signed, so we can't iteratively shift right from
    // the most significant bit
    const T mask = T(1);

    for (size_t i = 0; i < n_bits; i++)
    {
        os << (((mask << (n_bits - 1 - i)) & x) != 0);
    }
}

//////////////////////////////////////// string parsing utils
std::vector<std::string> split_string(const std::string& str);

bool is_int(const std::string& str);

bool string_starts_with(const std::string& str, const std::string& word);
bool string_ends_with(const std::string& str, const std::string& word);

//////////////////////////////////////// arithmetic operations
template <class T>
inline constexpr bool is_power_of_2(const T& n)
{
    static_assert(std::is_integral_v<T>);
    return n > 0 && !(n & (n - 1));
}

////////////////////////////////////////
relation relation_from_search_results(bool le_known, bool is_le, bool ge_known,
                                      bool is_ge);
