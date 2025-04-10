//---------------------------------------------------------------------------
// Solve impartial games for win/loss by minimax search
//---------------------------------------------------------------------------
#include "impartial_minimax_test.h"

#include "impartial_game.h"
#include "impartial_game_wrapper.h"
#include "clobber_1xn.h"
#include "kayles.h"

#include <cassert>
#include "cgt_move.h"
#include "test_utilities.h"

using std::string;

namespace {

void test_kayles()
{
    kayles g(0);
    assert_solve_impartial(g, false);
    for (int i=1; i < 10; ++i)
    {
        kayles g(i);
        assert_solve_impartial(g, true);
    }
}

void test_clobber(const string& s, const bool expected_result)
{
    clobber_1xn c(s);
    impartial_game_wrapper g(&c);
    assert_solve_impartial(g, expected_result);
}

void test_clobber_wrapper_all()
{
    test_clobber(clobber_1xn::xoxo(0), false); // nim value 0
    test_clobber(clobber_1xn::xoxo(1), true ); // nim value 1
    test_clobber(clobber_1xn::xoxo(2), true ); // nim value 3
    test_clobber(clobber_1xn::xoxo(3), false); // nim value 0
    test_clobber(clobber_1xn::xoxo(4), true ); // nim value 2
    test_clobber(clobber_1xn::xoxo(5), false); // nim value 0
}

} // namespace

void impartial_minimax_test_all()
{

}
