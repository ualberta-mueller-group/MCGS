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
    // Parts/coordinates, extracted from a move with "unpack" functions
    int p1_unpack;

    // Parts/coordinates, extracted from a move with "get" functions
    int p1_get;

    TEST_LOOP(p1, cgt_move_new::move1_layout, 1, {

        // Test different ways of creating the move
        const ::move m_create_parts =
            cgt_move_new::move1_create(p1);

        ::move m_set_parts = 0;
        cgt_move_new::move1_set_part_1(m_set_parts, p1);

        assert(m_create_parts == m_set_parts);

        const ::move m = m_create_parts;
        assert(0 == (m & (1 << N_BIT_INT_MAX_BITS)));

        // Test the different ways of getting move parts
        cgt_move_new::move1_unpack(m, p1_unpack);

        p1_get = cgt_move_new::move1_get_part_1(m);

        assert(p1 == p1_unpack && p1_unpack == p1_get);
    });
}

void test_move2()
{
    // Parts/coordinates, extracted from a move with "unpack" functions
    int p1_unpack, p2_unpack;
    int_pair c1_unpack;

    // Parts/coordinates, extracted from a move with "get" functions
    int p1_get, p2_get;
    int_pair c1_get;

    TEST_LOOP(p1, cgt_move_new::move2_layout, 1, {
    TEST_LOOP(p2, cgt_move_new::move2_layout, 2, {

        // Test different ways of creating the move
        const int_pair c1(p1, p2);

        const ::move m_create_parts =
            cgt_move_new::move2_create(p1, p2);
        const ::move m_create_coords =
            cgt_move_new::move2_create_from_coords(c1);

        ::move m_set_parts = 0;
        cgt_move_new::move2_set_part_1(m_set_parts, p1);
        cgt_move_new::move2_set_part_2(m_set_parts, p2);

        ::move m_set_coords = 0;
        cgt_move_new::move2_set_coord_1(m_set_coords, c1);

        assert(m_create_parts == m_create_coords && //
               m_create_coords == m_set_parts &&    //
               m_set_parts == m_set_coords          //
        );

        const ::move m = m_create_parts;
        assert(0 == (m & (1 << N_BIT_INT_MAX_BITS)));

        // Test the different ways of getting move parts

        cgt_move_new::move2_unpack(m, p1_unpack, p2_unpack);

        cgt_move_new::move2_unpack_coords(m, c1_unpack);

        p1_get = cgt_move_new::move2_get_part_1(m);
        p2_get = cgt_move_new::move2_get_part_2(m);

        c1_get = cgt_move_new::move2_get_coord_1(m);

        assert(p1 == p1_unpack && p1_unpack == p1_get);
        assert(p2 == p2_unpack && p2_unpack == p2_get);

        assert(c1 == c1_unpack && c1_unpack == c1_get);
    }}));
}

void test_move3()
{
    // Parts/coordinates, extracted from a move with "unpack" functions
    int p1_unpack, p2_unpack, p3_unpack;

    // Parts/coordinates, extracted from a move with "get" functions
    int p1_get, p2_get, p3_get;

    TEST_LOOP(p1, cgt_move_new::move3_layout, 1, {
    TEST_LOOP(p2, cgt_move_new::move3_layout, 2, {
    TEST_LOOP(p3, cgt_move_new::move3_layout, 3, {

        // Test different ways of creating the move
        const ::move m_create_parts =
            cgt_move_new::move3_create(p1, p2, p3);

        ::move m_set_parts = 0;
        cgt_move_new::move3_set_part_1(m_set_parts, p1);
        cgt_move_new::move3_set_part_2(m_set_parts, p2);
        cgt_move_new::move3_set_part_3(m_set_parts, p3);

        assert(m_create_parts == m_set_parts);

        const ::move m = m_create_parts;
        assert(0 == (m & (1 << N_BIT_INT_MAX_BITS)));

        // Test the different ways of getting move parts
        cgt_move_new::move3_unpack(m, p1_unpack, p2_unpack, p3_unpack);

        p1_get = cgt_move_new::move3_get_part_1(m);
        p2_get = cgt_move_new::move3_get_part_2(m);
        p3_get = cgt_move_new::move3_get_part_3(m);

        assert(p1 == p1_unpack && p1_unpack == p1_get);
        assert(p2 == p2_unpack && p2_unpack == p2_get);
        assert(p3 == p3_unpack && p3_unpack == p3_get);

    }}})));
}


void test_move4()
{
    // Parts/coordinates, extracted from a move with "unpack" functions
    int p1_unpack, p2_unpack, p3_unpack, p4_unpack;
    int_pair c1_unpack, c2_unpack;

    // Parts/coordinates, extracted from a move with "get" functions
    int p1_get, p2_get, p3_get, p4_get;
    int_pair c1_get, c2_get;

    TEST_LOOP(p1, cgt_move_new::move4_layout, 1, {
    TEST_LOOP(p2, cgt_move_new::move4_layout, 2, {
    TEST_LOOP(p3, cgt_move_new::move4_layout, 3, {
    TEST_LOOP(p4, cgt_move_new::move4_layout, 4, {

        // Test different ways of creating the move
        const int_pair c1(p1, p2);
        const int_pair c2(p3, p4);

        const ::move m_create_parts =
            cgt_move_new::move4_create(p1, p2, p3, p4);
        const ::move m_create_coords =
            cgt_move_new::move4_create_from_coords(c1, c2);

        ::move m_set_parts = 0;
        cgt_move_new::move4_set_part_1(m_set_parts, p1);
        cgt_move_new::move4_set_part_2(m_set_parts, p2);
        cgt_move_new::move4_set_part_3(m_set_parts, p3);
        cgt_move_new::move4_set_part_4(m_set_parts, p4);

        ::move m_set_coords = 0;
        cgt_move_new::move4_set_coord_1(m_set_coords, c1);
        cgt_move_new::move4_set_coord_2(m_set_coords, c2);

        assert(m_create_parts == m_create_coords && //
               m_create_coords == m_set_parts &&    //
               m_set_parts == m_set_coords          //
        );

        const ::move m = m_create_parts;
        assert(0 == (m & (1 << N_BIT_INT_MAX_BITS)));

        // Test the different ways of getting move parts

        cgt_move_new::move4_unpack(m, p1_unpack, p2_unpack, p3_unpack,
                                   p4_unpack);

        cgt_move_new::move4_unpack_coords(m, c1_unpack, c2_unpack);

        p1_get = cgt_move_new::move4_get_part_1(m);
        p2_get = cgt_move_new::move4_get_part_2(m);
        p3_get = cgt_move_new::move4_get_part_3(m);
        p4_get = cgt_move_new::move4_get_part_4(m);

        c1_get = cgt_move_new::move4_get_coord_1(m);
        c2_get = cgt_move_new::move4_get_coord_2(m);

        assert(p1 == p1_unpack && p1_unpack == p1_get);
        assert(p2 == p2_unpack && p2_unpack == p2_get);
        assert(p3 == p3_unpack && p3_unpack == p3_get);
        assert(p4 == p4_unpack && p4_unpack == p4_get);

        assert(c1 == c1_unpack && c1_unpack == c1_get);
        assert(c2 == c2_unpack && c2_unpack == c2_get);

    }}}}))));

}


void test_move6()
{
    // Parts/coordinates, extracted from a move with "unpack" functions
    int p1_unpack, p2_unpack, p3_unpack, p4_unpack, p5_unpack, p6_unpack;
    int_pair c1_unpack, c2_unpack, c3_unpack;

    // Parts/coordinates, extracted from a move with "get" functions
    int p1_get, p2_get, p3_get, p4_get, p5_get, p6_get;
    int_pair c1_get, c2_get, c3_get;

    TEST_LOOP(p1, cgt_move_new::move6_layout, 1, {
    TEST_LOOP(p2, cgt_move_new::move6_layout, 2, {
    TEST_LOOP(p3, cgt_move_new::move6_layout, 3, {
    TEST_LOOP(p4, cgt_move_new::move6_layout, 4, {
    TEST_LOOP(p5, cgt_move_new::move6_layout, 5, {
    TEST_LOOP(p6, cgt_move_new::move6_layout, 6, {

        // Test different ways of creating the move
        const int_pair c1(p1, p2);
        const int_pair c2(p3, p4);
        const int_pair c3(p5, p6);

        const ::move m_create_parts =
            cgt_move_new::move6_create(p1, p2, p3, p4, p5, p6);
        const ::move m_create_coords =
            cgt_move_new::move6_create_from_coords(c1, c2, c3);

        ::move m_set_parts = 0;
        cgt_move_new::move6_set_part_1(m_set_parts, p1);
        cgt_move_new::move6_set_part_2(m_set_parts, p2);
        cgt_move_new::move6_set_part_3(m_set_parts, p3);
        cgt_move_new::move6_set_part_4(m_set_parts, p4);
        cgt_move_new::move6_set_part_5(m_set_parts, p5);
        cgt_move_new::move6_set_part_6(m_set_parts, p6);

        ::move m_set_coords = 0;
        cgt_move_new::move6_set_coord_1(m_set_coords, c1);
        cgt_move_new::move6_set_coord_2(m_set_coords, c2);
        cgt_move_new::move6_set_coord_3(m_set_coords, c3);

        assert(m_create_parts == m_create_coords && //
               m_create_coords == m_set_parts &&    //
               m_set_parts == m_set_coords          //
        );

        const ::move m = m_create_parts;
        assert(0 == (m & (1 << N_BIT_INT_MAX_BITS)));

        // Test the different ways of getting move parts

        cgt_move_new::move6_unpack(m, p1_unpack, p2_unpack, p3_unpack,
                                   p4_unpack, p5_unpack, p6_unpack);

        cgt_move_new::move6_unpack_coords(m, c1_unpack, c2_unpack, c3_unpack);

        p1_get = cgt_move_new::move6_get_part_1(m);
        p2_get = cgt_move_new::move6_get_part_2(m);
        p3_get = cgt_move_new::move6_get_part_3(m);
        p4_get = cgt_move_new::move6_get_part_4(m);
        p5_get = cgt_move_new::move6_get_part_5(m);
        p6_get = cgt_move_new::move6_get_part_6(m);

        c1_get = cgt_move_new::move6_get_coord_1(m);
        c2_get = cgt_move_new::move6_get_coord_2(m);
        c3_get = cgt_move_new::move6_get_coord_3(m);

        assert(p1 == p1_unpack && p1_unpack == p1_get);
        assert(p2 == p2_unpack && p2_unpack == p2_get);
        assert(p3 == p3_unpack && p3_unpack == p3_get);
        assert(p4 == p4_unpack && p4_unpack == p4_get);
        assert(p5 == p5_unpack && p5_unpack == p5_get);
        assert(p6 == p6_unpack && p6_unpack == p6_get);

        assert(c1 == c1_unpack && c1_unpack == c1_get);
        assert(c2 == c2_unpack && c2_unpack == c2_get);
        assert(c3 == c3_unpack && c3_unpack == c3_get);

    }}}}}}))))));

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
