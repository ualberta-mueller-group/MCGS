/*
   Converts C++ int values to and from arbitrary (smaller) widths. Returned
   N bit int values are still of type int, but only use the N least
   significant bits.

   Used to encode/decode move parts.
*/
#pragma once

#include <climits>

// N bit ints must have widths in the interval [2, N_BIT_INT_MAX_BITS]
#define N_BIT_INT_MAX_BITS 31

static_assert(N_BIT_INT_MAX_BITS < sizeof(int) * CHAR_BIT);

//////////////////////////////////////////////////
enum signed_type_enum
{
    INT_UNSIGNED = 0,
    INT_SIGNED,
};

////////////////////////////////////////////////// utility functions
namespace n_bit_int {

// Get the index specifying the sign bit for an N bit int
template <int n_bits>
inline constexpr int sign_bit_idx()
{
    static_assert(2 <= n_bits && n_bits <= N_BIT_INT_MAX_BITS);
    return n_bits - 1;
}

// Get the bit mask of the sign bit for an N bit int
template <int n_bits>
inline constexpr int sign_bit_mask()
{
    static_assert(2 <= n_bits && n_bits <= N_BIT_INT_MAX_BITS);
    return 1 << sign_bit_idx<n_bits>();
}

// Get the bit mask for the N least significant bits, for an N bit int
template <int n_bits>
inline constexpr int value_mask()
{
    static_assert(2 <= n_bits && n_bits <= N_BIT_INT_MAX_BITS);
    return ~((unsigned int)(-1) << n_bits);
}

// Maximum value of an N bit int (can be signed or unsigned)
template <int n_bits, signed_type_enum signed_type>
inline constexpr int max_val()
{
    static_assert(2 <= n_bits && n_bits <= N_BIT_INT_MAX_BITS);

    switch (signed_type)
    {
        case INT_UNSIGNED:
            return value_mask<n_bits>();
        case INT_SIGNED:
            return sign_bit_mask<n_bits>() - 1;
    }
}

// Minimum value of an N bit int (can be signed or unsigned)
template <int n_bits, signed_type_enum signed_type>
inline constexpr int min_val()
{
    static_assert(2 <= n_bits && n_bits <= N_BIT_INT_MAX_BITS);

    switch (signed_type)
    {
        case INT_UNSIGNED:
            return 0;
        case INT_SIGNED:
            return -max_val<n_bits, signed_type>() - 1;
    }
}

// Convert a regular C++ int to an N bit int
template <int n_bits, signed_type_enum signed_type>
inline constexpr int shrink_int_to_n_bits(int val)
{
    static_assert(2 <= n_bits && n_bits <= N_BIT_INT_MAX_BITS);

    assert(                                        //
        (min_val<n_bits, signed_type>()) <= val && //
        val <= (max_val<n_bits, signed_type>())    //
    );                                             //

    return val & value_mask<n_bits>();
}

// Convert an N bit int to a regular C++ int
template <int n_bits, signed_type_enum signed_type>
inline constexpr int expand_int_from_n_bits(int n_bit_val)
{
    static_assert(2 <= n_bits && n_bits <= N_BIT_INT_MAX_BITS);

    assert(n_bit_val == (n_bit_val & value_mask<n_bits>()));

    if constexpr (signed_type == INT_SIGNED)
    {
        // Check sign bit
        const bool is_negative = (n_bit_val & sign_bit_mask<n_bits>()) != 0;

        // Sign extend by prepending 1s into most significant bits
        if (is_negative)
            n_bit_val |= ~value_mask<n_bits>();
    }

    return n_bit_val;
}

} // namespace n_bit_int

