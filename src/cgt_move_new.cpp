#include "cgt_move_new.h"
#include <iostream>

#define TEST_LOOP(var, n_bits, signed_type, body)                              \
    for (int var = n_bit_int::min_val<n_bits, signed_type>();                  \
         var <= n_bit_int::max_val<n_bits, signed_type>(); var++)              \
    {                                                                          \
        body                                                                   \
    }

using namespace std;

namespace {

void test_move1()
{
    unsigned int checksum = 0;

    TEST_LOOP(p1, 31, INT_SIGNED, {
        const ::move m = cgt_move_new::move1_create(p1);

        int p1_dec;
        cgt_move_new::move1_unpack(m, p1_dec);
        checksum += p1_dec;

        assert(p1 == p1_dec);
        assert(0 == (m & (1 << N_BIT_INT_MAX_BITS)));
    });

    cout << checksum << endl;
}


void test_move2()
{
    unsigned int checksum = 0;

    TEST_LOOP(p1, 16, INT_SIGNED,
    TEST_LOOP(p2, 15, INT_UNSIGNED, {
        const ::move m = cgt_move_new::move2_create(p1, p2);

        int p1_dec;
        int p2_dec;
        cgt_move_new::move2_unpack(m, p1_dec, p2_dec);
        checksum += p1_dec + p2_dec;

        assert(p1 == p1_dec);
        assert(p2 == p2_dec);
        assert(0 == (m & (1 << N_BIT_INT_MAX_BITS)));
    }));

    cout << checksum << endl;
}

void test_move3()
{
    unsigned int checksum = 0;

    TEST_LOOP(p1, 11, INT_SIGNED,
    TEST_LOOP(p2, 10, INT_UNSIGNED,
    TEST_LOOP(p3, 10, INT_UNSIGNED, {
        const ::move m = cgt_move_new::move3_create(p1, p2, p3);

        int p1_dec;
        int p2_dec;
        int p3_dec;
        cgt_move_new::move3_unpack(m, p1_dec, p2_dec, p3_dec);
        checksum += p1_dec + p2_dec + p3_dec;

        assert(p1 == p1_dec);
        assert(p2 == p2_dec);
        assert(p3 == p3_dec);

        assert(0 == (m & (1 << N_BIT_INT_MAX_BITS)));
    })));

    cout << checksum << endl;
}

void test_move4()
{
    unsigned int checksum = 0;

    TEST_LOOP(p1, 8, INT_SIGNED,
    TEST_LOOP(p2, 8, INT_SIGNED,
    TEST_LOOP(p3, 8, INT_SIGNED,
    TEST_LOOP(p4, 7, INT_UNSIGNED, {
        const ::move m = cgt_move_new::move4_create(p1, p2, p3, p4);

        int p1_dec;
        int p2_dec;
        int p3_dec;
        int p4_dec;
        cgt_move_new::move4_unpack(m, p1_dec, p2_dec, p3_dec, p4_dec);
        checksum += p1_dec + p2_dec + p3_dec + p4_dec;

        assert(p1 == p1_dec);
        assert(p2 == p2_dec);
        assert(p3 == p3_dec);
        assert(p4 == p4_dec);

        assert(0 == (m & (1 << N_BIT_INT_MAX_BITS)));
    }))));

    cout << checksum << endl;
}

void test_move6()
{
    unsigned int checksum = 0;

    TEST_LOOP(p1, 6, INT_SIGNED,
    TEST_LOOP(p2, 5, INT_UNSIGNED,
    TEST_LOOP(p3, 5, INT_UNSIGNED,
    TEST_LOOP(p4, 5, INT_UNSIGNED,
    TEST_LOOP(p5, 5, INT_UNSIGNED,
    TEST_LOOP(p6, 5, INT_UNSIGNED, {
        const ::move m = cgt_move_new::move6_create(p1, p2, p3, p4, p5, p6);

        int p1_dec;
        int p2_dec;
        int p3_dec;
        int p4_dec;
        int p5_dec;
        int p6_dec;
        cgt_move_new::move6_unpack(m, p1_dec, p2_dec, p3_dec, p4_dec, p5_dec, p6_dec);
        checksum += p1_dec + p2_dec + p3_dec + p4_dec + p5_dec + p6_dec;

        assert(p1 == p1_dec);
        assert(p2 == p2_dec);
        assert(p3 == p3_dec);
        assert(p4 == p4_dec);
        assert(p5 == p5_dec);
        assert(p6 == p6_dec);

        assert(0 == (m & (1 << N_BIT_INT_MAX_BITS)));
    }))))));

    cout << checksum << endl;
}



} // namespace


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

    test_move1();
    test_move2();
    test_move3();
    test_move4();
    test_move6();

}
