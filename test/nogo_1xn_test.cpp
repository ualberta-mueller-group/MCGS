//---------------------------------------------------------------------------
// Unit tests for the game of nogo 1xn
//---------------------------------------------------------------------------
#include "nogo_1xn_test.h"
#include "nogo_1xn.h"

#include <cassert>
#include <vector>
#include <exception>
#include "test_utilities.h"

namespace {
namespace nogo_1xn_test {

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

void file()
{
    assert_solve_test_file(UNIT_TEST_INPUT_DIR + "nogo_1xn.test", 8);
}

void exceptions()
{
    // clang-format off
    std::vector<std::string> boards =
    {
        ".........X....XOX.XOO.X.X.O...",
        "......OX....O.XXOX..O.XX......",
        "......X..XOXX.O...O.....O.XX..",
        "....XO..X...OXO......X...X..OX",
        "...OXXX.XO......XOX........O..",
        "...X.....XO.XX..........O.XXOO",
        "...XOX......O......XXOX.....OX",
        "..O.....X.OXO.X.XO.....X.X....",
        "..O..O....XXOX......OX...X...X",
        "..OX........XX.OXXO..X.....O..",
        ".O..........O....XO..X.X.XOX.X",
        ".OO.OX.X....X........X..XOX...",
        ".OXO..X.....X......X.X....OOX.",
        ".X....O....X...X.O...XOXO...X.",
        ".X....X...X..X.....O.X.O..OXO.",
        ".X...X...X.X...X..O....O...XOO",
        ".XOX.....O.X....XO.XXO........",
        ".XXO.....X..OOX...X.........XO",
        "O......XX......OX...OX.....XXO",
        "O.X.X.XOX.X.X.O.....O.........",
        "OX.....X.X.....XO....OX.X.O...",
        "X.....X..OX...X..O.....XOX..O.",
        "X....OXOX..OX..X........X...O.",
        "X..O....X.X.X.OXO...........OX",
        "X.O.....O.X.X.XX.OXO..........",
        "XO...........O.OX.X.X.O...X..X",
        "XO..O......XX...X.O.....X.XO..",
        "XOX..XXOO..X.X........O.......",
        "XXOX.O.......X......OXO....X..",
    };
    // clang-format on

    for (const std::string& board : boards)
    {
        bool did_throw = false;

        try
        {
            nogo_1xn g(board);
        }
        catch (std::exception& e)
        {
            did_throw = true;
        }

        assert(did_throw);
    }
}

} // namespace nogo_1xn_test

} // namespace

void nogo_1xn_test_all()
{
    nogo_1xn_test::solve_1x1();
    nogo_1xn_test::solve_1x2();
    nogo_1xn_test::solve_1x3();
    nogo_1xn_test::solve_1x4();
    nogo_1xn_test::solve_1x5();
    nogo_1xn_test::solve_1x6();
    nogo_1xn_test::file();
    nogo_1xn_test::exceptions();
}
