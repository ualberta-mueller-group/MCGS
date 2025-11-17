#include "cgt_move_new.h"
#include <iostream>

using namespace std;


//////////////////////////////////////////////////
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

    // Test move2
    {
        const int low1 = n_bit_int::min_val<16, INT_SIGNED>();
        const int high1 = n_bit_int::max_val<16, INT_SIGNED>();

        const int low2 = n_bit_int::min_val<15, INT_UNSIGNED>();
        const int high2 = n_bit_int::max_val<15, INT_UNSIGNED>();

        unsigned int checksum = 0;
        for (int p1 = low1; p1 <= high1; p1++)
        {
            for (int p2 = low2; p2 <= high2; p2++)
            {
                const ::move m = cgt_move_new::move2_create(p1, p2);

                int p1_dec, p2_dec;
                cgt_move_new::move2_unpack(m, p1_dec, p2_dec);
                checksum += p1_dec + p2_dec;

                //cout << p1 << ' ' << p2 << '\n';
                assert(p1 == p1_dec);
                assert(p2 == p2_dec);
                assert(0 == (m & (1 << 31)));
            }
        }

        cout << checksum << endl;
    }

    // Test move3
    {
        const int low1 = n_bit_int::min_val<11, INT_SIGNED>();
        const int high1 = n_bit_int::max_val<11, INT_SIGNED>();

        const int low2 = n_bit_int::min_val<10, INT_UNSIGNED>();
        const int high2 = n_bit_int::max_val<10, INT_UNSIGNED>();

        const int low3 = n_bit_int::min_val<10, INT_UNSIGNED>();
        const int high3 = n_bit_int::max_val<10, INT_UNSIGNED>();

        unsigned int checksum = 0;
        for (int p1 = low1; p1 <= high1; p1++)
        {
            for (int p2 = low2; p2 <= high2; p2++)
            {
                for (int p3 = low3; p3 <= high3; p3++)
                {
                    const ::move m = cgt_move_new::move3_create(p1, p2, p3);

                    int p1_dec, p2_dec, p3_dec;
                    cgt_move_new::move3_unpack(m, p1_dec, p2_dec, p3_dec);
                    checksum += p1_dec + p2_dec + p3_dec;

                    //cout << p1 << ' ' << p2 << ' ' << p3 << '\n';
                    assert(p1 == p1_dec);
                    assert(p2 == p2_dec);
                    assert(p3 == p3_dec);
                    assert(0 == (m & (1 << 31)));
                }
            }
        }

        cout << checksum << endl;
    }

}
