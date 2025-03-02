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
    mask <<= (n_bits - 1 - shift_amount); // Also makes sure sign doesn't flip

    return (mask & shiftee) != T1(0);
}

template <class T1, class T2>
bool safe_left_shift(T1& shiftee, const T2& shift_amount)
{
    if (left_shift_will_wrap(shiftee, shift_amount))
    {
        return false;
    }

    shiftee <<= shift_amount;
    return true;
}

template <class T>
bool safe_negate(T& x)
{
    static_assert(std::is_signed_v<T>);

    if (x == std::numeric_limits<T>::min())
    {
        return false;
    }

    assert(abs(x) == abs(-x));
    x = -x;
    return true;
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


static_assert(int32_t(-1) == int32_t(0xFFFFFFFF), "Not two's complement");

template <class T>
inline bool is_power_of_2(const T& n)
{
    static_assert(std::is_integral_v<T>);
    return n > 0 && !(n & (n - 1));
}

template <class T>
T pow2_mod(const T& x, const T& mod)
{
    static_assert(std::is_integral_v<T>);
    assert(mod > 0);
    assert(is_power_of_2(mod));

    if (x >= 0)
    {
        return x & (mod - 1);
    }

    assert(x != std::numeric_limits<T>::min());
    return -((-x) & (mod - 1));
}

template <class T1, class T2>
bool safe_mul2_shift(T1& shiftee, const T2& exponent)
{
    static_assert(std::is_integral_v<T1> && std::is_integral_v<T2>);
    const T2 n_bits = sizeof(T1) * CHAR_BIT;

    if (exponent == 0)
        return true;
    if (exponent >= n_bits || shiftee == std::numeric_limits<T1>::min())
        return false;

    bool flipped = false;
    T1 x = shiftee;
    if (x < 0)
    {
        assert(abs(x) == abs(-x));
        flipped = true;
        x = -x;
    }

    // Because shifting negative values is undefined:
    using T1_uns = typename std::make_unsigned<T1>::type; // NOLINT
    T1_uns mask_uns(-1);
    assert(n_bits >= 1 + exponent); // no underflow on next line
    mask_uns <<= (n_bits - 1 - exponent); // prevents wrapping AND sign flip

    const T1& mask = reinterpret_cast<T1&>(mask_uns);

    assert(x >= 0);
    if ((mask & x) != 0)
    {
        return false;
    }

    x <<= exponent;
    assert(x >= 0);

    if (flipped)
    {
        assert(abs(x) == abs(-x));
        x = -x;
        assert(x < 0);
    }

    shiftee = x;
    return true;
}


relation relation_from_search_results(bool le_known, bool is_le, bool ge_known, bool is_ge);

bool string_starts_with(const std::string& str, const std::string& word);
bool string_ends_with(const std::string& str, const std::string& word);
