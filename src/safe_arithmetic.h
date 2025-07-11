#pragma once
#include <limits>
#include <type_traits>
#include "utilities.h"
#include <cstdint>
#include <cassert>

// Some functions in this file assume two's complement (i.e. for bit shifts)
static_assert(int32_t(-1) == int32_t(0xFFFFFFFF), "Not two's complement");

static_assert(std::numeric_limits<int>::min() < 0);
static_assert(std::numeric_limits<int>::max() > 0);
static_assert(std::numeric_limits<int>::min() + 1 ==
              -std::numeric_limits<int>::max());

//////////////////////////////////////// arithmetic wrapping checks

template <class T> // NUMERIC
constexpr bool add_is_safe(const T& x, const T& y)
{
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);

    const T& min = std::numeric_limits<T>::min();
    const T& max = std::numeric_limits<T>::max();

    if (x > 0 && y > 0)
    {
        return x <= (max - y); // (x + y) <= max
    }
    else if (x < 0 && y < 0)
    {
        return x >= (min - y); // (x + y) >= min
    }

    return true;
}

template <class T> // NUMERIC
constexpr bool subtract_is_safe(const T& x, const T& y)
{
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);

    const T& min = std::numeric_limits<T>::min();
    const T& max = std::numeric_limits<T>::max();

    if (y > 0)
    {
        return x >= (min + y); // (x - y) >= min
    }
    else if (y < 0)
    {
        return x <= (max + y); // (x - y) <= max
    }

    return true;
}

template <class T> // SIGNED INTEGRAL
inline constexpr bool negate_is_safe(const T& x)
{
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_signed_v<T>);
    return x != std::numeric_limits<T>::min();
}

//////////////////////////////////////// safe arithmetic operations
// These should all be safe even with invalid arguments

template <class T> // NUMERIC
inline constexpr bool safe_add(T& x, const T& y)
{
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);

    if (!add_is_safe(x, y))
        return false;

    x += y;
    return true;
}

template <class T> // SIGNED INTEGRAL
inline constexpr bool safe_add_negatable(T& x, const T& y)
{
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_signed_v<T>);

    if (!add_is_safe(x, y))
        return false;

    T result = x + y;

    if (!negate_is_safe(result))
        return false;

    x = result;
    return true;
}

template <class T> // NUMERIC
inline constexpr bool safe_subtract(T& x, const T& y)
{
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);

    if (!subtract_is_safe(x, y))
        return false;

    x -= y;
    return true;
}

template <class T> // SIGNED INTEGRAL
inline constexpr bool safe_subtract_negatable(T& x, const T& y)
{
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_signed_v<T>);

    if (!subtract_is_safe(x, y))
        return false;

    T result = x - y;

    if (!negate_is_safe(result))
        return false;

    x = result;
    return true;
}

template <class T> // SIGNED INTEGRAL
constexpr bool safe_negate(T& x)
{
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_signed_v<T>);

    if (!negate_is_safe(x))
    {
        return false;
    }

    assert(abs(x) == abs(-x));
    x = -x;
    return true;
}

// if successful, result is negatable
template <class T1, class T2> // SIGNED INTEGRAL and INTEGRAL
constexpr bool safe_mul2_shift(T1& shiftee, const T2& exponent)
{
    static_assert(std::is_integral_v<T1> && std::is_integral_v<T2>);
    static_assert(std::is_signed_v<T1>);

    const T2 n_bits = size_in_bits<T1>();

    if (exponent >= n_bits || exponent < 0 || !negate_is_safe(shiftee))
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
    // NOLINTNEXTLINE(readability-identifier-naming)
    using T1_Uns = typename std::make_unsigned<T1>::type;
    T1_Uns mask_uns(-1);
    assert(n_bits >= 1 + exponent); // no underflow on next line
    // (exponent + 1) most significant bits; prevents wrapping AND sign flip
    mask_uns <<= (n_bits - 1 - exponent);
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

template <class T> // SIGNED INTEGRAL
constexpr bool safe_pow2_mod(T& x, const T& mod)
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
