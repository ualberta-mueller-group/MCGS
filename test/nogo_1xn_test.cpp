//---------------------------------------------------------------------------
// Unit tests for the game of nogo 1xn
//---------------------------------------------------------------------------
#include "nogo_1xn.h"

#include <cassert>
#include <iostream>
#include "cgt_move.h"
#include "test_case.h"
#include "test_utilities.h"

namespace nogo_test {

void solve_1x1()
{
    nogo_1xn g(".");
    assert_solve(g, BLACK, false);
    assert_solve(g, WHITE, false);
}

void solve_1x2()
{
    nogo_1xn g("..");
    assert_solve(g, BLACK, true);
    assert_solve(g, WHITE, true);
}

void solve_1x3()
{
    nogo_1xn g("...");
    assert_solve(g, BLACK, true);
    assert_solve(g, WHITE, true);
}

void solve_1x4()
{
    nogo_1xn g("....");
    assert_solve(g, BLACK, false);
    assert_solve(g, WHITE, false);
}

void solve_1x5()
{
    nogo_1xn g(".....");
    assert_solve(g, BLACK, true);
    assert_solve(g, WHITE, true);
}

} // namespace nogo_test

void nogo_1xn_test_all()
{
    nogo_test::solve_1x1();
    nogo_test::solve_1x2();
    nogo_test::solve_1x3();
    nogo_test::solve_1x4();
    nogo_test::solve_1x5();
}
