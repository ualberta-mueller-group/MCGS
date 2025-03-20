#include "cgt_integer_game_test.h"
#include "cgt_integer_game.h"
#include <cassert>
#include "test_utilities.h"

namespace {
namespace cgt_integer_game_test {

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

} // namespace cgt_integer_game_test

namespace cgt_integer_move_generator_test {

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

} // namespace cgt_integer_move_generator_test
} // namespace

void cgt_integer_game_test_all()
{
    cgt_integer_game_test::test1();
    cgt_integer_game_test::test2();
    cgt_integer_move_generator_test::test1();
    cgt_integer_move_generator_test::test2();
    cgt_integer_move_generator_test::test3();
    cgt_integer_move_generator_test::test4();
}

//---------------------------------------------------------------------------
