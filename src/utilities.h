#pragma once
#include <climits>
#include <type_traits>
#include <vector>
#include <cassert>
#include <string>
#include "cgt_basics.h"
#include <cstdint>
#include <cstddef>
#include <ostream>

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

// like Python's string split()
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

template <class T>
inline constexpr T rotate_right(const T& val, size_t distance)
{
    static_assert(std::is_integral_v<T>);
    assert(distance >= 0);

    // NOLINTNEXTLINE(readability-identifier-naming)
    using T_Unsigned = typename std::make_unsigned_t<T>;
    const T_Unsigned& val_unsigned = reinterpret_cast<const T_Unsigned&>(val);
    constexpr size_t N_BITS = sizeof(T_Unsigned) * CHAR_BIT;

    if (distance <= 0)
        return val;

    return (val_unsigned >> distance) | (val_unsigned << (N_BITS - distance));
}

template <class T>
inline constexpr T rotate_left(const T& val, size_t distance)
{
    static_assert(std::is_integral_v<T>);
    assert(distance >= 0);

    // NOLINTNEXTLINE(readability-identifier-naming)
    using T_Unsigned = typename std::make_unsigned_t<T>;
    const T_Unsigned& val_unsigned = reinterpret_cast<const T_Unsigned&>(val);
    constexpr size_t N_BITS = sizeof(T_Unsigned) * CHAR_BIT;

    distance %= N_BITS;

    if (distance <= 0)
        return val;

    return (val_unsigned << distance) | (val_unsigned >> (N_BITS - distance));
}

// ... 0101 0101
template <class T>
constexpr T alternating_mask()
{
    static_assert(std::is_integral_v<T>);
    static_assert(CHAR_BIT == 8);

    constexpr uint8_t BYTE_MASK = 0x55; // 0101 0101
    constexpr size_t SIZE = sizeof(T);

    T val = BYTE_MASK;

    // fix (false positive) compiler warning about shift overflow
    if constexpr (SIZE > 1)
    {
        for (size_t i = 1; i < SIZE; i++)
        {
            val <<= 8;
            val |= BYTE_MASK;
        }
    }

    return val;
}

template <class T>
constexpr T rotate_interleaved(const T& val, size_t distance)
{
    static_assert(std::is_integral_v<T>);

    constexpr T MASK1 = alternating_mask<T>();
    constexpr T MASK2 = MASK1 << 1;

    T val1 = rotate_left(val & MASK1, distance);
    T val2 = rotate_right(val & MASK2, distance);

    return val1 | val2;
}

uint64_t ms_since_epoch();

size_t new_vector_capacity(size_t access_idx, size_t current_capacity);

////////////////////////////////////////
relation relation_from_search_results(bool le_known, bool is_le, bool ge_known,
                                      bool is_ge);
