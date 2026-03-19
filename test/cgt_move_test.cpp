#include "cgt_move_test.h"

#include <cassert>
#include <utility>

#include "cgt_move.h"
#include "cgt_basics.h"
#include "int_pair.h"
#include "n_bit_int.h"
#include "cannibal_clobber_move.h"
#include "int_generator.h"

using namespace std;

#define TEST_LOOP(var, layout_t, move_part, full_interval, body)               \
    {                                                                          \
        int_generator<int64_t> var##_gen(move_part_min<layout_t, move_part>(), \
                                         move_part_max<layout_t, move_part>(), \
                                         (full_interval ? 40 : 20));        \
                                                                               \
        while (var##_gen)                                                      \
        {                                                                      \
            const int64_t var = var##_gen.gen_int();                           \
            ++(var##_gen);                                                     \
            body                                                               \
        }                                                                      \
    }

namespace {

////////////////////////////////////////////////// helpers
template <class Move_Layout_T, unsigned int move_part_number>
constexpr int64_t move_part_min()
{
    static_assert(cgt_move::move_layout_is_legal<Move_Layout_T>() && //
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
constexpr int64_t move_part_max()
{
    static_assert(cgt_move::move_layout_is_legal<Move_Layout_T>() && //
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
void test_colors_move2()
{
    ::move m = 0;
    m = cgt_move::set_color(m, BLACK);
    assert(cgt_move::get_color(m) == BLACK);

    cgt_move::move2_set_part_1(m, -642);
    cgt_move::move2_set_part_2(m, 216);

    assert(cgt_move::get_color(m) == BLACK &&       //
           cgt_move::move2_get_part_1(m) == -642 && //
           cgt_move::move2_get_part_2(m) == 216     //
    );

    m = cgt_move::remove_color(m);
    m = cgt_move::set_color(m, WHITE);

    assert(cgt_move::get_color(m) == WHITE &&       //
           cgt_move::move2_get_part_1(m) == -642 && //
           cgt_move::move2_get_part_2(m) == 216     //
    );
}

void test_colors_move4()
{
    ::move m = 0;
    m = cgt_move::set_color(m, WHITE);
    assert(cgt_move::get_color(m) == WHITE);

    cgt_move::move4_set_part_1(m, -60);
    cgt_move::move4_set_part_2(m, -31);
    cgt_move::move4_set_part_3(m, 40);
    cgt_move::move4_set_part_4(m, 120);

    assert(cgt_move::get_color(m) == WHITE &&      //
           cgt_move::move4_get_part_1(m) == -60 && //
           cgt_move::move4_get_part_2(m) == -31 && //
           cgt_move::move4_get_part_3(m) == 40 &&  //
           cgt_move::move4_get_part_4(m) == 120    //
    );

    m = cgt_move::remove_color(m);
    m = cgt_move::set_color(m, BLACK);

    assert(cgt_move::get_color(m) == BLACK &&      //
           cgt_move::move4_get_part_1(m) == -60 && //
           cgt_move::move4_get_part_2(m) == -31 && //
           cgt_move::move4_get_part_3(m) == 40 &&  //
           cgt_move::move4_get_part_4(m) == 120    //
    );

}

void test_move1(bool test_full_interval)
{
    // Parts/coordinates, extracted from a move with "unpack" functions
    int64_t p1_unpack;

    // Parts/coordinates, extracted from a move with "get" functions
    int64_t p1_get;

    TEST_LOOP(p1, cgt_move::move1_layout, 1, test_full_interval, {

        // Test different ways of creating the move
        const ::move m_create_parts =
            cgt_move::move1_create(p1);

        ::move m_set_parts = 0;
        cgt_move::move1_set_part_1(m_set_parts, p1);

        assert(m_create_parts == m_set_parts);

        const ::move m = m_create_parts;
        assert(0 == (m & (int64_t(1) << N_BIT_INT_MAX_BITS)));

        // Test the different ways of getting move parts
        cgt_move::move1_unpack(m, p1_unpack);

        p1_get = cgt_move::move1_get_part_1(m);

        assert(p1 == p1_unpack && p1_unpack == p1_get);
    });
}

void test_move2(bool test_full_interval)
{
    // Parts/coordinates, extracted from a move with "unpack" functions
    int p1_unpack, p2_unpack;
    int_pair c1_unpack;

    // Parts/coordinates, extracted from a move with "get" functions
    int p1_get, p2_get;
    int_pair c1_get;

    TEST_LOOP(p1, cgt_move::move2_layout, 1, test_full_interval, {
    TEST_LOOP(p2, cgt_move::move2_layout, 2, test_full_interval, {

        // Test different ways of creating the move
        const int_pair c1(p1, p2);

        const ::move m_create_parts =
            cgt_move::move2_create(p1, p2);
        const ::move m_create_coords =
            cgt_move::move2_create_from_coords(c1);

        ::move m_set_parts = 0;
        cgt_move::move2_set_part_1(m_set_parts, p1);
        cgt_move::move2_set_part_2(m_set_parts, p2);

        ::move m_set_coords = 0;
        cgt_move::move2_set_coord_1(m_set_coords, c1);

        assert(m_create_parts == m_create_coords && //
               m_create_coords == m_set_parts &&    //
               m_set_parts == m_set_coords          //
        );

        const ::move m = m_create_parts;
        assert(0 == (m & (int64_t(1) << N_BIT_INT_MAX_BITS)));

        // Test the different ways of getting move parts

        cgt_move::move2_unpack(m, p1_unpack, p2_unpack);

        cgt_move::move2_unpack_coords(m, c1_unpack);

        p1_get = cgt_move::move2_get_part_1(m);
        p2_get = cgt_move::move2_get_part_2(m);

        c1_get = cgt_move::move2_get_coord_1(m);

        assert(p1 == p1_unpack && p1_unpack == p1_get);
        assert(p2 == p2_unpack && p2_unpack == p2_get);

        assert(c1 == c1_unpack && c1_unpack == c1_get);
    }}));
}

void test_move3(bool test_full_interval)
{
    // Parts/coordinates, extracted from a move with "unpack" functions
    int p1_unpack, p2_unpack, p3_unpack;

    // Parts/coordinates, extracted from a move with "get" functions
    int p1_get, p2_get, p3_get;

    TEST_LOOP(p1, cgt_move::move3_layout, 1, test_full_interval, {
    TEST_LOOP(p2, cgt_move::move3_layout, 2, test_full_interval, {
    TEST_LOOP(p3, cgt_move::move3_layout, 3, test_full_interval, {

        // Test different ways of creating the move
        const ::move m_create_parts =
            cgt_move::move3_create(p1, p2, p3);

        ::move m_set_parts = 0;
        cgt_move::move3_set_part_1(m_set_parts, p1);
        cgt_move::move3_set_part_2(m_set_parts, p2);
        cgt_move::move3_set_part_3(m_set_parts, p3);

        assert(m_create_parts == m_set_parts);

        const ::move m = m_create_parts;
        assert(0 == (m & (int64_t(1) << N_BIT_INT_MAX_BITS)));

        // Test the different ways of getting move parts
        cgt_move::move3_unpack(m, p1_unpack, p2_unpack, p3_unpack);

        p1_get = cgt_move::move3_get_part_1(m);
        p2_get = cgt_move::move3_get_part_2(m);
        p3_get = cgt_move::move3_get_part_3(m);

        assert(p1 == p1_unpack && p1_unpack == p1_get);
        assert(p2 == p2_unpack && p2_unpack == p2_get);
        assert(p3 == p3_unpack && p3_unpack == p3_get);

    }}})));
}


void test_move4(bool test_full_interval)
{
    // Parts/coordinates, extracted from a move with "unpack" functions
    int p1_unpack, p2_unpack, p3_unpack, p4_unpack;
    int_pair c1_unpack, c2_unpack;

    // Parts/coordinates, extracted from a move with "get" functions
    int p1_get, p2_get, p3_get, p4_get;
    int_pair c1_get, c2_get;

    TEST_LOOP(p1, cgt_move::move4_layout, 1, test_full_interval, {
    TEST_LOOP(p2, cgt_move::move4_layout, 2, test_full_interval, {
    TEST_LOOP(p3, cgt_move::move4_layout, 3, test_full_interval, {
    TEST_LOOP(p4, cgt_move::move4_layout, 4, test_full_interval, {

        // Test different ways of creating the move
        const int_pair c1(p1, p2);
        const int_pair c2(p3, p4);

        const ::move m_create_parts =
            cgt_move::move4_create(p1, p2, p3, p4);
        const ::move m_create_coords =
            cgt_move::move4_create_from_coords(c1, c2);

        ::move m_set_parts = 0;
        cgt_move::move4_set_part_1(m_set_parts, p1);
        cgt_move::move4_set_part_2(m_set_parts, p2);
        cgt_move::move4_set_part_3(m_set_parts, p3);
        cgt_move::move4_set_part_4(m_set_parts, p4);

        ::move m_set_coords = 0;
        cgt_move::move4_set_coord_1(m_set_coords, c1);
        cgt_move::move4_set_coord_2(m_set_coords, c2);

        assert(m_create_parts == m_create_coords && //
               m_create_coords == m_set_parts &&    //
               m_set_parts == m_set_coords          //
        );

        const ::move m = m_create_parts;
        assert(0 == (m & (int64_t(1) << N_BIT_INT_MAX_BITS)));

        // Test the different ways of getting move parts

        cgt_move::move4_unpack(m, p1_unpack, p2_unpack, p3_unpack,
                                   p4_unpack);

        cgt_move::move4_unpack_coords(m, c1_unpack, c2_unpack);

        p1_get = cgt_move::move4_get_part_1(m);
        p2_get = cgt_move::move4_get_part_2(m);
        p3_get = cgt_move::move4_get_part_3(m);
        p4_get = cgt_move::move4_get_part_4(m);

        c1_get = cgt_move::move4_get_coord_1(m);
        c2_get = cgt_move::move4_get_coord_2(m);

        assert(p1 == p1_unpack && p1_unpack == p1_get);
        assert(p2 == p2_unpack && p2_unpack == p2_get);
        assert(p3 == p3_unpack && p3_unpack == p3_get);
        assert(p4 == p4_unpack && p4_unpack == p4_get);

        assert(c1 == c1_unpack && c1_unpack == c1_get);
        assert(c2 == c2_unpack && c2_unpack == c2_get);

    }}}}))));
}

void test_move6(bool test_full_interval)
{
    // Parts/coordinates, extracted from a move with "unpack" functions
    int p1_unpack, p2_unpack, p3_unpack, p4_unpack, p5_unpack, p6_unpack;
    int_pair c1_unpack, c2_unpack, c3_unpack;

    // Parts/coordinates, extracted from a move with "get" functions
    int p1_get, p2_get, p3_get, p4_get, p5_get, p6_get;
    int_pair c1_get, c2_get, c3_get;

    TEST_LOOP(p1, cgt_move::move6_layout, 1, test_full_interval, {
    TEST_LOOP(p2, cgt_move::move6_layout, 2, test_full_interval, {
    TEST_LOOP(p3, cgt_move::move6_layout, 3, test_full_interval, {
    TEST_LOOP(p4, cgt_move::move6_layout, 4, test_full_interval, {
    TEST_LOOP(p5, cgt_move::move6_layout, 5, test_full_interval, {
    TEST_LOOP(p6, cgt_move::move6_layout, 6, test_full_interval, {

        // Test different ways of creating the move
        const int_pair c1(p1, p2);
        const int_pair c2(p3, p4);
        const int_pair c3(p5, p6);

        const ::move m_create_parts =
            cgt_move::move6_create(p1, p2, p3, p4, p5, p6);
        const ::move m_create_coords =
            cgt_move::move6_create_from_coords(c1, c2, c3);

        ::move m_set_parts = 0;
        cgt_move::move6_set_part_1(m_set_parts, p1);
        cgt_move::move6_set_part_2(m_set_parts, p2);
        cgt_move::move6_set_part_3(m_set_parts, p3);
        cgt_move::move6_set_part_4(m_set_parts, p4);
        cgt_move::move6_set_part_5(m_set_parts, p5);
        cgt_move::move6_set_part_6(m_set_parts, p6);

        ::move m_set_coords = 0;
        cgt_move::move6_set_coord_1(m_set_coords, c1);
        cgt_move::move6_set_coord_2(m_set_coords, c2);
        cgt_move::move6_set_coord_3(m_set_coords, c3);

        assert(m_create_parts == m_create_coords && //
               m_create_coords == m_set_parts &&    //
               m_set_parts == m_set_coords          //
        );

        const ::move m = m_create_parts;
        assert(0 == (m & (int64_t(1) << N_BIT_INT_MAX_BITS)));

        // Test the different ways of getting move parts

        cgt_move::move6_unpack(m, p1_unpack, p2_unpack, p3_unpack,
                                   p4_unpack, p5_unpack, p6_unpack);

        cgt_move::move6_unpack_coords(m, c1_unpack, c2_unpack, c3_unpack);

        p1_get = cgt_move::move6_get_part_1(m);
        p2_get = cgt_move::move6_get_part_2(m);
        p3_get = cgt_move::move6_get_part_3(m);
        p4_get = cgt_move::move6_get_part_4(m);
        p5_get = cgt_move::move6_get_part_5(m);
        p6_get = cgt_move::move6_get_part_6(m);

        c1_get = cgt_move::move6_get_coord_1(m);
        c2_get = cgt_move::move6_get_coord_2(m);
        c3_get = cgt_move::move6_get_coord_3(m);

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

void test_cannibal_clobber_move(bool test_full_interval)
{
    // Parts/coordinates, extracted from a move with "unpack" functions
    int_pair c1_unpack, c2_unpack;
    int target_color_unpack;

    TEST_LOOP(p1, cannibal_clobber_move::custom_layout, 1, test_full_interval, {
    TEST_LOOP(p2, cannibal_clobber_move::custom_layout, 2, test_full_interval, {
    TEST_LOOP(p3, cannibal_clobber_move::custom_layout, 3, test_full_interval, {
    TEST_LOOP(p4, cannibal_clobber_move::custom_layout, 4, test_full_interval, {
    for (int p5 = 0; p5 <= 1; p5++) {

        // Test different ways of creating the move
        const int_pair c1(p1, p2);
        const int_pair c2(p3, p4);
        const int t1((p5 == 0) ? BLACK : WHITE);

        const ::move m_create_coords =
            cannibal_clobber_move::create_from_coords(c1, c2, t1);

        assert(0 == (m_create_coords & (int64_t(1) << N_BIT_INT_MAX_BITS)));

        // Test the different ways of getting move parts
        cannibal_clobber_move::unpack_coords(m_create_coords, c1_unpack,
                                             c2_unpack, target_color_unpack);

        assert(p1 == c1_unpack.first);
        assert(p2 == c1_unpack.second);
        assert(p3 == c2_unpack.first);
        assert(p4 == c2_unpack.second);
        assert(t1 == target_color_unpack);

    }}}}}))));
}


} // namespace

//////////////////////////////////////////////////
void cgt_move_test_all(bool extra_tests)
{
    test_colors_move2();
    test_colors_move4();

    test_move1(extra_tests);
    test_move2(extra_tests);
    test_move3(extra_tests);
    test_move4(extra_tests);
    test_move6(extra_tests);
    test_cannibal_clobber_move(extra_tests);
}
