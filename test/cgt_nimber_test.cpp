#include "cgt_nimber.h"
#include "cgt_move.h"

#include <cassert>
#include <iostream>
#include "test_utilities.h"

namespace cgt_nimber_test{

void test1()
{
    nimber g(0);
    assert_solve(g, BLACK, false);
    assert_solve(g, WHITE, false);
}

void test2()
{
    nimber g(3);
    assert_solve(g, BLACK, true);
    assert_solve(g, WHITE, true);
}

} // namespace cgt_nimber_test

namespace cgt_nimber_move_generator{

void test1()
{
    nimber g(0);
    assert_num_moves(g, BLACK, 0);
    assert_num_moves(g, WHITE, 0);
}

void test2()
{
    nimber g(12);
    assert_num_moves(g, BLACK, 12);
    assert_num_moves(g, WHITE, 12);
}

} // namespace cgt_nimber_move_generator

//---------------------------------------------------------------------------

void cgt_nimber_test_all()
{
    cgt_nimber_test::test1();
    cgt_nimber_test::test2();
    cgt_nimber_move_generator::test1();
    cgt_nimber_move_generator::test2();
}
//---------------------------------------------------------------------------
