#pragma once
#include <limits>
#include <type_traits>
#include "utilities.h"

// Some functions in this file assume two's complement (i.e. for bit shifts)
static_assert(int32_t(-1) == int32_t(0xFFFFFFFF), "Not two's complement");

static_assert(std::numeric_limits<int>::min() < 0);
static_assert(std::numeric_limits<int>::max() > 0);
static_assert(std::numeric_limits<int>::min() + 1 == -std::numeric_limits<int>::max());

//////////////////////////////////////// arithmetic wrapping checks
template <class T>
bool add_will_wrap(const T& x, const T& y) // NUMERIC
{
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);

    const T& min = std::numeric_limits<T>::min();
    const T& max = std::numeric_limits<T>::max();

    if (x > 0 && y > 0)
    {
        return x > (max - y); // (x + y) > max
    } else if (x < 0 && y < 0)
    {
        return x < (min - y); // (x + y) < min
    }

    return false;
}

template <class T>
bool subtract_will_wrap(const T& x, const T& y) // NUMERIC
{
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);

    const T& min = std::numeric_limits<T>::min();
    const T& max = std::numeric_limits<T>::max();

    if (y > 0)
    {
        return x < (min + y); // (x - y) < min
    } else if (y < 0)
    {
        return x > (max + y); // (x - y) > max
    }

    return false;
}


template <class T>
inline bool negate_will_wrap(const T& x) // SIGNED INTEGRAL
{
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_signed_v<T>);
    return x == std::numeric_limits<T>::min();
}

//////////////////////////////////////// safe arithmetic operations
// These should all be safe even with invalid arguments

template <class T>
inline bool safe_add(T& x, const T& y) // NUMERIC
{
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);

    if (add_will_wrap(x, y))
        return false;

    x += y;
    return true;
}

template <class T>
inline bool safe_add_negatable(T& x, const T& y) // SIGNED INTEGRAL
{
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_signed_v<T>);

    if (add_will_wrap(x, y))
        return false;

    T result = x + y;

    if (negate_will_wrap(result))
        return false;

    x = result;
    return true;
}

template <class T>
inline bool safe_subtract(T& x, const T& y) // NUMERIC
{ 
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);

    if (subtract_will_wrap(x, y))
        return false;

    x -= y;
    return true;
}

template <class T>
inline bool safe_subtract_negatable(T& x, const T& y) // SIGNED INTEGRAL
{
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_signed_v<T>);

    if (subtract_will_wrap(x, y))
        return false;

    T result = x - y;

    if (negate_will_wrap(result))
        return false;

    x = result;
    return true;
}

template <class T>
bool safe_negate(T& x) // SIGNED INTEGRAL
{
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_signed_v<T>);

    if (negate_will_wrap(x))
    {
        return false;
    }

    assert(abs(x) == abs(-x));
    x = -x;
    return true;
}

// if successful, result is negatable
template <class T1, class T2>
bool safe_mul2_shift(T1& shiftee, const T2& exponent) // SIGNED INTEGRAL and INTEGRAL
{
    static_assert(std::is_integral_v<T1> && std::is_integral_v<T2>);
    static_assert(std::is_signed_v<T1>);

    const T2 n_bits = size_in_bits<T1>();

    if (exponent >= n_bits || exponent < 0 || negate_will_wrap(shiftee))
        return false;
    if (exponent == 0)
        return true;

    T1 x = shiftee;
    bool flipped = false;
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
    mask_uns <<= (n_bits - 1 - exponent); // (exponent + 1) most significant bits; prevents wrapping AND sign flip

    const T1& mask = reinterpret_cast<const T1&>(mask_uns);

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

template <class T>
bool safe_pow2_mod(T& x, const T& mod) // SIGNED INTEGRAL
{
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_signed_v<T>);

    const T MIN = std::numeric_limits<T>::min();
    const T MAX = std::numeric_limits<T>::max();
    static_assert(MIN < 0 && MAX > 0 && MIN < -MAX);

    if (mod <= 0 || !is_power_of_2(mod))
    {
        return false;
    }

    if (x >= 0)
    {
        x = x & (mod - 1);
        return true;
    }

    if (x == MIN)
    {
        x += mod;
        assert(x > MIN && x < 0);
    }

    x = -((-x) & (mod - 1));
    return true;
}

