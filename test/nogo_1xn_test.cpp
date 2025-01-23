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

void solve_1x6()
{
    nogo_1xn g("..XO..");
    assert_solve(g, BLACK, false);
    assert_solve(g, WHITE, false);
}

void test(const test_case &c)
{
    nogo_1xn pos(c._game);
    const bw to_play = c._black_first ? BLACK : WHITE;
    alternating_move_game g(pos, to_play);
    const bool result = g.solve();
    if (result != c._is_win)
    {
        std::cout << "Test failed game " << pos << ' ' 
            << c._black_first << " expected " << c._is_win << std::endl;
        assert(false);
    }
    // TODO write_bw(to_play), not c._black_first
}

void file()
{
    std::vector<test_case> cases;
    std::string game_name;
    int version;
    if (! read_test_cases("input/nogo_1xn.test", game_name, version, cases))
        return;
    assert(game_name == "nogo_1xn");
    assert(version == 0);
    for (const test_case& c: cases)
    {
        test(c);
    }
}

} // namespace nogo_test

void nogo_1xn_test_all()
{
    nogo_test::solve_1x1();
    nogo_test::solve_1x2();
    nogo_test::solve_1x3();
    nogo_test::solve_1x4();
    nogo_test::solve_1x5();
    nogo_test::solve_1x6();
    nogo_test::file();
}
