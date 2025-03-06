#include "cgt_switch.h"
#include "cgt_move.h"

#include <cassert>
#include <iostream>
#include "game.h"
#include "test_utilities.h"
#include <memory>

namespace cgt_switch_test{

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
    assert(!g.is_rational());
    play_on(g, WHITE);
    assert(g.is_rational());
    assert(g.value() == 3);
    g.undo_move();
    assert(!g.is_rational());
}

void play_undo2()
{
    switch_game g(5, 3);
    play_on(g, WHITE);
    assert(g.value() == 3);
    play_on(g, BLACK);
    assert(g.is_rational());
    assert(g.value() == 2);
    g.undo_move();
    assert(g.value() == 3);
    g.undo_move();
    assert(!g.is_rational());
}

void play_undo3()
{
    switch_game g(fraction(91, 2), fraction(43, 8));
    play_on(g, WHITE);
    assert(g.value() == fraction(43, 8));
    play_on(g, BLACK);
    assert(g.is_rational());
    assert(g.value() == fraction(42, 8));
    play_on(g, WHITE);
    assert(g.value() == fraction(22, 4));
    g.undo_move();
    assert(g.value() == fraction(42, 8));
    g.undo_move();
    assert(g.value() == fraction(43, 8));
    g.undo_move();
    assert(!g.is_rational());

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

} // namespace cgt_switch_test

namespace cgt_switch_move_generator{

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
} // namespace cgt_switch_move_generator


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

    cgt_switch_move_generator::test1();
    cgt_switch_move_generator::test2();
    cgt_switch_move_generator::test3();
    cgt_switch_move_generator::test4();
}
//---------------------------------------------------------------------------
