#include "cgt_integer_game.h"
#include <cassert>
#include "test_utilities.h"

namespace cgt_integer_game {

void test1()
{
    integer_game g(3);
    assert_solve(g, BLACK, true);
    assert_solve(g, WHITE, false);
}

void test2()
{
    integer_game g(-5);
    assert_solve(g, BLACK, false);
    assert_solve(g, WHITE, true);
}

} // namespace cgt_integer_game

namespace cgt_integer_move_generator {

void test1()
{
    integer_game g(3);
    assert_num_moves(g, BLACK, 1);
}

void test2()
{
    integer_game g(3);
    assert_num_moves(g, WHITE, 0);
}

void test3()
{
    integer_game g(-5);
    assert_num_moves(g, BLACK, 0);
}

void test4()
{
    integer_game g(-5);
    assert_num_moves(g, WHITE, 1);
}

} // namespace cgt_integer_move_generator

void cgt_integer_game_test_all()
{
    cgt_integer_game::test1();
    cgt_integer_game::test2();
    cgt_integer_move_generator::test1();
    cgt_integer_move_generator::test2();
    cgt_integer_move_generator::test3();
    cgt_integer_move_generator::test4();
}

//---------------------------------------------------------------------------
