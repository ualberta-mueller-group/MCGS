#include "cgt_up_star.h"
#include <cassert>
#include "test_utilities.h"

namespace cgt_up_star_test {

void test_zero()
{
    up_star g(0, false);
    assert_solve(g, BLACK, false);
    assert_solve(g, WHITE, false);
}

void test_star()
{
    up_star g(0, true);
    assert_solve(g, BLACK, true);
    assert_solve(g, WHITE, true);
}

void test_up()
{
    up_star g(1, false); // up
    assert_solve(g, BLACK, true);
    assert_solve(g, WHITE, false);
}

void test_up_star()
{
    up_star g(1, true); // up star
    assert_solve(g, BLACK, true);
    assert_solve(g, WHITE, true);
}

void test_2up()
{
    up_star g(2, false); // double up
    assert_solve(g, BLACK, true);
    assert_solve(g, WHITE, false);
}

void test_2up_star()
{
    up_star g(2, true);
    assert_solve(g, BLACK, true);
    assert_solve(g, WHITE, false);
}

void test_down()
{
    up_star g(-1, false); // down
    assert_solve(g, BLACK, false);
    assert_solve(g, WHITE, true);
}

void test_down_star()
{
    up_star g(-1, true); // down star
    assert_solve(g, BLACK, true);
    assert_solve(g, WHITE, true);
}

void test_2down()
{
    up_star g(-2, false);
    assert_solve(g, BLACK, false);
    assert_solve(g, WHITE, true);
}

void test_2down_star()
{
    up_star g(-2, true);
    assert_solve(g, BLACK, false);
    assert_solve(g, WHITE, true);
}

} // namespace cgt_up_star_test

namespace cgt_up_star_move_generator {

void test_zero()
{
    up_star g(0, false);
    assert_num_moves(g, BLACK, 0);
    assert_num_moves(g, WHITE, 0);
}

void test_star()
{
    up_star g(0, true);
    assert_num_moves(g, BLACK, 1);
    assert_num_moves(g, WHITE, 1);
}

void test_up()
{
    up_star g(1, false);
    assert_num_moves(g, BLACK, 1);
    assert_num_moves(g, WHITE, 1);
}

void test_up_star()
{
    up_star g(1, true);
    assert_num_moves(g, BLACK, 2);
    assert_num_moves(g, WHITE, 1);
}

void test_2up()
{
    up_star g(2, false);
    assert_num_moves(g, BLACK, 1);
    assert_num_moves(g, WHITE, 1);
}

void test_2up_star()
{
    up_star g(2, true);
    assert_num_moves(g, BLACK, 1);
    assert_num_moves(g, WHITE, 1);
}

void test_down()
{
    up_star g(-1, false);
    assert_num_moves(g, BLACK, 1);
    assert_num_moves(g, WHITE, 1);
}

void test_down_star()
{
    up_star g(-1, true);
    assert_num_moves(g, BLACK, 1);
    assert_num_moves(g, WHITE, 2);
}

void test_2down()
{
    up_star g(-2, false);
    assert_num_moves(g, BLACK, 1);
    assert_num_moves(g, WHITE, 1);
}

void test_2down_star()
{
    up_star g(-2, true);
    assert_num_moves(g, BLACK, 1);
    assert_num_moves(g, WHITE, 1);
}
} // namespace cgt_up_star_move_generator

//---------------------------------------------------------------------------

void cgt_up_star_test_all()
{
    cgt_up_star_test::test_zero();
    cgt_up_star_test::test_star();
    cgt_up_star_test::test_up();
    cgt_up_star_test::test_up_star();
    cgt_up_star_test::test_2up();
    cgt_up_star_test::test_2up_star();
    cgt_up_star_test::test_down();
    cgt_up_star_test::test_down_star();
    cgt_up_star_test::test_2down();
    cgt_up_star_test::test_2down_star();

    cgt_up_star_move_generator::test_zero();
    cgt_up_star_move_generator::test_star();
    cgt_up_star_move_generator::test_up();
    cgt_up_star_move_generator::test_up_star();
    cgt_up_star_move_generator::test_2up();
    cgt_up_star_move_generator::test_2up_star();
    cgt_up_star_move_generator::test_down();
    cgt_up_star_move_generator::test_down_star();
    cgt_up_star_move_generator::test_2down();
    cgt_up_star_move_generator::test_2down_star();
}

//---------------------------------------------------------------------------
