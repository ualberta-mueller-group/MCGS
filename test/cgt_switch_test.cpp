#include "cgt_switch_test.h"
#include "cgt_switch.h"
#include "cgt_move_new.h"

#include <cassert>
#include "game.h"
#include "test_utilities.h"
#include <memory>
#include <vector>
#include <tuple>

namespace {
namespace cgt_switch_test {

void play_on(switch_game& g, bw player)
{
    std::unique_ptr<move_generator> mgp(g.create_move_generator(player));
    move_generator& gen = *mgp;

    assert(gen);
    move m = gen.gen_move();
    ++gen;
    assert(!gen);
    g.play(m, player);
}

void play_undo1()
{
    switch_game g(5, 3);
    assert(!g.is_rational() && g.kind() != SWITCH_KIND_RATIONAL);
    assert(g.left() == 5 && g.right() == 3);

    play_on(g, WHITE);
    assert(g.is_rational() && g.kind() == SWITCH_KIND_RATIONAL);
    assert(g.value() == 3);

    g.undo_move();
    assert(!g.is_rational() && g.kind() != SWITCH_KIND_RATIONAL);
}

void play_undo2()
{
    switch_game g(5, 3);
    assert(!g.is_rational() && g.kind() != SWITCH_KIND_RATIONAL);
    assert(g.left() == 5 && g.right() == 3);

    play_on(g, WHITE);
    assert(g.is_rational() && g.kind() == SWITCH_KIND_RATIONAL);
    assert(g.value() == 3);

    play_on(g, BLACK);
    assert(g.is_rational() && g.kind() == SWITCH_KIND_RATIONAL);
    assert(g.value() == 2);

    g.undo_move();
    assert(g.is_rational() && g.kind() == SWITCH_KIND_RATIONAL);
    assert(g.value() == 3);

    g.undo_move();
    assert(!g.is_rational() && g.kind() != SWITCH_KIND_RATIONAL);
}

void play_undo3()
{
    switch_game g(fraction(90, 2), fraction(43, 8));
    assert(!g.is_rational() && g.kind() != SWITCH_KIND_RATIONAL);
    assert(g.left() == fraction(45) && g.right() == fraction(43, 8));

    play_on(g, WHITE);
    assert(g.is_rational() && g.kind() == SWITCH_KIND_RATIONAL);
    assert(g.value() == fraction(43, 8));

    play_on(g, BLACK);
    assert(g.is_rational() && g.kind() == SWITCH_KIND_RATIONAL);
    assert(g.value() == fraction(42, 8));

    play_on(g, WHITE);
    assert(g.is_rational() && g.kind() == SWITCH_KIND_RATIONAL);
    assert(g.value() == fraction(22, 4));

    g.undo_move();
    assert(g.is_rational() && g.kind() == SWITCH_KIND_RATIONAL);
    assert(g.value() == fraction(42, 8));

    g.undo_move();
    assert(g.is_rational() && g.kind() == SWITCH_KIND_RATIONAL);
    assert(g.value() == fraction(43, 8));

    g.undo_move();
    assert(!g.is_rational() && g.kind() != SWITCH_KIND_RATIONAL);
}

void test1()
{
    switch_game g(3, -1);
    assert_solve(g, BLACK, true);
    assert_solve(g, WHITE, true);
}

void test2()
{
    switch_game g(1, -10);
    assert_solve(g, BLACK, true);
    assert_solve(g, WHITE, true);
}

void test3()
{
    switch_game g(5, 3);
    assert_solve(g, BLACK, true);
    assert_solve(g, WHITE, false);
}

void test4()
{
    switch_game g(-5, -13);
    assert_solve(g, BLACK, false);
    assert_solve(g, WHITE, true);
}

void test_kind()
{
    /*
        left fraction, right fraction, expected switch_kind
    */
    typedef std::tuple<fraction, fraction, switch_kind> test_case_t;

    // clang-format off
    std::vector<test_case_t> test_cases
    {
        {{5, 2}, {-4, 32}, SWITCH_KIND_PROPER},
        {{-5, 8}, {-12, 16}, SWITCH_KIND_PROPER},
        {{21, 2}, {5, 8}, SWITCH_KIND_PROPER},

        {{5}, {-5}, SWITCH_KIND_PROPER_NORMALIZED},
        {{5, 2}, {-5, 2}, SWITCH_KIND_PROPER_NORMALIZED},
        {{1}, {-1}, SWITCH_KIND_PROPER_NORMALIZED},

        {{0}, {0}, SWITCH_KIND_CONVERTIBLE_NUMBER},
        {{0, 1}, {0, 2}, SWITCH_KIND_CONVERTIBLE_NUMBER},
        {{-4}, {-2}, SWITCH_KIND_CONVERTIBLE_NUMBER},
        {{4, 2}, {9, 4}, SWITCH_KIND_CONVERTIBLE_NUMBER}
    };
    // clang-format on

    for (const test_case_t& test : test_cases)
    {
        const fraction& f1 = std::get<0>(test);
        const fraction& f2 = std::get<1>(test);
        const switch_kind& kind = std::get<2>(test);

        switch_game sg(f1, f2);
        assert(sg.kind() == kind);

        play_on(sg, BLACK);
        assert(sg.kind() == SWITCH_KIND_RATIONAL);
        sg.undo_move();
        assert(sg.kind() == kind);

        play_on(sg, WHITE);
        assert(sg.kind() == SWITCH_KIND_RATIONAL);
        sg.undo_move();
        assert(sg.kind() == kind);
    }
}

} // namespace cgt_switch_test

namespace cgt_switch_move_generator_test {

void test1()
{
    switch_game g(3, -1);
    assert_num_moves(g, BLACK, 1);
    assert_num_moves(g, WHITE, 1);
}

void test2()
{
    switch_game g(1, -10);
    assert_num_moves(g, BLACK, 1);
    assert_num_moves(g, WHITE, 1);
}

void test3()
{
    switch_game g(3, -1);
    assert_num_moves(g, BLACK, 1);
    assert_num_moves(g, WHITE, 1);
}

void test4()
{
    switch_game g(-5, -13);
    assert_num_moves(g, BLACK, 1);
    assert_num_moves(g, WHITE, 1);
}
} // namespace cgt_switch_move_generator_test
} // namespace

//---------------------------------------------------------------------------

void cgt_switch_test_all()
{
    cgt_switch_test::play_undo1();
    cgt_switch_test::play_undo2();
    cgt_switch_test::play_undo3();
    cgt_switch_test::test1();
    cgt_switch_test::test2();
    cgt_switch_test::test3();
    cgt_switch_test::test4();
    cgt_switch_test::test_kind();

    cgt_switch_move_generator_test::test1();
    cgt_switch_move_generator_test::test2();
    cgt_switch_move_generator_test::test3();
    cgt_switch_move_generator_test::test4();
}

//---------------------------------------------------------------------------
