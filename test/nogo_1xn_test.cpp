//---------------------------------------------------------------------------
// Unit tests for the game of nogo 1xn
//---------------------------------------------------------------------------
#include "nogo_1xn.h"

#include <cassert>
#include <iostream>
#include "cgt_move.h"
#include "test_case.h"
#include "test_utilities.h"


namespace {
void assert_solve(nogo_1xn& pos, bw to_play, bool expected_result)
{
    alternating_move_game g(pos, to_play);
    const bool result = g.solve();
    assert(result == expected_result);
}

void nogo_1x4_solve()
{
    nogo_1xn g("....");
    assert_solve(g, BLACK, false);
}

}

void nogo_1xn_test_all()
{
    nogo_1x4_solve();
}