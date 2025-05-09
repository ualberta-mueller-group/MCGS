//---------------------------------------------------------------------------
// Solve impartial games for win/loss by minimax search
//---------------------------------------------------------------------------
#include "impartial_minimax_test.h"

#include "impartial_game.h"
#include "impartial_game_wrapper.h"
#include "clobber_1xn.h"
#include "kayles.h"

#include <string>
#include <cassert>
#include "test_utilities.h"

using std::string;

namespace {

void test(impartial_game& g, const bool expected_result)
{
    test_one_game(g, expected_result, expected_result);
}

void test_kayles()
{
    for (int i = 0; i < 10; ++i)
    {
        kayles g(i);
        test(g, i != 0); // only kayles(0) is loss
    }
}

void test_clobber(const string& s, const bool expected_result)
{
    clobber_1xn c(s);
    impartial_game_wrapper g(&c);
    test(g, expected_result);
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
//---------------------------------------------------------------------------

void impartial_minimax_test_all()
{
    test_kayles();
    test_clobber_wrapper_all();
}
