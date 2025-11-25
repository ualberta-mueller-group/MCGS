#include "n_bit_int_test.h"

#include <iostream>
#include <cassert>

#include "n_bit_int.h"

using namespace std;


namespace {
void test_new_move_stuff()
{
    cout << __FILE__ << endl;

    // u4 limits
    assert((n_bit_int::min_val<4, INT_UNSIGNED>()) == 0);
    assert((n_bit_int::max_val<4, INT_UNSIGNED>()) == 15);

    // i4 limits
    assert((n_bit_int::min_val<4, INT_SIGNED>()) == -8);
    assert((n_bit_int::max_val<4, INT_SIGNED>()) == 7);

    // u7 limits
    assert((n_bit_int::min_val<7, INT_UNSIGNED>()) == 0);
    assert((n_bit_int::max_val<7, INT_UNSIGNED>()) == 127);

    // i7 limits
    assert((n_bit_int::min_val<7, INT_SIGNED>()) == -64);
    assert((n_bit_int::max_val<7, INT_SIGNED>()) == 63);

    // Shrink/expand
    const int n_bits = 7;

    const int u_low = n_bit_int::min_val<n_bits, INT_UNSIGNED>();
    const int u_high = n_bit_int::max_val<n_bits, INT_UNSIGNED>();

    const int i_low = n_bit_int::min_val<n_bits, INT_SIGNED>();
    const int i_high = n_bit_int::max_val<n_bits, INT_SIGNED>();

    // Unsigned range
    cout << "Test u" << n_bits << endl;

    for (int x = u_low; x <= u_high; x++)
    {
        const int enc =
            n_bit_int::shrink_int_to_n_bits<n_bits, INT_UNSIGNED>(x);

        const int dec =
            n_bit_int::expand_int_from_n_bits<n_bits, INT_UNSIGNED>(enc);

        cout << x << " " << enc << " " << dec << endl;
        assert(x == dec);
    }

    // Signed range
    cout << "Test i" << n_bits << endl;

    for (int x = i_low; x <= i_high; x++)
    {
        const int enc = n_bit_int::shrink_int_to_n_bits<n_bits, INT_SIGNED>(x);

        const int dec =
            n_bit_int::expand_int_from_n_bits<n_bits, INT_SIGNED>(enc);

        cout << x << " " << enc << " " << dec << endl;
        assert(x == dec);
    }


}
} // namespace

void n_bit_int_test_all()
{
    cout << "TODO write this test" << __FILE__ << endl;
}

