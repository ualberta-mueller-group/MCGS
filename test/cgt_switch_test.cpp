#include "cgt_switch.h"
#include "cgt_move.h"

#include <cassert>
#include <iostream>
#include "test_case.h"
#include "test_utilities.h"

namespace cgt_switch_test{

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
    // TODO assert_solve(g, WHITE, false);
}

void test4()
{
    switch_game g(-5, -13);
    // TODO assert_solve(g, BLACK, false);
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
