#include "cgt_move_test.h"
#include "cgt_move_new.h"
#include <cassert>
#include "cgt_basics.h"
#include "n_bit_int.h"


using namespace std;

#define TEST_LOOP(var, layout_t, move_part, body)                              \
    for (int var = move_part_min<layout_t, move_part>();                       \
         var <= move_part_max<layout_t, move_part>(); var++)                   \
    {                                                                          \
        body                                                                   \
    }

namespace {

////////////////////////////////////////////////// helpers
template <class Move_Layout_T, unsigned int move_part_number>
constexpr int move_part_min()
{
    static_assert(cgt_move_new::move_layout_is_legal<Move_Layout_T>() && //
                  1 <= move_part_number &&                               //
                  move_part_number <= Move_Layout_T::LAYOUT.size()       //
    );

    constexpr std::pair<int, signed_type_enum> PART_WIDTH_AND_SIGN_TYPE =
        Move_Layout_T::LAYOUT[move_part_number - 1];

    constexpr int N_BITS = PART_WIDTH_AND_SIGN_TYPE.first;
    constexpr signed_type_enum SIGN_TYPE = PART_WIDTH_AND_SIGN_TYPE.second;

    return n_bit_int::min_val<N_BITS, SIGN_TYPE>();
}

template <class Move_Layout_T, unsigned int move_part_number>
constexpr int move_part_max()
{
    static_assert(cgt_move_new::move_layout_is_legal<Move_Layout_T>() && //
                  1 <= move_part_number &&                               //
                  move_part_number <= Move_Layout_T::LAYOUT.size()       //
    );

    constexpr std::pair<int, signed_type_enum> PART_WIDTH_AND_SIGN_TYPE =
        Move_Layout_T::LAYOUT[move_part_number - 1];

    constexpr int N_BITS = PART_WIDTH_AND_SIGN_TYPE.first;
    constexpr signed_type_enum SIGN_TYPE = PART_WIDTH_AND_SIGN_TYPE.second;

    return n_bit_int::max_val<N_BITS, SIGN_TYPE>();
}


////////////////////////////////////////////////// main test functions 
void test_move1()
{
    unsigned int checksum = 0;

    TEST_LOOP(p1, cgt_move_new::move1_layout, 1, {
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

    TEST_LOOP(p1, cgt_move_new::move2_layout, 1, {
    TEST_LOOP(p2, cgt_move_new::move2_layout, 2, {
        const ::move m = cgt_move_new::move2_create(p1, p2);

        int p1_dec;
        int p2_dec;
        cgt_move_new::move2_unpack(m, p1_dec, p2_dec);
        checksum += p1_dec + p2_dec;

        assert(p1 == p1_dec);
        assert(p2 == p2_dec);
        assert(0 == (m & (1 << N_BIT_INT_MAX_BITS)));
    }}));

    cout << checksum << endl;
}

void test_move3()
{
    unsigned int checksum = 0;

    TEST_LOOP(p1, cgt_move_new::move3_layout, 1, {
    TEST_LOOP(p2, cgt_move_new::move3_layout, 2, {
    TEST_LOOP(p3, cgt_move_new::move3_layout, 3, {
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
    }}})));

    cout << checksum << endl;
}

void test_move4()
{
    unsigned int checksum = 0;

    TEST_LOOP(p1, cgt_move_new::move4_layout, 1, {
    TEST_LOOP(p2, cgt_move_new::move4_layout, 2, {
    TEST_LOOP(p3, cgt_move_new::move4_layout, 3, {
    TEST_LOOP(p4, cgt_move_new::move4_layout, 4, {
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
    }}}}))));

    cout << checksum << endl;
}

void test_move6()
{
    unsigned int checksum = 0;

    TEST_LOOP(p1, cgt_move_new::move6_layout, 1, {
    TEST_LOOP(p2, cgt_move_new::move6_layout, 2, {
    TEST_LOOP(p3, cgt_move_new::move6_layout, 3, {
    TEST_LOOP(p4, cgt_move_new::move6_layout, 4, {
    TEST_LOOP(p5, cgt_move_new::move6_layout, 5, {
    TEST_LOOP(p6, cgt_move_new::move6_layout, 6, {
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
    }}}}}}))))));

    cout << checksum << endl;
}


} // namespace



/*
using namespace cgt_move;

namespace {
namespace cgt_move_test {

void test_two_part_move()
{
    {
        const int part1 = 123;
        const int part2 = 456;
        const move m = two_part_move(part1, part2);
        assert(first(m) == part1);
        assert(second(m) == part2);
    }
    {
        const int part1 = MOVE_MAX_SIZE - 1;
        const int part2 = MOVE_MAX_SIZE - 1;
        const move m = two_part_move(part1, part2);
        assert(first(m) == part1);
        assert(second(m) == part2);
    }
    {
        const int part1 = 0;
        const int part2 = 0;
        const move m = two_part_move(part1, part2);
        assert(first(m) == part1);
        assert(second(m) == part2);
    }
}

void test_encode_decode()
{
    int mm = MOVE_MAX_SIZE - 1;
    const move m = two_part_move(mm, mm);
    const move m2 = encode(m, BLACK);
    assert(decode(m2) == m);
    assert(get_color(m2) == BLACK);
    const move m3 = encode(m, WHITE);
    assert(decode(m3) == m);
    assert(get_color(m3) == WHITE);
}

void test_encode_decode2()
{
    int mm = (MOVE_MAX_SIZE - 1) / 2;
    const move m = two_part_move(mm, mm);
    const move m2 = encode(m, BLACK);
    assert(decode(m2) == m);
    assert(get_color(m2) == BLACK);
    const move m3 = encode(m, WHITE);
    assert(decode(m3) == m);
    assert(get_color(m3) == WHITE);
}

void test_encode_decode3()
{
    const move m = two_part_move(-5, 8);
    const move m2 = encode(m, BLACK);
    assert(decode(m2) == m);
    assert(get_color(m2) == BLACK);
    const move m3 = encode(m, WHITE);
    assert(decode(m3) == m);
    assert(get_color(m3) == WHITE);
}

void test_encode_decode4()
{
    const move m = two_part_move(1, 2);
    const move m2 = encode(m, BLACK);
    assert(decode(m2) == m);
    assert(get_color(m2) == BLACK);
    const move m3 = encode(m, WHITE);
    assert(decode(m3) == m);
    assert(get_color(m3) == WHITE);
}
} // namespace cgt_move_test
} // namespace

*/

void cgt_move_test_all()
{
    std::cout << "TODO deleted tests in " << __FILE__ << std::endl;
    test_move1();
    test_move2();
    test_move3();
    test_move4();
    test_move6();

    //cgt_move_test::test_two_part_move();
    //cgt_move_test::test_encode_decode();
    //cgt_move_test::test_encode_decode2();
    //cgt_move_test::test_encode_decode3();
    //cgt_move_test::test_encode_decode4();
}
