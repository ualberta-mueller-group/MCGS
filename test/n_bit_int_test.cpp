#include "n_bit_int_test.h"

#include <cassert>

#include "n_bit_int.h"
#include "int_generator.h"

using namespace std;

namespace {

////////////////////////////////////////////////// Helpers

////////////////////////////////////////////////// Main tests
//////////////////////////////////////// Test limits

////////////////////////////// 1 bit
// u1 limits
static_assert((n_bit_int::min_val<1, INT_UNSIGNED>()) == 0);
static_assert((n_bit_int::max_val<1, INT_UNSIGNED>()) == 1);

// i1 limits
static_assert((n_bit_int::min_val<1, INT_SIGNED>()) == -1);
static_assert((n_bit_int::max_val<1, INT_SIGNED>()) == 0);
static_assert((n_bit_int::sign_bit_idx<1>()) == 0);
static_assert((n_bit_int::sign_bit_mask<1>()) == 0b1);
static_assert((n_bit_int::value_mask<1>()) == 0b1);


////////////////////////////// 2 bits
// u2 limits
static_assert((n_bit_int::min_val<2, INT_UNSIGNED>()) == 0);
static_assert((n_bit_int::max_val<2, INT_UNSIGNED>()) == 3);

// i2 limits
static_assert((n_bit_int::min_val<2, INT_SIGNED>()) == -2);
static_assert((n_bit_int::max_val<2, INT_SIGNED>()) == 1);
static_assert((n_bit_int::sign_bit_idx<2>()) == 1);
static_assert((n_bit_int::sign_bit_mask<2>()) == 0b10);
static_assert((n_bit_int::value_mask<2>()) == 0b11);

////////////////////////////// 3 bits
// u3 limits
static_assert((n_bit_int::min_val<3, INT_UNSIGNED>()) == 0);
static_assert((n_bit_int::max_val<3, INT_UNSIGNED>()) == 7);

// i3 limits
static_assert((n_bit_int::min_val<3, INT_SIGNED>()) == -4);
static_assert((n_bit_int::max_val<3, INT_SIGNED>()) == 3);
static_assert((n_bit_int::sign_bit_idx<3>()) == 2);
static_assert((n_bit_int::sign_bit_mask<3>()) == 0b100);
static_assert((n_bit_int::value_mask<3>()) == 0b111);

////////////////////////////// 4 bits
// u4 limits
static_assert((n_bit_int::min_val<4, INT_UNSIGNED>()) == 0);
static_assert((n_bit_int::max_val<4, INT_UNSIGNED>()) == 15);

// i4 limits
static_assert((n_bit_int::min_val<4, INT_SIGNED>()) == -8);
static_assert((n_bit_int::max_val<4, INT_SIGNED>()) == 7);
static_assert((n_bit_int::sign_bit_idx<4>()) == 3);
static_assert((n_bit_int::sign_bit_mask<4>()) == 0b1000);
static_assert((n_bit_int::value_mask<4>()) == 0b1111);

////////////////////////////// 7 bits
// u7 limits
static_assert((n_bit_int::min_val<7, INT_UNSIGNED>()) == 0);
static_assert((n_bit_int::max_val<7, INT_UNSIGNED>()) == 127);

// i7 limits
static_assert((n_bit_int::min_val<7, INT_SIGNED>()) == -64);
static_assert((n_bit_int::max_val<7, INT_SIGNED>()) == 63);
static_assert((n_bit_int::sign_bit_idx<7>()) == 6);
static_assert((n_bit_int::sign_bit_mask<7>()) == 0b1000000);
static_assert((n_bit_int::value_mask<7>()) == 0b1111111);

////////////////////////////// 31 bits
// u31 limits
static_assert((n_bit_int::min_val<31, INT_UNSIGNED>()) == 0);
static_assert((n_bit_int::max_val<31, INT_UNSIGNED>()) == 2147483647);

// i31 limits
static_assert((n_bit_int::min_val<31, INT_SIGNED>()) == -1073741824);
static_assert((n_bit_int::max_val<31, INT_SIGNED>()) == 1073741823);
static_assert((n_bit_int::sign_bit_idx<31>()) == 30);
static_assert((n_bit_int::sign_bit_mask<31>()) == 0x40000000);
static_assert((n_bit_int::value_mask<31>()) == 0x7FFFFFFF);


////////////////////////////// 32 bits
// u32 limits
static_assert((n_bit_int::min_val<32, INT_UNSIGNED>()) == 0);
static_assert((n_bit_int::max_val<32, INT_UNSIGNED>()) == 4294967295);

// i32 limits
static_assert((n_bit_int::min_val<32, INT_SIGNED>()) == -2147483648);
static_assert((n_bit_int::max_val<32, INT_SIGNED>()) == 2147483647);
static_assert((n_bit_int::sign_bit_idx<32>()) == 31);
static_assert((n_bit_int::sign_bit_mask<32>()) == 0x80000000);
static_assert((n_bit_int::value_mask<32>()) == 0xFFFFFFFF);

////////////////////////////// 41 bits
// u41 limits
static_assert((n_bit_int::min_val<41, INT_UNSIGNED>()) == 0);
static_assert((n_bit_int::max_val<41, INT_UNSIGNED>()) == 2199023255551);

// i41 limits
static_assert((n_bit_int::min_val<41, INT_SIGNED>()) == -1099511627776);
static_assert((n_bit_int::max_val<41, INT_SIGNED>()) == 1099511627775);
static_assert((n_bit_int::sign_bit_idx<41>()) == 40);
static_assert((n_bit_int::sign_bit_mask<41>()) == 0x10000000000);
static_assert((n_bit_int::value_mask<41>()) == 0x1FFFFFFFFFF);

////////////////////////////// 62 bits
// u62 limits
static_assert((n_bit_int::min_val<62, INT_UNSIGNED>()) == 0);
static_assert((n_bit_int::max_val<62, INT_UNSIGNED>()) == 4611686018427387903);

// i62 limits
static_assert((n_bit_int::min_val<62, INT_SIGNED>()) == -2305843009213693952);
static_assert((n_bit_int::max_val<62, INT_SIGNED>()) == 2305843009213693951);
static_assert((n_bit_int::sign_bit_idx<62>()) == 61);
static_assert((n_bit_int::sign_bit_mask<62>()) == 0x2000000000000000);
static_assert((n_bit_int::value_mask<62>()) == 0x3FFFFFFFFFFFFFFF);

////////////////////////////// 63 bits
// u63 limits
static_assert((n_bit_int::min_val<63, INT_UNSIGNED>()) == 0);
static_assert((n_bit_int::max_val<63, INT_UNSIGNED>()) == 9223372036854775807);

// i63 limits
static_assert((n_bit_int::min_val<63, INT_SIGNED>()) == -4611686018427387904);
static_assert((n_bit_int::max_val<63, INT_SIGNED>()) == 4611686018427387903);
static_assert((n_bit_int::sign_bit_idx<63>()) == 62);
static_assert((n_bit_int::sign_bit_mask<63>()) == 0x4000000000000000);
static_assert((n_bit_int::value_mask<63>()) == 0x7FFFFFFFFFFFFFFF);




//////////////////////////////////////// Test conversions

template <int n_bits, signed_type_enum signed_type>
void test_conversions()
{
    const int64_t low = n_bit_int::min_val<n_bits, signed_type>();
    const int64_t high = n_bit_int::max_val<n_bits, signed_type>();

    int_generator<int64_t> gen(low, high, 100'000);
    assert(gen);

    while (gen)
    {
        const int64_t val = gen.gen_int();
        ++gen;

        const int64_t encoded = n_bit_int::shrink_int_to_n_bits<n_bits, signed_type>(val);

        const int64_t decoded =
            n_bit_int::expand_int_from_n_bits<n_bits, signed_type>(encoded);

        assert(val == decoded);
    }
}
} // namespace

void n_bit_int_test_all()
{
    test_conversions<1, INT_UNSIGNED>();
    test_conversions<1, INT_SIGNED>();

    test_conversions<2, INT_UNSIGNED>();
    test_conversions<2, INT_SIGNED>();

    test_conversions<3, INT_UNSIGNED>();
    test_conversions<3, INT_SIGNED>();

    test_conversions<4, INT_UNSIGNED>();
    test_conversions<4, INT_SIGNED>();

    test_conversions<7, INT_UNSIGNED>();
    test_conversions<7, INT_SIGNED>();

    test_conversions<31, INT_UNSIGNED>();
    test_conversions<31, INT_SIGNED>();

    test_conversions<41, INT_UNSIGNED>();
    test_conversions<41, INT_SIGNED>();

    test_conversions<62, INT_UNSIGNED>();
    test_conversions<62, INT_SIGNED>();

    test_conversions<63, INT_UNSIGNED>();
    test_conversions<63, INT_SIGNED>();
}

