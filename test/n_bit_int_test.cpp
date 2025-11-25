#include "n_bit_int_test.h"

#include <iostream>
#include <cassert>

#include "n_bit_int.h"

using namespace std;

namespace {

////////////////////////////////////////////////// Helpers

////////////////////////////////////////////////// Main tests
//////////////////////////////////////// Test limits

// u2 limits
static_assert((n_bit_int::min_val<2, INT_UNSIGNED>()) == 0);
static_assert((n_bit_int::max_val<2, INT_UNSIGNED>()) == 3);

// i2 limits
static_assert((n_bit_int::min_val<2, INT_SIGNED>()) == -2);
static_assert((n_bit_int::max_val<2, INT_SIGNED>()) == 1);
static_assert((n_bit_int::sign_bit_idx<2>()) == 1);
static_assert((n_bit_int::sign_bit_mask<2>()) == 0b10);
static_assert((n_bit_int::value_mask<2>()) == 0b11);

// u3 limits
static_assert((n_bit_int::min_val<3, INT_UNSIGNED>()) == 0);
static_assert((n_bit_int::max_val<3, INT_UNSIGNED>()) == 7);

// i3 limits
static_assert((n_bit_int::min_val<3, INT_SIGNED>()) == -4);
static_assert((n_bit_int::max_val<3, INT_SIGNED>()) == 3);
static_assert((n_bit_int::sign_bit_idx<3>()) == 2);
static_assert((n_bit_int::sign_bit_mask<3>()) == 0b100);
static_assert((n_bit_int::value_mask<3>()) == 0b111);

// u4 limits
static_assert((n_bit_int::min_val<4, INT_UNSIGNED>()) == 0);
static_assert((n_bit_int::max_val<4, INT_UNSIGNED>()) == 15);

// i4 limits
static_assert((n_bit_int::min_val<4, INT_SIGNED>()) == -8);
static_assert((n_bit_int::max_val<4, INT_SIGNED>()) == 7);
static_assert((n_bit_int::sign_bit_idx<4>()) == 3);
static_assert((n_bit_int::sign_bit_mask<4>()) == 0b1000);
static_assert((n_bit_int::value_mask<4>()) == 0b1111);

// u7 limits
static_assert((n_bit_int::min_val<7, INT_UNSIGNED>()) == 0);
static_assert((n_bit_int::max_val<7, INT_UNSIGNED>()) == 127);

// i7 limits
static_assert((n_bit_int::min_val<7, INT_SIGNED>()) == -64);
static_assert((n_bit_int::max_val<7, INT_SIGNED>()) == 63);
static_assert((n_bit_int::sign_bit_idx<7>()) == 6);
static_assert((n_bit_int::sign_bit_mask<7>()) == 0b1000000);
static_assert((n_bit_int::value_mask<7>()) == 0b1111111);

// u31 limits
static_assert((n_bit_int::min_val<31, INT_UNSIGNED>()) == 0);
static_assert((n_bit_int::max_val<31, INT_UNSIGNED>()) == 2147483647);

// i31 limits
static_assert((n_bit_int::min_val<31, INT_SIGNED>()) == -1073741824);
static_assert((n_bit_int::max_val<31, INT_SIGNED>()) == 1073741823);
static_assert((n_bit_int::sign_bit_idx<31>()) == 30);
static_assert((n_bit_int::sign_bit_mask<31>()) == 0x40000000);
static_assert((n_bit_int::value_mask<31>()) == 0x7FFFFFFF);

//////////////////////////////////////// Test conversions

template <int n_bits, signed_type_enum signed_type>
void test_conversions()
{
    const int low = n_bit_int::min_val<n_bits, signed_type>();
    const int high = n_bit_int::max_val<n_bits, signed_type>();

    for (int i = low; i <= high; i++)
    {
        const int encoded = n_bit_int::shrink_int_to_n_bits<n_bits, signed_type>(i);

        const int decoded =
            n_bit_int::expand_int_from_n_bits<n_bits, signed_type>(encoded);

        assert(i == decoded);
    }
}
} // namespace

void n_bit_int_test_all()
{
    std::cout << __FILE__ << std::endl;

    test_conversions<2, INT_UNSIGNED>();
    test_conversions<2, INT_SIGNED>();

    test_conversions<3, INT_UNSIGNED>();
    test_conversions<3, INT_SIGNED>();

    test_conversions<7, INT_UNSIGNED>();
    test_conversions<7, INT_SIGNED>();
}

