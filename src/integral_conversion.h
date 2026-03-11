/*
    Utilities for doing (checked) integral casts.

    Type traits:
        typename get_widest_as_unsigned_t<Int1, Int2>
            Get the unsigned version of the largest given integral type.

        struct integral_cast_traits<Int_To, Int_From>
            Info about the static_cast: `Int_From` -> `Int_To`

    Casting/checking functions:
        bool static_cast_is_safe<Int_To>(Int_From from);
            True IFF `static_cast<Int_To>(from)` is safe

        Integral_Unsigned as_unsigned_unsafe<Integral>(val);
            static_cast `val` to Integral_Unsigned.
            Potentially unsafe for release builds (failure triggers an assert).

        Integral_Unsigned as_unsigned_checked<Integral>(val);
            static_cast `val` to Integral_Unsigned.
            Safe for release builds (failure throws an exception).

        Int_To integral_cast_unsafe<Int_To>(Int_From from)
            static_cast `from` to Int_To.
            Potentially unsafe for release builds (failure triggers an assert).

        Int_To integral_cast_checked<Int_To>(Int_From from)
            static_cast `from` to Int_To.
            Safe for release builds (failure throws an exception).
*/
#pragma once

#include <climits>
#include <type_traits>
#include <stdexcept>
#include <cassert>

////////////////////////////////////////////////// Custom type traits
// NOLINTBEGIN(readability-identifier-naming)

//////////////////////////////////////// get_widest_as_unsigned_t<Int1, Int2>
// Given 2 integral types, returns the unsigned version of the widest one

// Default case using SFINAE
template <class Integral1_T, class Integral2_T, class Enable = void>
struct get_widest_as_unsigned
{
    static_assert(std::is_integral_v<Integral1_T>);
    static_assert(std::is_integral_v<Integral2_T>);

    using type = std::make_unsigned_t<Integral1_T>;
};

// When Int2 is larger than Int1
template <class Integral1_T, class Integral2_T>
struct get_widest_as_unsigned<
    Integral1_T,
    Integral2_T,
    std::enable_if_t<
        sizeof(Integral1_T) < sizeof(Integral2_T),
        void
    >
>
{
    static_assert(std::is_integral_v<Integral1_T>);
    static_assert(std::is_integral_v<Integral2_T>);

    using type = std::make_unsigned_t<Integral2_T>;
};

// Helper
template <class Integral1_T, class Integral2_T>
using get_widest_as_unsigned_t =
    typename get_widest_as_unsigned<Integral1_T, Integral2_T>::type;

//////////////////////////////////////// integral_cast_traits<To, From>
// Traits for casting integral types

template <class To_Integral_T, class From_Integral_T>
struct integral_cast_traits
{
    static_assert(std::is_integral_v<From_Integral_T>);
    static_assert(std::is_integral_v<To_Integral_T>);

    static constexpr bool is_widening =
        sizeof(From_Integral_T) < sizeof(To_Integral_T);

    static constexpr bool is_narrowing = sizeof(From_Integral_T) >
                                         sizeof(To_Integral_T);

    static constexpr bool is_same_width =
        sizeof(From_Integral_T) == sizeof(To_Integral_T);

    static constexpr size_t to_integral_t_bits =
        sizeof(To_Integral_T) * CHAR_BIT;

    static constexpr size_t from_integral_t_bits =
        sizeof(From_Integral_T) * CHAR_BIT;

    using widest_unsigned_t =
        get_widest_as_unsigned_t<To_Integral_T, From_Integral_T>;
};

// NOLINTEND(readability-identifier-naming)

////////////////////////////////////////////////// Implementation details
// NOLINTBEGIN(readability-identifier-naming)
namespace __integral_conversion_impl {
/*
    u -> u
        Widening/Same: OK.
        Narrowing: Check that `From` fits into `To` using bit mask.
*/
template <class To_Integral_T, class From_Integral_T>
inline constexpr bool static_cast_is_safe_u_to_u(From_Integral_T from)
{
    static_assert(std::is_integral_v<From_Integral_T>);
    static_assert(std::is_unsigned_v<From_Integral_T>);
    static_assert(std::is_integral_v<To_Integral_T>);
    static_assert(std::is_unsigned_v<To_Integral_T>);

    using traits = integral_cast_traits<To_Integral_T, From_Integral_T>;

    if constexpr (traits::is_narrowing)
    {
        using wide_t = typename traits::widest_unsigned_t;

        const wide_t from_wide = (wide_t) from;
        constexpr wide_t MASK = (wide_t(-1) << traits::to_integral_t_bits);

        return (from_wide & MASK) == 0;
    }

    return true;
}

/*
    i -> u
        Widening/Same: Check that `From` >= 0.
        Narrowing: Check that `From` >= 0 AND that `From` fits into `To`
            using bit mask.
*/
template <class To_Integral_T, class From_Integral_T>
inline constexpr bool static_cast_is_safe_i_to_u(From_Integral_T from)
{
    static_assert(std::is_integral_v<From_Integral_T>);
    static_assert(std::is_signed_v<From_Integral_T>);
    static_assert(std::is_integral_v<To_Integral_T>);
    static_assert(std::is_unsigned_v<To_Integral_T>);

    if (from < 0)
        return false;

    using traits = integral_cast_traits<To_Integral_T, From_Integral_T>;

    if constexpr (traits::is_narrowing)
    {
        using wide_t = typename traits::widest_unsigned_t;

        const wide_t from_wide = (wide_t) from;
        constexpr wide_t MASK = (wide_t(-1) << traits::to_integral_t_bits);

        return (from_wide & MASK) == 0;
    }

    return true;
}

/*
    u -> i
        Widening/Same/Narrowing: Check that `From` uses at most
            `(TO_BITS - 1)` bits, using bit mask.
*/
template <class To_Integral_T, class From_Integral_T>
inline constexpr bool static_cast_is_safe_u_to_i(From_Integral_T from)
{
    static_assert(std::is_integral_v<From_Integral_T>);
    static_assert(std::is_unsigned_v<From_Integral_T>);
    static_assert(std::is_integral_v<To_Integral_T>);
    static_assert(std::is_signed_v<To_Integral_T>);

    using traits = integral_cast_traits<To_Integral_T, From_Integral_T>;
    using wide_t = typename traits::widest_unsigned_t;

    const wide_t from_wide = (wide_t) from;
    constexpr wide_t MASK = (wide_t(-1) << (traits::to_integral_t_bits - 1));

    return (from_wide & MASK) == 0;
}

/*
    i -> i
        Widening/Same: OK.
        Narrowing:
            If `From` >= 0: Check that `From` uses at most `(TO_BITS - 1)` bits,
                using bit mask.
            If `From` < 0:
                Check that `~From` uses at most `(TO_BITS - 1)` bits, using
                bit mask.

*/
template <class To_Integral_T, class From_Integral_T>
inline constexpr bool static_cast_is_safe_i_to_i(From_Integral_T from)
{
    static_assert(std::is_integral_v<From_Integral_T>);
    static_assert(std::is_signed_v<From_Integral_T>);
    static_assert(std::is_integral_v<To_Integral_T>);
    static_assert(std::is_signed_v<To_Integral_T>);

    using traits = integral_cast_traits<To_Integral_T, From_Integral_T>;

    if constexpr (traits::is_narrowing)
    {
        using wide_t = typename traits::widest_unsigned_t;

        const wide_t from_wide = (from < 0) ? ~((wide_t) from) : (wide_t) from;
        constexpr wide_t MASK = (wide_t(-1) << (traits::to_integral_t_bits - 1));

        return (from_wide & MASK) == 0;
    }

    return true;
}

// NOLINTEND(readability-identifier-naming)

} // namespace __integral_conversion_impl

////////////////////////////////////////////////// Casting functions
template <class To_Integral_T, class From_Integral_T>
inline constexpr bool static_cast_is_safe(From_Integral_T from)
{
    static_assert(std::is_integral_v<To_Integral_T>);
    static_assert(std::is_integral_v<From_Integral_T>);

    constexpr bool FROM_IS_UNSIGNED = std::is_unsigned_v<From_Integral_T>;
    constexpr bool TO_IS_UNSIGNED = std::is_unsigned_v<To_Integral_T>;

    using namespace __integral_conversion_impl;

    if constexpr (FROM_IS_UNSIGNED)
    {
        if constexpr (TO_IS_UNSIGNED)
            return static_cast_is_safe_u_to_u<To_Integral_T>(from);
        else
            return static_cast_is_safe_u_to_i<To_Integral_T>(from);
    }
    else
    {
        if constexpr (TO_IS_UNSIGNED)
            return static_cast_is_safe_i_to_u<To_Integral_T>(from);
        else
            return static_cast_is_safe_i_to_i<To_Integral_T>(from);
    }
}

// Cast integral value to its unsigned version. No safety in release builds
template <class Integral_T>
inline constexpr std::make_unsigned_t<Integral_T> as_unsigned_unsafe(
    Integral_T val_at_least_0)
{
    static_assert(std::is_integral_v<Integral_T>);
    static_assert(std::is_signed_v<Integral_T>);

    assert(val_at_least_0 >= 0);
    return static_cast<std::make_unsigned_t<Integral_T>>(val_at_least_0);
}

// Cast integral value to its unsigned version. Throws in release builds
template <class Integral_T>
inline constexpr std::make_unsigned_t<Integral_T> as_unsigned_checked(
    Integral_T val_at_least_0)
{
    static_assert(std::is_integral_v<Integral_T>);
    static_assert(std::is_signed_v<Integral_T>);

    if (!(val_at_least_0 >= 0))
        throw std::domain_error("as_unsigned_checked argument < 0!");

    return static_cast<std::make_unsigned_t<Integral_T>>(val_at_least_0);
}

template <class To_Integral_T, class From_Integral_T>
inline constexpr To_Integral_T integral_cast_unsafe(From_Integral_T from)
{
    static_assert(std::is_integral_v<To_Integral_T> &&
                  std::is_integral_v<From_Integral_T>);

    assert(static_cast_is_safe<To_Integral_T>(from));
    return static_cast<To_Integral_T>(from);
}

template <class To_Integral_T, class From_Integral_T>
inline constexpr To_Integral_T integral_cast_checked(From_Integral_T from)
{
    static_assert(std::is_integral_v<To_Integral_T> &&
                  std::is_integral_v<From_Integral_T>);

    if (!static_cast_is_safe<To_Integral_T>(from))
        throw std::domain_error("integral_cast_checked argument not safe!");

    return static_cast<To_Integral_T>(from);
}
